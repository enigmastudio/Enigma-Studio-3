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

#ifndef PROF_GRAPH_VIEW_HPP
#define PROF_GRAPH_VIEW_HPP

#include <QtGui/QLabel>
#include <QtCore/QQueue>

#include "../../eshared/eshared.hpp"

class eProfilerGraphView : public QLabel
{
public:
    eProfilerGraphView(QWidget *parent);

protected:
    virtual void    paintEvent(QPaintEvent *pe);
    virtual void    timerEvent(QTimerEvent *te);

private:
    eBool           _paintGraphAndInfos(QPainter &painter, const QString &name, const QColor &col, eInt y, const QQueue<eF32> &q) const;

private:
    eInt            m_timerId;
};

#endif // PROF_GRAPH_VIEW_HPP