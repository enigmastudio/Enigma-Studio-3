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

#include "patterneditor.hpp"
#include "mainwnd.hpp"

const eChar ePatternEditor::NOTE_NAMES[12][3] =
{
    "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

const ePatternEditor::KeyNoteOctave ePatternEditor::KEYS_TO_NOTE_OCTAVE[] =
{
    // y s x d c v g b h n j m , l . ö -
    {Qt::Key_Y, 0, 0},
    {Qt::Key_S, 0, 1},
    {Qt::Key_X, 0, 2},
    {Qt::Key_D, 0, 3},
    {Qt::Key_C, 0, 4},
    {Qt::Key_V, 0, 5},
    {Qt::Key_G, 0, 6},
    {Qt::Key_B, 0, 7},
    {Qt::Key_H, 0, 8},
    {Qt::Key_N, 0, 9},
    {Qt::Key_J, 0, 10},
    {Qt::Key_M, 0, 11},
    {Qt::Key_Comma, 1, 0},
    {Qt::Key_L, 1, 1},
    {Qt::Key_Period, 1, 2},
    {Qt::Key_Odiaeresis, 1, 3},
    {Qt::Key_Minus, 1, 4},

    // q 2 w 3 e r 5 t 6 z 7 u i 9 o 0 p ü ´ +
    {Qt::Key_Q, 1, 0},
    {Qt::Key_2, 1, 1},
    {Qt::Key_W, 1, 2},
    {Qt::Key_3, 1, 3},
    {Qt::Key_E, 1, 4},
    {Qt::Key_R, 1, 5},
    {Qt::Key_5, 1, 6},
    {Qt::Key_T, 1, 7},
    {Qt::Key_6, 1, 8},
    {Qt::Key_Z, 1, 9},
    {Qt::Key_7, 1, 10},
    {Qt::Key_U, 1, 11},
    {Qt::Key_I, 2, 0},
    {Qt::Key_9, 2, 1},
    {Qt::Key_O, 2, 2},
    {Qt::Key_0, 2, 3},
    {Qt::Key_P, 2, 4},
    {Qt::Key_Udiaeresis, 2, 5},
};

void ePatternEditor::Selection::update(eU32 oldRow, eU32 newRow, eU32 oldTrack, eU32 newTrack)
{
    if (rowCount == 0 && trackCount == 0)
    {
        // new selection starting
        if (oldRow > newRow)
        {
            row = newRow;
            rowCount = oldRow - newRow + 1;
        }
        else
        {
            row = oldRow;
            rowCount = newRow - oldRow + 1;
        }

        if (oldTrack > newTrack)
        {
            track = newTrack;
            trackCount = oldTrack - newTrack + 1;
        }
        else
        {
            track = oldTrack;
            trackCount = newTrack - oldTrack + 1;
        }
    }
    else
    {
        // modify existing selection
    }
}

ePatternEditor::ePatternEditor(QWidget *parent) : QGraphicsView(parent),
    m_pattern(eNULL),
    m_activeOctave(4),
    m_activeInstr(0),
    m_linesPerBeat(4),
    m_fontSize(10),
    m_cursorTrack(0),
    m_cursorRow(0),
    m_cursorSubIndex(0),
    m_displayTrack(0),
    m_drawTracks(0),
    m_editing(eFALSE),
    m_clipboard(eNULL),
	m_pi(eNULL)
{
    setScene(new QGraphicsScene);
}

void ePatternEditor::setMainWnd(eMainWnd *mainwnd)
{
    m_mainWnd = mainwnd;
}

void ePatternEditor::setPatternInstance(tfSong::PatternInstance *pi)
{
    m_pi = pi;
    m_pattern = (pi ? pi->pattern : eNULL);

    setSceneRect(0, 0, 0, 0);
    scene()->invalidate();
}

void ePatternEditor::setCursorRowViaTime(eF32 time)
{
    eASSERT(time >= 0.0f);

    if (m_pattern == eNULL)
    {
        return;
    }

    const eInt relRow = (eInt)m_pi->song.timeToRow(time)-m_pi->rowOffset;

    if (relRow >= 0 && relRow < (eInt)m_pattern->getRowCount())
    {
        m_cursorRow = relRow;
        scene()->invalidate();
    }
}

void ePatternEditor::setRowCount(eU32 rowCount)
{
    if (m_pattern == eNULL)
    {
        return;
    }

    eASSERT(rowCount > 0);

    m_pattern->setRowCount(rowCount);
    viewport()->update();
}

void ePatternEditor::setTrackCount(eU32 trackCount)
{
    if (m_pattern == eNULL)
    {
        return;
    }

    eASSERT(trackCount > 0);

    m_pattern->setTrackCount(trackCount);
    viewport()->update();
}

void ePatternEditor::setActiveOctave(eU32 activeOctave)
{
    eASSERT(activeOctave <= MAX_OCTAVE);
    m_activeOctave = activeOctave;
}

void ePatternEditor::setActiveInstrument(eU32 activeInstr)
{
    m_activeInstr = activeInstr;
}

void ePatternEditor::setLinesPerBeat(eU32 linesPerBeat)
{
    m_linesPerBeat = linesPerBeat;
    viewport()->update();
}

void ePatternEditor::setFontSize(eU32 fontSize)
{
    eASSERT(fontSize > 0);

    m_fontSize = fontSize;
    viewport()->update();
}

void ePatternEditor::setEditing(eBool on)
{
    m_editing = on;
}

void ePatternEditor::setCursorRow(eU32 row)
{
    m_cursorRow = row;
    scene()->invalidate();
}

void ePatternEditor::setDisplayTrack(eU32 track)
{
    m_displayTrack = track;

    if (m_cursorTrack >= m_drawTracks + m_displayTrack)
        m_cursorTrack = m_displayTrack + m_drawTracks - 1;
    else if (m_cursorTrack < m_displayTrack)
        m_cursorTrack = m_displayTrack;

    scene()->invalidate();
}

tfSong::PatternInstance * ePatternEditor::getPatternInstance() const
{
    return m_pi;
}

eU32 ePatternEditor::getCursosrRow() const
{
    return m_cursorRow;
}

eU32 ePatternEditor::getRowCount() const
{
    if (m_pattern == eNULL)
    {
        return 0;
    }

    return m_pattern->getRowCount();
}

eU32 ePatternEditor::getTrackCount() const
{
    if (m_pattern == eNULL)
    {
        return 0;
    }

    return m_pattern->getTrackCount();
}

eU32 ePatternEditor::getActiveOctave() const
{
    return m_activeOctave;
}

eU32 ePatternEditor::getActiveInstrument() const
{
    return m_activeInstr;
}

eU32 ePatternEditor::getLinesPerBeat() const
{
    return m_linesPerBeat;
}

eU32 ePatternEditor::getFontSize() const
{
    return m_fontSize;
}

eBool ePatternEditor::getEditing() const
{
    return m_editing;
}

void ePatternEditor::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if (m_pattern == eNULL)
    {
        m_mainWnd->m_patternVScroll->setRange(0, 0);
        m_mainWnd->m_patternHScroll->setRange(0, 0);
        return;
    }

    QFont orgBoldFont = painter->font();
    orgBoldFont.setPixelSize(m_fontSize);
    orgBoldFont.setBold(true);

    QFont fwFont = painter->font();
    fwFont.setFamily("Lucida Console");
    fwFont.setBold(eTRUE);
    fwFont.setPixelSize(m_fontSize);
    
    const eInt charWidth = QFontMetrics(fwFont).averageCharWidth();

    const eU32 trackColStart = QFontMetrics(fwFont).width("000")+COLUMN_DIST;
    const eU32 trackWidth = QFontMetrics(fwFont).width("...   .. 0000")+COLUMN_DIST;
    const eU32 rowHeight = QFontMetrics(fwFont).height();

    scene()->setSceneRect(0, 0, width()-4, height()-4);

    // separator line
    painter->setPen(QPen(QBrush(QColor(80,90,100)), 1));
    painter->save();
    painter->drawLine(0, rowHeight+FIRST_ROW_DIST/2-1, sceneRect().width(), rowHeight+FIRST_ROW_DIST/2-1);
    painter->drawLine(0, rowHeight+FIRST_ROW_DIST/2+1, sceneRect().width(), rowHeight+FIRST_ROW_DIST/2+1);
    painter->restore();

    const eU32 ROW_OFFSET[TRACK_OFFSET_COUNT] =
    {
        0, 3, 4, 6, 7, 9, 10, 11, 12
    };

    eU32 drawRows = (height()-FIRST_ROW_DIST) / rowHeight;
    m_drawTracks = eFloor(((eF32)width()-trackColStart) / trackWidth);
    eU32 cursorOffset = drawRows / 2;

    if (m_pattern->getTrackCount() < m_drawTracks)
    {
        m_drawTracks = m_pattern->getTrackCount();
    }

    m_mainWnd->m_patternVScroll->setRange(0, m_pattern->getRowCount()-1);
    m_mainWnd->m_patternHScroll->setRange(0, m_pattern->getTrackCount() - m_drawTracks);

    m_mainWnd->m_patternVScroll->setSliderPosition(m_cursorRow);
    m_mainWnd->m_patternHScroll->setSliderPosition(m_displayTrack);

    // paint current row highlight
    painter->setBrush(QBrush(Qt::darkGray));
    painter->setPen(Qt::darkGray);
    painter->drawRect(sceneRect().left(), (cursorOffset+1)*rowHeight+FIRST_ROW_DIST-1, sceneRect().right(), rowHeight);

    // paint cursor position caret if focused
    if (hasFocus())
    {
        eU32 cursorWidth = charWidth;
        if (m_cursorSubIndex == 0)
            cursorWidth *= 3;

        painter->setBrush(QBrush(QColor(0, 0, 0)));
        painter->setPen(QColor(0, 0, 0));
        painter->drawRect(trackColStart+(m_cursorTrack-m_displayTrack)*trackWidth+ROW_OFFSET[m_cursorSubIndex]*charWidth, 
                          (cursorOffset+1)*rowHeight+FIRST_ROW_DIST-1, 
                          cursorWidth, 
                          rowHeight);
    }

    // additional vertical separator line
    painter->setPen(QColor(80,90,100));
    painter->drawLine(trackColStart-COLUMN_DIST/2-2, 0, trackColStart-COLUMN_DIST/2-2, sceneRect().height());

    for (eU32 i=0; i<m_drawTracks; i++)
    {
        const tfSong::Track &track = m_pattern->getTrack(i+m_displayTrack);
        eASSERT(track.size() <= m_pattern->getRowCount());

        painter->setPen(QColor(80,90,100));
        painter->drawLine(trackColStart+i*trackWidth-COLUMN_DIST/2, 0, trackColStart+i*trackWidth-COLUMN_DIST/2, sceneRect().height());

		painter->setPen(QColor(140,150,160));
        painter->setFont(orgBoldFont);
        painter->drawText(QRectF(trackColStart+i*trackWidth, 0, trackWidth, rowHeight), QString("Track ")+QString::number(i+1+m_displayTrack));

        painter->setFont(fwFont);

        eASSERT(track.size() == m_pattern->getRowCount());

        for (eU32 j=0; j<drawRows; j++)
        {
            const eU32 row = m_cursorRow - cursorOffset + j;
            if (!(row >= 0 && row < m_pattern->getRowCount()))
                continue;

            QColor textColor(140,150,160);

            if (row % m_linesPerBeat == 0)
            {
                textColor = QColor(210,220,230);
            }

            painter->setPen(textColor);

            if (i == 0)
            {
                painter->drawText(QRectF(0, (j+1)*rowHeight+FIRST_ROW_DIST, QFontMetrics(fwFont).width("000"), rowHeight), Qt::AlignRight | Qt::AlignVCenter, QString::number(row));
            }

            const tfSong::NoteEvent &ne = track[row];
            QString text = "";

            if (ne.noteOct == 0) // note empty?
            {
                text = "...";
            }
            else if (ne.noteOct == 128) // note off?
            {
                text = "OFF";
            }
            else
            {
                const eU32 octave = (ne.noteOct-1)/12;
                const eU32 note = (ne.noteOct-1)%12;

                eASSERT(octave <= 8);
                eASSERT(ne.velocity <= 0x80);

                text = QString(NOTE_NAMES[note])+QString::number(octave);
            }

            if (ne.instrument >= 0)
                text += QString::number(ne.instrument, 16).rightJustified(2, '0').toUpper()+" ";
            else
                text += "   ";

            if (ne.velocity >= 0)
                text += QString::number(ne.velocity, 16).rightJustified(2, '0').toUpper()+" ";
            else
                text += ".. ";
               
            if (ne.effect) 
                text += QString::number(ne.effect, 16).rightJustified(4, '0').toUpper();
            else
                text += "....";

            painter->setPen(textColor);
            painter->drawText(QRectF(trackColStart+i*trackWidth, (j+1)*rowHeight+FIRST_ROW_DIST, trackWidth, rowHeight), text);
        }
    }
}

bool ePatternEditor::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) 
    {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);

        if (ke->key() == Qt::Key_Backtab) 
        {
            if (m_cursorTrack > 0)
            {
                m_cursorSubIndex = 0;
                m_cursorTrack--;
            }
            else
            {
                m_cursorSubIndex = 0;
                m_cursorTrack = m_pattern->getTrackCount()-1;
            }

            // Finally repaint view.
            eASSERT(scene() != eNULL);
            scene()->invalidate();

            return true;
        }
        else if (ke->key() == Qt::Key_Tab) 
        {
            if (m_cursorTrack < m_pattern->getTrackCount()-1)
            {
                m_cursorSubIndex = 0;
                m_cursorTrack++;
            }
            else
            {
                m_cursorSubIndex = 0;
                m_cursorTrack = 0;
            }

            // Finally repaint view.
            eASSERT(scene() != eNULL);
            scene()->invalidate();

            return true;
        }
    } 

    return QGraphicsView::event(event);
}

void ePatternEditor::keyPressEvent(QKeyEvent *ke)
{
    QGraphicsView::keyPressEvent(ke);
    
    if (m_pattern == eNULL)
    {
        return;
    }

    eU32 oldrow = m_cursorRow;
    eU32 oldtrack = m_cursorTrack;

    switch (ke->key())
    {
        case Qt::Key_Down:
        {
            m_cursorRow = ((eInt)m_cursorRow+1) % m_pattern->getRowCount();
            break;
        }

        case Qt::Key_Up:
        {
            m_cursorRow = ((eInt)m_cursorRow-1) % m_pattern->getRowCount();
            break;
        }
        
        case Qt::Key_Right:
        {
            if (m_cursorSubIndex < TRACK_OFFSET_COUNT-1)
            {
                m_cursorSubIndex++;
            }
            else if (m_cursorTrack < m_pattern->getTrackCount()-1)
            {
                m_cursorSubIndex = 0;
                m_cursorTrack++;

                if (m_cursorTrack >= m_displayTrack + m_drawTracks)
                    m_displayTrack++;
            }
            else
            {
                m_cursorSubIndex = 0;
                m_cursorTrack = 0;
                m_displayTrack = 0;
            }

            break;
        }

        case Qt::Key_Tab:
        {
            if (m_cursorTrack < m_pattern->getTrackCount()-1)
            {
                m_cursorSubIndex = 0;
                m_cursorTrack++;
            }
            else
            {
                m_cursorSubIndex = 0;
                m_cursorTrack = 0;
            }

            break;
        }

        case Qt::Key_Left:
        {
            if (m_cursorSubIndex > 0)
            {
                m_cursorSubIndex--;
            }
            else if (m_cursorTrack > 0)
            {
                m_cursorSubIndex = TRACK_OFFSET_COUNT-1;
                m_cursorTrack--;

                if (m_cursorTrack < m_displayTrack)
                    m_displayTrack--;
            }
            else
            {
                m_cursorSubIndex = TRACK_OFFSET_COUNT-1;
                m_cursorTrack = m_pattern->getTrackCount()-1;
                m_displayTrack = m_cursorTrack + 1 - m_drawTracks;
            }

            break;
        }

        case Qt::Key_Home:
        {
            m_cursorRow = 0;
            verticalScrollBar()->setSliderPosition(0);
            break;
        }

        case Qt::Key_End:
        {
            m_cursorRow = m_pattern->getRowCount()-1;
            verticalScrollBar()->setSliderPosition(verticalScrollBar()->maximum());
            break;
        }

        case Qt::Key_PageUp:
        {
            m_cursorRow = eMax((eInt)m_cursorRow-16, 0);
            break;
        }

        case Qt::Key_PageDown:
        {
            m_cursorRow = eMin(m_cursorRow+16, m_pattern->getRowCount()-1);
            break;
        }

        // Switch note off.
        case Qt::Key_CapsLock:
        {
            if (m_editing)
            {
                tfSong::NoteEvent &ne = m_pattern->getTrack(m_cursorTrack)[m_cursorRow];

                ne.instrument = -1;
                ne.noteOct = 128;

                m_cursorRow = (m_cursorRow+1) % m_pattern->getRowCount();
            }
            break;
        }

        case Qt::Key_Return:
        {
            for(eU32 i=0;i<m_pattern->getTrackCount();i++)
            {
                tfSong::NoteEvent &ne = m_pattern->getTrack(i)[m_cursorRow];

                eS32 vel = ne.velocity;
                if (vel<0) vel = 128;

                if (ne.noteOct > 0 && ne.noteOct < 128 && ne.instrument >= 0)
                    eDemo::getSynth().noteEvent(ne.instrument, ne.noteOct-1, vel, ne.effectHi, (eF32)ne.effectLo / 128.0f);
                if (ne.noteOct > 128 && ne.instrument >= 0)
                    eDemo::getSynth().noteEvent(ne.instrument, ne.noteOct-1, 0, 0, 0.0f);
            }

            m_cursorRow = (m_cursorRow+1) % m_pattern->getRowCount();

            break;
        }

        default:
        {
            if (m_editing)
            {
                tfSong::NoteEvent &ne = m_pattern->getTrack(m_cursorTrack)[m_cursorRow];

                if (m_cursorSubIndex == 0)
                {
                    eBool advance = eFALSE;

                    // See if a key of the claviature was pressed.
                    for (eU32 i=0; i<sizeof(KEYS_TO_NOTE_OCTAVE)/sizeof(KeyNoteOctave); i++)
                    {
                        const KeyNoteOctave &kno = KEYS_TO_NOTE_OCTAVE[i];

                        // If active octave is the maximum octave don't
                        // go one octave higher when using the upper claviature.
                        const eU32 octMod = (m_activeOctave == MAX_OCTAVE ? 0 : kno.octave);

                        if (kno.key == ke->key())
                        {
                            ne.noteOct = 1+(m_activeOctave+octMod)*12+kno.note;
                            ne.instrument = m_activeInstr;
                            advance = eTRUE;
                            eDemo::getSynth().noteEvent(m_activeInstr, ne.noteOct-1, 128, ne.effectHi, (eF32)ne.effectLo / 127.0f);
                            break;
                        }
                    }

                    // Should note be deleted?
                    if (ke->key() == Qt::Key_Delete)
                    {
                        ne.noteOct = 0;
                        ne.instrument = -1;
                        advance = eTRUE;
                    }

                    // Advance cursor one row down.
                    if (advance)
                    {
                        m_cursorRow = (m_cursorRow+1) % m_pattern->getRowCount();
                    }
                }
                else if (_isKeyHexChar(ke->key()) || ke->key() == Qt::Key_Delete)
                {
                    const eU8 hex = (QString("0x")+QString(ke->key())).toInt(eNULL, 16);
                    const eBool del = ke->key() == Qt::Key_Delete;

                    switch (m_cursorSubIndex)
                    {
                        case 1:
                        {
                            eS8 &instr = ne.instrument;
                            if (del)
                                instr = -1;
                            else
                            {
                                if (instr == -1) instr = 0;
                                instr = (instr&0x0f)|(eU8)(hex<<4);
                            }
                            break;
                        }

                        case 2:
                        {
                            eS8 &instr = ne.instrument;
                            if (del)
                                instr = -1;
                            else
                            {
                                if (instr == -1) instr = 0;
                                instr = (instr&0xf0)|hex;
                            }
                            break;
                        }

                        case 3:
                        {
                            eS8 &vel = ne.velocity;
                            if (del)
                                vel = -1;
                            else
                            {
                                if (vel == -1) vel = 0;
                                vel = eMin(0x80, (vel&0x0f)|(eU8)(hex<<4));
                            }
                            break;
                        }

                        case 4:
                        {
                            eS8 &vel = ne.velocity;
                            if (del)
                                vel = -1;
                            else
                            {
                                if (vel == -1) vel = 0;
                                vel = eMin(0x80, (vel&0xf0)|hex);
                            }
                            break;
                        }

                        case 5:
                        {
                            eU16 &eff = ne.effect;
                            if (del)
                                eff = 0;
                            else
                                eff = (eff&0x0fff)|(eU16)(hex<<12);
                            break;
                        }

                        case 6:
                        {
                            eU16 &eff = ne.effect;
                            if (del)
                                eff = 0;
                            else
                                eff = (eff&0xf0ff)|(eU16)(hex<<8);
                            break;
                        }

                        case 7:
                        {
                            eU16 &eff = ne.effect;
                            if (del)
                                eff = 0;
                            else
                                eff = (eff&0xff0f)|(eU16)(hex<<4);
                            break;
                        }

                        case 8:
                        {
                            eU16 &eff = ne.effect;
                            if (del)
                                eff = 0;
                            else
                                eff = (eff&0xfff0)|(eU16)hex;
                            break;
                        }
                    }

                    m_cursorRow = (m_cursorRow+1) % m_pattern->getRowCount();
                }
            }

            break;
        }
    }

    if (ke->modifiers() == Qt::ShiftModifier)
    {
        m_selection.update(oldrow, m_cursorRow, oldtrack, m_cursorTrack);
    }

    // Finally repaint view.
    eASSERT(scene() != eNULL);
    scene()->invalidate();
}

void ePatternEditor::keyReleaseEvent(QKeyEvent *ke)
{
    QGraphicsView::keyReleaseEvent(ke);
}

bool ePatternEditor::eventFilter(QObject *object, QEvent *event)
{
	if (object->isWidgetType())
	{
		QWidget *widget = (QWidget*)object;
		QString className = QString(widget->metaObject()->className());
		
		if (className == "QLineEdit")
			return false;

		if (className == "QExpandingLineEdit")
			return false;
	}

	if (event->type() == QEvent::KeyPress) 
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);

		if (!ke->isAutoRepeat())
		{
			if (!m_editing)
			{
				for (eU32 i=0; i<35; i++)
				{
					const ePatternEditor::KeyNoteOctave &kno = ePatternEditor::KEYS_TO_NOTE_OCTAVE[i];

					if (kno.key == ke->key())
					{
						eU32 note = (m_activeOctave + kno.octave)*12+kno.note;

						eDemo::getSynth().noteEvent(m_activeInstr, note, 128, 0, 0.0f);
						return true;
					}
				}
			}
		}
	}
	else if (event->type() == QEvent::KeyRelease) 
	{
		QKeyEvent *ke = static_cast<QKeyEvent *>(event);

		if (!ke->isAutoRepeat())
		{
			if (m_cursorSubIndex == 0)
			{
				// See if a key of the claviature was pressed.
				for (eU32 i=0; i<sizeof(KEYS_TO_NOTE_OCTAVE)/sizeof(KeyNoteOctave); i++)
				{
					const KeyNoteOctave &kno = KEYS_TO_NOTE_OCTAVE[i];

					if (kno.key == ke->key())
					{
						int note = 1+(m_activeOctave+kno.octave)*12+kno.note;

						eDemo::getSynth().noteEvent(m_activeInstr, note-1, 0, 0, 0.0f);
						return true;
					}
				}
			}
		}
	}

	return false;
}

void ePatternEditor::showEvent(QShowEvent *e)
{
	qApp->installEventFilter(this);
}

void ePatternEditor::hideEvent(QHideEvent *e)
{
	qApp->removeEventFilter(this);
}

void ePatternEditor::focusInEvent(QFocusEvent *fe)
{
    QGraphicsView::focusInEvent(fe);
    viewport()->update();
}

void ePatternEditor::focusOutEvent(QFocusEvent *fe)
{
    QGraphicsView::focusOutEvent(fe);
    viewport()->update();
}

eBool ePatternEditor::_isKeyHexChar(eInt key) const
{
    return ((key >= Qt::Key_0 && key <= Qt::Key_9) ||
            (key >= Qt::Key_A && key <= Qt::Key_F));
}