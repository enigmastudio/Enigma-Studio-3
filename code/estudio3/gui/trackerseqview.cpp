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

#include <QtGui/QScrollBar>

#include "trackerseqview.hpp"
#include "trackerseqitem.hpp"

eTrackerSeqScene::eTrackerSeqScene(tfSong &song, eTrackerSeqView *parent) : QGraphicsScene(parent),
    m_song(song),
    m_trackerSeqView(parent)
{
    eASSERT(parent != eNULL);

    for (eU32 i=0; i<m_song.getPatternInstanceCount(); i++)
    {
        addItem(new eTrackerSeqItem(&m_song.getPatternInstance(i), parent));
    }

    setItemIndexMethod(NoIndex);
    //setBackgroundBrush(QColor(48,64,80));
}

eTrackerSeqScene::~eTrackerSeqScene()
{
    clear();
    eDemoData::removeSong(m_song.getId());
}

void eTrackerSeqScene::saveToXml(QDomElement &node) const
{
    QDomDocument xml = node.ownerDocument();
    eASSERT(!xml.isNull());

    QDomElement songEl = xml.createElement("song");
    node.appendChild(songEl);
    songEl.setAttribute("id", m_song.getId());
    songEl.setAttribute("name", m_song.getUserName());
	songEl.setAttribute("bpm", m_song.getBpm());

    // Save all patterns.
    QDomElement patternsEl = xml.createElement("patterns");
    songEl.appendChild(patternsEl);
    
    for (eU32 i=0; i<m_song.getPatternCount(); i++)
    {
        const tfSong::Pattern &pattern = m_song.getPattern(i);

        QDomElement patternEl = xml.createElement("pattern");
        patternsEl.appendChild(patternEl);
        patternEl.setAttribute("rowcount", pattern.getRowCount());
        patternEl.setAttribute("trackcount", pattern.getTrackCount());
        patternEl.setAttribute("patternid", pattern.getPatternNumber());

        QDomElement tracksEl = xml.createElement("tracks");
        patternEl.appendChild(tracksEl);

        for (eU32 j=0; j<pattern.getTrackCount(); j++)
        {
            QDomElement trackEl = xml.createElement("track");
            tracksEl.appendChild(trackEl);

            const tfSong::Track &track = pattern.getTrack(j);

            for (eU32 k=0; k<track.size(); k++)
            {
                const tfSong::NoteEvent &ne = track[k];

                if (!ne.effect && !ne.noteOct && ne.instrument == -1 && ne.velocity == -1)
                {
                    continue;
                }

                QDomElement noteEventEl = xml.createElement("ne");
                trackEl.appendChild(noteEventEl);

                noteEventEl.setAttribute("r", k);
                noteEventEl.setAttribute("n", ne.noteOct);
                noteEventEl.setAttribute("i", ne.instrument);
                noteEventEl.setAttribute("v", ne.velocity);
                noteEventEl.setAttribute("e", ne.effect);
            }
        }
    }

    // Save all pattern instances.
    QDomElement pisEl = xml.createElement("patterninstances");

    for (eInt i=0; i<items().size(); i++)
    {
        eTrackerSeqItem *seqItem = (eTrackerSeqItem *)items().at(i);
        eASSERT(seqItem != eNULL);
        seqItem->saveToXml(pisEl);
    }

    songEl.appendChild(pisEl);
}

void eTrackerSeqScene::loadFromXml(const QDomElement &node)
{
	m_song.setUserName(node.attribute("name").toAscii().constData());
	m_song.setBpm(node.attribute("bpm").toInt());

    QDomElement xmlItem = node.firstChildElement("patterns");
    xmlItem = xmlItem.firstChildElement("pattern");

    while (!xmlItem.isNull())
    {
        const eU32 rowCount = xmlItem.attribute("rowcount").toInt(); 
        const eU32 trackCount = xmlItem.attribute("trackcount").toInt(); 
        const eU32 id = xmlItem.attribute("patternid").toInt(); 

        tfSong::Pattern &pattern = m_song.newPattern(rowCount, trackCount, id);

        QDomElement xmlItemTrack = xmlItem.firstChildElement("tracks");
        xmlItemTrack = xmlItemTrack.firstChildElement("track");

        eU32 i=0;
        while (!xmlItemTrack.isNull())
        {
            tfSong::Track &track = pattern.getTrack(i++);

            QDomElement xmlItemNote = xmlItemTrack.firstChildElement("ne");

            while (!xmlItemNote.isNull())
            {
                const eU32  row     = xmlItemNote.attribute("r").toInt(); 
                const eU8   note    = xmlItemNote.attribute("n").toInt(); 
                const eS8   instr   = xmlItemNote.attribute("i").toInt(); 
                const eS8   vel     = xmlItemNote.attribute("v").toInt(); 
                const eU16  effect  = xmlItemNote.attribute("e").toInt(); 

                tfSong::NoteEvent &ne = track[row];

                ne.noteOct = note;
                ne.instrument = instr;
                ne.velocity = vel;
                ne.effect = effect;

                xmlItemNote = xmlItemNote.nextSiblingElement("ne");
            }

            xmlItemTrack = xmlItemTrack.nextSiblingElement("track");
        }

        xmlItem = xmlItem.nextSiblingElement("pattern");
    }

    xmlItem = node.firstChildElement("patterninstances");
    xmlItem = xmlItem.firstChildElement("patterninstance");

    while (!xmlItem.isNull())
    {
        const eU32 rowOffset = xmlItem.attribute("rowoffset").toInt(); 
        const eU32 track = xmlItem.attribute("track").toInt(); 
        const eU32 patternId = xmlItem.attribute("patternid").toInt(); 

        for (eU32 i=0; i<m_song.getPatternCount(); i++)
        {
            tfSong::Pattern &pattern = m_song.getPattern(i);

            if (pattern.getPatternNumber() == patternId)
            {
                tfSong::PatternInstance &pi = m_song.newPatternInstance(pattern, rowOffset, track);
                addItem(new eTrackerSeqItem(&pi, m_trackerSeqView));
                break;
            }
        }

        xmlItem = xmlItem.nextSiblingElement("patterninstance");
    }
}

tfSong & eTrackerSeqScene::getSong()
{
    return m_song;
}

const tfSong & eTrackerSeqScene::getSong() const
{
    return m_song;
}

eU32 eTrackerSeqScene::getRowTextWidth() const
{
    return m_rowTextWidth;
}

void eTrackerSeqScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawForeground(painter, rect);

	QFont fwFont = painter->font();
    m_rowTextWidth = QFontMetrics(fwFont).width("0000");
    const eU32 fwHeight = QFontMetrics(fwFont).height();

    for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
    {
        if (m_song.getMuted(i))
        {
            painter->setPen(Qt::black);
            painter->fillRect(
                i*TRACK_WIDTH+m_rowTextWidth+2, 
                sceneRect().top(), 
                TRACK_WIDTH, 
                sceneRect().bottom(), 
                Qt::Dense4Pattern);
        }
    }
}

void eTrackerSeqScene::drawBackground(QPainter *painter, const QRectF &r)
{
    QGraphicsScene::drawBackground(painter, r);

    QFont fwFont = painter->font();
    const eU32 fwHeight = QFontMetrics(fwFont).height();
    const eU32 rowTextWidth = QFontMetrics(fwFont).width("0000");
    const eU32 rowStep = m_trackerSeqView->getRowStep();

    setSceneRect(0, 0, TRACK_WIDTH*tfSong::MAX_SEQ_TRACKS+rowTextWidth+FIRST_COL_DIST, (eF32)m_song.getLengthInRows()/(eF32)rowStep*(eF32)fwHeight+(eF32)FIRST_ROW_DIST);

    painter->setFont(fwFont);
    painter->setPen(QColor(130,140,150));
	
    // Paint row numbers.
    for (eU32 i=0; i<m_song.getLengthInRows()/rowStep; i++)
    {
        painter->drawText(QRectF(0, i*fwHeight+1, rowTextWidth-1, fwHeight), Qt::AlignRight | Qt::AlignVCenter, QString::number(i*rowStep));
    }

    // Paint horizontal separator line between track
    // numbers and pattern instance rectangles.

    painter->setPen(Qt::lightGray);

    // Paint track numbers and separator lines.
    const eU32 startX = rowTextWidth+FIRST_COL_DIST;

    painter->drawLine(rowTextWidth, sceneRect().top(), rowTextWidth, sceneRect().bottom());

    for (eU32 i=0; i<tfSong::MAX_SEQ_TRACKS; i++)
    {    
        painter->setPen(QColor(80,90,100));

        if (i > 0)
        {
            painter->setPen(QPen(QBrush(QColor(80,90,100)), 1, Qt::DashLine));
        }

        painter->drawLine(i*TRACK_WIDTH+rowTextWidth+2, sceneRect().top(), i*TRACK_WIDTH+rowTextWidth+2, sceneRect().bottom());
    }
}

eTrackerSeqView::eTrackerSeqView(QWidget *parent) : QGraphicsView(parent),
    m_rowStep(16),
    m_curRow(0)
{
}

eF32 eTrackerSeqView::getTime() const
{
    return 0.0f;// m_time;
}

eU32 eTrackerSeqView::getRowStep() const
{
    return m_rowStep;
}

tfSong * eTrackerSeqView::getSong() const
{
    if (scene())
    {
        return &((eTrackerSeqScene *)scene())->getSong();
    }

    return eNULL;
}

void eTrackerSeqView::setTime(eF32 time)
{
    eASSERT(time >= 0.0f);

    if (getSong() != eNULL)
    {
        m_curRow = getSong()->timeToRow(time);
        viewport()->update();
    }
}

void eTrackerSeqView::setRowStep(eU32 rowStep)
{
    eASSERT(rowStep > 0);

    m_rowStep = rowStep;

    for (eInt i=scene()->items().size()-1; i>=0; i--)
    {
        eTrackerSeqItem *item = (eTrackerSeqItem* )scene()->items().at(i);
        item->init();
    }

    viewport()->update();
}

void eTrackerSeqView::newPattern(eU32 rowCount, eU32 trackCount, eU32 seqTrack)
{
    eASSERT(rowCount > 0);
    eASSERT(trackCount > 0);
    eASSERT(seqTrack < tfSong::MAX_SEQ_TRACKS);

    if (scene() == eNULL)
    {
        return;
    }

    tfSong &song = ((eTrackerSeqScene *)scene())->getSong();

    tfSong::Pattern &pattern = song.newPattern(rowCount, trackCount);
    tfSong::PatternInstance &pi = song.newPatternInstance(pattern, song.getLengthInRows(), seqTrack);
    
    eTrackerSeqItem *item = new eTrackerSeqItem(&pi, this);
    scene()->addItem(item);

    scene()->clearSelection();
    item->setSelected(true);

    Q_EMIT onPatternInstSelected(pi);

    viewport()->update();
}

void eTrackerSeqView::removeSelectedInstances()
{
    if (scene() == eNULL)
    {
        return;
    }

    tfSong &song = ((eTrackerSeqScene *)scene())->getSong();

    for (eInt i=scene()->selectedItems().size()-1; i>=0; i--)
    {
        eTrackerSeqItem *item = (eTrackerSeqItem* )scene()->selectedItems().at(i);
        eASSERT(item != eNULL);
        eSAFE_DELETE(item);
    }

    viewport()->update();
}

void eTrackerSeqView::cloneSelectedPatterns()
{
    if (scene() == eNULL)
    {
        return;
    }

    if (scene()->selectedItems().size() == 1)
    {
        eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->selectedItems().at(0);
        eASSERT(item != eNULL);

        item = new eTrackerSeqItem(*item, eTRUE);
        scene()->addItem(item);

        scene()->clearSelection();
        item->setSelected(true);

        Q_EMIT onPatternInstSelected(*item->getPatternInstance());
    }

    // reinitialize items since they can be moved
    for(eInt i=0;i<scene()->items().size(); i++)
    {
        eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->items().at(i);
        item->init();
    }

    viewport()->update();
}

void eTrackerSeqView::cloneSelectedInstances()
{
    if (scene() == eNULL)
    {
        return;
    }

    if (scene()->selectedItems().count() == 0)
    {
        newPattern(64, 4, 0);
    }
    else
    {
        if (scene()->selectedItems().size() == 1)
        {
            eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->selectedItems().at(0);
            eASSERT(item != eNULL);

            item = new eTrackerSeqItem(*item, eFALSE);
            scene()->addItem(item);

            scene()->clearSelection();
            item->setSelected(true);

            Q_EMIT onPatternInstSelected(*item->getPatternInstance());
        }

        // reinitialize items since they can be moved
        for(eInt i=0;i<scene()->items().size(); i++)
        {
            eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->items().at(i);
            item->init();
        }
    }

    viewport()->update();
}

void eTrackerSeqView::cleanPatterns()
{
    if (scene() == eNULL)
    {
        return;
    }

    tfSong &song = ((eTrackerSeqScene *)scene())->getSong();
    song.clearUnusedPatterns();
}

void eTrackerSeqView::patternPlus()
{
    if (scene() == eNULL)
    {
        return;
    }

    if (scene()->selectedItems().size() != 1)
        return;

    eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->selectedItems().at(0);
    eASSERT(item != eNULL);

    item->getPatternInstance()->plus();
    scene()->invalidate();

    Q_EMIT onPatternInstSelected(*item->getPatternInstance());
}

void eTrackerSeqView::patternMinus()
{
    if (scene() == eNULL)
    {
        return;
    }

    if (scene()->selectedItems().size() != 1)
        return;

    eTrackerSeqItem *item = (eTrackerSeqItem *)scene()->selectedItems().at(0);
    eASSERT(item != eNULL);

    item->getPatternInstance()->minus();
    scene()->invalidate();

    Q_EMIT onPatternInstSelected(*item->getPatternInstance());
}

void eTrackerSeqView::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

    if (getSong() == eNULL)
    {
        return;
    }

    // Draw red current time marker.
    const eU32 fontHeight = QFontMetrics(painter->font()).height();
    const eF32 y = (eF32)(m_curRow/m_rowStep)*(eF32)fontHeight+(eF32)eTrackerSeqScene::FIRST_ROW_DIST+1.0f;

    painter->setPen(Qt::red);
    painter->drawLine(rect.left(), y, rect.right(), y);
}

void eTrackerSeqView::mouseMoveEvent(QMouseEvent *me)
{
    if (me->buttons() & Qt::RightButton && getSong() != eNULL)
    {
        const eU32 fontHeight = QFontMetrics(font()).height();
        const eF32 toSub = 2*fontHeight-eTrackerSeqScene::FIRST_ROW_DIST;

        const eF32 t = eClamp(0.0f, (eF32)(mapToScene(me->pos()).y()-toSub)/(eF32)(sceneRect().height()-toSub), 1.0f);
        m_curRow = (eU32)(t*(eF32)getSong()->getLengthInRows());

        scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
        Q_EMIT onTimeChanged(getSong()->rowToTime(m_curRow));
        me->accept();
    }
    else  if (me->buttons() & Qt::MidButton)
    {
        // Scroll view using middle mouse button.
        const QPoint delta = me->pos()-m_lastMousePos;

        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()-delta.y());

        m_lastMousePos = me->pos();
        me->accept();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(me);
    }
}

void eTrackerSeqView::mousePressEvent(QMouseEvent *me)
{
    m_lastMousePos = me->pos();

    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ClosedHandCursor);
        me->accept();
    }
    else if (me->button() == Qt::RightButton)
    {
        mouseMoveEvent(me);
        me->accept();
    }
    else
    {
        QGraphicsView::mousePressEvent(me);

        // Emit operator-selected signal, if there's
        // an operator under mouse.
        if (scene())
        {
            eTrackerSeqItem *seqItem = (eTrackerSeqItem *)scene()->itemAt(mapToScene(me->pos()));

            if (seqItem)
            {
                Q_EMIT onPatternInstSelected(*seqItem->getPatternInstance());
            }
        }
    }
}

void eTrackerSeqView::mouseReleaseEvent(QMouseEvent *me)
{
    QGraphicsView::mouseReleaseEvent(me);

    if (me->button() == Qt::MidButton)
    {
        setCursor(Qt::ArrowCursor);
    }
}