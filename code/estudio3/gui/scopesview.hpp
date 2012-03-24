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

#ifndef SCOPES_VIEW_HPP
#define SCOPES_VIEW_HPP

#include <QtGui/QGraphicsView>

#include "../../eshared/eshared.hpp"

class eScopesScene : public QGraphicsScene
{
public:
    eScopesScene(class eTrackerSeqView *seqView, class eScopesView *parent);

	eU32                getRowTextWidth() const;

protected:
    virtual void        drawBackground(QPainter *painter, const QRectF &rect);
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);

public:
    static const eU32   TRACK_WIDTH = 32;
	static const eU32   FIRST_COL_DIST = 2;

private:
    eScopesView *		m_scopesView;
	eTrackerSeqView *	m_seqView;
	eU32                m_rowTextWidth;
    eU32                m_headerHeight;
};

class eScopesView : public QGraphicsView
{
    Q_OBJECT

public:
    eScopesView(QWidget *parent=eNULL);

	void	setTrackerSeqView(eTrackerSeqView *seqView);

protected:
	void	mousePressEvent(QMouseEvent *me);
};

#endif // SCOPES_VIEW_HPP