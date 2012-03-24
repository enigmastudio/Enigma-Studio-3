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

#ifndef GUI_OP_PAGE_HPP
#define GUI_OP_PAGE_HPP

#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

#include "../../eshared/eshared.hpp"

class eGuiOperator;
class QDomElement;

class eGuiOpPage : public QGraphicsScene
{
public:
    eGuiOpPage(const QString &name="", QGraphicsView *parent=eNULL);
    eGuiOpPage(eOperatorPage *opPage, QGraphicsView *parent=eNULL);
    virtual ~eGuiOpPage();

    eBool               areSelectedOpsMovable(const ePoint &dist) const;

    eOperatorPage *     getPage() const;
    ePoint              getInsertAt() const;
    QPoint              getViewPosition() const;

    void                advanceInsertAt();
    eGuiOperator *      getGuiOperator(eID opId) const;

    void                saveToXml(QDomElement &node) const;
    void                loadFromXml(const QDomElement &node);

    void                addComment(const QString &text, const QPointF &pos);

private:
    void                _initialize();
    void                _updateViewingPos();
    void                _drawLinkLines(QPainter *painter);
    void                _drawInsertAtMarker(QPainter *painter);
    QRect               _getOperatorRect(const eIOperator *op) const;
    QPoint              _getNearestIntersectionPoint(const QRect &r, const QLine &line) const;

protected:
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);
    virtual void        drawBackground(QPainter *painter, const QRectF &rect);
    virtual void        mousePressEvent(QGraphicsSceneMouseEvent *me);
    virtual void        mouseReleaseEvent(QGraphicsSceneMouseEvent *me);

private:
    static const eInt   ARROW_SIZE = 5;

private:
    eOperatorPage *     m_opPage;
    ePoint              m_insertAt;
    QPoint              m_viewPos;

    Qt::MouseButtons    m_pressButtons;
    QPointF             m_pressPos;
};

typedef QMap<eID, eGuiOpPage *> eGuiOpPagePtrIdMap;

// A comment item on an operator page.
class eCommentItem : public QGraphicsTextItem
{
public:
    eCommentItem(const QString &text, const QPointF &pos, QGraphicsScene *parentScene=eNULL);

    void                paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public:
    virtual void        contextMenuEvent(QGraphicsSceneContextMenuEvent *scme);
};

#endif // GUI_OP_PAGE_HPP