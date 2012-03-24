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

#ifndef PATH_VIEW_HPP
#define PATH_VIEW_HPP

#include <QtGui/QGraphicsView>
#include <QtGui/QMenu>

#include "../../eshared/eshared.hpp"

class ePathView : public QGraphicsView
{
    Q_OBJECT

public:
    struct GuiWaypoint
    {
        enum Type
        {
            TYPE_ROTATION,
            TYPE_TRANSLATION,
            TYPE_SCALING
        };

        GuiWaypoint(eF32 &t) :
            time(t)
        {
        }

        eF32 &          time;
        eF32 *          value;
        eF32            minVal;
        eF32            maxVal;
        eBool           selected;
        eIPathOp *      pathOp;
        Type            type;
    };

public:
    ePathView(QWidget *parent);

    void                setVisiblePaths(eBool trans, eBool rot, eBool scale);
    void                setTime(eF32 time);

    eF32                getTime() const;

Q_SIGNALS:
    void                onFinishedEditing();
    void                onTimeChanged(eF32 time);
    void                onAddWaypoint();
    void                onRemoveWaypoint(ePathView::GuiWaypoint &guiWp);
    void                onSelectionChanged(ePathView::GuiWaypoint *guiWp);

public Q_SLOTS:
    void                setPathOpId(eID opId);
    void                setZoom(const eVector2 &zoom);
    void                setSelectedWpInterpolType(ePath::InterpolationType ip);

public:
    eIPathOp *          getPathOp() const;
    eVector2            getZoom() const;

private Q_SLOTS:
    void                _onAddWaypoint();
    void                _onRemoveWaypoint();

private:
    virtual void        drawBackground(QPainter *painter, const QRectF &rect);
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);
    virtual void        mousePressEvent(QMouseEvent *me);
    virtual void        mouseReleaseEvent(QMouseEvent *me);
    virtual void        mouseMoveEvent(QMouseEvent *me);

private:
    void                _drawCoordinateSystem(QPainter *painter, const QRectF &r) const;
    void                _drawTextAxisY(QPainter *painter, eF32 posY, const QString &text) const;
    void                _drawWaypoints(QPainter *painter, const ePath &path) const;
    void                _drawPath(QPainter *painter, const eVector2Array &samples, const QPen &pen) const;
    void                _drawPaths(QPainter *painter) const;
    void                _interpolatePath(const ePath &path, eF32 time, eVector3 &pos, eVector3 &rot, eVector3 &scale) const;
    void                _addGuiWaypoint(eF32 &time, eF32 *value, eIPathOp *pathOp, eInt paramIndex, GuiWaypoint::Type wpType);
    ePath               _constructPath() const;
    void                _updatePathSamples();

private:
    typedef eArray<GuiWaypoint> GuiWaypointArray;

private:
    static const eF32   XZOOM_MAX;
    static const eF32   YZOOM_MAX;
    static const eF32   XCOORD_STEP;
    static const eF32   YCOORD_STEP;
    static const eF32   WP_SIZE;

private:
    eVector2            m_zoom;
    eIPathOp *          m_pathOp;
    eBool               m_showTransPaths;
    eBool               m_showRotPaths;
    eBool               m_showScalePaths;
    GuiWaypointArray    m_guiWps;
    QMenu               m_menu;
    QPoint              m_lastMousePos;
    eF32                m_time;
    eVector2Array       m_samples[3][3];
};

#endif // PATH_VIEW_HPP