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

#include <QtXml/QDomDocument>

#include <QtGui/QHeaderView>
#include <QtGui/QMenu>

#include "pagetree.hpp"

ePageTree::ePageTree(QWidget *parent) : QTreeWidget(parent)
{
    header()->hide();

    _createActions();
    _makeConnections();
}

// Adds a new page with the given ID and name to
// the tree-widget. If parent is null, the currently
// selected item is used as parent (if no item is
// selected, new item is added as a top level item).
QTreeWidgetItem * ePageTree::addPage(const QString &name, eID pageId, QTreeWidgetItem *parent)
{
    if (parent == eNULL && selectedItems().size() == 1)
    {
        parent = selectedItems().first();
    }

    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    eASSERT(item != eNULL);
    item->setText(0, name);
    item->setData(0, Qt::UserRole, pageId);
    item->setFlags(item->flags() | Qt::ItemIsEditable);

    addTopLevelItem(item);

    if (parent)
    {
        parent->setExpanded(true);
    }

    return item;
}

// Selects page with the given ID.
void ePageTree::selectPage(eID pageId)
{
    eASSERT(pageId != eNOID);

    QTreeWidgetItemIterator iter(this);

    while (*iter)
    {
        const eID itemData = (*iter)->data(0, Qt::UserRole).toInt();
        eASSERT(pageId != eNOID);

        // Does item data matches with given ID?
        if (itemData == pageId)
        {
            (*iter)->setSelected(true);
            return;
        }

        iter++;
    }
}

void ePageTree::unselectAll()
{
    QTreeWidgetItemIterator iter(this);

    selectAll();

    while (*iter)
    {
        (*iter)->setSelected(false);
        iter++;
    }
}

void ePageTree::saveToXml(QDomElement &node) const
{
    QDomDocument &xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement tv = xml.createElement("pagetree");
    node.appendChild(tv);

    for (eInt i=0; i<topLevelItemCount(); i++)
    {
        _saveItemToXml(topLevelItem(i), tv);
    }
}

void ePageTree::loadFromXml(const QDomElement &node)
{
    _loadItemFromXml(invisibleRootItem(), node.firstChildElement("pagetree"));
}

QSize ePageTree::sizeHint() const
{
    return QSize(150, 0);
}

void ePageTree::_onItemChanged(QTreeWidgetItem *item, int column)
{
    eASSERT(item != eNULL);

    // Make sure item was renamed.
    if (selectedItems().size() == 1 && selectedItems().first() == item)
    {
        const eID pageId = item->data(0, Qt::UserRole).toInt();
        Q_EMIT onPageRenamed(pageId, item->text(0));
    }
}

void ePageTree::_onSelectionChanged()
{
    eID pageId = eNOID;

    if (selectedItems().size() > 0)
    {
        pageId = selectedItems().first()->data(0, Qt::UserRole).toInt();
    }

    Q_EMIT onPageSwitched(pageId);
}

void ePageTree::_onSortByName()
{
    sortItems(0, Qt::AscendingOrder);
}

// Removes the currently selected page and all
// pages, which are children of the selected page.
void ePageTree::_onRemovePage()
{
    QTreeWidgetItemIterator iter(this);

    while (*iter)
    {
        if ((*iter)->isSelected())
        {
            const eID pageId = (*iter)->data(0, Qt::UserRole).toInt();
            eASSERT(pageId != eNOID);

            Q_EMIT onPageRemoved(pageId);
            delete *iter; // Can't use eSAFE_DELETE here.
        }

        iter++;
    }
}

void ePageTree::_onRenamePage()
{
    if (selectedItems().size() == 1)
    {
        editItem(selectedItems().first(), 0);
    }
}

void ePageTree::_onAddPage()
{
    // Call signal to get ID (item data) for new page.
    eID pageId = eNOID;

    Q_EMIT onPageAdded(pageId);
    eASSERT(pageId != eNOID);

    // Add a new item to the tree-widget.
    // After adding set its item data.
    QTreeWidgetItem *newItem = addPage("New page", pageId);
    eASSERT(newItem != eNULL);
    newItem->setData(0, Qt::UserRole, pageId);
}

// Saves given item with all child items
// to XML stream.
void ePageTree::_saveItemToXml(const QTreeWidgetItem *item, QDomElement &node) const
{
    eASSERT(item != eNULL);

    QDomDocument &xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement pageEl = xml.createElement("item");
    pageEl.setAttribute("name", item->text(0));
    pageEl.setAttribute("pageid", item->data(0, Qt::UserRole).toInt());
    pageEl.setAttribute("expanded", item->isExpanded());
    node.appendChild(pageEl);

    for (eInt i=0; i<item->childCount(); i++)
    {
        _saveItemToXml(item->child(i), pageEl);
    }
}

void ePageTree::_loadItemFromXml(QTreeWidgetItem *parent, const QDomElement &node)
{
    QDomElement xmlItem = node.firstChildElement("item");

    while (!xmlItem.isNull())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        eASSERT(item != eNULL);
        item->setText(0, xmlItem.attribute("name"));
        item->setExpanded(xmlItem.attribute("expanded").toInt());
        item->setData(0, Qt::UserRole, xmlItem.attribute("pageid").toInt());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        _loadItemFromXml(item, xmlItem);
        xmlItem = xmlItem.nextSiblingElement("item");
    }
}

void ePageTree::_createActions()
{
    QAction *act = new QAction("Add new", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("a"));
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onAddPage()));
    addAction(act);

    act = new QAction("Remove", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence::Delete);
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onRemovePage()));
    addAction(act);

    act = new QAction("Rename", this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("r"));
    act->setShortcutContext(Qt::WidgetShortcut);
    connect(act, SIGNAL(triggered()), this, SLOT(_onRenamePage()));
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

void ePageTree::_makeConnections()
{
    connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(_onSelectionChanged()));
    connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)),
            this, SLOT(_onItemChanged(QTreeWidgetItem *, int)));
}