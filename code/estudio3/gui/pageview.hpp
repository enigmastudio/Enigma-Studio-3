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

#ifndef PAGE_VIEW_HPP
#define PAGE_VIEW_HPP

#include <QtCore/QTimeLine>

#include <QtGui/QGraphicsItemAnimation>
#include <QtGui/QGraphicsView>
#include <QtGui/QMenu>

#include "guioperator.hpp"
#include "addopdlg.hpp"

#include "../../eshared/eshared.hpp"

// Displays and lets the user edit the operators
// of the currently selected page.
class ePageView : public QGraphicsView
{
    Q_OBJECT

public:
    ePageView(QWidget *parent);

    void                    scrollTo(const QPoint &pos);
    void                    gotoOperator(eGuiOperator *guiOp);

Q_SIGNALS:
    void                    onOperatorShow(eID opId);
    void                    onOperatorAdded(eID opId);
    void                    onOperatorSelected(eID opId);
    void                    onOperatorRemoved(eID opId);
    void                    onPathOperatorEdit(eID opId);
    void                    onDemoOperatorEdit(eID opId);
    void                    onGotoLoadedOperator(eID opId);

private:
    void                    _createActions();
    void                    _createGotoAnimation();

    eBool                   _addOperator(const QString &opType, const ePoint &pos);

private Q_SLOTS:
    void                    _onZoomIn();
    void                    _onZoomOut();
    void                    _onShowGrid();
    void                    _onSelectAll();
    void                    _onCommentAdd();
    void                    _onOperatorAdd();
    void                    _onRemoveSelected();

    void                    _onOperatorShow();
    void                    _onOperatorRemove();
    void                    _onOperatorCut();
    void                    _onOperatorCopy();
    void                    _onOperatorPaste();
    void                    _onPathOperatorEdit();
    void                    _onDemoOperatorEdit();
    void                    _onLoadOperatorGoto();

private:
    virtual void            drawForeground(QPainter *painter, const QRectF &rect);
    virtual void            drawBackground(QPainter *painter, const QRectF &rect);

    virtual void            mouseDoubleClickEvent(QMouseEvent *de);
    virtual void            mouseMoveEvent(QMouseEvent *me);
    virtual void            mousePressEvent(QMouseEvent *me);
    virtual void            mouseReleaseEvent(QMouseEvent *me);
    virtual void            contextMenuEvent(QContextMenuEvent *ce);

private:
    QPoint                  m_lastMousePos;
    QPoint                  m_mouseDownPos;
    QPoint                  m_mouseMoveDist;
    eBool                   m_showGrid;
    eBool                   m_moving;
    QMenu                   m_opMenu;
    QMenu                   m_viewMenu;
    QMenu                   m_commentMenu;
    eAddOpDlg               m_addOpDlg;
    QTimeLine               m_gotoAnimTimeLine;
    QGraphicsItemAnimation  m_gotoAnim;
};

#endif // PAGE_VIEW_HPP