/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *   This file is part of
 *
 *    ------  /   /  /\   /  /---  -----  /  ----  /   /
 *       /   /   /  /  \ /  /---   -/-   /   \--  /---/   version 3
 *      /    \---  /    /  /---    /    /  ----/ /   /.
 *
 *       t i n y   m u s i c   s y n t h e s i z e r
 *
 *   Copyright © 2003-2010 Brain Control, all rights reserved.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TF_SPLINE_EDITOR_HPP
#define TF_SPLINE_EDITOR_HPP

#include <QtGui/QWidget>

#include "../../eshared/system/system.hpp"
#include "../../eshared/math/math.hpp"
#include "../../eshared/synth/tunefish3.hpp"

class tfInstrument;

class tfSplineEditor : public QWidget
{
    Q_OBJECT

public:
    tfSplineEditor(QWidget *parent=0);

	void            setSynth(tfInstrument *tf);

protected:
	virtual void	paintEvent(QPaintEvent * pe);

	virtual void    mouseMoveEvent(QMouseEvent *me);
    virtual void    mousePressEvent(QMouseEvent *me);
    virtual void    mouseReleaseEvent(QMouseEvent *me);

private:
	tfInstrument *  m_tf;
	eBool			m_dragging;
	eU32			m_dragPoint;
	eU32			m_pointOffset;
};

#endif // TF_SPLINE_EDITOR_HPP