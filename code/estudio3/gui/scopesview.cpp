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
#include <QtGui/QMouseEvent>

#include "scopesview.hpp"
#include "trackerseqview.hpp"

eScopesScene::eScopesScene(eTrackerSeqView *seqView, eScopesView *parent) : QGraphicsScene(parent),
	m_seqView(seqView)
{
	setItemIndexMethod(NoIndex);
}

void eScopesScene::drawBackground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawBackground(painter, rect);

	QFont fwFont = painter->font();
    const eU32 fwHeight = QFontMetrics(fwFont).height();
    const eU32 rowTextWidth = QFontMetrics(fwFont).width("0000");
	const eU32 startX = rowTextWidth+FIRST_COL_DIST;

	tfSong *song = m_seqView->getSong();
	if (!song)
		return;

	setSceneRect(0, 0, TRACK_WIDTH*tfSong::MAX_SEQ_TRACKS+rowTextWidth+FIRST_COL_DIST, (eF32)2.0f*(eF32)fwHeight);

	painter->setPen(QColor(80,90,100));
    painter->drawLine(sceneRect().left()+rowTextWidth+FIRST_COL_DIST, fwHeight, sceneRect().right(), fwHeight);

	painter->setFont(fwFont);
    painter->setPen(Qt::white);

	for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
    {
        if (song->getMuted(i))
        {
            painter->setPen(Qt::gray);
            painter->drawText(QRectF(i*TRACK_WIDTH+startX, fwHeight, TRACK_WIDTH, fwHeight), Qt::AlignCenter, "Muted");
        }
        else
        {
            eF32 peak = eDemo::getSynth().getPeakTrack(i) * 4;
            eU32 ipeak = eMin<eU32>(peak * (TRACK_WIDTH-4), TRACK_WIDTH-4);

            painter->setPen(Qt::white);
            painter->fillRect(i*TRACK_WIDTH+startX+2, fwHeight+2, ipeak, fwHeight-4, QBrush(Qt::white, Qt::SolidPattern));
        }

        painter->drawText(QRectF(i*TRACK_WIDTH+startX, 0, TRACK_WIDTH, fwHeight), Qt::AlignCenter, QString::number(i).rightJustified(2, '0'));

        painter->setPen(QColor(80,90,100));

        if (i > 0)
        {
            painter->setPen(QPen(QBrush(QColor(80,90,100)), 1, Qt::DashLine));
        }

        painter->drawLine(i*TRACK_WIDTH+rowTextWidth+2, sceneRect().top(), i*TRACK_WIDTH+rowTextWidth+2, sceneRect().bottom());
    }
}

void eScopesScene::drawForeground(QPainter *painter, const QRectF &rect)
{
	QGraphicsScene::drawForeground(painter, rect);

	QFont fwFont = painter->font();
    m_rowTextWidth = QFontMetrics(fwFont).width("0000");
    const eU32 fwHeight = QFontMetrics(fwFont).height();
    m_headerHeight = 2+2*fwHeight;

	tfSong *song = m_seqView->getSong();
	if (!song)
		return;

    for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
    {
        if (song->getMuted(i))
        {
            painter->setPen(Qt::black);
            painter->fillRect(
                i*TRACK_WIDTH+m_rowTextWidth+2, 
                sceneRect().top()+m_headerHeight, 
                TRACK_WIDTH, 
                sceneRect().bottom()-m_headerHeight, 
                Qt::Dense4Pattern);
        }
    }

	
}

eU32 eScopesScene::getRowTextWidth() const
{
    return m_rowTextWidth;
}

eScopesView::eScopesView(QWidget *parent) : QGraphicsView(parent)
{
}

void eScopesView::setTrackerSeqView(eTrackerSeqView *seqView)
{
	setScene(new eScopesScene(seqView, this));
	scene()->invalidate();
}

void eScopesView::mousePressEvent(QMouseEvent *me)
{
    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ClosedHandCursor);
        me->accept();
    }
    else if (me->button() == Qt::RightButton)
    {
        mouseMoveEvent(me);
        me->accept();

		// Emit operator-selected signal, if there's
        // an operator under mouse.
        if (scene())
        {
            eScopesScene *sscene = (eScopesScene*)scene();

			tfSong *song = eDemo::getSynth().getSong();
			if (song)
			{
				eU32 mx = me->pos().x() - sscene->getRowTextWidth();
				eU32 my = me->pos().y();

				eU32 track = mx / eScopesScene::TRACK_WIDTH;

				if (track >= 0 && track < tfSong::MAX_SEQ_TRACKS)
				{
					eU32 muted_count = 0;

					for(eU32 i=0;i<tfSong::MAX_SEQ_TRACKS;i++)
					{
						if (song->getMuted(i))
							muted_count++;
					}

					if (muted_count == tfSong::MAX_SEQ_TRACKS - 1 &&
						!song->getMuted(track))
					{
						for(eU32 i=0;i<tfSong::MAX_SEQ_TRACKS;i++)
						{
							song->setMuted(i, eFALSE);
						}
					}
					else
					{
						for(eU32 i=0;i<tfSong::MAX_SEQ_TRACKS;i++)
						{
							if (i == track)
								song->setMuted(i, eFALSE);
							else
								song->setMuted(i, eTRUE);
						}
					}

					scene()->invalidate();
				}
			}
        }
    }
    else
    {
        QGraphicsView::mousePressEvent(me);

        // Emit operator-selected signal, if there's
        // an operator under mouse.
        if (scene())
        {
            eScopesScene *sscene = (eScopesScene*)scene();

            eU32 mx = me->pos().x() - sscene->getRowTextWidth();
            eU32 my = me->pos().y();

			tfSong *song = eDemo::getSynth().getSong();
			if (song)
			{
                eU32 track = mx / eTrackerSeqScene::TRACK_WIDTH;

                if (track >= 0 && track < tfSong::MAX_SEQ_TRACKS)
                {
                    song->setMuted(track, !song->getMuted(track));
                    scene()->invalidate();
                }
            }
        }
    }
}