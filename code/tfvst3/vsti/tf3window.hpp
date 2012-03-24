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

#include "qwinwidget.h"
#include "ui_tf3window.hpp"
#include "tfdial.hpp"

#include "../../eshared/system/system.hpp"
#include "../vstsdk/AudioEffect.hpp"

class tf3Synth;

class tf3Window : public QWinWidget, protected Ui::Dialog
{
    Q_OBJECT

public:
    tf3Window(AudioEffect *fx, HWND handle);
	~tf3Window();

    void    initParameters();
    void    updateOscView();
    void    setParameter(eU32 index, eF32 value);

private:
    eU32    toIndex(eF32 value, eU32 min, eU32 max);
    eF32    fromIndex(eU32 value, eU32 min, eU32 max);

private Q_SLOTS:
    void    onChanged(int value);
    void    onChanged(double value);
	void	onClicked(bool checked);
    void    progChanged(int value);
    void    progRestore(bool checked);
    void    progSave(bool checked);
	void	manage(bool checked);

	void	presetSine(bool checked);
	void	presetTriangle(bool checked);
	void	presetSquare(bool checked);
	void	presetSawUp(bool checked);
	void	presetSawDown(bool checked);

	void	updateAfterPreset();
	void	setPresetValue(eU32 index, eF32 value);
    
private:
	void	_createIcons();
	void	_freeIcons();
    void    _updateInstrSelection(bool updateText);

    AudioEffect *   effect;
	tf3Synth *		m_synth;
    HWND            m_parent;

	QPixmap *		m_pixSine;
	QPixmap	*		m_pixSawUp;
	QPixmap	*		m_pixSawDown;
	QPixmap	*		m_pixPulse;
	QPixmap	*		m_pixNoise;
	QIcon *			m_iconSine;
	QIcon *			m_iconSawUp;
	QIcon *			m_iconSawDown;
	QIcon *			m_iconPulse;
	QIcon *			m_iconNoise;
};