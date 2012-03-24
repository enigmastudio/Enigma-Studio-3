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

#ifndef TRACKER_SEQ_VIEW_HPP
#define TRACKER_SEQ_VIEW_HPP

#include <QtGui/QGraphicsView>
#include <QtXml/QDomDocument>

#include "../../eshared/eshared.hpp"

// A scene for the tracker's sequencer view.
class eTrackerSeqScene : public QGraphicsScene
{
public:
    eTrackerSeqScene(tfSong &song, class eTrackerSeqView *parent);
    virtual ~eTrackerSeqScene();

    void                saveToXml(QDomElement &node) const;
    void                loadFromXml(const QDomElement &node);

    tfSong &            getSong();
    const tfSong &      getSong() const;

    eU32                getRowTextWidth() const;

protected:
    virtual void        drawBackground(QPainter *painter, const QRectF &rect);
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);

public:
    static const eU32   TRACK_WIDTH = 32;
    static const eU32   FIRST_ROW_DIST = 2;
    static const eU32   FIRST_COL_DIST = 2;

private:
    tfSong &            m_song;
    eTrackerSeqView *   m_trackerSeqView;
    eU32                m_rowTextWidth;
};

typedef QMap<eID, eTrackerSeqScene *> eTrackerSeqScenePtrIdMap;

// Sequencer view for the tracker.
class eTrackerSeqView : public QGraphicsView
{
    Q_OBJECT

public:
    eTrackerSeqView(QWidget *parent=eNULL);

    eF32                getTime() const;
    eU32                getRowStep() const;
    tfSong *            getSong() const;
	void				songChanged();

    void                setTime(eF32 time);

public Q_SLOTS:
    void                setRowStep(eU32 rowStep);

    void                newPattern(eU32 rowCount, eU32 trackCount, eU32 seqTrack);
    void                removeSelectedInstances();
    void                cloneSelectedInstances();
    void                cloneSelectedPatterns();
    void                cleanPatterns();
    void                patternPlus();
    void                patternMinus();

Q_SIGNALS:
    void                onPatternInstSelected(tfSong::PatternInstance &pi);
    void                onTimeChanged(eF32 time);

private:
    virtual void        drawForeground(QPainter *painter, const QRectF &rect);

    virtual void        mouseMoveEvent(QMouseEvent *me);
    virtual void        mousePressEvent(QMouseEvent *me);
    virtual void        mouseReleaseEvent(QMouseEvent *me);

private:
    void                _clearItems();

private:
    eU32                m_rowStep;
    QPoint              m_lastMousePos;
    eU32                m_curRow;
};

#endif // TRACKER_SEQ_VIEW_HPP