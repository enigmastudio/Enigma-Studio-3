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

#include <QtCore/QSignalMapper>

#include <QtGui/QDesktopWidget>
#include <QtGui/QApplication>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtGui/QShortcut>

#include "addopdlg.hpp"

QLabel * eButtonLabel::m_lastLbl = eNULL;

eButtonLabel::eButtonLabel(const QString &text, const QColor &hoverCol, QWidget *parent) : QLabel(text, parent),
    m_hoverCol(hoverCol)
{
    eASSERT(parent != eNULL);
}

void eButtonLabel::mouseReleaseEvent(QMouseEvent *me)
{
    QLabel::mouseReleaseEvent(me);

    if (underMouse())
    {
        setStyleSheet("");
        Q_EMIT clicked();
    }
}

// Sets parent's frame color instead of label's color,
// because eventually existing shortcut label's background
// also has to change.
void eButtonLabel::enterEvent(QEvent *ev)
{
    QLabel::enterEvent(ev);

    if (m_lastLbl)
    {
        parentWidget()->setStyleSheet("");
    }

    parentWidget()->setStyleSheet(QString("background-color: ")+m_hoverCol.name());
    m_lastLbl = this;
}

void eButtonLabel::leaveEvent(QEvent *ev)
{
    QLabel::leaveEvent(ev);
    parentWidget()->setStyleSheet("");
}

eAddOpDlg::eAddOpDlg(QWidget *parent) : QDialog(parent)
{
    // Hide context help button in caption bar.
    setWindowFlags(Qt::CustomizeWindowHint | Qt::Dialog);
    setWindowTitle(tr("Add operator"));
    setContentsMargins(0, 0, 0, 0);

    // Horizontal layout for category columns.
    QHBoxLayout *hbl = new QHBoxLayout(this);
    eASSERT(hbl != eNULL);
    hbl->setMargin(0);
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(hbl);

    // Create signal mappers for button labels.
    QSignalMapper *opSigMap = new QSignalMapper(this);
    eASSERT(opSigMap != eNULL);
    connect(opSigMap, SIGNAL(mapped(const QString &)), this, SLOT(_onLabelBtnClick(const QString &)));

    // Create all operators and sort them.
    eOpMetaInfosList metaInfos;

    for (eU32 i=0; i<eOpMetaInfoManager::getInfosCount(); i++)
    {
        metaInfos.append(&eOpMetaInfoManager::getInfos(i));
    }

    qSort(metaInfos.begin(), metaInfos.end(), _sortMetaInfosByName);

    // Create interface widgets.
    for (eU32 i=0, catIndex=0; i<(eU32)metaInfos.size(); i++)
    {
        // Check if the given column (category) already exists.
        const eIOperator::MetaInfos &mi = *metaInfos[i];

        if (!m_catColMap.contains(QString(mi.category)))
        {
            QFrame *frame = new QFrame(this);
            eASSERT(frame != eNULL);
            hbl->addWidget(frame);
            frame->setContentsMargins(0, 0, 0, 0);

            QVBoxLayout *vbl = new QVBoxLayout(frame);
            eASSERT(vbl != eNULL);
            vbl->setMargin(0);
            vbl->setContentsMargins(0, 0, 0, 0);

            const QString caption = QString("%1\t%2").arg(QString(mi.category)).arg(++catIndex);

            QLabel *catLabel = new QLabel(caption, frame);
            eASSERT(catLabel != eNULL);
            catLabel->setStyleSheet(QString("background-color: ")+QColor(mi.color.toArgb()).name());
            catLabel->setContentsMargins(4, 2, 4, 2);
            vbl->addWidget(catLabel);
            vbl->addStretch(1);

            // Insert column into map.
            Column col;

            col.index    = catIndex;
            col.catLabel = catLabel;
            col.vbl      = vbl;
            col.frame    = frame;

            m_catColMap.insert(QString(mi.category), col);
        }

        // Create button label for current operator.
        Column &col = m_catColMap[QString(mi.category)];

        QFrame *subFrame = new QFrame(col.frame);
        eASSERT(subFrame != eNULL);
        QHBoxLayout *subHbl = new QHBoxLayout(subFrame);
        eASSERT(subHbl != eNULL);
        subHbl->setContentsMargins(0, 0, 0, 0);
        subHbl->setSpacing(0);

        const QColor hoverCol = QColor::fromRgba(mi.color.toArgb());

        eButtonLabel *lblOp = new eButtonLabel(QString(mi.name), hoverCol, subFrame);
        eASSERT(lblOp != eNULL);
        lblOp->setContentsMargins(4, 0, 4, 0);
        lblOp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        subHbl->addWidget(lblOp);
        
        col.vbl->insertWidget(col.vbl->count()-1, subFrame);
        col.btnFrames.append(subFrame);
        col.opMis.append(&mi);

        // Connect slot to signal using signal mapper.
        connect(lblOp, SIGNAL(clicked()), opSigMap, SLOT(map()));
        opSigMap->setMapping(lblOp, QString(mi.type));

        // Set special text if shortcut is defined and
        // add map entry for shortcut.
        if (mi.shortcut != ' ')
        {
            const QString shortcut = QString(mi.shortcut).toUpper();

            eButtonLabel *lblShortcut = new eButtonLabel(shortcut, hoverCol, subFrame);
            eASSERT(lblShortcut != eNULL);
            lblShortcut->setContentsMargins(4, 0, 4, 0);

            subHbl->addWidget(lblShortcut);

            const Qt::Key catKey = (Qt::Key)(Qt::Key_0+col.index);
            const Qt::Key opKey = (Qt::Key)QString(mi.shortcut).toUpper()[0].toAscii();

            m_shortcutMap[catKey].insert(opKey, QString(mi.type));
        }
        else
        {
            subHbl->addSpacing(10);
        }
    }
}

void eAddOpDlg::setFilterOp(const eIOperator *filterOp)
{
    Q_FOREACH (const Column &col, m_catColMap)
    {
        col.frame->setVisible(false);

        eASSERT(col.opMis.size() == col.btnFrames.size());

        for (eInt i=0; i<col.btnFrames.size(); i++)
        {
            // Check if input of filtering operator is valid.
            eBool catCondOk = eTRUE;

            if (filterOp)
            {
                catCondOk = col.opMis[i]->allowedInput.isAllowed(filterOp);
            }

            // Check if input count is valid.
            const eU32 minInput = col.opMis[i]->minInput;
            const eU32 maxInput = col.opMis[i]->maxInput;

            const eBool inCountCondOk = ((filterOp == eNULL && minInput == 0) || (filterOp && maxInput > 0));

            // Both conditions have to be true.
            if (catCondOk && inCountCondOk)
            {
                col.frame->setVisible(true);
                col.btnFrames[i]->setVisible(true);
            }
            else
            {
                col.btnFrames[i]->setVisible(false);
            }
        }
    }
}

const QString & eAddOpDlg::getChosenOpType() const
{
    return m_chosenOpType;
}

void eAddOpDlg::keyPressEvent(QKeyEvent *ke)
{
    QDialog::keyPressEvent(ke);

    const Qt::Key key = (Qt::Key)ke->key();

    if (m_shortcutMap.contains(key))
    {
        m_catKey = key;

        const eInt hotCatIndex = key-Qt::Key_1+1;

        Q_FOREACH (const Column &col, m_catColMap)
        {
            QFont font = col.catLabel->font();
            font.setBold(col.index == hotCatIndex ? true : false);
            col.catLabel->setFont(font);
        }
    }
    else
    {
        if (m_shortcutMap[m_catKey].contains(key))
        {
            m_chosenOpType = m_shortcutMap[m_catKey][key];
            accept();
        }
    }
}

void eAddOpDlg::showEvent(QShowEvent *se)
{
    QDialog::showEvent(se);

    // If dialog would be partially invisible move it.
    const QPoint screenMax = QApplication::desktop()->availableGeometry(this).bottomRight();
    const QSize dlgSize = frameGeometry().size();
    
    QPoint dlgPos = pos();

    if (dlgPos.y()+dlgSize.height() >= screenMax.y())
    {
        dlgPos.setY(screenMax.y()-dlgSize.height());
    }

    if (dlgPos.x()+dlgSize.width() >= screenMax.x())
    {
        dlgPos.setX(screenMax.x()-dlgSize.width());
    }

    move(dlgPos);
}

void eAddOpDlg::_onLabelBtnClick(const QString &opType)
{
    m_chosenOpType = opType.toAscii().constData();
    accept();
}

eBool eAddOpDlg::_sortMetaInfosByName(const eIOperator::MetaInfos *mi0, const eIOperator::MetaInfos *mi1)
{
    eASSERT(mi0 != eNULL);
    eASSERT(mi1 != eNULL);

    const QString s0(mi0->name);
    const QString s1(mi1->name);

    return (s0.compare(s1, Qt::CaseInsensitive) < 0);
}