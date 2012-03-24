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

#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QtGui/QGraphicsView>
#include <QtGui/QKeyEvent>

#include "../../eshared/eshared.hpp"

class ePatternEditor : public QGraphicsView
{
    Q_OBJECT

public:
    ePatternEditor(QWidget *parent=eNULL);

public Q_SLOTS:
    void                        setPatternInstance(tfSong::PatternInstance *pi);
    void                        setCursorRowViaTime(eF32 time);
    void                        setRowCount(eU32 rowCount);
    void                        setTrackCount(eU32 trackCount);
    void                        setActiveOctave(eU32 activeOctave);
    void                        setActiveInstrument(eU32 activeInstr);
    void                        setLinesPerBeat(eU32 linesPerBeat);
    void                        setFontSize(eU32 fontSize);
    void                        setEditing(eBool on);
    void                        setCursorRow(eU32 row);
    void                        setDisplayTrack(eU32 track);

public:
    void                        setMainWnd(class eMainWnd *mainwnd);
    tfSong::PatternInstance *   getPatternInstance() const;
    eU32                        getCursosrRow() const;
    eU32                        getRowCount() const;
    eU32                        getTrackCount() const;
    eU32                        getActiveOctave() const;
    eU32                        getActiveInstrument() const;
    eU32                        getLinesPerBeat() const;
    eU32                        getFontSize() const;
    eBool                       getEditing() const;

private:
    virtual void                drawBackground(QPainter *painter, const QRectF &rect);

    virtual bool                event(QEvent *event);
	virtual bool                eventFilter(QObject *object, QEvent *e);
	virtual void				showEvent(QShowEvent *e);
	virtual void				hideEvent(QHideEvent *e);
    virtual void                keyPressEvent(QKeyEvent *ke);
    virtual void                keyReleaseEvent(QKeyEvent *ke);
    virtual void                focusInEvent(QFocusEvent *fe);
    virtual void                focusOutEvent(QFocusEvent *fe);

private:
    eBool                       _isKeyHexChar(eInt key) const;

public:
    struct KeyNoteOctave
    {
        eInt    key;
        eU8     octave;
        eU8     note;
    };

    struct Selection
    {
        Selection() :
            row(0),
            track(0),
            rowCount(0),
            trackCount(0)
        {
        }

        void    update(eU32 oldRow, eU32 newRow, eU32 oldTrack, eU32 newTrack);

        eU32    row;
        eU32    track;
        eU32    rowCount;
        eU32    trackCount;
    };

private:
    static const eU32           MAX_OCTAVE = 8;
    static const eU32           FIRST_ROW_DIST = 8;
    static const eU32           TRACK_OFFSET_COUNT = 9;
    static const eU32           COLUMN_DIST = 10;
    static const eChar          NOTE_NAMES[12][3];

public:
    static const KeyNoteOctave  KEYS_TO_NOTE_OCTAVE[];

private:
    class eMainWnd *            m_mainWnd;

    tfSong::Pattern *           m_pattern;
    tfSong::PatternInstance *   m_pi;
    eU32                        m_activeOctave;
    eU32                        m_activeInstr;
    eU32                        m_linesPerBeat;
    eU32                        m_fontSize;
    eBool                       m_editing;
    Selection                   m_selection;
    tfSong::Pattern *           m_clipboard;

    eU32                        m_displayTrack;
    eU32                        m_drawTracks;
    eU32                        m_cursorTrack;
    eU32                        m_cursorRow;
    eU32                        m_cursorSubIndex;
};

#endif // PATTERN_EDITOR_HPP