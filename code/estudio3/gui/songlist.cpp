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

#include <QtGui/QMenu>

#include "songlist.hpp"

eSongList::eSongList(QWidget *parent) : QListWidget(parent)
{
    _createActions();

    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(_onSelectionChanged()));
}

void eSongList::saveToXml(QDomElement &node) const
{
    QDomDocument &xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement sl = xml.createElement("songlist");
    node.appendChild(sl);

    for (eInt i=0; i<count(); i++)
    {
        const eID songId = item(i)->data(Qt::UserRole).toInt();
        const tfSong *song = eDemoData::getSongById(songId);
        eASSERT(song != eNULL);

        QDomElement songEl = xml.createElement("song");
        songEl.setAttribute("name", song->getUserName());
        songEl.setAttribute("id", song->getId());
        sl.appendChild(songEl);
    }
}

void eSongList::loadFromXml(const QDomElement &node)
{
    QDomElement xmlItem = node.firstChildElement("song");

    while (!xmlItem.isNull())
    {
        tfSong *song = eDemoData::newSong();
        song->setUserName(xmlItem.attribute("name").toAscii().constData());
        eASSERT(song != eNULL);

        QListWidgetItem *item = new QListWidgetItem(this);
        eASSERT(item != eNULL);
        item->setText(song->getUserName());
        item->setData(Qt::UserRole, QVariant(song->getId()));
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        Q_EMIT onSongAdded(song->getId());

        xmlItem = xmlItem.nextSiblingElement("song");
    }
}

void eSongList::_onSelectionChanged()
{
    if (selectedItems().size() > 0)
    {
        QListWidgetItem *item = selectedItems().at(0);
        eASSERT(item != eNULL);
        const eID songId = item->data(Qt::UserRole).toInt();

        Q_EMIT onSongSwitched(songId);
    }
}

void eSongList::_onSortByName()
{
    sortItems(Qt::AscendingOrder);
}

void eSongList::_onRemoveSong()
{
    for (eInt i=selectedItems().size()-1; i>=0; i--)
    {
        QListWidgetItem *item = selectedItems().at(i);
        eASSERT(item != eNULL);
        tfSong *song = eDemoData::getSongById(item->data(Qt::UserRole).toInt());
        eASSERT(song != eNULL);

        Q_EMIT onSongRemoved(song->getId());
        eDemoData::removeSong(song->getId());
        eSAFE_DELETE(item);
    }
}

void eSongList::_onRenameSong()
{
    if (selectedItems().size() == 1)
    {
        editItem(selectedItems().first());
    }
}

void eSongList::_onAddSong()
{
    tfSong *song = eDemoData::newSong();
    eASSERT(song != eNULL);

    QListWidgetItem *item = new QListWidgetItem(this);
    eASSERT(item != eNULL);
    item->setText(song->getUserName());
    item->setData(Qt::UserRole, QVariant(song->getId()));
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    Q_EMIT onSongAdded(song->getId());

    item->setSelected(true);
}

void eSongList::_createActions()
{
    QAction *act = new QAction("Add new", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("a"));
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onAddSong()));
    addAction(act);

    act = new QAction("Remove", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence::Delete);
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onRemoveSong()));
    addAction(act);

    act = new QAction("Rename", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("r"));
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onRenameSong()));
    addAction(act);

    act = new QAction(this);
    eASSERT(act != eNULL);
    act->setSeparator(true);
    addAction(act);

    act = new QAction("Sort by name", this);
    eASSERT(act != eNULL);
    connect(act, SIGNAL(triggered()), this, SLOT(_onSortByName()));
    addAction(act);
}