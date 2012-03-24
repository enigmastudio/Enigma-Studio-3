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

#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsScene>
#include <QtGui/QFontMetrics>
#include <QtGui/QPalette>
#include <QtGui/QPainter>

#include "demoseqitem.hpp"

eDemoSeqItem::eDemoSeqItem(eISequencerOp *op, eF32 scale) : QGraphicsItem(eNULL),
    m_op(op),
    m_scale(scale),
    m_resizing(eFALSE)
{
    eASSERT(op != eNULL);
    eASSERT(scale > 0.0f);

    setFlags(ItemIsFocusable | ItemIsMovable | ItemIsSelectable | ItemClipsToShape | ItemSendsGeometryChanges);
    _updatePosition();
}

void eDemoSeqItem::setScale(eF32 scale)
{
    m_scale = scale;
    _updatePosition();
}

const eISequencerOp * eDemoSeqItem::getOperator() const
{
    return m_op;
}

eF32 eDemoSeqItem::getStartTime() const
{
    eASSERT(m_op != eNULL);
    return m_op->getParameter(START_TIME).getValue().flt;
}

eF32 eDemoSeqItem::getEndTime() const
{
    return getStartTime()+getDuration();
}

eF32 eDemoSeqItem::getDuration() const
{
    eASSERT(m_op != eNULL);
    return m_op->getParameter(DURATION).getValue().flt;
}

eU32 eDemoSeqItem::getTrack() const
{
    eASSERT(m_op != eNULL);
    return (eU32)m_op->getParameter(TRACK).getValue().integer;
}

QRectF eDemoSeqItem::boundingRect() const
{
    return QRectF(0, 0, getDuration()*m_scale, (eF32)HEIGHT-1);
}

void eDemoSeqItem::_updatePosition()
{
    setPos(getStartTime()*m_scale, (eF32)(HEIGHT+getTrack()*HEIGHT+1));
}

static QColor mulColors(QColor c1, QColor c2)
{
	return QColor((c1.redF() * c2.redF())*255,
		(c1.greenF() * c2.greenF())*255,
		(c1.blueF() * c2.blueF())*255);
}


void eDemoSeqItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QString caption = m_op->getName();

    if (m_op->getUserName() != "")
    {
        caption = QString('"')+m_op->getUserName()+'"';
    }

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
	rCol.setBottom(3);
	QRect &rDark = boundingRect().toRect();
	rDark.setTop(r.height()-1);

    QColor grColor2 = mulColors(QColor(60, 70, 80), opColorDesat);
    QColor grColor = mulColors(QColor(110, 120, 130), opColorDesat);
    QColor grSelColor2 = mulColors(QColor(110, 120, 130), opColorDesat);
    QColor grSelColor = mulColors(QColor(100, 110, 120), opColorDesat);

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
    }
    else
    {
        gradientCol.setColorAt(0.0, opColor.lighter(150));
        gradientCol.setColorAt(1.0, opColor.darker(150));
		gradient.setColorAt(0.0, grColor);
        gradient.setColorAt(1.0, grColor2);
    }

    qDrawShadeRect(painter, r, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradient));
	qDrawShadeRect(painter, rCol, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientCol));
	qDrawShadeRect(painter, rDark, QPalette(QColor(10,20,30)), false, 0, 0, &QBrush(gradientDark));

    // Draw resizing area, marked with some dots.
    painter->setPen(Qt::darkGray);

    for (eF32 i=r.right()-(eF32)RESIZE_AREA; i<r.right()-2; i+=2.0f)
    {
        for (eInt j=3; j<HEIGHT-3; j+=2)
        {
            painter->drawPoint(i, (eF32)j);
        }
    }

    painter->setPen(Qt::white);
    painter->drawText(r.adjusted(2.0f, 2.0f, -2.0f-(eF32)RESIZE_AREA, -2.0f), Qt::AlignCenter, caption);
}

void eDemoSeqItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsItem::mouseReleaseEvent(me);
    m_resizing = eFALSE;
}

void eDemoSeqItem::mousePressEvent(QGraphicsSceneMouseEvent *me)
{
    QGraphicsItem::mousePressEvent(me);

    if (me->button() & Qt::LeftButton)
    {
        if (me->pos().x() >= boundingRect().width()-RESIZE_AREA)
        {
            m_resizing = eTRUE;
        }
    }
}

void eDemoSeqItem::mouseMoveEvent(QGraphicsSceneMouseEvent *me)
{
    if (m_resizing)
    {
        const eF32 change = (eF32)(me->pos().x()-me->lastPos().x())/m_scale;
        eF32 &duration = m_op->getParameter(DURATION).getValue().flt;

        duration += change;

        if (duration < 0.0f)
        {
            duration = 0.0f;
        }

        m_op->setChanged();

        prepareGeometryChange();
        scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
    }
    else
    {
        QGraphicsItem::mouseMoveEvent(me);
    }
}

QVariant eDemoSeqItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange)
    {
        QPointF newPos = value.toPointF();

        if (newPos.x() < 0.0f)
        {
            newPos.setX(0.0f);
        }

        const eInt newTrack = eClamp(0, ((eInt)newPos.y()-HEIGHT)/HEIGHT, eSequencer::MAX_TRACKS-1);
        const eF32 newStart = eClamp(0.0f, (eF32)newPos.x()/m_scale, (eF32)eDemo::MAX_RUNNING_TIME_MINS*60.0f-getDuration());

        m_op->getParameter(TRACK).getValue().integer = newTrack;
        m_op->getParameter(START_TIME).getValue().flt = newStart;
        m_op->setChanged();

        if (scene())
        {
            scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
        }

        return QPoint(newPos.x(), newTrack*HEIGHT+HEIGHT+1);
    }
 
    return QGraphicsItem::itemChange(change, value);
}