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

#include <QtGui/QMouseEvent>
#include <QtGui/QScrollBar>

#include "demoseqview.hpp"

const eF32 eDemoSeqView::MAX_SCALE = 20.0f;

eDemoSeqView::eDemoSeqView(QWidget *parent) : QGraphicsView(parent),
    m_renderer(eNULL),
    m_time(0.0f),
    m_showGaps(eTRUE),
    m_demoOp(eNULL)
{
    setScene(new QGraphicsScene(this));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_lineItem = scene()->addRect(QRectF(), QPen(Qt::red));
    m_lineItem->setZValue(2);
    m_lineItem->setFlags(m_lineItem->flags() | QGraphicsItem::ItemIgnoresTransformations);

    setScale(0.5f);

    // Create context menu.
    QAction *act = m_menu.addAction("Finish editing timeline", this, SIGNAL(onFinishedEditing()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("Q"));
    addAction(act);
}

eDemoSeqView::~eDemoSeqView()
{
    eSAFE_DELETE(m_lineItem);
}

void eDemoSeqView::clear()
{
    for (eInt i=0; i<m_seqItems.size(); i++)
    {
        eSAFE_DELETE(m_seqItems[i]);
    }

    m_seqItems.clear();
}

void eDemoSeqView::setRenderer(eIRenderer *renderer)
{
    eASSERT(renderer != eNULL);
    m_renderer = renderer;
}

void eDemoSeqView::setTime(eF32 time)
{
    eASSERT(scene() != eNULL);

    m_time = eClamp(0.0f, time, (eF32)eDemo::MAX_RUNNING_TIME_MINS*60.0f);
    _updateLineItem();
}

void eDemoSeqView::setScale(eF32 scale)
{
    eASSERT(scale >= 0.0f && scale <= 1.0f);
    eASSERT(scene() != eNULL);

    m_scale = eLerp(0.0f, MAX_SCALE, scale);

    for (eInt i=0; i<m_seqItems.size(); i++)
    {
        m_seqItems[i]->setScale((eF32)TIME_PIXEL_STEP/m_scale);
    }

    // Resize scene accordingly.
    const eF32 sceneWidth = (eF32)(eDemo::MAX_RUNNING_TIME_MINS*60*TIME_PIXEL_STEP)/m_scale;

    scene()->setSceneRect(0.0f, 0.0f, sceneWidth, (eF32)((eSequencer::MAX_TRACKS+1)*eDemoSeqItem::HEIGHT));
    scene()->invalidate();
    _updateLineItem();
}

void eDemoSeqView::setShowGaps(eBool showGaps)
{
    eASSERT(scene() != eNULL);

    m_showGaps = showGaps;
    scene()->invalidate();
}

eF32 eDemoSeqView::getTime() const
{
    return m_time;
}

void eDemoSeqView::setDemoOpId(eID opId)
{
    eASSERT(scene() != eNULL);
    eASSERT(m_renderer != eNULL);

    m_demoOp = (eIDemoOp *)eDemoData::findOperator(opId);

    if (m_demoOp == eNULL)
    {
        scene()->invalidate();
        return;
    }

    eASSERT(m_demoOp->getType() == "Misc : Demo");
    m_demoOp->process(m_renderer, 0.0f);

    scene()->invalidate();
}

void eDemoSeqView::showEvent(QShowEvent *se)
{
    QGraphicsView::showEvent(se);
    _createItems();
}

void eDemoSeqView::mouseMoveEvent(QMouseEvent *me)
{
    if (me->buttons() & Qt::RightButton)
    {
        const eF32 t = mapToScene(me->pos()).x()/sceneRect().width();
        setTime(t*((eF32)eDemo::MAX_RUNNING_TIME_MINS*60.0f));
        
        Q_EMIT onTimeChanged(m_time);
        me->accept();
    }
    else if (me->buttons() & Qt::MidButton)
    {
        // Scroll view using middle mouse button.
        const QPoint delta = me->pos()-m_lastMousePos;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());

        m_lastMousePos = me->pos();
        me->accept();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(me);
    }
}

void eDemoSeqView::mousePressEvent(QMouseEvent *me)
{
    m_lastMousePos = me->pos();

    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ClosedHandCursor);
        me->accept();
    }
    else if (me->button() == Qt::RightButton)
    {
        mouseMoveEvent(me);
        me->accept();
    }
    else
    {
        QGraphicsView::mousePressEvent(me);
    }
}

void eDemoSeqView::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ArrowCursor);
        me->accept();
    }
    else
    {
        // Emit operator-selected signal, if there's
        // an operator under mouse.
        QGraphicsView::mouseReleaseEvent(me);

        eASSERT(scene() != eNULL);

        QGraphicsItem *item = scene()->itemAt(mapToScene(me->pos()));

        if (item && item != m_lineItem)
        {
            eDemoSeqItem *seqItem = (eDemoSeqItem *)item;
            Q_EMIT onOperatorSelected(seqItem->getOperator()->getId());
        }
    }
}

void eDemoSeqView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    QString buffer;

    for (eInt i=TIME_PIXEL_STEP, j=1; i<sceneRect().toRect().width(); i+=TIME_PIXEL_STEP, j++)
    {
        const eF32 time = (eF32)j*m_scale;
        const eU32 mins = (eU32)time/60;
        const eU32 secs = (eU32)time%60;
        const eU32 hund = (eU32)((time-(eU32)time)*100.0f);

        buffer.sprintf("%.2i:%.2i:%.2i", mins, secs, hund);

        painter->setPen(palette().foreground().color());
        painter->drawText(i-fontMetrics().width(buffer)/2, 15, buffer);
        painter->setPen(QPen(QBrush(QColor(70, 80, 90)), 1, Qt::DashLine));
        painter->drawLine(i, TRACKS_START+5, i, rect.bottom());
    }

    painter->setPen(QPen(QBrush(QColor(70, 80, 90)), Qt::SolidLine));
    painter->drawLine(rect.left(), eDemoSeqItem::HEIGHT-2, rect.right(), eDemoSeqItem::HEIGHT-2);

    for (eInt i=0; i<eSequencer::MAX_TRACKS; i++)
    {
        const eInt y = rect.top()+(i+1)*eDemoSeqItem::HEIGHT;
        painter->drawLine(rect.left(), y, rect.right(), y);
    }
}

void eDemoSeqView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    // Draw gaps (places on time line where no item is).
    if (m_showGaps)
    {
        _drawGaps(painter, rect);
    }

    // visualize overlapping items.
    _drawOverlaps(painter);
}

void eDemoSeqView::_createItems()
{
    clear();

    if (m_demoOp == eNULL)
    {
        return;
    }

    for (eU32 i=0; i<m_demoOp->getInputCount(); i++)
    {
        eISequencerOp *seqOp = (eISequencerOp *)m_demoOp->getInputOperator(i);
        eASSERT(seqOp != eNULL);

        eDemoSeqItem *item = new eDemoSeqItem(seqOp, (eF32)TIME_PIXEL_STEP/m_scale);
        eASSERT(item != eNULL);
        m_seqItems.append(item);
        scene()->addItem(item);
    }
}

 static eBool sortByStartTime(const eDemoSeqItem *item0, const eDemoSeqItem *item1)
 {
     eASSERT(item0 != eNULL);
     eASSERT(item1 != eNULL);

     return (item0->getStartTime() < item1->getStartTime());
 }

void eDemoSeqView::_drawGaps(QPainter *painter, const QRectF &rect)
{
    eASSERT(painter != eNULL);

    // First, all items which are completely overlayed
    // by another item are removed and the items are
    // sorted by start time.
    eDemoSeqItemPtrList items = m_seqItems;

    for (eInt i=items.size()-1; i>=0; i--)
    {
        for (eInt j=0; j<items.size(); j++)
        {
            if (i != j)
            {
                const eDemoSeqItem *item0 = items[i];
                const eDemoSeqItem *item1 = items[j];
                eASSERT(item0 != eNULL);
                eASSERT(item1 != eNULL);

                if (item1->getStartTime() <= item0->getStartTime())
                {
                    if (item1->getEndTime() >= item0->getEndTime())
                    {
                        items.removeAt(i);
                        break;
                    }
                }
            }
        }
    }

    qSort(items.begin(), items.end(), sortByStartTime);

    // Second, find out gaps between remaining items.
    GapList gaps;

    for (eInt i=0; i<items.size()-1; i++)
    {
        if (items[i]->getEndTime() < items[i+1]->getStartTime())
        {
            Gap gap;

            gap.startTime = items[i]->getEndTime();
            gap.endTime = items[i+1]->getStartTime();

            gaps.append(gap);
        }
    }

    // Third, is there a gap between time 0 and the first item?
    if (items.size() > 0 && items[0]->getStartTime() > 0.0f)
    {
        Gap gap;

        gap.startTime = 0.0f;
        gap.endTime = items[0]->getStartTime();

        gaps.append(gap);
    }

    // Draw gaps in cyan.
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(Qt::cyan, Qt::Dense4Pattern));

    for (eInt i=0; i<gaps.size(); i++)
    {
        const eF32 x0 = gaps[i].startTime*(eF32)TIME_PIXEL_STEP/m_scale;
        const eF32 x1 = gaps[i].endTime*(eF32)TIME_PIXEL_STEP/m_scale;

        painter->drawRect(QRectF(x0, eDemoSeqItem::HEIGHT+1, x1-x0, rect.bottom()));
    }

    painter->restore();
}

void eDemoSeqView::_drawOverlaps(QPainter *painter)
{
    eASSERT(painter != eNULL);
    eASSERT(scene() != eNULL);

    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));

    for (eInt i=0; i<m_seqItems.size(); i++)
    {
        QList<QGraphicsItem *> collided = m_seqItems[i]->collidingItems();

        for (eInt j=0; j<collided.size(); j++)
        {
            const QRectF overlap = collided[j]->sceneBoundingRect().intersect(m_seqItems[i]->sceneBoundingRect());
            painter->drawRect(overlap);
        }
    }

    painter->restore();
}

void eDemoSeqView::_updateLineItem()
{
    const eF32 x = m_time*(eF32)TIME_PIXEL_STEP/m_scale;
    const eF32 h = (eF32)scene()->sceneRect().height();

    m_lineItem->setRect(QRectF(x, 0.0f, 0.1f, h));
}