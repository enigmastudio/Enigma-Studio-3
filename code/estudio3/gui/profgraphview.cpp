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

#include <QtGui/QPainter>

#include "profgraphview.hpp"

QList<QQueue<eF32>> q; 
QQueue<eF32> qms;

eProfilerGraphView::eProfilerGraphView(QWidget *parent) : QLabel(parent)
{
    m_timerId = startTimer(10);
}

    static const eInt H=20;

void eProfilerGraphView::paintEvent(QPaintEvent *pe)
{
    QLabel::paintEvent(pe);

    QPainter painter(this);
    eF32 l = 0.0f;


    static const eInt MAX_SAMPLES = 200;

    eInt y=H;
        if (qms.size() > MAX_SAMPLES)
        {
            qms.dequeue();
        }
        qms.enqueue(eProfiler::getLastFrameTimeMs());
        _paintGraphAndInfos(painter, "ms/frame", Qt::white, y, qms);
        y+=H;

    for (eU32 i=0; i<eProfiler::getZoneCount(); i++)
    {
        const eProfiler::Zone &zone = eProfiler::getZone(i);
        const QColor zoneCol(zone.getColor().toArgb());

        const eF32 w = (eF32)zone.getSelfTimeMs()/(eF32)eProfiler::getLastFrameTimeMs()*(eF32)width();
        const eF32 r = (i == eProfiler::getZoneCount()-1 ? width()-l : w);

        /*
        if (w > 0.0f)
        {
            painter.setPen(zoneCol);
            painter.setBrush(QBrush(zoneCol));
            painter.drawRect(QRectF(l, 0.0f, r, height()));

            l += w;
        }
        */

        ///
        if ((eU32)q.size() <= i)
            q.push_back(QQueue<eF32>());

        if (q[i].size() > MAX_SAMPLES)
        {
            q[i].dequeue();
        }

        q[i].enqueue(zone.getSelfTimeMs());

        if (q[i].size() > 0)
        {
            if (_paintGraphAndInfos(painter, zone.getName(), zoneCol, y, q[i]))
            {
                y += H;
            }
        }
        ///
    }
}

void eProfilerGraphView::timerEvent(QTimerEvent *te)
{
    QLabel::timerEvent(te);
    update();
}

eBool eProfilerGraphView::_paintGraphAndInfos(QPainter &painter, const QString &name, const QColor &col, eInt y, const QQueue<eF32> &q) const
{
    eF32 max = q[0];
    eF32 min = max;
    eF32 mean = q[0];

    for (eInt k=1; k<q.size(); k++)
    {
        eF32 v = q[k];
        max = eMax(max, v);
        min = eMin(min, v);
        mean += v;
    }

    mean /= (eF32)q.size();
    eF32 var = 0.0f;

    for (eInt k=1; k<q.size(); k++)
    {
        eF32 d = q[k]-mean;
        var += d*d;
    }

    eF32 stdDev = eSqrt(var);

    if (max > 0)
    {
        painter.setPen(col);

        painter.drawText(0, y, name+" (min="+QString::number(min, 'f', 2)+", max="+QString::number(max, 'f', 2)+", mean="+QString::number(mean, 'f', 2)+", deviation="+QString::number(stdDev, 'f', 2)+")");

        for (eInt j=0; j<q.size()-1; j++)
        {
            QPointF p0((eF32)(width()-q.size()+j), (eF32)y-((eF32)q[j]/max)*(eF32)H);
            QPointF p1((eF32)(width()-q.size()+j+1), (eF32)y-((eF32)q[j+1]/max)*(eF32)H);

            painter.drawLine(p0, p1);
        }

        return eTRUE;
    }

    return eFALSE;
}