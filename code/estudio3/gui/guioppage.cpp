/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *       _______   ______________  ______     _____
 *      / ____/ | / /  _/ ____/  |/  /   |   |__  /
 *     / __/ /  |/ // // / __/ /|_/ / /| |    /_ <
 *    / /___/ /|  // // /_/ / /  / / ___ |  ___/ /
 *   /_____/_/ |_/___/\____/_/  /_/_/  |_| /____/.
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <QtXml/QDomDocument>

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QScrollBar>
#include <QtGui/QTransform>

#include "guioppage.hpp"
#include "guioperator.hpp"

eGuiOpPage::eGuiOpPage(const QString &name, QGraphicsView *parent) : QGraphicsScene(parent)
{
    _initialize();

    // Create new "engine" page, which is wrapped
    // by the current instance of this class.
    m_opPage = eDemoData::addPage();
    eASSERT(m_opPage != eNULL);
    m_opPage->setUserName(eString(name.toAscii()));
}

eGuiOpPage::eGuiOpPage(eOperatorPage *opPage, QGraphicsView *parent) : QGraphicsScene(parent)
{
    eASSERT(opPage != eNULL);

    m_opPage = opPage;
    _initialize();
}

eGuiOpPage::~eGuiOpPage()
{
    // Loop from size down to 0, because
    // elements are deleted inside loop.
    for (eInt i=items().size()-1; i>=0; i--)
    {
        QGraphicsItem *item = items().at(i);
        eSAFE_DELETE(item);
    }

    eDemoData::removePage(m_opPage->getId());
}

eBool eGuiOpPage::areSelectedOpsMovable(const ePoint &dist) const
{
    eIOperatorPtrArray opsToMove;

    for (eInt i=0; i<selectedItems().size(); i++)
    {
        const eGuiOperator *guiOp = qgraphicsitem_cast<eGuiOperator *>(selectedItems()[i]);
        eASSERT(guiOp != eNULL);

        if (guiOp->isSelected())
        {
            opsToMove.append(guiOp->getOperator());
        }
    }

    return m_opPage->areOpsMovable(opsToMove, dist);
}

eOperatorPage * eGuiOpPage::getPage() const
{
    return m_opPage;
}

ePoint eGuiOpPage::getInsertAt() const
{
    return m_insertAt;
}

QPoint eGuiOpPage::getViewPosition() const
{
    return m_viewPos;
}

// Increments insert at position's y-coordinate
// by one. This function is used when adding
// new operators to a page.
void eGuiOpPage::advanceInsertAt()
{
    if (m_insertAt.y < eOperatorPage::HEIGHT-1)
    {
        m_insertAt.y++;
    }
}

eGuiOperator * eGuiOpPage::getGuiOperator(eID opId) const
{
    for (eInt i=0; i<items().size(); i++)
    {
        // Do not check comment items.
        if (items().at(i)->type() != QGraphicsTextItem::Type)
        {
            eGuiOperator *guiOp = (eGuiOperator *)items().at(i);
            eASSERT(guiOp != eNULL);

            if (guiOp->getOperator()->getId() == opId)
            {
                return guiOp;
            }
        }
    }
    
    return eNULL;
}

void eGuiOpPage::saveToXml(QDomElement &node) const
{
    QDomDocument xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement pageEl = xml.createElement("page");
    pageEl.setAttribute("id", m_opPage->getId());
    pageEl.setAttribute("name", QString(m_opPage->getUserName()));
    node.appendChild(pageEl);

    for (eInt i=0; i<items().size(); i++)
    {
        QGraphicsItem *item = items().at(i);
        eASSERT(item != eNULL);

        if (item->type() == QGraphicsTextItem::Type)
        {
            // Save comment item.
            QDomElement cmtEl = xml.createElement("comment");
            cmtEl.setAttribute("xpos", item->pos().x());
            cmtEl.setAttribute("ypos", item->pos().y());
            cmtEl.setAttribute("text", ((QGraphicsTextItem *)item)->toPlainText());
            pageEl.appendChild(cmtEl);
        }
        else
        {
            // Save GUI operator.
            ((eGuiOperator *)item)->saveToXml(pageEl);
        }
    }
}

void eGuiOpPage::loadFromXml(const QDomElement &node)
{
    m_opPage->setUserName(eString(node.attribute("name").toAscii()));

    // Load in operators of this page.
    QDomElement xmlOp = node.firstChildElement("operator");

    while (!xmlOp.isNull())
    {
        const QString opType = xmlOp.attribute("type");
        const eID opId = xmlOp.attribute("id").toInt();
        const eU32 xpos = xmlOp.attribute("xpos").toInt();
        const eU32 ypos = xmlOp.attribute("ypos").toInt();
        const eU32 width = xmlOp.attribute("width").toInt();

        eIOperator *op =  m_opPage->addOperator(opType.toAscii().constData(), ePoint(xpos, ypos), width, eFALSE, opId);
        eASSERT(op != eNULL);
        eGuiOperator *guiOp = new eGuiOperator(op, this);
        eASSERT(guiOp != eNULL);
        addItem(guiOp);
        guiOp->loadFromXml(xmlOp);

        xmlOp = xmlOp.nextSiblingElement("operator");
    }

    // Load in comments of this page.
    QDomElement xmlCmt = node.firstChildElement("comment");

    while (!xmlCmt.isNull())
    {
        const eF32 xpos = xmlCmt.attribute("xpos").toFloat();
        const eF32 ypos = xmlCmt.attribute("ypos").toFloat();
        const QString text = xmlCmt.attribute("text");

        addComment(text, QPointF(xpos, ypos));

        xmlCmt = xmlCmt.nextSiblingElement("comment");
    }
}

void eGuiOpPage::addComment(const QString &text, const QPointF &pos)
{
    eASSERT(text.length() > 0);

    eCommentItem *cmtItem = new eCommentItem(text, pos, this);
    eASSERT(cmtItem != eNULL);
}

void eGuiOpPage::_initialize()
{
    const eU32 width = eOperatorPage::WIDTH*eGuiOperator::WIDTH;
    const eU32 height = eOperatorPage::HEIGHT*eGuiOperator::HEIGHT;

    setSceneRect(0, 0, width, height);
    setItemIndexMethod(NoIndex);
}

// Update viewing position of this page.
void eGuiOpPage::_updateViewingPos()
{
    eASSERT(views().size() > 0);
    QGraphicsView *view = views().at(0);
    eASSERT(view != eNULL);

    m_viewPos.setX(view->horizontalScrollBar()->value());
    m_viewPos.setY(view->verticalScrollBar()->value());
}

// Draw connection lines between linking
// operators (to visualize dependencies).
void eGuiOpPage::_drawLinkLines(QPainter *painter)
{
    eASSERT(painter != eNULL);

    static const QPoint arrowPts[3] =
    {
        QPoint(0,            0),
        QPoint(-ARROW_SIZE,  ARROW_SIZE),
        QPoint(-ARROW_SIZE, -ARROW_SIZE)
    };

    painter->save();
    painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
    painter->setPen(QColor(80, 90, 100));
    painter->setBrush(QBrush(QColor(120, 130, 140), Qt::SolidPattern));

    for (eU32 i=0; i<m_opPage->getOperatorCount(); i++)
    {
        const eIOperator *op = m_opPage->getOperatorByIndex(i);
        eASSERT(op != eNULL);

        for (eU32 j=0; j<op->getLinkingCount(); j++)
        {
            const eID linkingId = op->getLinkingOperator(j);
            const eIOperator *linkingOp = eDemoData::findOperator(linkingId);

            // Lies linking operator on same page?
            if (linkingOp && linkingOp->getOwnerPage() == m_opPage)
            {
                // Yes, so draw line between them.
                const QRect opRect = _getOperatorRect(op);
                const QRect linkingRect = _getOperatorRect(linkingOp);

                // Calculate nearest intersection point and
                // angle to rotate arrow.
                const QPoint nip = _getNearestIntersectionPoint(opRect, QLine(linkingRect.center(), opRect.center()));
                const QLine line(nip, linkingRect.center());
                const QPoint p = line.p1()-line.p2();
                const eF32 angle = eRadToDeg(eATan2(p.y(), p.x()));

                painter->drawLine(line);
                painter->save();
                painter->translate(line.p1().x(), line.p1().y());
                painter->rotate(angle);
                painter->drawConvexPolygon(arrowPts, 3);
                painter->restore();
            }
        }
    }

    painter->restore();
}

void eGuiOpPage::_drawInsertAtMarker(QPainter *painter)
{
    eASSERT(painter != eNULL);

    // Default width and height of an operator widget.
    eInt w=eGuiOperator::WIDTH*3, h=eGuiOperator::HEIGHT;

    painter->save();

    painter->translate(QPoint(m_insertAt.x*eGuiOperator::WIDTH, m_insertAt.y*eGuiOperator::HEIGHT));

    // Fade from half transparency to full transparency.
    QLinearGradient gradient(0, 0, w, 0);
    gradient.setColorAt(0, QColor(255, 255, 255, 127));
    gradient.setColorAt(1, QColor(255, 255, 255, 0));

    // Draw the background.
    qDrawPlainRect(painter, 0, 0, w+1, h+1, Qt::white, 0, &QBrush(gradient));

    // Fade from no transparency to full transparency.
    gradient.setColorAt(0, Qt::white);

    // Draw the left, upper and lower lines.
    painter->setPen(QPen(QBrush(gradient), 0));
    painter->drawLine(0, 0, 0, h);
    painter->drawLine(0, 0, w, 0);
    painter->drawLine(0, h, w, h);

    painter->restore();
}

// Returns rectangle of operator in scene
// coordinates.
QRect eGuiOpPage::_getOperatorRect(const eIOperator *op) const
{
    eASSERT(op != eNULL);

    return QRect(op->getPosition().x*eGuiOperator::WIDTH,
                 op->getPosition().y*eGuiOperator::HEIGHT,
                 op->getWidth()*eGuiOperator::WIDTH,
                 eGuiOperator::HEIGHT);
}

QPoint eGuiOpPage::_getNearestIntersectionPoint(const QRect &r, const QLine &line) const
{
    // First, calculate intersections with all four
    // rectangle sides.
    const QLineF lines[4] =
    {
        QLineF(r.topLeft(),     r.topRight()),
        QLineF(r.topRight(),    r.bottomRight()),
        QLineF(r.bottomRight(), r.bottomLeft()),
        QLineF(r.bottomLeft(),  r.topLeft())
    };

    QVector<QPoint> ips;
    QPointF ip;

    for (eInt i=0; i<4; i++)
    {
        if (lines[i].intersect(line, &ip) == QLineF::BoundedIntersection)
        {
            ips.append(ip.toPoint());
        }
    }

    eASSERT(ips.size() != eNULL);

    // Second, find nearest intersection point.
    QPoint nip = ips[0];
    eU32 minDist = (nip-line.p2()).manhattanLength();

    for (eInt i=1; i<ips.size(); i++)
    {
        eU32 dist = (ips[i]-line.p2()).manhattanLength();

        if (dist < minDist)
        {
            minDist = dist;
            nip = ips[i];
        }
    }

    return nip;
}

void eGuiOpPage::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);

    _updateViewingPos();
}

void eGuiOpPage::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);
    _drawInsertAtMarker(painter);
    _drawLinkLines(painter);
}

void eGuiOpPage::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsScene::mousePressEvent(me);

    m_pressButtons = me->buttons();
    m_pressPos = me->scenePos();
}

void eGuiOpPage::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsScene::mouseReleaseEvent(me);

    // Do not modify the insert-at cursor for drag & drop movements.
    const QPointF dist = me->scenePos()-m_pressPos;

    if (eAbs((eInt)dist.x()) > eGuiOperator::WIDTH || eAbs((eInt)dist.y()) > eGuiOperator::HEIGHT)
    {
        return;
    }

    // Reposition insert-at cursor on left/right mouse click.
    if (m_pressButtons & (Qt::LeftButton | Qt::RightButton))
    {
        m_insertAt.x = me->scenePos().x()/eGuiOperator::WIDTH;
        m_insertAt.y = me->scenePos().y()/eGuiOperator::HEIGHT;

        invalidate();
    }
}

// Implementation of comment item class.

eCommentItem::eCommentItem(const QString &text, const QPointF &pos, QGraphicsScene *parentScene) : QGraphicsTextItem(text, eNULL, parentScene)
{
    setPos(pos);
    setTextInteractionFlags(Qt::TextEditorInteraction);
    setDefaultTextColor(Qt::white);
    setFlags(flags() | QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
}

void eCommentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();
    painter->setPen(QPen(QBrush(Qt::white), 2));
    painter->setBrush(QBrush(Qt::darkGray, Qt::Dense4Pattern));
    painter->drawRect(boundingRect());
    painter->restore();

    QGraphicsTextItem::paint(painter, option, widget);
}

// Overwritte do disable the context menu.
void eCommentItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *scme)
{
    QGraphicsItem::contextMenuEvent(scme);
}