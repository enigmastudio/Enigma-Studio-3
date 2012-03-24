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

#ifndef PAGE_TREE_HPP
#define PAGE_TREE_HPP

#include <QtGui/QTreeWidget>

#include "../../eshared/eshared.hpp"

class QDomElement;

// Encapsulates tree view for pages.
class ePageTree : public QTreeWidget
{
    Q_OBJECT

public:
    ePageTree(QWidget *parent);

    QTreeWidgetItem *   addPage(const QString &name, eID pageId, QTreeWidgetItem *parent=eNULL);
    void                selectPage(eID pageId);
    void                unselectAll();

    void                saveToXml(QDomElement &parent) const;
    void                loadFromXml(const QDomElement &parent);

public:
    virtual QSize       sizeHint() const;

Q_SIGNALS:
    void                onPageAdded(eID &pageId);
    void                onPageRemoved(eID pageId);
    void                onPageSwitched(eID pageId);
    void                onPageRenamed(eID pageId, const QString &newName);

private Q_SLOTS:
    void                _onItemChanged(QTreeWidgetItem *item, int column);
    void                _onSelectionChanged();
    void                _onSortByName();
    void                _onRemovePage();
    void                _onRenamePage();
    void                _onAddPage();

private:
    void                _saveItemToXml(const QTreeWidgetItem *item, QDomElement &node) const;
    void                _loadItemFromXml(QTreeWidgetItem *parent, const QDomElement &node);
    void                _createActions();
    void                _makeConnections();
};

#endif // PAGE_TREE_HPP