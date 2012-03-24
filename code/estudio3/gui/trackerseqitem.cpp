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

#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QPixmapCache>
#include <QtGui/QPainter>

#include "trackerseqitem.hpp"
#include "trackerseqview.hpp"

#include "../../eshared/eshared.hpp"

eTrackerSeqItem::eTrackerSeqItem(tfSong::PatternInstance *pi, eTrackerSeqView *view) :
    m_pi(pi),
    m_trackerSeqView(view),
    m_fontMetrics(QFontMetrics(view->font()))
{
    init();
}

// Clones the given item. The whole pattern instance is exactly
// copied, placed below the original, if no space is left, other instances will be moved down
eTrackerSeqItem::eTrackerSeqItem(eTrackerSeqItem &tsi, eBool clonePattern) :
    m_trackerSeqView(tsi.m_trackerSeqView),
    m_fontMetrics(tsi.m_fontMetrics),
    m_pi(eNULL)
{
    tfSong::PatternInstance &origInstance = *tsi.getPatternInstance();
    tfSong::Pattern &origPattern = *origInstance.pattern;
    tfSong &song = origInstance.song;

    eU32 rowCount = origPattern.getRowCount();
    eU32 trackCount = origPattern.getTrackCount();
    eU32 seqTrack = origInstance.seqTrack;
    eU32 newRowOffset = origInstance.rowOffset + rowCount;
    
    if (!song.isTrackFreeAt(seqTrack, newRowOffset, rowCount))
    {
        eU32 count = song.getPatternInstanceCount();

        for(eU32 i=0;i<count;i++)
        {
            tfSong::PatternInstance &pi = song.getPatternInstance(i);
            if (pi.rowOffset >= newRowOffset)
                pi.rowOffset += rowCount;
        }
    }

    if (clonePattern)
    {
        tfSong::Pattern &pattern = song.newPattern(rowCount, trackCount);

        tfSong::Pattern *copyPattern = origPattern.copy();
        pattern.paste(copyPattern);
        eSAFE_DELETE(copyPattern);

        m_pi = &song.newPatternInstance(pattern, newRowOffset, seqTrack);
    }
    else
    {
        m_pi = &song.newPatternInstance(origPattern, newRowOffset, seqTrack);
    }

    init();
}

eTrackerSeqItem::~eTrackerSeqItem()
{
    // Remove pattern instance from song when item is deleted.
    for (eU32 i=0; i<m_pi->song.getPatternInstanceCount(); i++)
    {
        if (&m_pi->song.getPatternInstance(i) == m_pi)
        {
            m_pi->song.removePatternInstance(i);
            break;
        }
    }
}

void eTrackerSeqItem::saveToXml(QDomElement &node)
{
    QDomDocument xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement piEl = xml.createElement("patterninstance");
    piEl.setAttribute("rowoffset", m_pi->rowOffset);
    piEl.setAttribute("track", m_pi->seqTrack);
    piEl.setAttribute("patternid", m_pi->pattern->getPatternNumber());
    node.appendChild(piEl);
}

void eTrackerSeqItem::loadFromXml(QDomElement &node)
{
}

tfSong::PatternInstance * eTrackerSeqItem::getPatternInstance()
{
    return m_pi;
}

const tfSong::PatternInstance * eTrackerSeqItem::getPatternInstance() const
{
    return m_pi;
}

QRectF eTrackerSeqItem::boundingRect() const
{
    const eU32 rowStep = m_trackerSeqView->getRowStep();
    const eF32 height =  (eF32)m_fontMetrics.height()*((eF32)m_pi->pattern->getRowCount()/(eF32)rowStep);

    return QRectF(1, 1, eTrackerSeqScene::TRACK_WIDTH-1, height);
}

void eTrackerSeqItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QColor color = QColor(86, 125, 148);

    painter->setClipRect(option->exposedRect); // Optimizes drawing a lot!
    //qDrawShadeRect(painter, boundingRect().toRect(), QPalette(Qt::white), isSelected(), 1, 0, &QBrush(color, Qt::SolidPattern));
    
    QPixmap pixmap;

    if (!QPixmapCache::find(_cacheKey(), pixmap))
    {
        pixmap = _createPixmap();
        QPixmapCache::insert(_cacheKey(), pixmap);
    }

    painter->drawPixmap(0, 0, pixmap);

    painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_pi->pattern->getPatternNumber()).rightJustified(2, '0'));
}

QPixmap eTrackerSeqItem::_createPixmap() const
{
    QPixmap pixmap(boundingRect().size().toSize());
    QPainter painter(&pixmap);
    eU32 height = boundingRect().size().toSize().height();
	eU32 width = boundingRect().size().toSize().width();

    // Get color of operator. Set to dark,
    // red, if operator is invalid.
    QColor opColor2 = QColor(60, 70, 80);
    QColor opColor = QColor(80, 90, 100);
    QColor opSelColor2 = QColor(110, 120, 130);
    QColor opSelColor = QColor(100, 110, 120);
    
    // Draw operator body.
    const QRect &r = boundingRect().toRect();
    QLinearGradient gradient(0.0f, 0.0f, width, height);

    // Make button color lighter using gradient if
    // button is selected.
    if (isSelected())
    {
        gradient.setColorAt(0.0, opSelColor);
        gradient.setColorAt(1.0, opSelColor2);
    }
    else
    {
        gradient.setColorAt(0.0, opColor);
        gradient.setColorAt(1.0, opColor2);
    }

    qDrawShadeRect(&painter, r, QPalette(QColor(10,20,30)), false, 1, 0, &QBrush(gradient));

    return pixmap;
}

// Returns key, uniquely identifying look of
// this item. Used to find pixmap in cache.
const QString & eTrackerSeqItem::_cacheKey() const
{
    static QString key;

    key = eIntToStr(boundingRect().size().toSize().height());
    key += (isSelected() ? "_1" : "_0");

    return key;
}

QVariant eTrackerSeqItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange)
    {
        QPointF newPos = value.toPointF();

        if (newPos.x() < 0.0f)
        {
            newPos.setX(0.0f);
        }

        const eU32 rowStep = m_trackerSeqView->getRowStep();

        eF32 fNewSeqTrack = (eF32)(newPos.x()-m_fontMetrics.width("0000")-eTrackerSeqScene::FIRST_COL_DIST)/(eF32)eTrackerSeqScene::TRACK_WIDTH;
        eF32 fNewRowOffset = eRound((eF32)(newPos.y()-eTrackerSeqScene::FIRST_ROW_DIST)/(eF32)m_fontMetrics.height())*rowStep;

        eU32 newSeqTrack = eClamp(0, (eInt)eRound(fNewSeqTrack), (eInt)tfSong::MAX_SEQ_TRACKS-1);
        eU32 newRowOffset = (eU32)eMax<eInt>(0, (eInt)fNewRowOffset);

        if (scene())
        {
            for (eInt i=0; i<scene()->selectedItems().size(); i++)
            {
                eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->selectedItems().at(i);
                eASSERT(item != eNULL);

                const tfSong::PatternInstance &pi = *item->getPatternInstance();

                if (!pi.song.isTrackFreeAt(newSeqTrack, newRowOffset, m_pi->pattern->getRowCount(), m_pi))
                {
                    return pos();
                }   
            }
        }

        if (scene())
        {
            scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
        }

        m_pi->seqTrack = newSeqTrack;
        m_pi->rowOffset = newRowOffset;

        const eU32 startX = m_fontMetrics.width("0000")+eTrackerSeqScene::FIRST_COL_DIST;

        return QPointF(m_pi->seqTrack*eTrackerSeqScene::TRACK_WIDTH+startX,
                       (eF32)m_pi->rowOffset/(eF32)rowStep*(eF32)m_fontMetrics.height()+(eF32)eTrackerSeqScene::FIRST_ROW_DIST);
    }
 
    return QGraphicsItem::itemChange(change, value);
}

void eTrackerSeqItem::init()
{
    const eU32 startX = m_fontMetrics.width("0000")+eTrackerSeqScene::FIRST_COL_DIST;

    const eU32 rowStep = m_trackerSeqView->getRowStep();
    eInt h = m_fontMetrics.height();

    setFlags(ItemIsFocusable | ItemIsMovable | ItemIsSelectable | ItemClipsToShape | ItemSendsGeometryChanges);
    setPos(m_pi->seqTrack*eTrackerSeqScene::TRACK_WIDTH+startX,
           (eF32)m_pi->rowOffset/(eF32)rowStep*(eF32)m_fontMetrics.height()+(eF32)eTrackerSeqScene::FIRST_ROW_DIST);
}