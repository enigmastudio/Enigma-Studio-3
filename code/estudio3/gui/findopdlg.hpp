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

#ifndef FIND_OP_DLG_HPP
#define FIND_OP_DLG_HPP

#include <QtGui/QDialog>

#include "ui_findopdlg.hpp"
#include "../../eshared/eshared.hpp"

// Dialog used to select (find) operators when
// linking. Mainly used when editing "load"
// operator.
class eFindOpDlg : public QDialog, protected Ui::FindOpDlg
{
    Q_OBJECT

public:
    eFindOpDlg(const QStringList &allowedLinks, QWidget *parent=eNULL);

    eID         getSelectedOpId() const;

private:
    void        _initOpTree();
    void        _initPageTree();
    void        _makeConnections();

    void        _addAllStored();
    void        _addByAlpha(eChar alpha);
    void        _addByPage(const eOperatorPage *page, eChar prefix=' ');
    void        _addFound(const QString &name);

    void        _addToOpTree(const eIOperator *op);

private Q_SLOTS:
    void        _onPageTreeSelChanged();
    void        _onOpTreeSelChanged();
    void        _onOpTreeDoubleClick(QTreeWidgetItem *item);
    void        _onFindClicked();

private:
    QStringList m_allowedLinks;
};

#endif // FIND_OP_DLG_HPP