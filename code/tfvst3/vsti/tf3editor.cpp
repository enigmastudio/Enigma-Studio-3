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

#include "tfvsti.hpp"
#include <windows.h>

tfEditor::tfEditor (AudioEffect *effect) : AEffEditor (effect)
{
    QApplication::setWheelScrollLines(1);
    window = eNULL;
    effect->setEditor(this);
}

tfEditor::~tfEditor()
{
	eSAFE_DELETE(window);
}

void clientResize(HWND h_parent, int width, int height)
{
	RECT rcClient, rcWindow;
	POINT ptDiff;
	GetClientRect(h_parent, &rcClient);
	GetWindowRect(h_parent, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	MoveWindow(h_parent, rcWindow.left, rcWindow.top, width + ptDiff.x, height + ptDiff.y, TRUE);
}

long tfEditor::open (void *ptr)
{
	AEffEditor::open (ptr);

    eU32 width = 1040;
    eU32 height = 665;
       
	window = new tf3Window(effect, static_cast<HWND>(ptr));
	window->move( 0, 0 );
	window->setMinimumSize(width, height);
	window->adjustSize();
	rectangle.top = 0;
	rectangle.left = 0;
	rectangle.bottom = window->height();
	rectangle.right = window->width();
	window->setMinimumSize(window->size());
	window->show();

	//clientResize(static_cast<HWND>(ptr), window->width(), window->height());
 
	return true;
}

bool tfEditor::keysRequired()
{
    return true;
}

long tfEditor::onKeyDown (VstKeyCode &keyCode)
{
	return -1;
}

long tfEditor::onKeyUp(VstKeyCode &keyCode)
{
	return -1; 
}

long tfEditor::getRect(ERect** rect)
{
	*rect = &rectangle;
	return 1;
}

void tfEditor::close()
{
	eSAFE_DELETE(window);
}

void tfEditor::idle()
{
    QApplication::processEvents();
}

void tfEditor::setParameter(long index, float value)
{
    postUpdate();
	// call this to be sure that the graphic will be updated
}

