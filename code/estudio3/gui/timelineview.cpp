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

#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPainter>

#include "timelineview.hpp"

eTimelineView::eTimelineView(QWidget *parent) : QFrame(parent),
    m_time(0.0f),
    m_playing(eFALSE),
	m_loopStart(0.0f),
	m_loopEnd(0.0f)
{
}

void eTimelineView::setTime(eF32 time)
{
    if (!eAreFloatsEqual(m_time, time))
    {
        m_time = time;

		if (m_loopStart != m_loopEnd)
		{
			const eF32 loopLen = m_loopEnd-m_loopStart;

			while (m_time >= m_loopEnd)
			{
				m_time -= loopLen;
			}
		}

        _clampTime();
        repaint();
    }
}

eBool eTimelineView::isPlaying()
{
    return m_playing;
}

eF32 eTimelineView::getTime() const
{
    return m_time;
}

void eTimelineView::setLoop(eF32 startTime, eF32 endTime)
{
    eASSERT(startTime >= 0.0f);
    eASSERT(endTime >= 0.0f);
    eASSERT(endTime >= startTime);

    if ((m_time < startTime || m_time >= endTime) && startTime != endTime)
    {
        m_time = startTime;
    }

	m_loopStart = startTime;
	m_loopEnd = endTime;
}

void eTimelineView::onDemoPlayPause()
{
    if (m_playing)
    {
        killTimer(m_timerId);
        m_playing = eFALSE;
        eDemo::getSynth().stop();

        setLoop(0.0f, 0.0f);
        eDemo::getSynth().setLoop(0, 0);
    }
    else
    {
        m_playing = eTRUE;
        m_timer.restart();
        m_playStartTime = m_time;
        m_timerId = startTimer(20);
        eDemo::getSynth().play(m_time);
    }
}

void eTimelineView::onDemoSkipForward()
{
    setTime((eF32)MAX_DEMO_SECS);
    m_playStartTime = m_time;
    m_timer.restart();

    if (m_playing)
    {
        eDemo::getSynth().play(m_time);
    }

    Q_EMIT onTimeChanged(m_time);
}

void eTimelineView::onDemoSkipBackward()
{
    setTime(0.0f);

    m_playStartTime = 0.0f;
    m_timer.restart();

    if (m_playing)
    {
        eDemo::getSynth().play(0.0f);
    }

    Q_EMIT onTimeChanged(m_time);
}

void eTimelineView::_drawTimeMarker(QPainter *painter)
{
    eASSERT(painter != eNULL);

    const eF32 xpos = eMin(eTrunc((eF32)(geometry().width()/eDemo::MAX_RUNNING_TIME_MINS)/60.0f*m_time), size().width()-1);

    painter->save();
    painter->setPen(Qt::red);
    painter->drawLine(QLineF(xpos, 4.0f, xpos, geometry().height()));
    painter->restore();
}

void eTimelineView::_drawTimeline(QPainter *painter)
{
    eASSERT(painter != eNULL);
    painter->save();

    // Horizontal step between two lines.
    const eF32 step = (eF32)geometry().width()/(eF32)eDemo::MAX_RUNNING_TIME_MINS/20.0f;

    for (eU32 i=1; i<eDemo::MAX_RUNNING_TIME_MINS*20; i++)
    {
        QPointF p(i*step, 3.0f);

        if (i%20 == 0) // Minutes
        {
            const QString time = QString("0%1:00").arg(i/20);

            painter->drawText((eU32)p.x()-fontMetrics().width(time)/2, 11, time);
            p.setY(13.0f);
        }
        else if (i%10 == 0) // Half seconds
        {
            p.setY(18.0f);
        }
        else // 3/60 lines
        {
            p.setY(21.0f);
        }
		
        painter->drawLine(p, QPointF(p.x(), geometry().height()));
    }

    painter->restore();
}

void eTimelineView::_clampTime()
{
    m_time = eClamp(0.0f, m_time, (eF32)MAX_DEMO_SECS);
}

void eTimelineView::mouseMoveEvent(QMouseEvent *me)
{
    QFrame::mouseMoveEvent(me);

    setTime((eF32)me->pos().x()/(eF32)geometry().width()*(eF32)MAX_DEMO_SECS);
    m_playStartTime = m_time;
    m_timer.restart();

    Q_EMIT onTimeChanged(m_time);
}

void eTimelineView::mousePressEvent(QMouseEvent *me)
{
    QFrame::mousePressEvent(me);
    mouseMoveEvent(me);
}

void eTimelineView::paintEvent(QPaintEvent *pe)
{
    QFrame::paintEvent(pe);

    QPainter p(this);

    _drawTimeline(&p);
    _drawTimeMarker(&p);
}

void eTimelineView::timerEvent(QTimerEvent *te)
{
    QFrame::timerEvent(te);

    setTime(m_playStartTime+0.001f*(eF32)m_timer.getElapsedMs());
    Q_EMIT onTimeChanged(m_time);
}