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

#include <QtGui/QHeaderView>

#include "findopdlg.hpp"

// Allowed links says, operators of which
// category are supposed to be displayed
// and therefore are allowed to be selected.
eFindOpDlg::eFindOpDlg(const QStringList &allowedLinks, QWidget *parent) : QDialog(parent),
    m_allowedLinks(allowedLinks)
{
    setupUi(this);

    // Hide context help button in caption bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    _initOpTree();
    _initPageTree();
    _makeConnections();
}

eID eFindOpDlg::getSelectedOpId() const
{
    if (m_opTree->selectedItems().size() == 0)
    {
        return eNOID;
    }

    return m_opTree->selectedItems()[0]->data(0, Qt::UserRole).toInt();
}

void eFindOpDlg::_initOpTree()
{
    QStringList labels;

    labels << "Name" << "Type" << "Category";
    m_opTree->setHeaderLabels(labels);
}

void eFindOpDlg::_initPageTree()
{
    m_pageTree->header()->hide();
    m_pageTree->addTopLevelItem(new QTreeWidgetItem(QStringList("All stored")));
    
    // Add "a" to "z" tree items for
    // alphabetically sorting.
    m_pageTree->addTopLevelItem(new QTreeWidgetItem(QStringList("Alphabetically")));

    for (eU32 i=0; i<='Z'-'A'; i++)
    {
        m_pageTree->topLevelItem(1)->addChild(new QTreeWidgetItem(QStringList(QString('A'+i))));
    }

    // Add all pages to "by page" item.
    QTreeWidgetItem *byPageItem = new QTreeWidgetItem(QStringList("By page"));
    eASSERT(byPageItem != eNULL);

    m_pageTree->addTopLevelItem(byPageItem);

    for (eU32 i=0; i<eDemoData::getPageCount(); i++)
    {
        eOperatorPage *page = eDemoData::getPageByIndex(i);
        eASSERT(page != eNULL);

        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(QString(page->getUserName())));
        eASSERT(item != eNULL);
        item->setData(0, Qt::UserRole, page->getId());
        m_pageTree->topLevelItem(2)->addChild(item);
    }

    byPageItem->setExpanded(true);

    // Add tree item for found operators.
    m_pageTree->addTopLevelItem(new QTreeWidgetItem(QStringList("Found")));

    // Select first item in page tree.
    m_pageTree->topLevelItem(0)->setSelected(true);
    _onPageTreeSelChanged();

    // Sort operator list by first column (name).
    m_opTree->sortItems(0, Qt::AscendingOrder);
}

void eFindOpDlg::_makeConnections()
{
    connect(m_pageTree, SIGNAL(itemSelectionChanged()), this, SLOT(_onPageTreeSelChanged()));
    connect(m_opTree, SIGNAL(itemSelectionChanged()), this, SLOT(_onOpTreeSelChanged()));
    connect(m_findBtn, SIGNAL(clicked()), this, SLOT(_onFindClicked()));
    connect(m_selectBtn, SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_opTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
            this, SLOT(_onOpTreeDoubleClick(QTreeWidgetItem *)));
}

// Adds all stored operators to the list widget.
void eFindOpDlg::_addAllStored()
{
    for (eU32 i=0; i<eDemoData::getPageCount(); i++)
    {
        _addByPage(eDemoData::getPageByIndex(i));
    }
}

void eFindOpDlg::_addByAlpha(eChar alpha)
{
    for (eU32 i=0; i<eDemoData::getPageCount(); i++)
    {
        _addByPage(eDemoData::getPageByIndex(i), alpha);
    }
}

// Adds all stored operators on the page with
// the given prefix to the list widget. If prefix
// is != ' ', the user-name has to begin with
// prefix.
void eFindOpDlg::_addByPage(const eOperatorPage *page, eChar prefix)
{
    eASSERT(page != eNULL);

    for (eU32 i=0; i<page->getOperatorCount(); i++)
    {
        const eIOperator *op = page->getOperatorByIndex(i);
        eASSERT(op != eNULL);

        // User name must have been set and if
        // prefix is given, first character of
        // user-name has to be equal to prefix.
        if (op->getRealType() != "Misc : Load" && op->getUserName() != "")
        {
            if (prefix == ' ' || prefix == QString(op->getUserName()).toUpper()[0])
            {
                _addToOpTree(op);
            }
        }
    }
}

// Adds all operators to the list widget, which
// user names contain the given string.
void eFindOpDlg::_addFound(const QString &name)
{
    // No items should be added if name is an
    // empty string.
    if (name == "")
    {
        return;
    }

    // Start searching...
    for (eU32 i=0; i<eDemoData::getPageCount(); i++)
    {
        eOperatorPage *page = eDemoData::getPageByIndex(i);
        eASSERT(page != eNULL);

        for (eU32 j=0; j<page->getOperatorCount(); j++)
        {
            const eIOperator *op = page->getOperatorByIndex(j);
            eASSERT(op != eNULL);

            // Is given name a substring of
            // the user-name?
            if (op->getRealType() != "Misc : Load" && QString(op->getUserName()).contains(name))
            {
                _addToOpTree(op);
            }
        }
    }
}

void eFindOpDlg::_addToOpTree(const eIOperator *op)
{
    eASSERT(op != eNULL);

    // Operator category must fit.
    if (!m_allowedLinks.contains(QString(op->getCategory())) && !m_allowedLinks.contains("All"))
    {
        return;
    }

    QStringList texts;
    texts << QString(op->getUserName()) << QString(op->getName()) << QString(op->getCategory());

    QTreeWidgetItem *item = new QTreeWidgetItem(texts);
    eASSERT(item != eNULL);
    item->setData(0, Qt::UserRole, op->getId());
    m_opTree->addTopLevelItem(item);
}

// Add operators to list widget, depending
// on selected tree item.
void eFindOpDlg::_onPageTreeSelChanged()
{
    m_opTree->clear();

    if (m_pageTree->topLevelItem(0)->isSelected())
    {
        _addAllStored();
    }
    else if (m_pageTree->currentItem()->parent() == m_pageTree->topLevelItem(1))
    {
        _addByAlpha(m_pageTree->currentItem()->text(0).toAscii()[0]);
    }
    else if (m_pageTree->currentItem()->parent() == m_pageTree->topLevelItem(2))
    {
        const eID pageId = m_pageTree->currentItem()->data(0, Qt::UserRole).toInt();
        _addByPage(eDemoData::getPageById(pageId));
    }
    else if (m_pageTree->topLevelItem(3)->isSelected())
    {
        _addFound(m_nameEdit->text());
    }
}

// Enable select button, if selected item
// in operator tree is an operator.
void eFindOpDlg::_onOpTreeSelChanged()
{
    m_selectBtn->setEnabled(m_opTree->selectedItems().size() == 1);
}

// User can select item by double clicking.
void eFindOpDlg::_onOpTreeDoubleClick(QTreeWidgetItem *item)
{
    if (item)
    {
        accept();
    }
}

void eFindOpDlg::_onFindClicked()
{
    m_pageTree->setCurrentItem(m_pageTree->topLevelItem(3));
    _onPageTreeSelChanged();
}