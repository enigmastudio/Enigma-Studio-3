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

#include "../vstsdk/AEffEditor.hpp"

//-----------------------------------------------------------------------------
class tfEditor : public AEffEditor
{
public:
	tfEditor (AudioEffect *effect);
	virtual ~tfEditor ();

	void                suspend ();
	void                resume ();
	bool                keysRequired ();
	void                setParameter (long index, float value);
    void                idle();

protected:
	virtual long        open (void *ptr);
	virtual void        close ();
	virtual long		getRect (ERect** rect);

	// VST 2.1
	virtual long        onKeyDown (VstKeyCode &keyCode);
	virtual long        onKeyUp (VstKeyCode &keyCode);

private:
    tf3Window *window;
	ERect rectangle;
};

