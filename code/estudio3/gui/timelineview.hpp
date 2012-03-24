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

#ifndef TIMELINE_VIEW_HPP
#define TIMELINE_VIEW_HPP

#include <QtGui/QFrame>

#include "../../eshared/eshared.hpp"

// Timeline widget at the bottom of the render view.
class eTimelineView : public QFrame
{
    Q_OBJECT

public:
    eTimelineView(QWidget *parent);

    void                setTime(eF32 time);
    eF32                getTime() const;
	void				setLoop(eF32 startTime, eF32 endTime);
    eBool               isPlaying();
    void                switchPlayPauseIcon();

public Q_SLOTS:
    void                onDemoPlayPause();
    void                onDemoSkipForward();
    void                onDemoSkipBackward();

Q_SIGNALS:
    void                onTimeChanged(eF32 time);

private:
    void                _drawTimeMarker(QPainter *painter);
    void                _drawTimeline(QPainter *painter);
    void                _clampTime();

private:
    virtual void        mouseMoveEvent(QMouseEvent *me);
    virtual void        mousePressEvent(QMouseEvent *me);
    virtual void        paintEvent(QPaintEvent *pe);
    virtual void        timerEvent(QTimerEvent *te);

private:
    static const eU32   MAX_DEMO_SECS = 60*eDemo::MAX_RUNNING_TIME_MINS;

private:
    eF32                m_time;
    eF32                m_playStartTime;
    eInt                m_timerId;
    eBool               m_playing;
    eTimer              m_timer;
	eF32				m_loopStart;
	eF32				m_loopEnd;
};

#endif // TIMELINE_VIEW_HPP