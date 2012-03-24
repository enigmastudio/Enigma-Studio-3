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

#ifndef SONG_LIST_HPP
#define SONG_LIST_HPP

#include <QtGui/QListWidget>
#include <QtXml/QDomElement>

#include "../../eshared/eshared.hpp"

class eSongList : public QListWidget
{
    Q_OBJECT

public:
    eSongList(QWidget *parent=nullptr);

    void            saveToXml(QDomElement &node) const;
    void            loadFromXml(const QDomElement &node);

Q_SIGNALS:
    void            onSongAdded(eID song);
    void            onSongRemoved(eID song);
    void            onSongSwitched(eID song);

private Q_SLOTS:
    void            _onSelectionChanged();
    void            _onSortByName();
    void            _onRemoveSong();
    void            _onRenameSong();
    void            _onAddSong();

private:
    void            _createActions();
};

#endif // SONG_LIST_HPP