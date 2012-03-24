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

#include <QtGui/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QScrollBar>
#include <QtGui/QMenu>

#include "guioperator.hpp"
#include "guioppage.hpp"
#include "pageview.hpp"

ePageView::ePageView(QWidget *parent) : QGraphicsView(parent),
    m_addOpDlg(this),
    m_showGrid(eTRUE),
    m_moving(eFALSE)
{
    _createActions();
    _createGotoAnimation();
}

// Scrolls graphics-view to given position.
void ePageView::scrollTo(const QPoint &pos)
{
    eASSERT(pos.x() >= 0);
    eASSERT(pos.y() >= 0);

    // Fixes wrong initial scroll position
    // after startup.
    centerOn(0, 0);

    // Set position using scrollbars.
    horizontalScrollBar()->setValue(pos.x());
    verticalScrollBar()->setValue(pos.y());
}

void ePageView::gotoOperator(eGuiOperator *guiOp)
{
    eASSERT(guiOp != eNULL);

    ensureVisible(guiOp);

    m_gotoAnim.setItem(guiOp);
    m_gotoAnimTimeLine.start();
}

void ePageView::_createActions()
{
    // Create actions, shown when right clicking
    // somewhere on the page view (not on an operator).
    QAction *act = m_viewMenu.addAction("Show grid", this, SLOT(_onShowGrid()));
    eASSERT(act != eNULL);
    act->setCheckable(true);
    act->setChecked(true);

    act = m_viewMenu.addAction("Zoom in", this, SLOT(_onZoomIn()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("+"));

    act = m_viewMenu.addAction("Zoom out", this, SLOT(_onZoomOut()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("-"));

    m_viewMenu.addSeparator();

    act = m_viewMenu.addAction("Select all", this, SLOT(_onSelectAll()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("Ctrl+A"));

    QAction *addOpAct = m_viewMenu.addAction("Add operator", this, SLOT(_onOperatorAdd()));
    eASSERT(addOpAct != eNULL);
    addOpAct->setShortcutContext(Qt::WidgetShortcut);
    addOpAct->setShortcut(QKeySequence("A"));

    QAction *addCmtAct = m_viewMenu.addAction("Add comment", this, SLOT(_onCommentAdd()));
    eASSERT(addCmtAct != eNULL);
    addCmtAct->setShortcutContext(Qt::WidgetShortcut);
    addCmtAct->setShortcut(QKeySequence("O"));

    QAction *removeAct = m_viewMenu.addAction("Remove", this, SLOT(_onRemoveSelected()));
    eASSERT(removeAct != eNULL);
    removeAct->setShortcutContext(Qt::WidgetShortcut);
    removeAct->setShortcut(QKeySequence(QKeySequence::Delete));

    QAction *cutAct = m_viewMenu.addAction("Cut", this, SLOT(_onOperatorCut()));
    eASSERT(cutAct != eNULL);
    cutAct->setShortcutContext(Qt::WidgetShortcut);
	QList<QKeySequence>l;
	l.append(QKeySequence("X"));
	l.append(QKeySequence("Ctrl+X"));
    cutAct->setShortcuts(l);

    QAction *copyAct = m_viewMenu.addAction("Copy", this, SLOT(_onOperatorCopy()));
    eASSERT(copyAct != eNULL);
    copyAct->setShortcutContext(Qt::WidgetShortcut);
	l.clear();
	l.append(QKeySequence("C"));
	l.append(QKeySequence("Ctrl+C"));
    copyAct->setShortcuts(l);

    QAction *pasteAct = m_viewMenu.addAction("Paste", this, SLOT(_onOperatorPaste()));
    eASSERT(pasteAct != eNULL);
    pasteAct->setShortcutContext(Qt::WidgetShortcut);
	l.clear();
	l.append(QKeySequence("V"));
	l.append(QKeySequence("Ctrl+V"));
    pasteAct->setShortcuts(l);

    addActions(m_viewMenu.actions());

    // Create actions, shown when right clicking
    // on an operator.
    act = m_opMenu.addAction("Show", this, SLOT(_onOperatorShow()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("S"));
    addAction(act);

    act = m_opMenu.addAction("Goto", this, SLOT(_onLoadOperatorGoto()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("G"));
    addAction(act);

    act = m_opMenu.addAction("Edit path", this, SLOT(_onPathOperatorEdit()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("P"));
    addAction(act);

    act = m_opMenu.addAction("Edit timeline", this, SLOT(_onDemoOperatorEdit()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("Q"));
    addAction(act);

    m_opMenu.addSeparator();
    m_opMenu.addAction(removeAct);
    m_opMenu.addAction(cutAct);
    m_opMenu.addAction(copyAct);
    m_opMenu.addAction(pasteAct);
}

void ePageView::_createGotoAnimation()
{
    m_gotoAnimTimeLine.setDuration(333);
    m_gotoAnimTimeLine.setLoopCount(3);
    m_gotoAnim.setTimeLine(&m_gotoAnimTimeLine);

    for (eInt i=0; i<=50; i++)
    {
        const eF32 t = (eF32)i/50.0f;

        const eF32 scale0 = eLerp(1.0f, 1.5f, t);
        const eF32 scale1 = eLerp(1.5f, 1.0f, t);

        m_gotoAnim.setScaleAt((eF32)i/100.0f, scale0, scale0);
        m_gotoAnim.setScaleAt((eF32)(i+50)/100.0f, scale1, scale1);
    }
}

eBool ePageView::_addOperator(const QString &opType, const ePoint &pos)
{
    eASSERT(scene() != eNULL);

    eGuiOperator *guiOp = new eGuiOperator(opType, pos, (eGuiOpPage *)scene());
    eASSERT(guiOp != eNULL);

    // Fail if we were unable to add the operator (e.g. due to overlap).
    if (!guiOp->getOperator())
    {
        eSAFE_DELETE(guiOp);
        return eFALSE;
    }

    scene()->clearSelection();
    scene()->addItem(guiOp);
    guiOp->setSelected(true);
    guiOp->setFocus();

    eGuiOperator::setEditingOp(guiOp);

    Q_EMIT onOperatorAdded(guiOp->getOperator()->getId());
    Q_EMIT onOperatorSelected(guiOp->getOperator()->getId());

    return eTRUE;
}

void ePageView::_onZoomIn()
{
    scale(1.5f, 1.5f);
}

void ePageView::_onZoomOut()
{
    scale(1.0/1.5f, 1.0f/1.5f);
}

void ePageView::_onShowGrid()
{
    m_showGrid = !m_showGrid;

    if (scene())
    {
        scene()->invalidate();
    }
}

void ePageView::_onCommentAdd()
{
    eGuiOpPage *guiPage = (eGuiOpPage *)scene();
    eASSERT(guiPage != eNULL);

    const ePoint insertAt = guiPage->getInsertAt();
    const QPoint pos((eF32)(insertAt.x*eGuiOperator::WIDTH), (eF32)(insertAt.y*eGuiOperator::HEIGHT));

    guiPage->addComment("New comment", pos);
}

void ePageView::_onSelectAll()
{
    for (eInt i=0; i<items().size(); i++)
    {
        items().at(i)->setSelected(true);
    }
}

void ePageView::_onOperatorAdd()
{
    eGuiOpPage *guiPage = (eGuiOpPage *)scene();
    eASSERT(guiPage != eNULL);

    // Move dialog to current mouse position.
    const ePoint insertAt = guiPage->getInsertAt();
    QPoint dlgPos(insertAt.x*eGuiOperator::WIDTH, insertAt.y*eGuiOperator::HEIGHT);

    dlgPos = mapToGlobal(mapFromScene(dlgPos));

    m_addOpDlg.move(dlgPos);
    m_addOpDlg.setFilterOp(guiPage->getPage()->getOperatorByPos(ePoint(insertAt.x, insertAt.y-1), 3));

    // Execute dialog and add operator if one was selected.
    if (m_addOpDlg.exec() == QDialog::Accepted)
    {
        if (_addOperator(m_addOpDlg.getChosenOpType(), insertAt))
        {
            guiPage->advanceInsertAt();
        }
    }
}

void ePageView::_onRemoveSelected()
{
    eASSERT(scene() != eNULL);

    for (eInt i=scene()->selectedItems().size()-1; i>=0; i--)
    {
        QGraphicsItem *item = scene()->selectedItems().at(i);
        eASSERT(item != eNULL);

        if (item->type() != QGraphicsTextItem::Type)
        {
            eGuiOperator *guiOp = (eGuiOperator *)item;
            eASSERT(guiOp != eNULL);
            const eID opId = guiOp->getOperator()->getId();

            Q_EMIT onOperatorRemoved(opId);
        }

        eSAFE_DELETE(item);
        scene()->invalidate();
    }

    scene()->invalidate();
}

void ePageView::_onOperatorShow()
{
    if (scene() && scene()->selectedItems().size() > 0)
    {
        eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems().first();
        eASSERT(guiOp != eNULL);

        eGuiOperator::setViewingOp(guiOp);
        Q_EMIT onOperatorShow(guiOp->getOperator()->getId());
        invalidateScene();
    }
}

void ePageView::_onOperatorRemove()
{
    _onRemoveSelected();
}

void ePageView::_onOperatorCut()
{
    _onOperatorCopy();
    _onRemoveSelected();
}

void ePageView::_onOperatorCopy()
{
    eASSERT(scene() != eNULL);

    QDomDocument xml;
    QDomElement rootEl = xml.createElement("operators");
    xml.appendChild(rootEl);

    for (eInt i=0; i<scene()->selectedItems().size(); i++)
    {
        const eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems().at(i);
        eASSERT(guiOp != eNULL);

        guiOp->saveToXml(rootEl);
    }

    QMimeData *md = new QMimeData;
    md->setData("operator/xml", QByteArray(xml.toString().toAscii()));
    QApplication::clipboard()->setMimeData(md);
}

void ePageView::_onOperatorPaste()
{
    eGuiOpPage *guiPage = (eGuiOpPage *)scene();
    eASSERT(guiPage != eNULL);

    // Check if clipboard format is correct.
    const QMimeData *md = QApplication::clipboard()->mimeData();
    eASSERT(md != eNULL);

    if (!md->hasFormat("operator/xml"))
    {
        return;
    }

    // Get operator with smallest (x, y) position,
    // needed to calculate new positions for
    // pasted operators.
    QDomDocument xml;
    xml.setContent(md->data("operator/xml"));
    QDomElement xmlOp = xml.firstChild().firstChildElement("operator");

    ePoint minPos(eS32_MAX, eS32_MAX);

    while (!xmlOp.isNull())
    {
        const eInt x = xmlOp.attribute("xpos").toInt();
        const eInt y = xmlOp.attribute("ypos").toInt();

        if (x < minPos.x)
        {
            minPos.x = x;
        }

        if (y < minPos.y)
        {
            minPos.y = y;
        }

        xmlOp = xmlOp.nextSiblingElement("operator");
    }

    // Iterate the second time through operators. This
    // time the operators are added to the page using
    // the previously calculated minimum relative
    // position.
    xmlOp = xml.firstChild().firstChildElement("operator");

    while (!xmlOp.isNull())
    {
        const QString opType = xmlOp.attribute("type");
        const eU32 xpos = xmlOp.attribute("xpos").toInt();
        const eU32 ypos = xmlOp.attribute("ypos").toInt();
        const eU32 width = xmlOp.attribute("width").toInt();
        const eID opId = xmlOp.attribute("id").toInt();

        const ePoint newPos = guiPage->getInsertAt()+(ePoint(xpos, ypos)-minPos);
        eASSERT(newPos.x >= 0 && newPos.y >= 0);

        // Only add operator, if its position
        // lies on operator page.
        if (newPos.y < eOperatorPage::HEIGHT &&
            newPos.x+width <= eOperatorPage::WIDTH)
        {
            eGuiOperator *guiOp = new eGuiOperator(opType.toAscii().constData(), newPos, guiPage, width, eFALSE, opId);
            eASSERT(guiOp != eNULL);

            // Was adding of operator successful?
            if (guiOp->getOperator())
            {
                guiOp->loadFromXml(xmlOp);
                scene()->addItem(guiOp);
            }
            else
            {
                eSAFE_DELETE(guiOp);
            }
        }

        xmlOp = xmlOp.nextSiblingElement("operator");
    }

    // Finally, after adding all operators,
    // update links on page.
    guiPage->getPage()->updateLinks();
    scene()->invalidate();
}

void ePageView::_onPathOperatorEdit()
{
    eASSERT(scene() != eNULL);

    if (scene()->selectedItems().size() == 1)
    {
        eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems().at(0);
        eASSERT(guiOp != eNULL);

        if (TEST_CATEGORY(guiOp->getOperator(), "Path", Path_CID))
        {
            Q_EMIT onPathOperatorEdit(guiOp->getOperator()->getId());
        }
    }
}

void ePageView::_onDemoOperatorEdit()
{
    eASSERT(scene() != eNULL);

    if (scene()->selectedItems().size() == 1)
    {
        eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems().at(0);
        eASSERT(guiOp != eNULL);

        if (guiOp->getOperator()->getType() == "Misc : Demo")
        {
            Q_EMIT onDemoOperatorEdit(guiOp->getOperator()->getId());
        }
    }
}

void ePageView::_onLoadOperatorGoto()
{
    eASSERT(scene() != eNULL);

    if (scene()->selectedItems().size() == 1)
    {
        eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems().at(0);
        eASSERT(guiOp != eNULL);
        const eIOperator *op = guiOp->getOperator();
        eASSERT(op != eNULL);

        if (op->getRealType() == "Misc : Load")
        {
            Q_EMIT onGotoLoadedOperator(op->getParameter(0).getValue().linkedOpId);
        }
    }
}

void ePageView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    if (!m_moving || !scene())
    {
        return;
    }

    for (eInt i=0; i<scene()->selectedItems().size(); i++)
    {
        const eGuiOperator *guiOp = (eGuiOperator *)scene()->selectedItems()[i];
        eASSERT(guiOp != eNULL);

        const QPoint newPos = m_mouseMoveDist;
        const eF32 x = (eF32)((newPos.x()/eGuiOperator::WIDTH+guiOp->getOperator()->getPosition().x)*eGuiOperator::WIDTH);
        const eF32 y = (eF32)((newPos.y()/eGuiOperator::HEIGHT+guiOp->getOperator()->getPosition().y)*eGuiOperator::HEIGHT);

        QRectF r = guiOp->boundingRect();
        r.moveTopLeft(QPointF(x, y));

        painter->drawRect(r);
    }
}

void ePageView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Render alignment grid.
    if (m_showGrid)
    {
        // Setup integral grid rectangle.
        const QRect gr = rect.toRect();
        const eInt startX = gr.left()+eGuiOperator::WIDTH-(gr.left()%eGuiOperator::WIDTH);
        const eInt startY = gr.top()+eGuiOperator::HEIGHT-(gr.top()%eGuiOperator::HEIGHT);

        // Grid color is page view's background
        // color, but only 90% intensity.
        const QColor gridCol = QColor(60, 70, 80).lighter(90);

        painter->save();
        painter->setPen(gridCol);
        painter->setOpacity(0.7f);

        for (eInt x=startX; x<=gr.right(); x+=eGuiOperator::WIDTH)
        {
            painter->drawLine(x, gr.top(), x, gr.bottom());
        }

        for (eInt y=startY; y<=gr.bottom(); y+=eGuiOperator::HEIGHT)
        {
            painter->drawLine(gr.left(), y, gr.right(), y);
        }

        painter->restore();
    }

    // Call base class' function in the end,
    // because grid should be drawn behind
    // everything else.
    QGraphicsView::drawBackground(painter, rect);
}

void ePageView::mouseDoubleClickEvent(QMouseEvent *de)
{
    QGraphicsView::mouseDoubleClickEvent(de);

    if (scene())
    {
        eGuiOperator *guiOp = (eGuiOperator *)scene()->itemAt(mapToScene(de->pos()));

        if (guiOp)
        {
            eGuiOperator::setViewingOp(guiOp);
            Q_EMIT onOperatorShow(guiOp->getOperator()->getId());
            invalidateScene();
        }
    }
}

void ePageView::mouseMoveEvent(QMouseEvent *me)
{
    if (m_moving)
    {
        eASSERT(scene() != eNULL);

        m_mouseMoveDist = me->pos()-m_mouseDownPos;
        scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
    }

    // Scroll view using middle mouse button.
	if (me->buttons() & Qt::MidButton)
    {
        const QPoint delta = me->pos()-m_lastMousePos;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());

        m_lastMousePos = me->pos();
		setCursor(Qt::ClosedHandCursor);
        me->accept();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(me);
    }
}

void ePageView::mousePressEvent(QMouseEvent *me)
{
    m_mouseDownPos = me->pos();
    m_lastMousePos = me->pos();

    if (me->button() == Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(me);

        // Emit operator-selected signal, if there's
        // an operator under mouse.
        if (scene())
        {
            eGuiOperator *guiOp = (eGuiOperator *)scene()->itemAt(mapToScene(me->pos()));

            if (guiOp && !guiOp->isResizing())
            {
                m_moving = eTRUE;

                eGuiOperator::setEditingOp(guiOp);
                Q_EMIT onOperatorSelected(guiOp->getOperator()->getId());
            }
        }
    }
    else if (me->button() == Qt::RightButton && scene())
    {
        // When right-clicking clear all selected
        // operators if clicked operator does not
        // lie with selection.
        eGuiOperator *guiOp = (eGuiOperator *)scene()->itemAt(mapToScene(me->pos()));

        if (guiOp)
        {
            if (!scene()->selectedItems().contains(guiOp))
            {
                scene()->clearSelection();
            }

            guiOp->setSelected(true);
            eGuiOperator::setEditingOp(guiOp);
            Q_EMIT onOperatorSelected(guiOp->getOperator()->getId());
        }
    }

    mouseMoveEvent(me);
}

void ePageView::mouseReleaseEvent(QMouseEvent *me)
{
    QGraphicsView::mouseReleaseEvent(me);

    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ArrowCursor);
    }

    // Update scene, because moving could have beed
    // switched off and black operator movement markers
    // have to be removed from foreground.
    if (m_moving)
    {
        eASSERT(scene() != eNULL);

        const QList<QGraphicsItem *> &selItems = scene()->selectedItems();
        eGuiOpPage *guiPage = (eGuiOpPage *)scene();

        const ePoint moveDist(m_mouseMoveDist.x()/eGuiOperator::WIDTH, (m_mouseMoveDist.y())/eGuiOperator::HEIGHT);

        if (guiPage->areSelectedOpsMovable(moveDist))
        {
            for (eInt i=0; i<selItems.size(); i++)
            {
                eGuiOperator *guiOp = (eGuiOperator *)selItems[i];
                eASSERT(guiOp != eNULL);

                guiOp->movePosition(moveDist);
            }
        }

        guiPage->getPage()->updateLinks();
        scene()->invalidate(QRectF(), QGraphicsScene::ItemLayer | QGraphicsScene::ForegroundLayer);
        m_moving = eFALSE;
    }
}

void ePageView::contextMenuEvent(QContextMenuEvent *ce)
{
    QGraphicsView::contextMenuEvent(ce);

	// show menu only if the mouse was still
    if (scene() && (m_mouseDownPos-ce->pos()).manhattanLength() < 4)
    {
        // Show operator's context menu.
        QGraphicsItem *item = scene()->itemAt(mapToScene(ce->pos()));

        if (item && item->type() != QGraphicsTextItem::Type)
        {
            // Depending on the type, menu entries
            // might have to be set to invisble.
            eIOperator *op = ((eGuiOperator *)item)->getOperator();
            eASSERT(op != eNULL);

            const eBool oneOpSel = (scene()->selectedItems().size() == 1);

            m_opMenu.actions().at(0)->setVisible(oneOpSel);
            m_opMenu.actions().at(1)->setVisible(oneOpSel && op->getRealType() == "Misc : Load");
            m_opMenu.actions().at(2)->setVisible(oneOpSel && op->getCategory() == "Path");
            m_opMenu.actions().at(3)->setVisible(oneOpSel && op->getType() == "Misc : Demo");

            m_opMenu.exec(QCursor::pos());
        }
        else
        {
            // Show view's context menu.
            m_viewMenu.exec(QCursor::pos());
        }
    }
}