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

#include "tfdial.hpp"
#include "../../eshared/system/system.hpp"

tfDial::tfDial(QWidget *parent) : QDial(parent)
{
}

void tfDial::paintEvent(QPaintEvent *pe)
{
    eU32 arcrad = (this->rect().width() > this->rect().height() ? this->rect().height() : this->rect().height()) / 2;
    eS32 thickness = this->rect().width() / 6;
    eS32 thicknessH = thickness/2;
    eF32 amount = (eF32)(this->value() - this->minimum()) / (eF32)(this->maximum() - this->minimum());
    eS32 angle = (eS32)(amount * 360.0f);

    QPainter painter(this);
    QRect rect(this->rect().center() - QPoint(arcrad-2, arcrad-2), this->rect().center() + QPoint(arcrad-2, arcrad-2));
    QRect rect2(this->rect().center() - QPoint(arcrad-thickness-4, arcrad-thickness-4), this->rect().center() + QPoint(arcrad-thickness-4, arcrad-thickness-4));
    QConicalGradient gradient(rect.center(), 270);
    QColor color(this->palette().button().color());
    gradient.setColorAt(0, color.darker(80));
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(1, color.darker(120));

    QPen pen(QBrush(gradient), thickness);
    QPen penBlack(QBrush(Qt::black), 2.0f);
    QPen penText(this->palette().button(), 1);
    QBrush brush(QColor(40,50,60));
    QBrush brush2(QColor(80,90,100));
    QPen penLight(QColor(0, 0, 0), 2.0f);
    QPen penDark(QColor(0, 0, 0), 2.0f);
                
    /*QVector<qreal> dashes;
    dashes << 0.4 << 0.2;
    pen.setDashPattern(dashes);*/
    pen.setCapStyle(Qt::FlatCap);
    painter.setRenderHint(QPainter::Antialiasing);
  
    // draw background
    painter.setPen(penBlack);
    painter.setBrush(brush);
    painter.drawEllipse(rect);
    painter.setBrush(brush2);
    painter.drawEllipse(rect2);

    // render shadows
    painter.setPen(penDark);
    painter.drawArc(rect, 225 * 16, -180 * 16);  
    painter.setPen(penLight);
    painter.drawArc(rect, 225 * 16, 180 * 16);  

    // render shadows
    painter.setPen(penLight);
    painter.drawArc(rect2, 225 * 16, -180 * 16);  
    painter.setPen(penDark);
    painter.drawArc(rect2, 225 * 16, 180 * 16);  

    rect.adjust(thicknessH+2, thicknessH+2, -thicknessH-2, -thicknessH-2);
    
    // render value arc
    painter.setPen(pen);
    painter.drawArc(rect, 270 * 16, -angle * 16);  

    // render text
    painter.setPen(penText);
    painter.drawText(rect, Qt::AlignCenter, QString::number(this->value()));
}