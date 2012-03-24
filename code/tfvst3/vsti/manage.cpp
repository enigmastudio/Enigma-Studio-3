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

#include <QtGui/QMessageBox>

#include "tfvsti.hpp"

tfInstrumentTreeWidgetItem::tfInstrumentTreeWidgetItem(QString name, eU32 instrIndex) : QTreeWidgetItem(QStringList(name)),
	m_instrIndex(instrIndex),
	m_action(TFINSTR_ACTION_NONE),
	m_copyFrom(0)
{

}

eU32 tfInstrumentTreeWidgetItem::getInstrIndex()
{
	return m_instrIndex;
}

void tfInstrumentTreeWidgetItem::setAction(tfInstrumentAction action)
{
	m_action = action;

	switch(m_action)
	{
		case TFINSTR_ACTION_NONE:		this->setText(1, ""); break;
		case TFINSTR_ACTION_RESTORE:	this->setText(1, "Restore"); break;
		case TFINSTR_ACTION_SAVE:		this->setText(1, "Save"); break;
		case TFINSTR_ACTION_COPY:		
			{
				QString text("Copy from ");
				text.append(QString::number(m_copyFrom));
				this->setText(1, text); 
				break;
			}
	}
}

tfInstrumentAction tfInstrumentTreeWidgetItem::getAction()
{
	return m_action;
}

void tfInstrumentTreeWidgetItem::setCopyFrom(eU32 index)
{
	m_copyFrom = index;
}

eU32 tfInstrumentTreeWidgetItem::getCopyFrom()
{
	return m_copyFrom;
}

Manage::Manage(tf3Synth* synth, QWidget *parent) : QDialog(parent),
	m_synth(synth)
{
    setupUi(this);

    // Hide context help button in caption bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(m_restore, SIGNAL(clicked(bool)), this, SLOT(_restore(bool)));
	connect(m_restoreAll, SIGNAL(clicked(bool)), this, SLOT(_restoreAll(bool)));
    connect(m_save, SIGNAL(clicked(bool)), this, SLOT(_save(bool)));
    connect(m_saveAll, SIGNAL(clicked(bool)), this, SLOT(_saveAll(bool)));
    connect(m_copy, SIGNAL(clicked(bool)), this, SLOT(_copy(bool)));
    connect(m_paste, SIGNAL(clicked(bool)), this, SLOT(_paste(bool)));
	connect(m_cancel, SIGNAL(clicked(bool)), this, SLOT(_cancel(bool)));
	connect(m_apply, SIGNAL(clicked(bool)), this, SLOT(_apply(bool)));
	connect(m_removeAction, SIGNAL(clicked(bool)), this, SLOT(_removeAction(bool)));
	connect(m_removeAllActions, SIGNAL(clicked(bool)), this, SLOT(_removeAllActions(bool)));
	connect(m_instrName, SIGNAL(textEdited(QString)), this, SLOT(_onProgNameChanged(QString)));
    connect(m_instrList, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(_onSelectionChanged(QTreeWidgetItem*,int)));

	eChar name[128];
	for(eU32 i=0;i<kNumPrograms;i++)
    {
        m_synth->getProgramNameIndexed(0, i, name);
		tfInstrumentTreeWidgetItem *item = new tfInstrumentTreeWidgetItem(name, i);
	
        m_instrList->addTopLevelItem(item);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
    }
}

void Manage::_onProgNameChanged(QString name)
{
	for (eS32 i=0;i < m_instrList->selectedItems().count(); i++)
    {
        m_instrList->selectedItems()[i]->setText(0, name);
    }
}

void Manage::_onSelectionChanged(QTreeWidgetItem *item, int column)
{
    m_instrName->setText(item->text(0));
}

void Manage::_restore(bool checked)
{
	_action(TFINSTR_ACTION_RESTORE);
}

void Manage::_restoreAll(bool checked)
{
	_actionAll(TFINSTR_ACTION_RESTORE);
}

void Manage::_save(bool checked)
{
	_action(TFINSTR_ACTION_SAVE);
}

void Manage::_saveAll(bool checked)
{
	_actionAll(TFINSTR_ACTION_SAVE);
}

void Manage::_removeAction(bool checked)
{
	_action(TFINSTR_ACTION_NONE);
}

void Manage::_removeAllActions(bool checked)
{
	_actionAll(TFINSTR_ACTION_NONE);
}

void Manage::_action(tfInstrumentAction action)
{
	QList<QTreeWidgetItem*> items = m_instrList->selectedItems();
	for (eS32 i=0; i<items.count(); i++)
	{
		tfInstrumentTreeWidgetItem *instrItem = (tfInstrumentTreeWidgetItem*)items[i];
		instrItem->setAction(action);
	}
}

void Manage::_actionAll(tfInstrumentAction action)
{
	for(eU32 i=0;i<kNumPrograms;i++)
	{
		tfInstrumentTreeWidgetItem *instrItem = (tfInstrumentTreeWidgetItem *)m_instrList->topLevelItem(i);
		instrItem->setAction(action);
	}
}

void Manage::_copy(bool checked)
{
	if (m_instrList->selectedItems().count() != 1)
		QMessageBox::critical(this, "TF3", "Select one instrument to copy from!");
	else
	{
		tfInstrumentTreeWidgetItem *instrItem = (tfInstrumentTreeWidgetItem*)m_instrList->selectedItems()[0];
		m_copyFrom = instrItem->getInstrIndex();
	}
}

void Manage::_paste(bool checked)
{
	QList<QTreeWidgetItem*> items = m_instrList->selectedItems();
	for (eS32 i=0; i<items.count(); i++)
	{
		tfInstrumentTreeWidgetItem *instrItem = (tfInstrumentTreeWidgetItem*)items[i];
		instrItem->setCopyFrom(m_copyFrom);
		instrItem->setAction(TFINSTR_ACTION_COPY);
	}
}

void Manage::_cancel(bool checked)
{
	accept();
}

void Manage::_apply(bool checked)
{
	_saveChanges();
	accept();
}

void Manage::_saveChanges()
{ 
    tf3SynthProgram programs[kNumPrograms];

	for(eU32 i=0;i<kNumPrograms;i++)
	{
		tfInstrumentTreeWidgetItem *item = (tfInstrumentTreeWidgetItem *)m_instrList->topLevelItem(i);
		int instrIndex = item->getInstrIndex();
        QString itemName = item->text(0);
		eChar name[24]; 
        eStrNCopy(name, itemName.toAscii().constData(), 24);
        name[23] = '\0';

		switch(item->getAction())
		{
			case TFINSTR_ACTION_NONE: break;
			case TFINSTR_ACTION_RESTORE: 
                {
                    m_synth->loadProgram(instrIndex);
                    break;
                }
			case TFINSTR_ACTION_SAVE: 
                {
                    m_synth->saveProgram(instrIndex);
                    break;
                }
            case TFINSTR_ACTION_COPY: 
                {
                    tf3SynthProgram prog;
                    m_synth->getProgramData(item->getCopyFrom(), &prog);
                    m_synth->setProgramData(instrIndex, &prog);
                    break;
                }
		}

        m_synth->getProgramData(instrIndex, &programs[i]);

		m_synth->setProgramNameIndexed(0, instrIndex, name);
	}

    for(eU32 i=0;i<kNumPrograms;i++)
    {
        m_synth->setProgramData(i, &programs[i]);
        m_synth->saveProgram(i);
    }
}
