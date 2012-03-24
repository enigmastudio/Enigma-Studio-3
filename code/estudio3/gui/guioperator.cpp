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

#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QPixmapCache>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

#include "guioperator.hpp"

// Initialize static members.
eID eGuiOperator::m_viewingOpId = eNOID;
eID eGuiOperator::m_editingOpId = eNOID;

// Set update-links to false, when adding many
// operators in a very short time (e.g. when
// loading a project). Links should be updated
// after adding all operators.
eGuiOperator::eGuiOperator(const QString &opType,
                           const ePoint &pos,
                           const eGuiOpPage *ownerPage,
                           eInt width,
                           eBool updateLinks,
                           eID opId) : QGraphicsObject(eNULL)
{
    eASSERT(ownerPage != eNULL);

    eOperatorPage *page = ownerPage->getPage();
    eASSERT(page != eNULL);

    m_op = page->addOperator(opType.toAscii().constData(), pos, width, updateLinks, opId);

    if (m_op)
    {
        _initialize();
    }
}

eGuiOperator::eGuiOperator(eIOperator *op, const eGuiOpPage *ownerPage) : QGraphicsObject(eNULL)
{
    eASSERT(ownerPage != eNULL);
    eASSERT(op != eNULL);

    m_op = op;
    _initialize();
}

eGuiOperator::~eGuiOperator()
{
    if (m_op)
    {
        m_op->getOwnerPage()->removeOperator(m_op->getId());
    }
}

void eGuiOperator::movePosition(const ePoint &dist, eBool updateLinks)
{
    eASSERT(m_op != eNULL);

    const ePoint &pos = m_op->getPosition()+dist;

    if (pos != m_op->getPosition())
    {
        setPos((eF32)(pos.x*WIDTH), (eF32)(pos.y*HEIGHT));
        m_op->setPosition(pos);

        if (updateLinks)
        {
            eOperatorPage *opPage = m_op->getOwnerPage();
            eASSERT(opPage != eNULL);
            opPage->updateLinks();
        }
    }
}

eIOperator * eGuiOperator::getOperator() const
{
    return m_op;
}

void eGuiOperator::saveToXml(QDomElement &node) const
{
    QDomDocument xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement opEl = xml.createElement("operator");
    opEl.setAttribute("type", QString(m_op->getRealType()));
    opEl.setAttribute("id", m_op->getId());
    opEl.setAttribute("username", QString(m_op->getUserName()));
    opEl.setAttribute("xpos", m_op->getPosition().x);
    opEl.setAttribute("ypos", m_op->getPosition().y);
    opEl.setAttribute("width", m_op->getWidth());
    opEl.setAttribute("bypassed", m_op->getBypassed());
    opEl.setAttribute("hidden", m_op->getHidden());
    node.appendChild(opEl);

    for (eU32 i=0; i<m_op->getParameterCount(); i++)
    {
        const eParameter &param = m_op->getParameter(i);

        QDomElement xmlParam = xml.createElement("parameter");
        xmlParam.setAttribute("name", QString(param.getName()));
        xmlParam.setAttribute("animchan", param.getAnimationChannel());
        xmlParam.setAttribute("animoffset", param.getAnimationTimeOffset());
        xmlParam.setAttribute("animscale", param.getAnimationTimeScale());
        xmlParam.setAttribute("animated", param.isAnimated());
        xmlParam.setAttribute("pathid", param.getAnimationPathOpId());

        switch (param.getType())
        {
            case eParameter::TYPE_TSHADERCODE:
            case eParameter::TYPE_LABEL:
            case eParameter::TYPE_STRING:
            case eParameter::TYPE_TEXT:
            case eParameter::TYPE_FILE:
            case eParameter::TYPE_SYNTH:
            {
                xmlParam.setAttribute("value", param.getValue().text);
                break;
            }

            case eParameter::TYPE_FLOAT:
            case eParameter::TYPE_FXY:
            case eParameter::TYPE_FXYZ:
            case eParameter::TYPE_FXYZW:
            {
				// Write default value if parameter is animated
				// to avoid problems when merging project files.
                const eVector4 value = param.getValue().fxyzw;
				const eVector4 def = param.getDefault().fxyzw;

                for (eU32 j=0; j<param.getComponentCount(); j++)
                {
					const eF32 v = (param.isAnimated() ? def[j] : value[j]);
                    xmlParam.setAttribute(QString("value")+eIntToStr(j), v);
                }

                break;
            }

            default:
            {
				// Write default value if parameter is animated
				// to avoid problems when merging project files.
                const eRect value = param.getValue().ixyxy;
				const eRect def = param.getDefault().ixyxy;

                for (eU32 j=0; j<param.getComponentCount(); j++)
                {
					const eInt v = (param.isAnimated() ? def[j] : value[j]);
                    xmlParam.setAttribute(QString("value")+eIntToStr(j), v);
                }

                break;
            }
        }

        opEl.appendChild(xmlParam);
    }
}

void eGuiOperator::loadFromXml(const QDomElement &node)
{
    // Load other properties.
    m_op->setUserName(node.attribute("username").toAscii().constData());
    m_op->setBypassed(node.attribute("bypassed").toInt());
    m_op->setHidden(node.attribute("hidden").toInt());

    // Load parameters of operator.
    QDomElement xmlParam = node.firstChildElement("parameter");

    while (!xmlParam.isNull())
    {
        const QString &paramName = xmlParam.attribute("name");

        // Find parameter with current parameter name.
        for (eU32 i=0; i<m_op->getParameterCount(); i++)
        {
            if (paramName == m_op->getParameter(i).getName())
            {
                // Found, so load in parameter data.
                eParameter &param = m_op->getParameter(i);

                param.setAnimationChannel((eParameter::AnimationChannel)xmlParam.attribute("animchan").toInt());
                param.setAnimationTimeOffset(xmlParam.attribute("animoffset").toFloat());
                param.setAnimationTimeScale(xmlParam.attribute("animscale").toFloat());
                param.setAnimated((eBool)xmlParam.attribute("animated").toInt());
                param.setAnimationPathOpId(xmlParam.attribute("pathid").toInt());

                switch (param.getType())
                {
                    case eParameter::TYPE_TSHADERCODE:
                    case eParameter::TYPE_LABEL:
                    case eParameter::TYPE_STRING:
                    case eParameter::TYPE_TEXT:
                    case eParameter::TYPE_FILE:
                    case eParameter::TYPE_SYNTH:
                    {
                        const QByteArray value = xmlParam.attribute("value").toAscii();
                        eStrNCopy(param.getValue().text, value, eParameter::MAX_TEXT_LENGTH);
                        break;
                    }

                    case eParameter::TYPE_FLOAT:
                    case eParameter::TYPE_FXY:
                    case eParameter::TYPE_FXYZ:
                    case eParameter::TYPE_FXYZW:
                    {
                        eVector4 value;

                        for (eU32 j=0; j<param.getComponentCount(); j++)
                        {
                            value[j] = xmlParam.attribute(QString("value")+eIntToStr(j)).toFloat();
                        }

                        param.getValue().fxyzw = value;
                        break;
                    }

                    default:
                    {
                        eRect value;

                        for (eU32 j=0; j<param.getComponentCount(); j++)
                        {
                            value[j] = xmlParam.attribute(QString("value")+eIntToStr(j)).toInt();
                        }

                        param.getValue().ixyxy = value;
                        break;
                    }
                }

                break;
            }
        }

        // Jump to next parameter in XML.
        xmlParam = xmlParam.nextSiblingElement("parameter");
    }
}

eBool eGuiOperator::isResizing() const
{
    return m_resizing;
}

QRectF eGuiOperator::boundingRect() const
{
    return QRectF(0, 0, m_op->getWidth()*WIDTH, HEIGHT);
}

void eGuiOperator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Try to find pixmap for operator in
    // cache. If it can't be found, recreate
    // the pixmap and insert it into cache.
    QPixmap pixmap;

    if (!QPixmapCache::find(_cacheKey(), pixmap))
    {
        pixmap = _createPixmap();
        QPixmapCache::insert(_cacheKey(), pixmap);
    }

    // Optimizes drawing a lot!
    painter->setClipRect(option->exposedRect);
    painter->drawPixmap(0, 0, pixmap);

    _paintStatusMarker(painter);
}

void eGuiOperator::setViewingOp(eGuiOperator *guiOp)
{
    if (guiOp == eNULL)
    {
        m_viewingOpId = eNOID;
    }
    else
    {
        m_viewingOpId = guiOp->getOperator()->getId();
        guiOp->scene()->invalidate();
    }
}

void eGuiOperator::setEditingOp(eGuiOperator *guiOp)
{
    if (guiOp == eNULL)
    {
        m_editingOpId = eNOID;
    }
    else
    {
        m_editingOpId = guiOp->getOperator()->getId();
        guiOp->scene()->invalidate();
    }
}

// Sets position and flags graphics-item
// (to be called from constructors).
void eGuiOperator::_initialize()
{
    eASSERT(m_op != eNULL);

    m_resizing = eFALSE;

    setFlags(ItemIsFocusable | ItemIsSelectable | ItemClipsToShape);
    setPos(m_op->getPosition().x*WIDTH, m_op->getPosition().y*HEIGHT);
}

// Paints a small rectangle if this operator is
// shown in viewport (red) and another rectangle
// if it's shown in paramter frame (blue). No
// to put these rectangles in a pixmap, because
// they're drawn too seldom.
void eGuiOperator::_paintStatusMarker(QPainter *painter)
{
    eASSERT(painter != eNULL);

    if (m_viewingOpId == m_op->getId())
    {
        painter->setPen(Qt::gray);
        painter->setBrush(QBrush(Qt::red, Qt::SolidPattern));
        painter->drawRect(2, 2, 5, 5);
    }

    if (m_editingOpId == m_op->getId())
    {
        painter->setPen(Qt::gray);
        painter->setBrush(QBrush(Qt::blue, Qt::SolidPattern));
        painter->drawRect(2, 9, 5, 5);
    }
}

static QColor mulColors(QColor c1, QColor c2)
{
	return QColor((c1.redF() * c2.redF())*255,
		(c1.greenF() * c2.greenF())*255,
		(c1.blueF() * c2.blueF())*255);
}

QPixmap eGuiOperator::_createPixmap() const
{
    QPixmap pixmap(boundingRect().size().toSize());
    QPainter painter(&pixmap);

    // Get color of operator. Set to dark,
    // red, if operator is invalid.
    QColor opColor = QColor(m_op->getColor().toArgb());
	opColor.setHsv(opColor.hsvHue(), 160, 255);

	QColor opColorDesat = opColor;
	opColorDesat.setHsv(opColor.hsvHue(), 40, 255);

    if (!m_op->getValid())
    {
        opColor = QColor(230, 0, 0);
    }

    // Draw operator body.
    const QRect &r = boundingRect().toRect();
	QRect &rCol = boundingRect().toRect();
	rCol.setTop(1);
	rCol.setLeft(1);
	rCol.setRight(r.width()-2);
	rCol.setBottom(2);
	QRect &rDark = boundingRect().toRect();
	rDark.setTop(r.height()-1); 

    QColor grColor2 = mulColors(QColor(60, 70, 80), opColorDesat);
    QColor grColor = mulColors(QColor(110, 120, 130), opColorDesat);
    QColor grSelColor2 = mulColors(QColor(70, 80, 90), opColorDesat);
    QColor grSelColor = mulColors(QColor(30, 40, 50), opColorDesat);

    QLinearGradient gradient(0.0f, 0.0f, r.width(), HEIGHT);
	QLinearGradient gradientCol(0.0f, 0.0f, r.width(), HEIGHT);
	QLinearGradient gradientDark(0.0f, 0.0f, r.width(), HEIGHT);

	gradientDark.setColorAt(0.0, QColor(10,20,30));
    gradientDark.setColorAt(1.0, QColor(60,70,80));

    // Make button color lighter using gradient if
    // button is selected.
    if (isSelected())
    {
        gradientCol.setColorAt(0.0, opColor.lighter(130));
        gradientCol.setColorAt(1.0, opColor.darker(60));
		gradient.setColorAt(0.0, grSelColor);
        gradient.setColorAt(1.0, grSelColor2);

        qDrawShadeRect(&painter, r, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradient));
	    qDrawShadeRect(&painter, rCol, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientDark));
	    qDrawShadeRect(&painter, rDark, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientCol));
    }
    else
    {
        gradientCol.setColorAt(0.0, opColor.lighter(150));
        gradientCol.setColorAt(1.0, opColor.darker(150));
		gradient.setColorAt(0.0, grColor);
        gradient.setColorAt(1.0, grColor2);

        qDrawShadeRect(&painter, r, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradient));
	    qDrawShadeRect(&painter, rCol, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientCol));
	    qDrawShadeRect(&painter, rDark, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientDark));
    }

	painter.setPen(QColor(170, 180, 190));
    for (eInt i=r.right()-RESIZE_AREA+2; i<r.right()-2; i+=2)
    {
        for (eInt j=3; j<HEIGHT-3; j+=2)
        {
            painter.drawPoint(i, j);
        }
    }

    // Paint operator-name or user-name (if set).
    // Text is written in gray, if operator is hidden
    // or bypassed.
    if (!m_op->getBypassed() && !m_op->getHidden())
    {
        painter.setPen(Qt::white);
    }

    QString caption = m_op->getName();

    if (m_op->getUserName() != "")
    {
        caption = QString('"')+m_op->getUserName()+'"';
    }

    painter.drawText(r.adjusted(2, 3, -2-RESIZE_AREA, -2), Qt::AlignCenter, caption);
    return pixmap;
}

// Returns key, uniquely identifying look of
// this operator. Used to find pixmap in cache.
const QString & eGuiOperator::_cacheKey() const
{
    static QString key;

    key = m_op->getRealType();
    key += m_op->getUserName();
    key += eIntToStr(m_op->getWidth());
    key += (m_op->getValid() ? "1" : "0");
    key += (isSelected() ? "1" : "0");
    key += (m_op->getBypassed() ? "1" : "0");
    key += (m_op->getHidden() ? "1" : "0");

    return key;
}

void eGuiOperator::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsItem::mouseReleaseEvent(me);
    m_resizing = eFALSE;
}

void eGuiOperator::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsItem::mousePressEvent(me);

    // User clicked with left mouse button into
    // resize area of operator => start resizing.
    if (me->button() == Qt::LeftButton &&
        me->pos().x() >= boundingRect().width()-RESIZE_AREA)
    {
        m_resizing = eTRUE;
    }
}

void eGuiOperator::mouseMoveEvent(QGraphicsSceneMouseEvent *me)
{
    if (m_resizing)
    {
        eASSERT(scene() != eNULL);

        // Calculate new width of operator.
        eInt newWidth = (eInt)eRound(me->pos().x()/(eF32)WIDTH);
        newWidth = eClamp(1, newWidth, eOperatorPage::WIDTH-m_op->getPosition().x);

        if (newWidth != m_op->getWidth())
        {
            prepareGeometryChange();

            m_op->setWidth(newWidth);
            m_op->getOwnerPage()->updateLinks();

            scene()->invalidate();
        }
    }
    else
    {
        // Move operator per default implementation.
        QGraphicsItem::mouseMoveEvent(me);
    }
}