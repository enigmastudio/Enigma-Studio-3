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

#ifndef GUI_OPERATOR_HPP
#define GUI_OPERATOR_HPP

#include <QtGui/QGraphicsObject>

#include "guioppage.hpp"
#include "../../eshared/eshared.hpp"

class QDomDocument;

// Encapsulates an operator on a GUI operator-
// page (eGuiOpPage).
class eGuiOperator : public QGraphicsObject
{
public:
    eGuiOperator(const QString &opType, const ePoint &pos, const eGuiOpPage *ownerPage, eInt width=-1, eBool updateLinks=eTRUE, eID opId=eNOID);
    eGuiOperator(eIOperator *op, const eGuiOpPage *ownerPage);
    virtual ~eGuiOperator();

    void                movePosition(const ePoint &dist, eBool updateLinks=eFALSE);
    eIOperator *        getOperator() const;

    void                saveToXml(QDomElement &node) const;
    void                loadFromXml(const QDomElement &node);

    eBool               isResizing() const;

public:
    virtual QRectF      boundingRect() const;
    virtual void        paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public:
    static void         setViewingOp(eGuiOperator *guiOp);
    static void         setEditingOp(eGuiOperator *guiOp);

private:
    void                _initialize();
    void                _paintStatusMarker(QPainter *painter);
    QPixmap             _createPixmap() const;
    const QString &     _cacheKey() const;

protected:
    virtual void        mouseReleaseEvent(QGraphicsSceneMouseEvent *me);
    virtual void        mousePressEvent(QGraphicsSceneMouseEvent *me);
    virtual void        mouseMoveEvent(QGraphicsSceneMouseEvent *me);

public:
    // Display sizes of an operator.
    static const eInt   WIDTH = 20;
    static const eInt   HEIGHT = 17;
    static const eInt   RESIZE_AREA = 12;

private:
    static eID          m_viewingOpId;
    static eID          m_editingOpId;

private:
    eIOperator *        m_op;
    eBool               m_resizing;
};

#endif // GUI_OPERATOR_HPP