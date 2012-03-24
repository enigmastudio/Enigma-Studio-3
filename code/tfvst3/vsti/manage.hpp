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

#ifndef MANAGE_HPP
#define MANAGE_HPP

#include <QtGui/QDialog>

#include "../../eshared/system/system.hpp"
#include "ui_manage.hpp"

class tf3Synth;

enum tfInstrumentAction
{
	TFINSTR_ACTION_NONE = 0,
	TFINSTR_ACTION_RESTORE = 1,
	TFINSTR_ACTION_SAVE = 2,
	TFINSTR_ACTION_COPY = 3
};

class tfInstrumentTreeWidgetItem : public QTreeWidgetItem
{
public:
	tfInstrumentTreeWidgetItem(QString name, eU32 instrIndex);

	eU32				getInstrIndex();
	void				setAction(tfInstrumentAction action);
	tfInstrumentAction	getAction();
	void				setCopyFrom(eU32 index);
	eU32				getCopyFrom();

private:
	eU32				m_instrIndex;
	tfInstrumentAction	m_action;
	eU32				m_copyFrom;
};

class Manage : public QDialog, protected Ui::Manage
{
    Q_OBJECT

public:
    Manage(tf3Synth* synth, QWidget *parent=0);

private Q_SLOTS:
	void	_onProgNameChanged(QString name);
    void    _onSelectionChanged(QTreeWidgetItem *item, int column);
	void    _restore(bool checked);
	void    _restoreAll(bool checked);
    void    _save(bool checked);
    void    _saveAll(bool checked);
	void	_removeAction(bool checked);
	void	_removeAllActions(bool checked);

	void	_action(tfInstrumentAction action);
	void	_actionAll(tfInstrumentAction action);

	void    _copy(bool checked);
	void    _paste(bool checked);

	void	_cancel(bool checked);
	void	_apply(bool checked);

private:
	void	_saveChanges();

	tf3Synth *	m_synth;
	eU32		m_copyFrom;
};

#endif // MANAGE_HPP