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

#include <QtGui/QScrollBar>
#include <QtGui/QMouseEvent>

#include "pathview.hpp"

const eF32 ePathView::XZOOM_MAX     = 100.0f;
const eF32 ePathView::YZOOM_MAX     = 50.0f;
const eF32 ePathView::XCOORD_STEP   = 10.0f;
const eF32 ePathView::YCOORD_STEP   = 5.0f;
const eF32 ePathView::WP_SIZE       = 8.0f;

ePathView::ePathView(QWidget *parent) : QGraphicsView(parent),
    m_pathOp(eNULL),
    m_showTransPaths(eTRUE),
    m_showRotPaths(eTRUE),
    m_showScalePaths(eTRUE),
    m_time(0.0f)
{
    setScene(new QGraphicsScene(this));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    setZoom(eVector2(0.5f, 0.5f));

    // Create context menu.
    QAction *act = m_menu.addAction("Finish editing path", this, SIGNAL(onFinishedEditing()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("P"));
    addAction(act);

    act = m_menu.addSeparator();
    eASSERT(act != eNULL);
    addAction(act);

    act = m_menu.addAction("Add waypoint", this, SLOT(_onAddWaypoint()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence("A"));
    addAction(act);

    act = m_menu.addAction("Remove waypoint", this, SLOT(_onRemoveWaypoint()));
    eASSERT(act != eNULL);
    act->setShortcutContext(Qt::WidgetShortcut);
    act->setShortcut(QKeySequence::Delete);
    addAction(act);
 }

void ePathView::setVisiblePaths(eBool pos, eBool rot, eBool scale)
{
    m_showTransPaths = pos;
    m_showRotPaths   = rot;
    m_showScalePaths = scale;

    if (scene())
    {
        scene()->invalidate();
    }
}

void ePathView::setTime(eF32 time)
{
    m_time = eClamp(0.0f, time, (eF32)eDemo::MAX_RUNNING_TIME_MINS*60.0f);
    scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
}

eF32 ePathView::getTime() const
{
    return m_time;
}

void ePathView::setPathOpId(eID opId)
{
    eASSERT(scene() != eNULL);

    m_pathOp = (eIPathOp *)eDemoData::findOperator(opId);
    m_guiWps.clear();

    if (m_pathOp == eNULL)
    {
        scene()->invalidate();
        return;
    }

    _updatePathSamples();

    for (eU32 i=0; i<m_pathOp->getInputCount(); i++)
    {
        eIPathOp *op = (eIPathOp *)m_pathOp->getInputOperator(i);
        eASSERT(op != eNULL);

        eF32 &time = op->getParameter(0).getValue().flt;

        const eString &opType = op->getType();

        if (opType == "Path : TShader WP")
        {
            for (eU32 j=0; j<3; j++)
            {
                _addGuiWaypoint(time, eNULL, op, -1, (GuiWaypoint::Type)j);
            }
        }
        else if (opType == "Path : Full WP")
        {
            eFXYZ &pos = op->getParameter(2).getValue().fxyz;
            eFXYZ &rot = op->getParameter(3).getValue().fxyz;
            eFXYZ &scale = op->getParameter(4).getValue().fxyz;

            for (eU32 j=0; j<3; j++)
            {
                _addGuiWaypoint(time, &((eF32 *)&pos)[j], op, 2, GuiWaypoint::TYPE_TRANSLATION);
                _addGuiWaypoint(time, &((eF32 *)&rot)[j], op, 3, GuiWaypoint::TYPE_ROTATION);
                _addGuiWaypoint(time, &((eF32 *)&scale)[j], op, 4, GuiWaypoint::TYPE_SCALING);
            }
        }
        else if (opType == "Path : Scalar WP")
        {
            eFXYZ &pos = op->getParameter(2).getValue().fxyz;
            _addGuiWaypoint(time, &pos.x, op, 2, GuiWaypoint::TYPE_TRANSLATION);
        }
        else if (opType == "Path : Color WP" || opType == "Path : Vector WP")
        {
            for (eU32 j=0; j<3; j++)
            {
                eFXYZ &pos = op->getParameter(2).getValue().fxyz;
                _addGuiWaypoint(time, &((eF32 *)&pos)[j], op, 2, GuiWaypoint::TYPE_TRANSLATION);
            }

            // Add waypoints for alpha channel.
            if (opType == "Path : Color WP")
            {
                eFXYZW &rot = op->getParameter(2).getValue().fxyzw;
                _addGuiWaypoint(time, &rot.w, op, 2, GuiWaypoint::TYPE_ROTATION);
            }
        }
    }
}

void ePathView::setZoom(const eVector2 &zoom)
{
    eASSERT(zoom.x >= 0.0f && zoom.x <= 1.0f);
    eASSERT(zoom.y >= 0.0f && zoom.x <= 1.0f);
    eASSERT(scene() != eNULL);

    m_zoom.x = eLerp(0.0f, XZOOM_MAX, zoom.x);
    m_zoom.y = eLerp(0.0f, YZOOM_MAX, zoom.y);

    _updatePathSamples();

    centerOn(horizontalScrollBar()->value(), sceneRect().center().y());
    scene()->invalidate();
}

void ePathView::setSelectedWpInterpolType(ePath::InterpolationType ip)
{
    if (m_pathOp == eNULL)
    {
        return;
    }

    for (eU32 i=0; i<m_guiWps.size(); i++)
    {
        GuiWaypoint &guiWp = m_guiWps[i];

        if (guiWp.selected)
        {
            guiWp.pathOp->getParameter(1).getValue().enumSel = ip;
            guiWp.pathOp->setChanged();

            _updatePathSamples();
            scene()->invalidate();

            break;
        }
    }
}

eIPathOp * ePathView::getPathOp() const
{
    return m_pathOp;
}

eVector2 ePathView::getZoom() const
{
    return m_zoom;
}

void ePathView::_onAddWaypoint()
{
    if (m_pathOp)
    {
        Q_EMIT onAddWaypoint();

        // Recreate waypoints by resetting operator.
        setPathOpId(m_pathOp->getId());
    }
}

void ePathView::_onRemoveWaypoint()
{
    if (m_pathOp)
    {
        for (eU32 i=0; i<m_guiWps.size(); i++)
        {
            GuiWaypoint &guiWp = m_guiWps[i];

            if (guiWp.selected)
            {
                Q_EMIT onRemoveWaypoint(guiWp);

                // Recreate waypoints by resetting operator.
                setPathOpId(m_pathOp->getId());
                break;
            }
        }
    }
}

void ePathView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    resetCachedContent();

    _drawCoordinateSystem(painter, rect);
    _drawPaths(painter);
}

void ePathView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    // Draw red current time marker.
    const eF32 x = m_time*m_zoom.x;

    painter->setPen(Qt::red);
    painter->drawLine(x, rect.top(), x, rect.bottom());
}

void ePathView::mousePressEvent(QMouseEvent *me)
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
        //me->accept();
    }
    else if (me->buttons() & Qt::LeftButton)
    {
        // First set all waypoints to be not selected.
        for (eU32 i=0; i<m_guiWps.size(); i++)
        {
            m_guiWps[i].selected = eFALSE;
        }

        // Loop over waypoints to select one.
        const eF32 ycenter = sceneRect().center().y();
        const QPointF &p = mapToScene(me->pos());

        for (eU32 i=0; i<m_guiWps.size(); i++)
        {
            GuiWaypoint &guiWp = m_guiWps[i];

            const eF32 y = (guiWp.value ? *guiWp.value : 0.0f);
            const QRectF r(guiWp.time*m_zoom.x-WP_SIZE*0.5f, ycenter-y*m_zoom.y-WP_SIZE*0.5f, WP_SIZE, WP_SIZE);

            if (r.contains(p))
            {
                guiWp.selected = eTRUE;
                scene()->invalidate();
                Q_EMIT onSelectionChanged(&guiWp);
                return;
            }
        }

        // No new waypoint was selected.
        Q_EMIT onSelectionChanged(eNULL);
    }
    else
    {
        QGraphicsView::mousePressEvent(me);
    }
}

void ePathView::mouseReleaseEvent(QMouseEvent *me)
{
    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ArrowCursor);
        me->accept();
    }
    else
    {
        QGraphicsView::mouseReleaseEvent(me);
    }
}

void ePathView::mouseMoveEvent(QMouseEvent *me)
{
    // Scroll view using middle mouse button.
    if (me->buttons() & Qt::MidButton)
    {
        const QPoint delta = me->pos()-m_lastMousePos;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());

        m_lastMousePos = me->pos();
        me->accept();
    }
    else if ((me->buttons() & Qt::LeftButton) && scene())
    {
        // Move selected waypoint.
        for (eU32 i=0; i<m_guiWps.size(); i++)
        {
            const GuiWaypoint &guiWp = m_guiWps[i];

            if (guiWp.selected)
            {
                const QPointF &p = mapToScene(me->pos());

                if (guiWp.value)
                {
                    *guiWp.value = eClamp(guiWp.minVal, (eF32)((sceneRect().center().y()-p.y())/m_zoom.y), guiWp.maxVal);
                }

                guiWp.time = eMax(0.0f, (eF32)p.x()/m_zoom.x);
                guiWp.pathOp->setChanged();
            }
        }

        if (m_pathOp)
        {
            _updatePathSamples();
        }

        me->accept();
    }
    else if (me->buttons() & Qt::RightButton)
    {
        const eF32 t = mapToScene(me->pos()).x()/m_zoom.x;
        setTime(t);
        
        Q_EMIT onTimeChanged(m_time);
        me->accept();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(me);
    }
}

void ePathView::_drawCoordinateSystem(QPainter *painter, const QRectF &rect) const
{
    eASSERT(painter != eNULL);

    // Draw vertical lines.
    const QRectF &r = sceneRect();
    const eF32 ycenter = r.center().y();

    painter->save();
    painter->setPen(QPen(QBrush(QColor(70, 80, 90)), 1, Qt::DashLine));

    const eF32 xStep = m_zoom.x*XCOORD_STEP;
    eU32 time = (eU32)XCOORD_STEP;
    QString buffer;

    for (eF32 x=xStep; x<r.right(); x+=xStep, time+=(eInt)XCOORD_STEP)
    {
        painter->drawLine(x, r.top(), x, r.bottom());

        const eU32 mins = (eU32)time/60;
        const eU32 secs = (eU32)time%60;
        buffer.sprintf("%.2i:%.2i", mins, secs);

        painter->save();
        painter->setPen(palette().foreground().color());
        painter->drawText(x-fontMetrics().width(buffer)/2, (eInt)ycenter+15, buffer);
        painter->restore();
    }

    // Draw horizontal lines.
    const eF32 step = m_zoom.y*YCOORD_STEP;
    const eInt lines = (eInt)(ycenter/step);

    eF32 ya = ycenter;
    eF32 yb = ycenter;

    for (eInt i=0; i<lines; i++)
    {
        ya -= step;
        yb += step;

        painter->drawLine(r.left(), ya, r.right(), ya);
        painter->drawLine(r.left(), yb, r.right(), yb);

        painter->save();
        painter->setPen(Qt::white);
        _drawTextAxisY(painter, ya, eIntToStr((i+1)*5));
        _drawTextAxisY(painter, yb, eIntToStr((-1-i)*5));
        painter->restore();
    }

    painter->setPen(QPen(QBrush(QColor(90, 100, 110)), 3, Qt::SolidLine));
    painter->drawLine(r.left(), ycenter, r.right(), ycenter);
    painter->drawLine(1.0f, r.top(), 1.0f, r.bottom());
    painter->restore();
}

void ePathView::_drawTextAxisY(QPainter *painter, eF32 posY, const QString &text) const
{
    eASSERT(painter != eNULL);

    const QFontMetrics &fm = painter->fontMetrics();

    QRectF br = fm.boundingRect(text);
    br.moveCenter(QPointF(15.0f, posY));
    painter->drawText(br, Qt::AlignVCenter | Qt::AlignLeft, text);
}

void ePathView::_drawWaypoints(QPainter *painter, const ePath &path) const
{
    eASSERT(painter != eNULL);

    const eF32 coordSysY = sceneRect().center().y();

    painter->save();
    painter->setPen(Qt::black);
    painter->setBrush(Qt::black);

    for (eU32 i=0; i<m_guiWps.size(); i++)
    {
        const GuiWaypoint &guiWp = m_guiWps[i];

        if ((guiWp.type == GuiWaypoint::TYPE_TRANSLATION && m_showTransPaths) ||
            (guiWp.type == GuiWaypoint::TYPE_ROTATION && m_showRotPaths) ||
            (guiWp.type == GuiWaypoint::TYPE_SCALING && m_showScalePaths))
        {
            if (guiWp.selected)
            {
                painter->setBrush(Qt::white);
                painter->setPen(Qt::white);
            }
            else
            {
                painter->setBrush(Qt::black);
                painter->setPen(Qt::black);
            }

            const eF32 v = (guiWp.value ? *guiWp.value : 0.0f);
            const eF32 x = -WP_SIZE*0.5f+guiWp.time*m_zoom.x;
            const eF32 y = coordSysY-(WP_SIZE*0.5f+v*m_zoom.y);

            painter->drawRect(QRectF(x, y, WP_SIZE, WP_SIZE));
        }
    }

    painter->restore();
}

void ePathView::_drawPath(QPainter *painter, const eVector2Array &m_samples, const QPen &pen) const
{
    eASSERT(painter != eNULL);

    const eF32 coordSysY = sceneRect().center().y();

    painter->save();
    painter->setPen(pen);

    for (eInt i=0; i<(eInt)m_samples.size()-1; i++)
    {
        const eVector2 &s0 = m_samples[i];
        const eVector2 &s1 = m_samples[i+1];

        const QPointF p0(s0.x*m_zoom.x, coordSysY-s0.y*m_zoom.y);
        const QPointF p1(s1.x*m_zoom.x, coordSysY-s1.y*m_zoom.y);

        painter->drawLine(p0, p1);
    }

    painter->restore();
}

void ePathView::_drawPaths(QPainter *painter) const
{
    eASSERT(painter != eNULL);

    if (m_pathOp == eNULL)
    {
        return;
    }

    // Draw path between waypoints.
    const QBrush WP_BRUSHS[3] =
    {
        Qt::red,
        Qt::green,
        Qt::blue
    };

    for (eU32 i=0; i<3; i++)
    {
        if (m_showRotPaths)
        {
            _drawPath(painter, m_samples[0][i], QPen(WP_BRUSHS[i], 1, Qt::DashLine));
        }

        if (m_showTransPaths)
        {
            _drawPath(painter, m_samples[1][i], QPen(WP_BRUSHS[i], 1, Qt::SolidLine));
        }

        if (m_showScalePaths)
        {
            _drawPath(painter, m_samples[2][i], QPen(WP_BRUSHS[i], 1, Qt::DotLine));
        }
    }

    // Finally draw waypoints.
    const ePath path = _constructPath();
    _drawWaypoints(painter, path);
}

void ePathView::_addGuiWaypoint(eF32 &time, eF32 *value, eIPathOp *pathOp, eInt paramIndex, GuiWaypoint::Type wpType)
{
    eASSERT(time >= 0.0f);
    eASSERT(pathOp != eNULL);

    GuiWaypoint wp(time);

    wp.value = value;
    wp.pathOp = pathOp;
    wp.type = wpType;
    wp.selected = eFALSE;

    if (paramIndex >= 0)
    {
        pathOp->getParameter(paramIndex).getMinMax(wp.minVal, wp.maxVal);
    }
    else
    {
        wp.minVal = eF32_MIN;
        wp.maxVal = eF32_MAX;
    }

    m_guiWps.append(wp);
}

ePath ePathView::_constructPath() const
{
    eASSERT(m_pathOp != eNULL);

    m_pathOp->process(eNULL, 0.0f);
    return m_pathOp->getResult().path;
}

void ePathView::_updatePathSamples()
{
    const eF32 DEFAULT_MAX_TIME = 20.0f;
    const eF32 DEFAULT_MAX_VALUE = 7.5f;

    eF32 maxAbsValue = DEFAULT_MAX_VALUE;
    eF32 maxTime = DEFAULT_MAX_TIME;

    if (m_pathOp)
    {
        // Sample path and get minimum and maximum
        // coordiante system values.
        const ePath path = _constructPath();

        path.sample(m_samples[0], m_samples[1], m_samples[2], this);

        for (eU32 i=0; i<3; i++)
        {
            for (eU32 j=0; j<3; j++)
            {
                for (eU32 k=0; k<m_samples[i][j].size(); k++)
                {
                    maxTime = eMax(maxTime, m_samples[i][j][k].x);
                    maxAbsValue = eMax(maxAbsValue, eAbs(m_samples[i][j][k].y));
                }
            }
        }

        // Add some spacing on x- and y-axes.
        maxAbsValue += 2.5f;
        maxTime += 2.5f;
    }

    // Set scene dimensions and update scene.
    scene()->setSceneRect(0.0f, 0.0f, m_zoom.x*maxTime, m_zoom.y*2.0f*maxAbsValue);
    scene()->invalidate();
}