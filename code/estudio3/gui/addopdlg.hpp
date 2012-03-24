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

#ifndef ADD_OP_DLG_HPP
#define ADD_OP_DLG_HPP

#include <QtCore/QList>
#include <QtCore/QMap>

#include <QtGui/QVBoxLayout>
#include <QtGui/QDialog>
#include <QtGui/QLabel>

#include "../../eshared/eshared.hpp"

class eButtonLabel : public QLabel
{
    Q_OBJECT

public:
    eButtonLabel(const QString &text, const QColor &hoverCol, QWidget *parent);

Q_SIGNALS:
    void                    clicked();

private:
    virtual void            mouseReleaseEvent(QMouseEvent *me);
    virtual void            enterEvent(QEvent *ev);
    virtual void            leaveEvent(QEvent *ev);

private:
    static QLabel *         m_lastLbl;

private:
    QColor                  m_hoverCol;
};

// Dialog used to add new operators.
class eAddOpDlg : public QDialog
{
    Q_OBJECT

public:
    eAddOpDlg(QWidget *parent=eNULL);

    void                    setFilterOp(const eIOperator *filterOp);
    const QString &         getChosenOpType() const;

private:
    virtual void            keyPressEvent(QKeyEvent *ke);
    virtual void            showEvent(QShowEvent *se);

private Q_SLOTS:
    void                    _onLabelBtnClick(const QString &opType);

private:
    static eBool            _sortMetaInfosByName(const eIOperator::MetaInfos *mi0, const eIOperator::MetaInfos *mi1);

private:
    typedef QList<QFrame *> QFrameList;
    typedef QList<const eIOperator::MetaInfos *> eOpMetaInfosList;

    struct Column
    {
        eU32                index;
        QFrame *            frame;
        QVBoxLayout *       vbl;
        QLabel *            catLabel;
        QFrameList          btnFrames;
        eOpMetaInfosList    opMis;
    };

private:
    typedef QMap<Qt::Key, QMap<Qt::Key, QString>> KeyOpTypeMap;
    typedef QMap<QString, Column> CategoryColumnMap;

private:
    QString                 m_chosenOpType;
    KeyOpTypeMap            m_shortcutMap;
    CategoryColumnMap       m_catColMap;
    Qt::Key                 m_catKey;
};

#endif // ADD_OP_DLG_HPP