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

#ifndef MAIN_WND_HPP
#define MAIN_WND_HPP

#include <QtGui/QLabel>

#include "guioppage.hpp"
#include "ui_mainwnd.hpp"

// Enigma Studio's application main window.
class eMainWnd : public QMainWindow, public Ui::MainWnd
{
    Q_OBJECT

private:
    enum StatusBarPane
    {
        SBPANE_OPINFOS,
        SBPANE_STATISTICS,
        SBPANE_VIEWTIME,
        SBPANE_PRJINFOS,
        SBPANE_COUNT
    };

public:
    eMainWnd();
    virtual ~eMainWnd();

    eU32                        getActiveOctave();
    eU32                        getActiveInstrument();

	ePatternEditor *			getPatternEditor();
	eInstrumentList *			getInstrumentList();
	eScopesView *				getScopesView();

private:
    void                        _clearProject();

    void                        _setupViewMenu();
    void                        _setupStatusBar();
    void                        _makeConnections();
    void                        _addDefaultPage();
    void                        _createRecentFileActs();
    void                        _updateRecentFileActs();
    void                        _updateStatusBar();
    void                        _updateProfilerTree();
    void                        _setStatusText(StatusBarPane pane, const QString &text);
    void                        _saveBackup();

    eBool                       _askForSaving();
    void                        _setCurrentFile(const QString &filePath);
    eBool                       _loadFromXml(const QString &filePath);
    eBool                       _saveToXml(const QString &filePath, eBool backup=eFALSE);
    void                        _newProject(eBool loadDefaultPrj);
	void						_loadRecentProject();

    void                        _writeSettings() const;
    void                        _readSettings();

    eU32                        _synthToIndex(eF32 value, eU32 min, eU32 max);
    eF32                        _synthFromIndex(eU32 value, eU32 min, eU32 max);
    void                        _synthSetParameter(eU32 index, eF32 value);
    void                        _synthInitParameters();
    void                        _synthUpdateOscView();
    void                        _synthSetActiveInstrument(eU32 index);
    tfInstrument *              _synthGetActiveInstrument();
    void                        _synthInitInstrumentDropdown();
    void                        _synthUpdateInstrumentName();

    eBool                       _synthInstrumentLoad(eU32 index);
    eBool                       _synthInstrumentSave(eU32 index);
	void						_clearAllInstruments();

	void						updateAfterPreset();
	void						setPresetValue(eU32 index, eF32 value);

private Q_SLOTS:
    void                        _onAbout();
    void                        _onFileNew();
    void                        _onFileOpen();
    eBool                       _onFileSave();
    eBool                       _onFileSaveAs();
    void                        _onFileMake();
    void                        _onFileOpenRecent();

    void                        _onPageAdded(eID &pageId);
    void                        _onPageRemoved(eID pageId);
    void                        _onPageSwitched(eID pageId);
    void                        _onPageRenamed(eID pageId, const QString &newName);

    void                        _onOperatorShow(eID opId);
    void                        _onOperatorAdded(eID opId);
    void                        _onOperatorRemoved(eID opId);
    void                        _onOperatorSelected(eID opId);
    void                        _onOperatorChanged(const eParameter *param);
    void                        _onGotoOperator(eID opId);
    void                        _onPathOperatorEdit(eID opId);
    void                        _onDemoOperatorEdit(eID opId);
    void                        _onSwitchToPageView();

    void                        _onToggleAppFullscreen();
    void                        _onToggleViewportFullscreen();

    void                        _onDemoSeqScaleChanged(int value);
    void                        _onDemoSeqTimeChanged(eF32 time);
    void                        _onTrackerSeqTimeChanged(eF32 time);
    void                        _onPathViewTimeChanged(eF32 time);
    void                        _onTimelineTimeChanged(eF32 time);
    void                        _onTimeEditEdited(const QString &text);
    void                        _onPathViewZoomChanged(int value);
    void                        _onPathViewShowPathsChanged(int state);
    void                        _onPathViewChangeInterpolType(int index);
    void                        _onPathViewSelectionChanged(ePathView::GuiWaypoint *guiWp);
    void                        _onPathViewAddWaypoint();
    void                        _onPathViewRemoveWaypoint(ePathView::GuiWaypoint &guiWp);

    void                        _onTrackCountChanged(int trackCount);
    void                        _onActiveOctaveChanged(int activeOctave);
    void                        _onLinesPerBeatChanged(int linesPerBeat);
    void                        _onRowCountChanged(int rowCountIndex);
    void                        _onPatternInstSelected(tfSong::PatternInstance &pi);
    void                        _onBpmChanged(int bpm);
    void                        _onNewPattern();
    void                        _onRemovePattern();
    void                        _onClonePattern();
    void                        _onCleanPatterns();
    void                        _onPatternPlus();
    void                        _onPatternMinus();
    void                        _onPanic();
    void                        _onMute(bool clicked);
    void                        _onPatternLoop(bool clicked);
    void                        _onMetronome(bool clicked);
    void                        _onTransposeUpNote();
    void                        _onTransposeUpOct();
    void                        _onTransposeDownNote();
    void                        _onTransposeDownOct();
    void                        _onSynthVolumeChanged(int value);
    void                        _onSongAdded(eID songId);
    void                        _onSongRemoved(eID songId);
    void                        _onSongSwitched(eID songId);
	void						_onSynthClicked(bool checked);
    void                        _onSynthChanged(double value);
    void                        _onSynthChanged(int value);
    void                        _onSynthInstrumentChanged(int value);
    void                        _onSynthInstrNameChanged(const QString &name);
    void                        _onInstrumentChanged();
    void                        _onRemoveInstrument();
    void                        _onAddInstrument();
    void                        _onEditing();
    void                        _onPatternVScroll(int value);
    void                        _onPatternHScroll(int value);

	void                        _synthInstrumentLoad();
    void                        _synthInstrumentsLoadAll();
    void                        _synthInstrumentSave();
    void                        _synthInstrumentSaveAll();
    void                        _synthInstrumentCopy();
    void                        _synthInstrumentPaste();

	void						_presetSine(bool checked);
	void						_presetTriangle(bool checked);
	void						_presetSquare(bool checked);
	void						_presetSawUp(bool checked);
	void						_presetSawDown(bool checked);

private:
    virtual void                closeEvent(QCloseEvent *ce);
    virtual void                timerEvent(QTimerEvent *te);

	void						_createIcons();
	void						_freeIcons();

private:
    class ProfilerTreeItem : public QTreeWidgetItem
    {
    public:
        virtual bool            operator < (const QTreeWidgetItem &item) const;
    };

    typedef QList<ProfilerTreeItem *> ProfilerTreeItemList;

private:
    static const QString        PROJECT_FILTER;
    static const QString        SCRIPT_FILTER;
    static const QString        EDITOR_CAPTION;
    static const QString        BACKUP_FILENAME;

    static const eU32           MAX_RECENT_FILES = 5;
    static const eU32           AUTO_BACKUP_TIME_MS = 60*1000;
    static const eU32           TF_NUM_PRESETS = 128;

private:
    ProfilerTreeItemList        m_profItems;

    QAction *                   m_recSepAct;
    QAction *                   m_recentActs[MAX_RECENT_FILES];
    QString                     m_lastPrjPath;
    QLabel                      m_sbLabels[SBPANE_COUNT];
    eGuiOpPagePtrIdMap          m_guiOpPages;
    eTrackerSeqScenePtrIdMap    m_trackerSeqScenes;
    QString                     m_prjFilePath;
    QString                     m_modulePath;
    eInt                        m_statusBarTimerId;
    eInt                        m_backupTimerId;
    tfInstrument *              m_activeInstr;
    QGraphicsScene              m_oscViewScene;
    tfInstrument::Data          m_synthInstrs[TF_NUM_PRESETS];
    eU32                        m_synthCurInstr;
    eU32                        m_synthCurrentInstrument;

	QPixmap *					m_pixSine;
	QPixmap	*					m_pixSawUp;
	QPixmap	*					m_pixSawDown;
	QPixmap	*					m_pixPulse;
	QPixmap	*					m_pixNoise;
	QIcon *						m_iconSine;
	QIcon *						m_iconSawUp;
	QIcon *						m_iconSawDown;
	QIcon *						m_iconPulse;
	QIcon *						m_iconNoise;
};

#endif // MAIN_WND_HPP