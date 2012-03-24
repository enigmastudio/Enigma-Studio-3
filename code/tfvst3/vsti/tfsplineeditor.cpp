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

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include "tfsplineeditor.hpp"

tfSplineEditor::tfSplineEditor(QWidget *parent) : QWidget(parent),
	m_tf(eNULL),
	m_dragging(eFALSE),
	m_dragPoint(0),
	m_pointOffset(0)
{
}

void tfSplineEditor::setSynth(tfInstrument *tf)
{
	m_tf = tf;
}

void tfSplineEditor::paintEvent(QPaintEvent * pe)
{
	const eU32 viewWidth = this->width()-1;
    const eU32 viewHeight = this->height()-1;

	QPainter painter(this);
	
	QLinearGradient grad(QPointF(0.0f, 0.0f), QPointF(viewWidth, viewHeight));
	grad.setColorAt(0, QColor(20, 28, 30));
	grad.setColorAt(1, QColor(40, 48, 50));
	QBrush backgroundBrush(grad);
	QPen pen(Qt::black);

	painter.setBrush(backgroundBrush);
	painter.setPen(pen);
	painter.drawRect(this->rect());

    if (m_tf == eNULL)
		return;

    tfSpline &splines = m_tf->getOscillator()->getSpline();

    splines.update(m_tf->getParams(), eNULL, eNULL);
    eF32 oldY = 0.0f;
	eF32 oldY2 = 0.0f;
	eF32 oldY3 = 0.0f;

	eF32 drive = m_tf->getParam(TF_OSC_DRIVE);
	drive = (drive * drive * drive) * 32.0f + 1.0f;

	QPen penRefLines(QColor(60,60,80));
	QPen penSplineSrc(QBrush(QColor(90,140,190)), 0.7f);
	QPen penSplineDrv(QBrush(Qt::darkGray), 0.5f);
	QPen penSplineFin(QBrush(Qt::white), 1.0f);
	QPen penSep(Qt::gray, 0.5f, Qt::DashLine);

	// Draw background and spline.
	painter.setPen(penRefLines);
    painter.drawLine(0, viewHeight/2, viewWidth, viewHeight/2);
    painter.drawLine(0, viewHeight/4, viewWidth, viewHeight/4);
    painter.drawLine(0, viewHeight/4*3, viewWidth, viewHeight/4*3);

	painter.setRenderHint(QPainter::Antialiasing);
    for (eU32 x=0; x<viewWidth; x++)
    {
        eF32 y = splines.calc(1.0f/(eF32)(viewWidth-1)*(eF32)x);
		eF32 y2 = y;
		y2 *= drive;
		eF32 y3 = y2;
		if (y3 > 1.0f) y3 = 1.0f;
		if (y3 < -1.0f) y3 = -1.0f;

        y *= (eF32)(viewHeight/4);
        y += (eF32)(viewHeight/2);
		y2 *= (eF32)(viewHeight/4);
        y2 += (eF32)(viewHeight/2);
		y3 *= (eF32)(viewHeight/4);
        y3 += (eF32)(viewHeight/2);

		painter.setPen(penSplineSrc);
        painter.drawLine(x-1, viewHeight-oldY, x, viewHeight-y);
		painter.setPen(penSplineDrv);
		painter.drawLine(x-1, viewHeight-oldY2, x, viewHeight-y2);
		painter.setPen(penSplineFin);
		painter.drawLine(x-1, viewHeight-oldY3, x, viewHeight-y3);

        oldY = y;
		oldY2 = y2;
		oldY3 = y3;
    }
	painter.setRenderHint(QPainter::Antialiasing, eFALSE);

    // Draw spline control points.
    for (eU32 i=0; i<splines.getPointCount(); i++)
    {
        eVector2 p = splines.getPoint(i);

        p.y *= (eF32)(viewHeight/4);
        p.y += (eF32)(viewHeight/2);

        p.x *= (eF32)viewWidth;

		painter.setPen(penSep);
		painter.drawLine(p.x, 0, p.x, viewHeight);

		if (m_dragging && m_dragPoint+1 == i)
		{
			painter.setPen(QPen(Qt::yellow));
			painter.setBrush(QBrush(Qt::yellow));
		}
		else
		{
			painter.setPen(QPen(Qt::white));
			painter.setBrush(QBrush(Qt::white));
		}

        painter.drawRect(p.x-2.0f, viewHeight-p.y-2.0f, 4.0f, 4.0f);
    }
}

void tfSplineEditor::mouseMoveEvent(QMouseEvent *me)
{
	if (me->buttons() & Qt::LeftButton && m_dragging == eTRUE)
    {
		const eU32 viewWidth = this->width() - 1;
		const eU32 viewHeight = this->height() - 1;
		
		eF32 value = 1.0f - ((eClamp<eF32>(-1.0f, ((eF32)me->y() - (eF32)viewHeight / 2) / ((eF32)viewHeight / 4), 1.0f) + 1.0f) / 2.0f);
		eF32 offset = eClamp<eF32>(0.0f, (eF32)(me->x()-m_pointOffset) / (eF32)(viewWidth-m_pointOffset), 1.0f);

		m_tf->setParamUI(TF_OSC_POINT1_VALUE + (3 * m_dragPoint), value);
		m_tf->setParamUI(TF_OSC_POINT1_OFFSET + (3 * m_dragPoint), offset);

		update();
	}
}

void tfSplineEditor::mousePressEvent(QMouseEvent *me)
{
	if (me->buttons() & Qt::LeftButton)
    {
		const eU32 viewWidth = this->width()-1;
		const eU32 viewHeight = this->height()-1;
		const eS32 mouseX = me->x();
		const eS32 mouseY = me->y();
		tfSpline &splines = m_tf->getOscillator()->getSpline();

		m_pointOffset = 0;
		for (eU32 i=1; i<splines.getPointCount()-1; i++)
		{
			eVector2 p = splines.getPoint(i);
			p.y *= (eF32)(viewHeight/4);
			p.y += (eF32)(viewHeight/2);
			p.x *= (eF32)viewWidth;
			p.y = viewHeight - p.y;

			if (mouseX >= p.x-2.0f &&
				mouseX <= p.x+2.0f &&
				mouseY >= p.y-2.0f &&
				mouseY <= p.y+2.0f)
			{
				m_dragging = eTRUE;
				m_dragPoint = i-1;
				update();
				return;
			}

			m_pointOffset = p.x;
		}
	}
	else if (me->buttons() & Qt::RightButton)
    {
		const eU32 viewWidth = this->width()-1;
		const eS32 mouseX = me->x();
		tfSpline &splines = m_tf->getOscillator()->getSpline();

		for (eU32 i=1; i<splines.getPointCount()-1; i++)
		{
			eVector2 p = splines.getPoint(i);
			p.x *= (eF32)viewWidth;
			
			if (mouseX < p.x-2.0f)
			{
				eF32 intrp = m_tf->getParam(TF_OSC_POINT1_INTERPOLATION + (3 * (i-1)));
				if (intrp < 0.33f) 
					m_tf->setParamUI(TF_OSC_POINT1_INTERPOLATION + (3 * (i-1)), 0.5f);
				else if (intrp < 0.66f) 
					m_tf->setParamUI(TF_OSC_POINT1_INTERPOLATION + (3 * (i-1)), 1.0f);
				else 
					m_tf->setParamUI(TF_OSC_POINT1_INTERPOLATION + (3 * (i-1)), 0.0f);

				update();
				return;
			}
		}

		eF32 intrp = m_tf->getParam(TF_OSC_FINAL_INTERPOLATION);
		if (intrp < 0.33f) 
			m_tf->setParamUI(TF_OSC_FINAL_INTERPOLATION, 0.5f);
		else if (intrp < 0.66f) 
			m_tf->setParamUI(TF_OSC_FINAL_INTERPOLATION, 1.0f);
		else 
			m_tf->setParamUI(TF_OSC_FINAL_INTERPOLATION, 0.0f);

		update();
	}
}

void tfSplineEditor::mouseReleaseEvent(QMouseEvent *me)
{
	m_dragging = eFALSE;
}