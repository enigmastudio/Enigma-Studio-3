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

#ifndef INSTRUMENT_LIST_HPP
#define INSTRUMENT_LIST_HPP

#include <QtGui/QTableWidget>
#include <QtXml/QDomElement>

class eInstrumentList : public QTableWidget
{
    Q_OBJECT

public:
    eInstrumentList(QWidget *parent);

    void    initRows();
    void    saveToXml(QDomElement &node) const;
    void    loadFromXml(const QDomElement &node);
};

#endif // INSTRUMENT_LIST_HPP