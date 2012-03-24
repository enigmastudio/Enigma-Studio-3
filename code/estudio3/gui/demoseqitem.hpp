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

#ifndef DEMO_SEQ_ITEM_HPP
#define DEMO_SEQ_ITEM_HPP

#include "../../eshared/eshared.hpp"

#include <QtGui/QGraphicsItem>

// Represents an item in the sequencer view.
class eDemoSeqItem : public QGraphicsItem
{
public:
    eDemoSeqItem(eISequencerOp *op, eF32 scale);

    void                    setScale(eF32 scale);

    const eISequencerOp *   getOperator() const;
    eF32                    getStartTime() const;
    eF32                    getEndTime() const;
    eF32                    getDuration() const;
    eU32                    getTrack() const;

public:
    virtual QRectF          boundingRect() const;

private:
    eBool                   _isMoveValid(eInt newTrack, eF32 newStart, eF32 newDuration) const;
    void                    _updatePosition();

private:
    virtual void            paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    virtual void            mouseReleaseEvent(QGraphicsSceneMouseEvent *me);
    virtual void            mousePressEvent(QGraphicsSceneMouseEvent *me);
    virtual void            mouseMoveEvent(QGraphicsSceneMouseEvent *me);
    virtual QVariant        itemChange(GraphicsItemChange change, const QVariant &value);

public:
    static const eInt       HEIGHT = 25;
    static const eInt       RESIZE_AREA = 12;

private:
    enum ParameterIndices
    {
        START_TIME = 0,
        DURATION = 1,
        TRACK = 2,
    };

private:
    eISequencerOp *         m_op;
    eF32                    m_scale;
    eBool                   m_resizing;
};

typedef QList<eDemoSeqItem *> eDemoSeqItemPtrList;

#endif // DEMO_SEQ_ITEM_HPP