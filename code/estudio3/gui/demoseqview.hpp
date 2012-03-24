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

#ifndef DEMO_SEQ_VIEW_HPP
#define DEMO_SEQ_VIEW_HPP

#include <QtGui/QGraphicsView>
#include <QtGui/QMenu>

#include "demoseqitem.hpp"

#include "../../eshared/eshared.hpp"

class eDemoSeqView : public QGraphicsView
{
    Q_OBJECT

public:
    eDemoSeqView(QWidget *parent);
    virtual ~eDemoSeqView();

    void                clear();

    void                setRenderer(eIRenderer *renderer);
    void                setTime(eF32 time);
    void                setScale(eF32 scale);
    void                setShowGaps(eBool showGaps);

    eF32                getTime() const;

public Q_SLOTS:
    void                setDemoOpId(eID opId);

Q_SIGNALS:
    void                onFinishedEditing();
    void                onTimeChanged(eF32 time);
    void                onOperatorSelected(eID opId);

private:
    virtual void        showEvent(QShowEvent *se);
    virtual void        mouseMoveEvent(QMouseEvent *me);
    virtual void        mousePressEvent(QMouseEvent *me);
    virtual void        mouseReleaseEvent(QMouseEvent *me);

    virtual void        drawBackground(QPainter *painter, const QRectF &rect);
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);

private:
    void                _createItems();
    void                _drawGaps(QPainter *painter, const QRectF &rect);
    void                _drawOverlaps(QPainter *painter);
    void                _updateLineItem();

private:
    struct Gap
    {
        eF32            startTime;
        eF32            endTime;
    };

    typedef QList<Gap> GapList;

private:
    static const eInt   TRACK_HEIGHT = 26;
    static const eInt   TRACKS_START = 22;
    static const eInt   TIME_PIXEL_STEP = 100;
    static const eF32   MAX_SCALE;

private:
    QGraphicsRectItem * m_lineItem;
    eIRenderer *        m_renderer;
    eIDemoOp *          m_demoOp;
    eF32                m_time;
    eF32                m_scale;
    eDemoSeqItemPtrList m_seqItems;
    QPoint              m_lastMousePos;
    eBool               m_showGaps;
    QMenu               m_menu;
};

#endif // DEMO_SEQ_VIEW_HPP