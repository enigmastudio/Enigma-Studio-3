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

#include <QtCore/QtAlgorithms>
#include <QtCore/QTextStream>
#include <QtCore/QSettings>

#include <QtXml/QDomDocument>

#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QHeaderView>

#include "mainwnd.hpp"
#include "guioperator.hpp"
#include "trackerseqitem.hpp"

#include "../../configinfo.hpp"

// Initialize static members.
const QString eMainWnd::PROJECT_FILTER = "Enigma Studio 3 projects (*.e3prj)";
const QString eMainWnd::SCRIPT_FILTER = "Enigma Studio 3 script (*.e3scr)";
const QString eMainWnd::EDITOR_CAPTION = QString("Enigma Studio ")+eENIGMA3_VERSION;
const QString eMainWnd::BACKUP_FILENAME = "backup.e3prj";

eMainWnd::eMainWnd()
{
    setupUi(this);

    m_demoSeqView->setRenderer(m_renderView->getRenderer());

    m_instrTable->initRows();
    m_synthCurInstr = 0;
    _synthSetActiveInstrument(0);
    _synthInstrumentsLoadAll();
    _synthInitInstrumentDropdown();
    m_patternEditor->setMainWnd(this);
	m_trackerScopes->setTrackerSeqView(m_trackerSeqView);

    _readSettings();
    _setupViewMenu();
    _setupStatusBar();
    _makeConnections();
	_createRecentFileActs();
    _setCurrentFile("");
    _setStatusText(SBPANE_VIEWTIME, "Time: 00:00:00");
	_createIcons();

    // Create shortcut for demo play/pause button.
    QAction *act = new QAction(this);
    eASSERT(act != eNULL);
    act->setShortcut(QKeySequence("Space"));
    act->setShortcutContext(Qt::WindowShortcut);
    connect(act, SIGNAL(triggered()), m_btnDemoPlayPause, SLOT(click()));
    addAction(act);

    // Group actions from settings menu.
    QActionGroup *ac = new QActionGroup(this);
    eASSERT(ac != eNULL);
    ac->addAction(m_actTexRes50);
    ac->addAction(m_actTexRes100);
    ac->addAction(m_actTexRes200);

    ac = new QActionGroup(this);
    eASSERT(ac != eNULL);
    ac->addAction(m_actLowShadowQuali);
    ac->addAction(m_actMediumShadowQuali);
    ac->addAction(m_actHighShadowQuali);

    // Initialize profiler tree view.
    m_profilerTree->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    m_profilerTree->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    m_profilerTree->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    m_profilerTree->header()->setResizeMode(3, QHeaderView::ResizeToContents);
    m_profilerTree->header()->setSortIndicator(1, Qt::DescendingOrder);

    // Launch timers for updating the statusbar and backuping.
    m_statusBarTimerId = startTimer(250);
    m_backupTimerId = startTimer(AUTO_BACKUP_TIME_MS);

    // Open file from command line if there is at
    // least one additional argument specified next
    // to the application name.
    const QStringList &args = QApplication::instance()->arguments();

    if (args.size() >= 2)
    {
        _loadFromXml(args[1]);
    }
    else
    {
        _loadRecentProject();
    }
}

eMainWnd::~eMainWnd()
{
    killTimer(m_backupTimerId);
    killTimer(m_statusBarTimerId);

    for (eInt i=0; i<m_profItems.size(); i++)
    {
        eSAFE_DELETE(m_profItems[i]);
    }

	_freeIcons();
    _clearProject();
    _writeSettings();
}

ePatternEditor * eMainWnd::getPatternEditor()
{
	return m_patternEditor;
}

eInstrumentList * eMainWnd::getInstrumentList()
{
	return m_instrTable;
}

eScopesView * eMainWnd::getScopesView()
{
	return m_trackerScopes;
}

void eMainWnd::_clearProject()
{
    eDemo::getSynth().setSong(eNULL);

    m_demoSeqView->clear();
    m_pageTree->clear();
    m_songList->clear();

    Q_FOREACH (eGuiOpPage *page, m_guiOpPages)
    {
        eSAFE_DELETE(page);
    }

    m_guiOpPages.clear();

    Q_FOREACH (eTrackerSeqScene *scene, m_trackerSeqScenes)
    {
        eSAFE_DELETE(scene);
    }

    m_trackerSeqScenes.clear();
	_clearAllInstruments();
}

// Sets up the view menu, which has actions
// to show/hide the dockable widgets.
void eMainWnd::_setupViewMenu()
{
    m_viewMenu->addAction(m_renderDock->toggleViewAction());
    m_viewMenu->addAction(m_paramDock->toggleViewAction());
    m_viewMenu->addAction(m_profilerDock->toggleViewAction());
    m_viewMenu->addAction(m_synthDock->toggleViewAction());

    m_renderDock->toggleViewAction()->setShortcut(QKeySequence("Alt+F1"));
    m_paramDock->toggleViewAction()->setShortcut(QKeySequence("Alt+F2"));
    m_profilerDock->toggleViewAction()->setShortcut(QKeySequence("Alt+F3"));
    m_synthDock->toggleViewAction()->setShortcut(QKeySequence("Alt+F4"));
}

// Add panes to statusbar.
void eMainWnd::_setupStatusBar()
{
    // Set minimum width, so statusbar panes
    // can get smaller than they have to be
    // to display all their contents.
    statusBar()->setMinimumWidth(2);

    // Create statusbar labels.
    static const eInt stretchs[SBPANE_COUNT] = {4, 8, 2, 2};

    for (eU32 i=0; i<SBPANE_COUNT; i++)
    {
        QLabel &lbl = m_sbLabels[i];

        lbl.setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
        statusBar()->addPermanentWidget(&lbl, stretchs[i]);
    }
}

void eMainWnd::_makeConnections()
{
    connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(_onAbout()));
    connect(m_fileNewAct, SIGNAL(triggered()), this, SLOT(_onFileNew()));
    connect(m_fileOpenAct, SIGNAL(triggered()), this, SLOT(_onFileOpen()));
    connect(m_fileSaveAct, SIGNAL(triggered()), this, SLOT(_onFileSave()));
    connect(m_fileSaveAsAct, SIGNAL(triggered()), this, SLOT(_onFileSaveAs()));
    connect(m_fileMakeAct, SIGNAL(triggered()), this, SLOT(_onFileMake()));
    connect(m_fileExitAct, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_fullScreenAct, SIGNAL(triggered()), this, SLOT(_onToggleAppFullscreen()));

    connect(m_pageTree, SIGNAL(onPageAdded(eID &)), this, SLOT(_onPageAdded(eID &)));
    connect(m_pageTree, SIGNAL(onPageRemoved(eID)), this, SLOT(_onPageRemoved(eID)));
    connect(m_pageTree, SIGNAL(onPageSwitched(eID)), this, SLOT(_onPageSwitched(eID)));
    connect(m_pageTree, SIGNAL(onPageRenamed(eID, const QString &)), this, SLOT(_onPageRenamed(eID, const QString &)));

    connect(m_pageView, SIGNAL(onOperatorShow(eID)), this, SLOT(_onOperatorShow(eID)));
    connect(m_pageView, SIGNAL(onOperatorAdded(eID)), this, SLOT(_onOperatorAdded(eID)));
    connect(m_pageView, SIGNAL(onOperatorRemoved(eID)), this, SLOT(_onOperatorRemoved(eID)));
    connect(m_pageView, SIGNAL(onOperatorSelected(eID)), this, SLOT(_onOperatorSelected(eID)));
    connect(m_pageView, SIGNAL(onPathOperatorEdit(eID)), this, SLOT(_onPathOperatorEdit(eID)));
    connect(m_pageView, SIGNAL(onDemoOperatorEdit(eID)), this, SLOT(_onDemoOperatorEdit(eID)));
    connect(m_pageView, SIGNAL(onGotoLoadedOperator(eID)), this, SLOT(_onGotoOperator(eID)));
    connect(m_pathView, SIGNAL(onFinishedEditing()), this, SLOT(_onSwitchToPageView()));
    connect(m_demoSeqView, SIGNAL(onFinishedEditing()), this, SLOT(_onSwitchToPageView()));
    connect(m_demoSeqView, SIGNAL(onOperatorSelected(eID)), this, SLOT(_onOperatorSelected(eID)));
    connect(m_paramFrame, SIGNAL(onOperatorChanged(const eParameter *)), this, SLOT(_onOperatorChanged(const eParameter *)));
    connect(m_paramFrame, SIGNAL(onGotoOperator(eID)), this, SLOT(_onGotoOperator(eID)));
    connect(m_renderView, SIGNAL(onToggleFullscreenMode()), this, SLOT(_onToggleViewportFullscreen()));
    connect(m_demoSeqScaleSlider, SIGNAL(valueChanged(int)), this, SLOT(_onDemoSeqScaleChanged(int)));

    connect(m_timelineView, SIGNAL(onTimeChanged(eF32)), this, SLOT(_onTimelineTimeChanged(eF32)));
    connect(m_timeEdit, SIGNAL(textEdited(const QString &)), this, SLOT(_onTimeEditEdited(const QString &)));
    connect(m_pathView, SIGNAL(onTimeChanged(eF32)), this, SLOT(_onPathViewTimeChanged(eF32)));
    connect(m_trackerSeqView, SIGNAL(onTimeChanged(eF32)), this, SLOT(_onTrackerSeqTimeChanged(eF32)));
    connect(m_demoSeqView, SIGNAL(onTimeChanged(eF32)), this, SLOT(_onDemoSeqTimeChanged(eF32)));
    connect(m_timeSlider, SIGNAL(valueChanged(int)), this, SLOT(_onPathViewZoomChanged(int)));
    connect(m_rangeSlider, SIGNAL(valueChanged(int)), this, SLOT(_onPathViewZoomChanged(int)));
    connect(m_showTransCb, SIGNAL(stateChanged(int)), this, SLOT(_onPathViewShowPathsChanged(int)));
    connect(m_showRotCb, SIGNAL(stateChanged(int)), this, SLOT(_onPathViewShowPathsChanged(int)));
    connect(m_showScaleCb, SIGNAL(stateChanged(int)), this, SLOT(_onPathViewShowPathsChanged(int)));
    connect(m_cbInterpol, SIGNAL(currentIndexChanged(int)), this, SLOT(_onPathViewChangeInterpolType(int)));
    connect(m_pathView, SIGNAL(onSelectionChanged(ePathView::GuiWaypoint *)), this, SLOT(_onPathViewSelectionChanged(ePathView::GuiWaypoint *)));
    connect(m_pathView, SIGNAL(onAddWaypoint()), this, SLOT(_onPathViewAddWaypoint()));
    connect(m_pathView, SIGNAL(onRemoveWaypoint(ePathView::GuiWaypoint &)), this, SLOT(_onPathViewRemoveWaypoint(ePathView::GuiWaypoint &)));

    connect(m_btnDemoPlayPause, SIGNAL(clicked()), m_timelineView, SLOT(onDemoPlayPause()));
    connect(m_btnDemoSkipForward, SIGNAL(clicked()), m_timelineView, SLOT(onDemoSkipForward()));
    connect(m_btnDemoSkipBackward, SIGNAL(clicked()), m_timelineView, SLOT(onDemoSkipBackward()));

    connect(m_rowCountCb, SIGNAL(currentIndexChanged(int)), this, SLOT(_onRowCountChanged(int)));
    connect(m_trackCountSb, SIGNAL(valueChanged(int)), this, SLOT(_onTrackCountChanged(int)));
    connect(m_octaveSb, SIGNAL(valueChanged(int)), this, SLOT(_onActiveOctaveChanged(int)));
    connect(m_linesPerBeatSb, SIGNAL(valueChanged(int)), this, SLOT(_onLinesPerBeatChanged(int)));
    connect(m_trackerSeqView, SIGNAL(onPatternInstSelected(tfSong::PatternInstance &)), this, SLOT(_onPatternInstSelected(tfSong::PatternInstance &)));
    connect(m_beatsPerMinuteSb, SIGNAL(valueChanged(int)), this, SLOT(_onBpmChanged(int)));
    connect(m_btnNewPattern, SIGNAL(clicked()), this, SLOT(_onNewPattern()));
    connect(m_btnRemovePattern, SIGNAL(clicked()), this, SLOT(_onRemovePattern()));
    connect(m_btnPatternPlus, SIGNAL(clicked()), this, SLOT(_onPatternPlus()));
    connect(m_btnPatternMinus, SIGNAL(clicked()), this, SLOT(_onPatternMinus()));
    connect(m_btnCleanPatterns, SIGNAL(clicked()), this, SLOT(_onCleanPatterns()));
    connect(m_btnClonePattern, SIGNAL(clicked()), this, SLOT(_onClonePattern()));
    connect(m_btnSynthPanic, SIGNAL(clicked()), this, SLOT(_onPanic()));
    connect(m_btnSynthMute, SIGNAL(clicked(bool)), this, SLOT(_onMute(bool)));
    connect(m_btnSynthLoopPattern, SIGNAL(clicked(bool)), this, SLOT(_onPatternLoop(bool)));
    connect(m_btnSynthMetronome, SIGNAL(clicked(bool)), this, SLOT(_onMetronome(bool)));
    connect(m_btnSynthTransP1, SIGNAL(clicked()), this, SLOT(_onTransposeUpNote()));
    connect(m_btnSynthTransP12, SIGNAL(clicked()), this, SLOT(_onTransposeUpOct()));
    connect(m_btnSynthTransM1, SIGNAL(clicked()), this, SLOT(_onTransposeDownNote()));
    connect(m_btnSynthTransM12, SIGNAL(clicked()), this, SLOT(_onTransposeDownOct()));
    connect(m_hslSynthVolume, SIGNAL(valueChanged(int)), this, SLOT(_onSynthVolumeChanged(int)));
    connect(m_btnTrackerMode, SIGNAL(clicked()), this, SLOT(_onEditing()));
    connect(m_patternVScroll, SIGNAL(valueChanged(int)), this, SLOT(_onPatternVScroll(int)));
    connect(m_patternHScroll, SIGNAL(valueChanged(int)), this, SLOT(_onPatternHScroll(int)));
    connect(m_songList, SIGNAL(onSongAdded(eID)), this, SLOT(_onSongAdded(eID)));
    connect(m_songList, SIGNAL(onSongSwitched(eID)), this, SLOT(_onSongSwitched(eID)));
    connect(m_songList, SIGNAL(onSongRemoved(eID)), this, SLOT(_onSongRemoved(eID)));

    connect(m_instrCreate, SIGNAL(clicked()), this, SLOT(_onAddInstrument()));
    connect(m_instrDelete, SIGNAL(clicked()), this, SLOT(_onRemoveInstrument()));
    connect(m_instrTable, SIGNAL(itemSelectionChanged()), this, SLOT(_onInstrumentChanged()));

    //  Create all synthesizer connections.

	QMenu *presetMenu = new QMenu(this);
	QAction *actionSine = new QAction("Sine", this);
	QAction *actionTriangle = new QAction("Triangle", this);
	QAction *actionSquare = new QAction("Square", this);
	QAction *actionSawUp = new QAction("Saw up", this);
	QAction *actionSawDown = new QAction("Saw down", this);

	presetMenu->addAction(actionSine);
	presetMenu->addAction(actionTriangle);
	presetMenu->addAction(actionSquare);
	presetMenu->addAction(actionSawUp);
	presetMenu->addAction(actionSawDown);
	m_oscPresets->setMenu(presetMenu);

	connect(m_instrSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(_onSynthInstrumentChanged(int)));
	//connect(m_instrRestore, SIGNAL(clicked(bool)), this, SLOT(_progRestore(bool)));
	//connect(m_instrSave, SIGNAL(clicked(bool)), this, SLOT(_progSave(bool)));
	//connect(m_manageInstruments, SIGNAL(clicked(bool)), this, SLOT(_manage(bool)));

    connect(m_gainAmount, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(actionSine, SIGNAL(triggered(bool)), this, SLOT(_presetSine(bool)));
	connect(actionTriangle, SIGNAL(triggered(bool)), this, SLOT(_presetTriangle(bool)));
	connect(actionSquare, SIGNAL(triggered(bool)), this, SLOT(_presetSquare(bool)));
	connect(actionSawUp, SIGNAL(triggered(bool)), this, SLOT(_presetSawUp(bool)));
	connect(actionSawDown, SIGNAL(triggered(bool)), this, SLOT(_presetSawDown(bool)));

	connect(m_gainAmount, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_oscPoints, SIGNAL(currentIndexChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscPolyphony, SIGNAL(currentIndexChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscUni1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni2, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni3, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni4, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni5, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni6, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni7, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni8, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni9, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscUni10, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscSub0, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscSub1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOctm4, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOctm3, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOctm2, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOctm1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOct0, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOct1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOct2, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOct3, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscOct4, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_oscVolume, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscPanning, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscDetune, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscSpread, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscGlide, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscDrive, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_oscSlop, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_addVolume, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_addSingle, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addDetuned, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addGauss, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addSpread, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOctm4, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOctm3, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOctm2, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOctm1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOct0, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOct1, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOct2, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOct3, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addOct4, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_addBandwidth, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_addDamp, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_addHarmonics, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_addScale, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_addDrive, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_noiseAmount, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_noiseFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_noiseBW, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_lpFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lpRes, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lpOn, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_hpFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_hpRes, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_hpOn, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_bpFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_bpQ, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_bpOn, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_ntFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_ntQ, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_ntOn, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_adsr1A, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr1D, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr1S, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr1R, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr1Slope, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_adsr2A, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr2D, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr2S, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr2R, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_adsr2Slope, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_lfo1Rate, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lfo1Depth, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lfo1ShapeSine, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo1ShapeSawUp, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo1ShapeSawDown, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo1ShapePulse, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo1ShapeNoise, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo1Sync, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_lfo2Rate, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lfo2Depth, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_lfo2ShapeSine, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo2ShapeSawUp, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo2ShapeSawDown, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo2ShapePulse, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo2ShapeNoise, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_lfo2Sync, SIGNAL(stateChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_mm1Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm1Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm2Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm2Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm3Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm3Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm4Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm4Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm5Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm5Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm6Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm6Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm7Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm7Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm8Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm8Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm9Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm9Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm10Src, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_mm10Dest, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_mm1Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm2Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm3Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm4Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm5Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm6Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm7Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm8Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm9Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));
	connect(m_mm10Mod, SIGNAL(valueChanged(double)), this, SLOT(_onSynthChanged(double)));

	connect(m_effect1, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect2, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect3, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect4, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect5, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect6, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect7, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect8, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect9, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_effect10, SIGNAL(activated(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_distAmount, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_chorusFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_chorusDepth, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_chorusGain, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_delayLeft, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_delayRight, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_delayDecay, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_revRoomsize, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_revDamp, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_revWet, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_revWidth, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_flangLfo, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_flangFreq, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_flangAmp, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_flangWet, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_formantA, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_formantE, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_formantI, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_formantO, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_formantU, SIGNAL(clicked(bool)), this, SLOT(_onSynthClicked(bool)));
	connect(m_formantWet, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));

	connect(m_eqLow, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_eqMid, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
	connect(m_eqHigh, SIGNAL(valueChanged(int)), this, SLOT(_onSynthChanged(int)));
}

void eMainWnd::_addDefaultPage()
{
    eGuiOpPage *page = new eGuiOpPage("Start page");
    eASSERT(page != eNULL);

    const eID pageId = page->getPage()->getId();

    m_guiOpPages.insert(pageId, page);
    m_pageTree->addPage("Start page", pageId);
    m_pageTree->selectPage(pageId);
}

void eMainWnd::_createRecentFileActs()
{
    QMenu *fileMenu =  menuBar()->actions().at(0)->menu();
    eASSERT(fileMenu != eNULL);

    m_recSepAct = fileMenu->insertSeparator(fileMenu->actions().at(fileMenu->actions().size()-2));
    eASSERT(m_recSepAct != eNULL);
    m_recSepAct->setVisible(false);

    for (eU32 i=0; i<MAX_RECENT_FILES; i++)
    {
        m_recentActs[i] = new QAction(this);
        eASSERT(m_recentActs[i] != eNULL);
        m_recentActs[i]->setVisible(false);
        connect(m_recentActs[i], SIGNAL(triggered()), this, SLOT(_onFileOpenRecent()));
        fileMenu->insertAction(fileMenu->actions().at(fileMenu->actions().size()-2), m_recentActs[i]);
    }

    _updateRecentFileActs();
}

void eMainWnd::_updateRecentFileActs()
{
    QSettings settings;

    settings.beginGroup("Main window");
    const QStringList files = settings.value("Recent files").toStringList();
    const eInt fileCount = eMin(files.size(), (eInt)MAX_RECENT_FILES);

    for (eInt i=0; i<fileCount; i++)
    {
        const QString fileName = QFileInfo(files[i]).fileName();
        const QString text = tr("&%1 %2").arg(i+1).arg(fileName);

        m_recentActs[i]->setText(text);
        m_recentActs[i]->setData(files[i]);
        m_recentActs[i]->setVisible(true);
    }

    for (eInt j=fileCount; j<MAX_RECENT_FILES; j++)
    {
        m_recentActs[j]->setVisible(false);
    }

    m_recSepAct->setVisible(fileCount > 0);
    settings.endGroup();
}

void eMainWnd::_updateStatusBar()
{
    // Update pane with rendering statistics.
    const eRenderStats stats = m_renderView->getRenderer()->getGraphicsApi()->getRenderStats();

    QString buffer;

    buffer.sprintf("%06.2f", stats.fps);
    buffer += " Frames (";
    buffer += QString().sprintf("%05.2f", eProfiler::getLastFrameTimeMs());
    buffer += " ms/Frame), ";
    buffer += eIntToStr(stats.batches);
    buffer += " Batches, ";
    buffer += eIntToStr(stats.triangles);
    buffer += " Triangles, ";
    buffer += eIntToStr(stats.lines);
    buffer += " Lines, ";
    buffer += eIntToStr(stats.vertices);
    buffer += " Vertices, ";
    buffer += eIntToStr(m_renderView->getLastCalcMs());
    buffer += " ms/Processing";
    _setStatusText(SBPANE_STATISTICS, buffer);

    // Update last pane with project information.
    buffer = "Pages: ";
    buffer += eIntToStr(eDemoData::getPageCount());
    buffer += ", Operators: ";
    buffer += eIntToStr(eDemoData::getTotalOpCount());

    _setStatusText(SBPANE_PRJINFOS, buffer);

    // If operator doesn't exists, clear panes.
    eIOperator *op = eDemoData::findOperator(m_renderView->getOperator());

    if (op == eNULL)
    {
        _setStatusText(SBPANE_OPINFOS, "-");
        return;
    }

    // Operator exists => update panes.
    const eString &category = op->getCategory();
    buffer = QString("[")+QString(category)+"] ";

    if (category == "Bitmap")
    {
        const eIBitmapOp::Result &res = ((eIBitmapOp *)op)->getResult();

        buffer += QString("Size is ")+eIntToStr(res.width)+"x"+eIntToStr(res.height)+", ";
        buffer += QString::number(m_renderView->getZoomFactor())+"x zoom";
    }
    else if (category == "Mesh")
    {
        const eIMeshOp::Result &res = ((eIMeshOp *)op)->getResult();
        const eVector3 camPos = m_renderView->getCameraPos();

        QString camAt;
        camAt.sprintf("Camera at (%.2f/%.2f/%.2f), ", camPos.x, camPos.y, camPos.z);

        buffer += camAt;
        buffer += QString(eIntToStr(res.mesh.getVertexCount()))+" Vertices, ";
        buffer += QString(eIntToStr(res.mesh.getFaceCount()))+" Faces";
    }
    else if (category == "Model")
    {
        const eVector3 camPos = m_renderView->getCameraPos();

        QString camAt;
        camAt.sprintf("Camera at (%.2f/%.2f/%.2f)", camPos.x, camPos.y, camPos.z);

        buffer += camAt;
    }

    _setStatusText(SBPANE_OPINFOS, buffer);
}

void eMainWnd::_updateProfilerTree()
{
    // Remove items from last frame from tree-widget.
    for (eInt i=0; i<m_profilerTree->topLevelItemCount(); i++)
    {
        m_profilerTree->takeTopLevelItem(i);
    }

    // Setup tree-widget items for current frame.
    for (eU32 i=0, index=0; i<eProfiler::getZoneCount(); i++)
    {
        const eProfiler::Zone &zone = eProfiler::getZone(i);

        if (zone.getCallCount() != 0)
        {
            // Allocate a new item if we ran out of items.
            if (index >= (eU32)m_profItems.size())
            {
                m_profItems.append(new ProfilerTreeItem);
            }
             
            ProfilerTreeItem *item = m_profItems[index];
            eASSERT(item != nullptr);

            // Add item to profiler list.
            QStringList texts;

            texts << zone.getName();
            texts << QString::number(zone.getSelfTimeMs(), 'f', 3);
            texts << QString::number(zone.getHierTimeMs(), 'f', 3);
            texts << eIntToStr(zone.getCallCount());

            const QColor zoneCol(zone.getColor().toArgb());

            for (eInt j=0; j<m_profilerTree->columnCount(); j++)
            {
                item->setText(j, texts[j]);
                item->setBackground(j, QBrush(zoneCol));
                item->setForeground(j, QBrush(Qt::white));
            }

            m_profilerTree->addTopLevelItem(item);
            index++;
        }
    }
}

void eMainWnd::_setStatusText(StatusBarPane pane, const QString &text)
{
    eASSERT(pane >= 0 && pane < SBPANE_COUNT);
    m_sbLabels[pane].setText(text);
}

void eMainWnd::_saveBackup()
{
#ifndef eDEBUG
    QString fileName = BACKUP_FILENAME;

    // If there is a given project file name,
    // insert '.backup' before file extension.
    if (m_prjFilePath != "")
    {
        const eInt extSepIndex = m_prjFilePath.lastIndexOf(".");
        eASSERT(extSepIndex != -1);

        fileName = m_prjFilePath;
        fileName.insert(extSepIndex, ".backup");
    }

    _saveToXml(fileName, eTRUE);
#endif
}

eBool eMainWnd::_askForSaving()
{
    if (isWindowModified())
    {
        switch (QMessageBox::warning(this, "Warning",
                                     "The document has been modified.\n"
                                     "Do you want to save your changes?",
                                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                     QMessageBox::Save))
        {
            case QMessageBox::Save:
            {
                return _onFileSave();
            }

            case QMessageBox::Cancel:
            {
                return eFALSE;
            }
        }
    }

    return eTRUE;
}

void eMainWnd::_setCurrentFile(const QString &filePath)
{
    QString filePathFinal = filePath;

    if (filePathFinal != "")
    {
        QFile file(filePathFinal + ".orig");

        if (file.open(QIODevice::ReadOnly))
        {
            filePathFinal = QString(file.readLine());
            file.close();
        }
    }
    
    m_prjFilePath = filePathFinal;
    setWindowModified(false);

    // Update recent files list in settings.
    if (filePathFinal != "")
    {
        QSettings settings;

        settings.beginGroup("Main window");
        QStringList files = settings.value("Recent files").toStringList();
        files.removeAll(filePathFinal);
        files.prepend(filePathFinal);

        while (files.size() > MAX_RECENT_FILES)
        {
            files.removeLast();
        }

        settings.setValue("Recent files", files);
        settings.endGroup();

        // Update recent file actions and
        // window title.
        setWindowTitle(EDITOR_CAPTION+" - ["+filePathFinal+"[*]]");
        _updateRecentFileActs();
    }
    else
    {
        setWindowTitle(EDITOR_CAPTION+" - [untitled.e3prj[*]]");
    }
}

eBool eMainWnd::_loadFromXml(const QString &filePath)
{
    // Load in project.
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't open project file!");
        return eFALSE;
    }

    // Load XML data and check for correct version.
    QDomDocument xml;
    xml.setContent(&file);
    file.close();

    if (xml.firstChildElement("enigma").attribute("version") != eENIGMA3_VERSION)
    {
        if (QMessageBox::warning(this,
                                 "Warning",
                                 "This project was created using another version of Enigma Studio 3.\n"\
                                 "Do you want to try opening this project anyway (could crash)?",
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        {
            return eFALSE;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Clear all yet existing data.
    _newProject(eFALSE);

    // Load pages and operators.
    const QDomElement rootEl = xml.documentElement();
    QDomElement pagesEl = rootEl.firstChildElement("pages").firstChildElement("page");

    while (!pagesEl.isNull())
    {
        const eID pageId = pagesEl.attribute("id").toInt();
        eOperatorPage *opPage = eDemoData::addPage(pageId);
        eASSERT(opPage != eNULL);

        eGuiOpPage *guiPage = new eGuiOpPage(opPage);
        eASSERT(guiPage != eNULL);

        m_guiOpPages.insert(pageId, guiPage);
        guiPage->loadFromXml(pagesEl);

        pagesEl = pagesEl.nextSiblingElement("page");
    }

    // Finally connect operators on all pages
    // and load tree-view items from XML.
    m_pageTree->loadFromXml(rootEl);
    _setCurrentFile(filePath);
    eDemoData::updateAllPageLinks();

    // Select first page in tree-view.
    if (m_pageTree->topLevelItemCount() > 0)
    {
        m_pageTree->topLevelItem(0)->setSelected(true);
    }

    // Load song list data.
    QDomElement songlistEl = rootEl.firstChildElement("songlist");
    m_songList->loadFromXml(songlistEl);

    // Save all song data.
    QDomElement songEl = rootEl.firstChildElement("songs").firstChildElement("song");

    for (eTrackerSeqScenePtrIdMap::iterator iter=m_trackerSeqScenes.begin(); iter!=m_trackerSeqScenes.end(); iter++)
    {
        iter.value()->loadFromXml(songEl);
        songEl = songEl.nextSiblingElement("song");
    }

    // Load instruments
    QDomElement instrumentsEl = rootEl.firstChildElement("instruments");
    m_instrTable->loadFromXml(instrumentsEl);

    QApplication::restoreOverrideCursor();
    setWindowModified(false);
    return eTRUE;
}

eBool eMainWnd::_saveToXml(const QString &filePath, eBool backup)
{
    // Try to open file.
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly))
    {
        return eFALSE;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Initialize XML stream.
    QDomDocument xml;
    xml.appendChild(xml.createProcessingInstruction("xml", "version=\"1.0\""));
    QDomElement rootEl = xml.createElement("enigma");
    rootEl.setAttribute("version", eENIGMA3_VERSION);
    xml.appendChild(rootEl);

    // Save all pages with their operators.
    QDomElement pagesEl = xml.createElement("pages");
    rootEl.appendChild(pagesEl);

    for (eGuiOpPagePtrIdMap::iterator iter=m_guiOpPages.begin(); iter!=m_guiOpPages.end(); iter++)
    {
        iter.value()->saveToXml(pagesEl);
    }

    // Write page tree-view data.
    m_pageTree->saveToXml(rootEl);

    // Save all song data.
    QDomElement songsEl = xml.createElement("songs");
    rootEl.appendChild(songsEl);

    for (eTrackerSeqScenePtrIdMap::iterator iter=m_trackerSeqScenes.begin(); iter!=m_trackerSeqScenes.end(); iter++)
    {
        iter.value()->saveToXml(songsEl);
    }

    // Save song list data.
    m_songList->saveToXml(rootEl);

    // Save instruments
    m_instrTable->saveToXml(rootEl);

    // Finally write XML stream to text file.
    QTextStream ts(&file);
    xml.save(ts, 2);
    file.close();

    if (!backup)
    {
        _setCurrentFile(filePath);
    }

    QApplication::restoreOverrideCursor();
    return eTRUE;
}

void eMainWnd::_loadRecentProject()
{
    if (QFile::exists(m_lastPrjPath) && _loadFromXml(m_lastPrjPath))
    {
        return;
    }
    else
    {
	    const QString defaultFilePath = QApplication::applicationDirPath()+"/default.e3prj";

	    if (QFile::exists(defaultFilePath))
        {
		    _loadFromXml(defaultFilePath);
            return;
        }
    }

    _addDefaultPage();
}

void eMainWnd::_newProject(eBool loadDefaultPrj)
{
    _clearProject();

	if (loadDefaultPrj)
    {
		_loadRecentProject();
    }

    m_paramFrame->setOperator(eNOID);
    m_renderView->setOperator(eNOID);

    eGuiOperator::setViewingOp(eNOID);
    eGuiOperator::setEditingOp(eNOID);

    setWindowModified(false);
}

void eMainWnd::_writeSettings() const
{
    QSettings settings;

    settings.beginGroup("Main window");
    settings.setValue("Geometry", saveGeometry());
    settings.setValue("State", saveState());
    settings.setValue("Tracker state", m_trackerSplitter->saveState());
    settings.setValue("Last project", m_prjFilePath);
    settings.endGroup();
}

void eMainWnd::_readSettings()
{
    QSettings settings;

    settings.beginGroup("Main window");
    restoreGeometry(settings.value("Geometry").toByteArray());
    restoreState(settings.value("State").toByteArray()); 
    m_trackerSplitter->restoreState(settings.value("Tracker state").toByteArray());
    m_lastPrjPath = settings.value("Last project", "").toString();
    settings.endGroup();
}

// Displays HTML formatted about dialog.
void eMainWnd::_onAbout()
{
    static QString text = QString(
        "<html>"
          "<body>"
            "<b>Enigma Studio %1 (%2)</b><br>"
            "<br>"
            "Copyright &copy; 2003-2010 by Brain Control,<br>"
            "all rights reserved."
            "<br>"
            "<table width=""240"" height=""100%"">"
              "<tr>"
                "<td><i>Programming:</i></td>"
                "<td>&bull; David 'hunta' Geier</td>"
              "</tr>"
              "<tr>"
                "<td></td>"
                "<td>&bull; Chris 'payne' Loos</td>"
              "</tr>"
              "<tr>"
                "<td></td>"
                "<td>&bull; Martin 'pap' Raack</td>"
              "</tr>"
            "</table>"
            "<br>"
            "<br>"
            "Visit us under <a href=""http://www.braincontrol.org"">www.braincontrol.org</a>."
          "</body>"
        "</html>").arg(eENIGMA3_VERSION, QString(eENIGMA3_CONFIG));

    QMessageBox::about(this, "About", text);
}

void eMainWnd::_onFileNew()
{
    if (_askForSaving())
    {
        _newProject(eFALSE);
        _setCurrentFile("");
    }
}

void eMainWnd::_onFileOpen()
{
    if (_askForSaving())
    {
        const QString filePath = QFileDialog::getOpenFileName(this, "", "", PROJECT_FILTER);

        if (filePath != "")
        {
            _loadFromXml(filePath);
        }
    }
}

eBool eMainWnd::_onFileSave()
{
    if (m_prjFilePath == "")
    {
        return _onFileSaveAs();
    }

    return _saveToXml(m_prjFilePath);
}

eBool eMainWnd::_onFileSaveAs()
{
    const QString filePath = QFileDialog::getSaveFileName(this, "", "", PROJECT_FILTER);

    if (filePath == "")
    {
        return eFALSE;
    }

    return _saveToXml(filePath);
}

void eMainWnd::_onFileMake()
{
    // Check if there is a demo operator currently selected.
    if (m_pageView->scene() == eNULL ||
        m_pageView->scene()->selectedItems().size() != 1 ||
        ((eGuiOperator *)m_pageView->scene()->selectedItems().at(0))->getOperator()->getType() != "Misc : Demo")
    {
        QMessageBox::information(this, "Information", "You have to select a demo operator first!");
        return;
    }

    // Retrieve currently selected demo operator.
    eIDemoOp *demoOp = (eIDemoOp *)((eGuiOperator *)m_pageView->scene()->selectedItems().at(0))->getOperator();
    eASSERT(demoOp != eNULL);

    // Create script of selected demo operator.
    const QString filePath = QFileDialog::getSaveFileName(this, "", "", SCRIPT_FILTER);

    if (filePath == "")
    {
        return;
    }

    eDemoScript script;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    eDemoData::store(script, demoOp);
    eString usedOpsCpp = eDemoData::getUsedOpCpp();

    // Store binary file.
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't create script file!");
        return;
    }

    const eByteArray &data = script.getFinalScript();
    file.write((const eChar *)&data[0], data.size());
    file.close();

    // Store header file of used operators.
    QFile opsFile(filePath+".ops.h");

    if (!opsFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't create header file of used operators!");
        return;
    }

    opsFile.write(script.getUsedOpNames());
    opsFile.close();

    // Create Generator File.
    QFile opsGenFile("productionsources.gen");

    if (!opsGenFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't create generator file !");
        return;
    }

    opsGenFile.write(usedOpsCpp);
    opsGenFile.close();

    // call generator
    system("java -jar ./eplayercppcollector.jar");

    // Store script as header file with hex array.
    QString hex = "const unsigned char data[] =\r\n{\r\n    ";

    for (eU32 i=0; i<data.size(); i++)
    {
        hex += "0x";
        hex += QString::number(data[i], 16).rightJustified(2, '0');
        hex += ", ";

        if ((i+1)%16 == 0)
        {
            hex += "\r\n    ";
        }
    }

    hex += "\r\n};";

    QFile hexFile(filePath + ".bin.h");

    if (!hexFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't create hex header file!");
        return;
    }

    hexFile.write(hex.toAscii());
    hexFile.close();


    // Store statistics file .
    QFile statsFile(filePath+".stats");

    if (!statsFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, "Error", "Couldn't create stats file!");
        return;
    }
    statsFile.write("TotalSize (Nr of Instances) Avg. Instance Size : Op Type\r\n");
    eU32 opsTotal = 0;
    eU32 opsTotalSize = 0;
    for(eU32 i = 0; i < eDemoData::stats.size(); i++) {
        eDemoData::tStatRecord& stat = eDemoData::stats[i];
        QString s = QString::number(stat.accsize) + "  (" + QString::number(stat.cnt) + ")  " + QString::number((eF32)stat.accsize / (eF32)stat.cnt) + " : ";
        statsFile.write(s.toAscii());
        statsFile.write((*stat.name));
        statsFile.write("\r\n");
        opsTotal += stat.cnt;
        opsTotalSize += stat.accsize;
    }
    QString s = QString::number(opsTotal) + " operators = " + QString::number(opsTotalSize) + " bytes";
    statsFile.write(s.toAscii());
    statsFile.write("\r\n");

    statsFile.close();


    QApplication::restoreOverrideCursor();
}

void eMainWnd::_onFileOpenRecent()
{
    const QAction *action = qobject_cast<QAction *>(sender());
    eASSERT(action != eNULL);

    if (_askForSaving())
    {
        _loadFromXml(action->data().toString());
    }
}

void eMainWnd::_onPageAdded(eID &pageId)
{
    eGuiOpPage *page = new eGuiOpPage("New page");
    eASSERT(page != eNULL);
    pageId = page->getPage()->getId();
    m_guiOpPages.insert(pageId, page);
    m_pageTree->selectPage(page->getPage()->getId());

    setWindowModified(true);
}

void eMainWnd::_onPageRemoved(eID pageId)
{
    eASSERT(pageId != eNOID);

    if (m_guiOpPages.contains(pageId))
    {
        eGuiOpPage *guiPage = m_guiOpPages.value(pageId);
        eASSERT(guiPage != eNULL);
        eSAFE_DELETE(guiPage);
        m_guiOpPages.remove(pageId);
        setWindowModified(true);
    }
}

void eMainWnd::_onPageSwitched(eID pageId)
{
    if (pageId == eNOID)
    {
        m_pageView->setScene(eNULL);
        return;
    }

    if (m_guiOpPages.contains(pageId))
    {
        eGuiOpPage *guiPage = m_guiOpPages.value(pageId);
        eASSERT(guiPage != eNULL);

        m_pageView->setScene(guiPage);
        m_pageView->scrollTo(guiPage->getViewPosition());
        m_pageView->scene()->invalidate();
    }
}

void eMainWnd::_onPageRenamed(eID pageId, const QString &newName)
{
    eASSERT(pageId != eNOID);

    eOperatorPage *page = eDemoData::getPageById(pageId);
    eASSERT(page != eNULL);
    page->setUserName(eString(newName.toAscii()));

    setWindowModified(true);
}

void eMainWnd::_onOperatorShow(eID opId)
{
    m_renderView->setOperator(opId);
}

// A new operator was added, so display its
// parameters in parameter frame.
void eMainWnd::_onOperatorAdded(eID opId)
{
    m_paramFrame->setOperator(opId);
    setWindowModified(true);
}

void eMainWnd::_onOperatorRemoved(eID opId)
{
    // Is this operator shown in parameter frame?
    // Yes => set current operator to 0.
    if (m_paramFrame->getOperator() == opId)
    {
        m_paramFrame->setOperator(eNOID);
    }

    setWindowModified(true);
}

void eMainWnd::_onOperatorSelected(eID opId)
{
    m_paramFrame->setOperator(opId);
}

void eMainWnd::_onOperatorChanged(const eParameter *param)
{
    // Parameter is null => name changed, or hide
    // or bypass buttons clicked (repaint needed).
    // Otherwise, if changed parameter is a link
    // repaint, too.
    if (param == eNULL || param->getType() == eParameter::TYPE_LINK)
    {
        m_pageView->scene()->invalidate();
    }

    setWindowModified(true);

    // Only save backup, if the currently changed
    // operator is different to the last changed operator.
    // This way dragging an operator doesn't slow
    // down the tool too much, because of time needed
    // for saving.
    if (param)
    {
        static const eIOperator *lastSavedOp = eNULL;
        const eIOperator *curOp = param->getOwnerOp();

        if (lastSavedOp != curOp)
        {
            lastSavedOp = curOp;
        }
    }
}

void eMainWnd::_onGotoOperator(eID opId)
{
    eIOperator *op = eDemoData::findOperator(opId);

    if (op == eNULL)
    {
        return;
    }

    const eID pageId = op->getOwnerPage()->getId();
    eASSERT(m_guiOpPages.contains(pageId) == true);

    eGuiOpPage *guiPage = m_guiOpPages.value(pageId);
    eASSERT(guiPage != eNULL);
    eGuiOperator *guiOp = guiPage->getGuiOperator(opId);
    eASSERT(guiOp != eNULL);

    m_pageTree->unselectAll();
    m_pageTree->selectPage(pageId);
    m_pageView->setScene(guiPage);
    m_pageView->scene()->invalidate();
    m_pageView->gotoOperator(guiOp);
}

void eMainWnd::_onPathOperatorEdit(eID opId)
{
    m_pathView->setPathOpId(opId);
    m_tabWidget->setCurrentWidget(m_tabPathView);
}

void eMainWnd::_onDemoOperatorEdit(eID opId)
{
    m_demoSeqView->setDemoOpId(opId);
    m_tabWidget->setCurrentWidget(m_tabDemoSeqView);
}

void eMainWnd::_onSwitchToPageView()
{
    m_tabWidget->setCurrentWidget(m_tabPageView);
}

void eMainWnd::_onToggleAppFullscreen()
{
    if (isFullScreen())
    {
        showNormal();
    }
    else
    {
        showFullScreen();
    }
}

// Toggles viewport (render frame) fullscreen state,
// by hiding/showing all other widgets on main window
// except the render frame (of course) and the statusbar.
void eMainWnd::_onToggleViewportFullscreen()
{
    // Depending on visibility of central
    // widget toggle fullscreen mode.
    const eBool fullScreen = centralWidget()->isVisible();

    // Store main window's dock widget states,
    // before hiding them.
    static QByteArray state;

    if (fullScreen)
    {
        state = saveState();

        centralWidget()->setVisible(false);
        m_paramDock->close();
        m_profilerDock->close();
        m_synthDock->close();
    }
    else
    {
        centralWidget()->setVisible(true);
        restoreState(state);
    }
}

void eMainWnd::_onDemoSeqScaleChanged(int value)
{
    m_demoSeqView->setScale((eF32)value/(eF32)m_demoSeqScaleSlider->maximum());
}

void eMainWnd::_onDemoSeqTimeChanged(eF32 time)
{
    _onTimelineTimeChanged(time);
}

void eMainWnd::_onTrackerSeqTimeChanged(eF32 time)
{
    _onTimelineTimeChanged(time);
}

void eMainWnd::_onPathViewTimeChanged(eF32 time)
{
    _onTimelineTimeChanged(time);
}

void eMainWnd::_onTimelineTimeChanged(eF32 time)
{
    m_trackerSeqView->setTime(time);
    m_demoSeqView->setTime(time);
    m_renderView->setTime(time);
    m_pathView->setTime(time);
    m_timelineView->setTime(time);
    m_patternEditor->setCursorRowViaTime(time);
	m_trackerScopes->scene()->invalidate();

    // Update pane with current view time.
    const eU32 mins = (eU32)(time/60.0f);
    const eU32 secs = (eU32)time%60;
    const eU32 hund = (eU32)((time-(eU32)time)*100.0f);

    QString buffer;

    buffer.sprintf("%.2i:%.2i:%.2i", mins, secs, hund);
    _setStatusText(SBPANE_VIEWTIME, QString("Time: ")+buffer);
    m_timeEdit->setText(buffer);
}

void eMainWnd::_onTimeEditEdited(const QString &text)
{
    const eU32 mins = text.mid(0, 2).toInt();
    const eU32 secs = text.mid(3, 2).toInt();
    const eU32 hund = text.mid(6, 2).toInt();

    const eF32 time = (eF32)mins*60.0f+(eF32)secs+(eF32)hund/100.0f;

    _onTimelineTimeChanged(time);
}

void eMainWnd::_onPathViewZoomChanged(int value)
{
    const eVector2 zoom((eF32)m_timeSlider->value()/(eF32)m_timeSlider->maximum(),
                        (eF32)m_rangeSlider->value()/(eF32)m_rangeSlider->maximum());

    m_pathView->setZoom(zoom);
}

void eMainWnd::_onPathViewShowPathsChanged(int state)
{
    const eBool trans = m_showTransCb->isChecked();
    const eBool rot = m_showRotCb->isChecked();
    const eBool scale = m_showScaleCb->isChecked();

    m_pathView->setVisiblePaths(trans, rot, scale);
}

void eMainWnd::_onPathViewChangeInterpolType(int index)
{
    m_pathView->setSelectedWpInterpolType((ePath::InterpolationType)index);
}

void eMainWnd::_onPathViewSelectionChanged(ePathView::GuiWaypoint *guiWp)
{
    m_btnRemoveWp->setEnabled(true);

    if (guiWp && guiWp->pathOp->getType() != "Path : Shader WP")
    {
        m_lblInterpol->setEnabled(true);
        m_cbInterpol->setEnabled(true);

        m_cbInterpol->setCurrentIndex(guiWp->pathOp->getParameter(1).getValue().enumSel);
    }
    else
    {
        m_btnRemoveWp->setEnabled(false);
        m_lblInterpol->setEnabled(false);
        m_cbInterpol->setEnabled(false);
    }
}

void eMainWnd::_onPathViewAddWaypoint()
{
    eGuiOpPage *guiOpPage = (eGuiOpPage *)m_pageView->scene();
    eASSERT(guiOpPage != eNULL);
    eIOperator *pathOp = m_pathView->getPathOp();
    eASSERT(pathOp != eNULL);

    // Try finding a free input position of the path operator.
    ePoint pos;
    
    if (pathOp->getFreeInputPosition(pos))
    {
        // Find out waypoint type to add.
        QString opType = "Path : Full WP";

        if (pathOp->getInputCount() > 0)
        {
            opType = pathOp->getInputOperator(pathOp->getInputCount()-1)->getType();
        }

        // Add new waypoint operator at free position.
        eGuiOperator *guiOp = new eGuiOperator(opType, pos, guiOpPage);
        eASSERT(guiOp != eNULL);
        guiOpPage->addItem(guiOp);
        guiOp->getOperator()->getParameter(0).getValue().flt = m_pathView->getTime();

        // Copy current camera position intro operator.
        if (opType == "Full WP")
        {
            guiOp->getOperator()->getParameter(2).getValue().fxyz = m_renderView->getCameraPos();
            guiOp->getOperator()->getParameter(3).getValue().fxyz = m_renderView->getCameraLookAt();
        }

        setWindowModified(true);
    }
}

void eMainWnd::_onPathViewRemoveWaypoint(ePathView::GuiWaypoint &guiWp)
{
    // Iterate over all pages.
    for (eGuiOpPagePtrIdMap::iterator iter=m_guiOpPages.begin(); iter!=m_guiOpPages.end(); iter++)
    {
        // Iterate over all operators on current page.
        const eGuiOpPage *guiOpPage = (eGuiOpPage *)iter.value();
        eASSERT(guiOpPage != eNULL);

        for (eInt i=0; i<guiOpPage->items().size(); i++)
        {
            // Operator found?
            eGuiOperator *guiOp = (eGuiOperator *)guiOpPage->items().at(i);
            eASSERT(guiOp != eNULL);

            if (guiOp->getOperator() == guiWp.pathOp)
            {
                // Yes => free memory.
                eSAFE_DELETE(guiOp);
                setWindowModified(true);
                return;
            }
        }
    }
}

void eMainWnd::_onTrackCountChanged(int trackCount)
{
    m_patternEditor->setTrackCount(trackCount);
}

void eMainWnd::_onActiveOctaveChanged(int activeOctave)
{
    m_patternEditor->setActiveOctave((eU32)activeOctave);
}

void eMainWnd::_onLinesPerBeatChanged(int linesPerBeat)
{
    m_patternEditor->setLinesPerBeat((eU32)linesPerBeat);
}

void eMainWnd::_onRowCountChanged(int index)
{
    const eU32 rowCount = m_rowCountCb->currentText().toInt();

    m_patternEditor->setRowCount(rowCount);
    m_trackerSeqView->viewport()->update();
}

void eMainWnd::_onEditing()
{
    m_patternEditor->setEditing(m_btnTrackerMode->isChecked());
}

void eMainWnd::_onPatternInstSelected(tfSong::PatternInstance &pi)
{
    tfSong::Pattern &p = *pi.pattern;

    const eInt itemIndex = m_rowCountCb->findText(QString::number(p.getRowCount()));
    eASSERT(itemIndex != -1);

    m_patternEditor->setPatternInstance(&pi);
    m_rowCountCb->setCurrentIndex(itemIndex);
    m_trackCountSb->setValue(p.getTrackCount());
}

void eMainWnd::_onBpmChanged(int bpm)
{
    eASSERT(bpm > 0);

    if (m_trackerSeqView->getSong())
    {
        m_trackerSeqView->getSong()->setBpm(bpm);
    }
}

void eMainWnd::_onNewPattern()
{
    m_trackerSeqView->cloneSelectedInstances();
}

void eMainWnd::_onRemovePattern()
{
    if (m_trackerSeqView->scene() == eNULL)
        return;

    for (eInt i=0; i<m_trackerSeqView->scene()->selectedItems().size(); i++)
    {
        eTrackerSeqItem *item = (eTrackerSeqItem *)m_trackerSeqView->scene()->selectedItems().at(i);
        eASSERT(item != eNULL);

        if (item->getPatternInstance() == m_patternEditor->getPatternInstance())
        {
            m_patternEditor->setPatternInstance(eNULL);
            break;
        }
    }

    m_trackerSeqView->removeSelectedInstances();
}

void eMainWnd::_onCleanPatterns()
{
    if (QMessageBox::warning(this, "Warning",
                            "Really clean up unused patterns?",
                            QMessageBox::Yes | QMessageBox::Cancel,
                            QMessageBox::Cancel) == QMessageBox::Yes)
    {
        m_trackerSeqView->cleanPatterns();
    }
}

void eMainWnd::_onClonePattern()
{
    m_trackerSeqView->cloneSelectedPatterns();
}

void eMainWnd::_onPatternPlus()
{
    m_trackerSeqView->patternPlus();
}

void eMainWnd::_onPatternMinus()
{
    m_trackerSeqView->patternMinus();
}

void eMainWnd::_onPanic()
{
    eDemo::getSynth().panic();
}

void eMainWnd::_onMute(bool clicked)
{
    eDemo::getSynth().mute(clicked);
}

void eMainWnd::_onPatternLoop(bool clicked)
{
	tfSong::PatternInstance *pi = m_patternEditor->getPatternInstance();
	tfSong *song = eDemo::getSynth().getSong();

	if (pi && song)
	{
        if (!m_timelineView->isPlaying())
        {
		    eU32 loopStartRow = pi->rowOffset;
		    eU32 loopEndRow = loopStartRow+pi->pattern->getRowCount();
			
		    eF32 loopStartTime = song->rowToTime(loopStartRow);
		    eF32 loopEndTime = song->rowToTime(loopEndRow);

		    m_timelineView->setLoop(loopStartTime, loopEndTime);
		    eDemo::getSynth().setLoop(loopStartRow, loopEndRow);

            m_btnDemoPlayPause->setChecked(true);
        }
        else
        {
            m_btnDemoPlayPause->setChecked(false);
        }

        m_timelineView->onDemoPlayPause();
	}
}

void eMainWnd::_onMetronome(bool clicked)
{

}

void eMainWnd::_onTransposeUpNote()
{

}

void eMainWnd::_onTransposeUpOct()
{

}

void eMainWnd::_onTransposeDownNote()
{

}

void eMainWnd::_onTransposeDownOct()
{

}

void eMainWnd::_onSynthVolumeChanged(int value)
{
    eDemo::getSynth().setVolume((eF32)value / 50.0f);
}

void eMainWnd::_onSongAdded(eID songId)
{
    eASSERT(songId != eNOID);

    tfSong *song = eDemoData::getSongById(songId);
    eASSERT(song != eNULL);
    eTrackerSeqScene *seqScene = new eTrackerSeqScene(*song, m_trackerSeqView);
    eASSERT(seqScene != eNULL);
    
    m_trackerSeqScenes.insert(song->getId(), seqScene);

    setWindowModified(true);
}

void eMainWnd::_onSongRemoved(eID songId)
{
    eASSERT(songId != eNOID);

    if (m_trackerSeqScenes.contains(songId))
    {
        eTrackerSeqScene *seqScene = m_trackerSeqScenes.value(songId);
        eASSERT(seqScene != eNULL);

        // Is a pattern of the currently deleted song
        // shown in the pattern editor? Yes => don't show.
        const tfSong &song = seqScene->getSong();

        for (eU32 i=0; i<song.getPatternInstanceCount(); i++)
        {
            if (&song.getPatternInstance(i) == m_patternEditor->getPatternInstance())
            {
                m_patternEditor->setPatternInstance(eNULL);
                break;
            }
        }

        // Remove the song.
        eSAFE_DELETE(seqScene);
        m_trackerSeqScenes.remove(songId);
        setWindowModified(true);
    }
}

void eMainWnd::_onSongSwitched(eID songId)
{
    if (songId == eNOID)
    {
        m_pageView->setScene(eNULL);
        return;
    }

    if (m_trackerSeqScenes.contains(songId))
    {
        eTrackerSeqScene *seqScene = m_trackerSeqScenes.value(songId);
        eASSERT(seqScene != eNULL);

        m_trackerSeqView->setScene(seqScene);

		tfSong &song = seqScene->getSong();

		m_beatsPerMinuteSb->setValue(song.getBpm());
    }
}

void eMainWnd::closeEvent(QCloseEvent *ce)
{
    QMainWindow::closeEvent(ce);

    // Only ask for saving in release mode. When
    // testing in debug mode it's just nerving.
#ifdef eRELEASE
    if (!_askForSaving())
    {
        ce->ignore();
        return;
    }
#endif

    ce->accept();

    // Stop playback that synthesizer is quiet.
    if (m_timelineView->isPlaying())
    {
        m_timelineView->onDemoPlayPause();
    }
}

void eMainWnd::timerEvent(QTimerEvent *te)
{
    QMainWindow::timerEvent(te);

    if (te->timerId() == m_statusBarTimerId)
    {
        _updateStatusBar();
        _updateProfilerTree();

        // Check for shader changes.
        eShaderManager::update();
    }
    else if (te->timerId() == m_backupTimerId)
    {
        _saveBackup();
    }
}

void eMainWnd::_onInstrumentChanged()
{
    if (m_instrTable->selectedItems().count() == 0)
    {
        return;
    }

    QTableWidgetItem *item = m_instrTable->selectedItems().at(0);
    eU32 row = item->row();
    _synthSetActiveInstrument(row);
}

void eMainWnd::_onRemoveInstrument()
{
    if (m_instrTable->selectedItems().count() == 0)
        return;

    QTableWidgetItem *item = m_instrTable->selectedItems().at(0);
    eU32 row = item->row();
    tfPlayer *player = &eDemo::getSynth();
    player->removeInstrument(row);
    _synthSetActiveInstrument(row);
    _synthUpdateInstrumentName();
}

void eMainWnd::_clearAllInstruments()
{
	tfPlayer &player = eDemo::getSynth();
    player.stop();
	player.panic();
	player.clearInstruments();

	for (eU32 i=0; i<TF_MAX_INPUTS; i++)
    {
		QTableWidgetItem *item = m_instrTable->item(i, 0);
		eASSERT(item != eNULL);
		item->setForeground(QBrush(Qt::gray));
		item->setText("<empty>");		

	}

	_synthSetActiveInstrument(0);
	_synthUpdateInstrumentName();
}

void eMainWnd::_onAddInstrument()
{
    if (m_instrTable->selectedItems().count() == 0)
    {
        return;
    }

    const QTableWidgetItem *item = m_instrTable->selectedItems().at(0);
    eASSERT(item != eNULL);
    const eU32 row = item->row();

    eDemo::getSynth().addInstrument(row);
    _synthSetActiveInstrument(row);
    _synthUpdateInstrumentName();
	_onSynthInstrumentChanged(0);
}

void eMainWnd::_synthSetActiveInstrument(eU32 index)
{
    m_synthCurrentInstrument = index;
    m_patternEditor->setActiveInstrument(index);
    m_activeInstr = eDemo::getSynth().getInstrument(index);
    m_synthAreaContents->setEnabled(m_activeInstr != eNULL);
	m_oscView->setSynth(m_activeInstr);

    _synthInitParameters();
    _synthUpdateOscView();
}

tfInstrument * eMainWnd::_synthGetActiveInstrument()
{
    return m_activeInstr;
}

void eMainWnd::_synthInitParameters()
{
    if (!m_activeInstr)
        return;

    for(eU32 i=0;i<TF_PARAM_COUNT;i++)
    {
        //if (i == TF_FORMANT_WET) __asm int 3;

        eF32 value = m_activeInstr->getParam(i);
        _synthSetParameter(i, value);
    }
}

void eMainWnd::_synthInitInstrumentDropdown()
{
    for(eU32 i=0;i<TF_NUM_PRESETS;i++)
    {
        m_instrSelection->addItem(QString(m_synthInstrs[i].name));
    }
}

void eMainWnd::_synthUpdateOscView()
{
    m_oscView->update();
}

eU32 eMainWnd::_synthToIndex(eF32 value, eU32 min, eU32 max)
{
    return (eU32)eRound(value * (max - min)) + min;
}

eF32 eMainWnd::_synthFromIndex(eU32 value, eU32 min, eU32 max)
{
    return (eF32)(value - min) / (max - min);
}

void eMainWnd::_synthSetParameter(eU32 index, eF32 value)
{
	eU32 knob = eRound(value*99);

    if (m_activeInstr)
        m_activeInstr->setIgnoreParamUI(eTRUE);

	switch(index)
	{
	case TF_GAIN_AMOUNT:                m_gainAmount->setValue(knob); break;
	case TF_OSC_POINTCOUNT:             m_oscPoints->setCurrentIndex(_synthToIndex(value, 0, TF_OSCILLATOR_POINTS-3)); break;
	case TF_OSC_VOLUME:                 m_oscVolume->setValue(knob); break;
	case TF_OSC_FREQ:                   m_oscFreq->setValue(knob); break;
	case TF_OSC_PAN:                    m_oscPanning->setValue(knob); break;
	case TF_OSC_DETUNE:                 m_oscDetune->setValue(knob); break;
	case TF_OSC_POLYPHONY:              m_oscPolyphony->setCurrentIndex(_synthToIndex(value, 0, TF_MAXVOICES-1)); break;
	case TF_OSC_UNISONO:                
		{
			switch(_synthToIndex(value, 0, TF_MAXUNISONO-1))
			{
			case 0: m_oscUni1->setChecked(true); break;
			case 1: m_oscUni2->setChecked(true); break;
			case 2: m_oscUni3->setChecked(true); break;
			case 3: m_oscUni4->setChecked(true); break;
			case 4: m_oscUni5->setChecked(true); break;
			case 5: m_oscUni6->setChecked(true); break;
			case 6: m_oscUni7->setChecked(true); break;
			case 7: m_oscUni8->setChecked(true); break;
			case 8: m_oscUni9->setChecked(true); break;
			case 9: m_oscUni10->setChecked(true); break;
			}
			break;
		}
	case TF_OSC_SPREAD:                 m_oscSpread->setValue(knob); break;
	case TF_OSC_SUBOSC:                 
		{
			switch (_synthToIndex(value, 0, TF_MAXSUBOSC))
			{
			case 0:	m_oscSub0->setChecked(true); break;
			case 1: m_oscSub1->setChecked(true); break;
			}
			break;
		}
	case TF_OSC_GLIDE:                  m_oscGlide->setValue(knob); break;
	case TF_OSC_DRIVE:                  m_oscDrive->setValue(knob); break;
	case TF_OSC_OCTAVE:                 
		{
			switch (_synthToIndex(value, 0, TF_MAXOCTAVES-1))
			{
			case 8:	m_oscOctm4->setChecked(true); break;
			case 7: m_oscOctm3->setChecked(true); break;
			case 6: m_oscOctm2->setChecked(true); break;
			case 5: m_oscOctm1->setChecked(true); break;
			case 4: m_oscOct0->setChecked(true); break;
			case 3: m_oscOct1->setChecked(true); break;
			case 2: m_oscOct2->setChecked(true); break;
			case 1: m_oscOct3->setChecked(true); break;
			case 0: m_oscOct4->setChecked(true); break;
			}
			break;
		}
	case TF_OSC_SLOP:                   m_oscSlop->setValue(knob); break;

	case TF_ADD_VOLUME:                 m_addVolume->setValue(knob); break;
	case TF_ADD_PROFILE:                
		{
			switch (_synthToIndex(value, 0, TF_ADDSYNTHPROFILES-1))
			{
			case 0: m_addSingle->setChecked(true); break;
			case 1: m_addDetuned->setChecked(true); break;
			case 3: m_addGauss->setChecked(true); break;
			case 2: m_addSpread->setChecked(true); break;
			}
			break;
		}
	case TF_ADD_OCTAVE:                 
		{
			switch (_synthToIndex(value, 0, TF_MAXOCTAVES-1))
			{
			case 8:	m_addOctm4->setChecked(true); break;
			case 7: m_addOctm3->setChecked(true); break;
			case 6: m_addOctm2->setChecked(true); break;
			case 5: m_addOctm1->setChecked(true); break;
			case 4: m_addOct0->setChecked(true); break;
			case 3: m_addOct1->setChecked(true); break;
			case 2: m_addOct2->setChecked(true); break;
			case 1: m_addOct3->setChecked(true); break;
			case 0: m_addOct4->setChecked(true); break;
			}
			break;
		}
	case TF_ADD_BANDWIDTH:              m_addBandwidth->setValue(knob); break;
	case TF_ADD_DAMP:                   m_addDamp->setValue(knob); break;
	case TF_ADD_HARMONICS:              m_addHarmonics->setValue(knob); break;
	case TF_ADD_SCALE:                  m_addScale->setValue(knob); break;
	case TF_ADD_DRIVE:                  m_addDrive->setValue(knob); break;

	case TF_NOISE_AMOUNT:               m_noiseAmount->setValue(knob); break;
	case TF_NOISE_FREQ:                 m_noiseFreq->setValue(knob); break;
	case TF_NOISE_BW:                   m_noiseBW->setValue(knob); break;

	case TF_LP_FILTER_ON:               m_lpOn->setChecked(value > 0.5f); break;
	case TF_LP_FILTER_CUTOFF:           m_lpFreq->setValue(knob); break;
	case TF_LP_FILTER_RESONANCE:        m_lpRes->setValue(knob); break;

	case TF_HP_FILTER_ON:               m_hpOn->setChecked(value > 0.5f); break;
	case TF_HP_FILTER_CUTOFF:           m_hpFreq->setValue(knob); break;
	case TF_HP_FILTER_RESONANCE:        m_hpRes->setValue(knob); break;

	case TF_BP_FILTER_ON:               m_bpOn->setChecked(value > 0.5f); break;
	case TF_BP_FILTER_CUTOFF:           m_bpFreq->setValue(knob); break;
	case TF_BP_FILTER_Q:                m_bpQ->setValue(knob); break;

	case TF_NT_FILTER_ON:               m_ntOn->setChecked(value > 0.5f); break;
	case TF_NT_FILTER_CUTOFF:           m_ntFreq->setValue(knob); break;
	case TF_NT_FILTER_Q:                m_ntQ->setValue(knob); break;

	case TF_ADSR1_ATTACK:               m_adsr1A->setValue(knob); break;            
	case TF_ADSR1_DECAY:                m_adsr1D->setValue(knob); break;    
	case TF_ADSR1_SUSTAIN:              m_adsr1S->setValue(knob); break;    
	case TF_ADSR1_RELEASE:              m_adsr1R->setValue(knob); break;    
	case TF_ADSR1_SLOPE:                m_adsr1Slope->setValue(knob); break;    

	case TF_ADSR2_ATTACK:               m_adsr2A->setValue(knob); break;    
	case TF_ADSR2_DECAY:                m_adsr2D->setValue(knob); break;    
	case TF_ADSR2_SUSTAIN:              m_adsr2S->setValue(knob); break;    
	case TF_ADSR2_RELEASE:              m_adsr2R->setValue(knob); break;    
	case TF_ADSR2_SLOPE:                m_adsr2Slope->setValue(knob); break;    

	case TF_LFO1_RATE:                  m_lfo1Rate->setValue(knob); break;    
	case TF_LFO1_DEPTH:                 m_lfo1Depth->setValue(knob); break;       
	case TF_LFO1_SHAPE:
		{
			switch (_synthToIndex(value, 0, TF_LFOSHAPECOUNT))
			{
			case 0:	m_lfo1ShapeSine->setChecked(true); break;
			case 1: m_lfo1ShapeSawUp->setChecked(true); break;
			case 2: m_lfo1ShapeSawDown->setChecked(true); break;
			case 3: m_lfo1ShapePulse->setChecked(true); break;
			case 4: m_lfo1ShapeNoise->setChecked(true); break;
			}
			break;
		}
	case TF_LFO1_SYNC:                  m_lfo1Sync->setChecked(value > 0.5f); break;

	case TF_LFO2_RATE:                  m_lfo2Rate->setValue(knob); break;    
	case TF_LFO2_DEPTH:                 m_lfo2Depth->setValue(knob); break;    
	case TF_LFO2_SHAPE:  
		{
			switch (_synthToIndex(value, 0, TF_LFOSHAPECOUNT))
			{
			case 0:	m_lfo2ShapeSine->setChecked(true); break;
			case 1: m_lfo2ShapeSawUp->setChecked(true); break;
			case 2: m_lfo2ShapeSawDown->setChecked(true); break;
			case 3: m_lfo2ShapePulse->setChecked(true); break;
			case 4: m_lfo2ShapeNoise->setChecked(true); break;
			}
			break;
		}
	case TF_LFO2_SYNC:                  m_lfo2Sync->setChecked(value > 0.5f); break;

	case TF_MM1_SOURCE:                 m_mm1Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM1_MOD:                    m_mm1Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM1_TARGET:                 m_mm1Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM2_SOURCE:                 m_mm2Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM2_MOD:                    m_mm2Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM2_TARGET:                 m_mm2Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM3_SOURCE:                 m_mm3Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM3_MOD:                    m_mm3Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM3_TARGET:                 m_mm3Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM4_SOURCE:                 m_mm4Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM4_MOD:                    m_mm4Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM4_TARGET:                 m_mm4Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM5_SOURCE:                 m_mm5Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM5_MOD:                    m_mm5Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM5_TARGET:                 m_mm5Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM6_SOURCE:                 m_mm6Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM6_MOD:                    m_mm6Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM6_TARGET:                 m_mm6Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM7_SOURCE:                 m_mm7Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM7_MOD:                    m_mm7Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM7_TARGET:                 m_mm7Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM8_SOURCE:                 m_mm8Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM8_MOD:                    m_mm8Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM8_TARGET:                 m_mm8Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM9_SOURCE:                 m_mm9Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM9_MOD:                    m_mm9Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM9_TARGET:                 m_mm9Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;
	case TF_MM10_SOURCE:                m_mm10Src->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); break;
	case TF_MM10_MOD:                   m_mm10Mod->setValue((value - 0.5f) * TF_MM_MODRANGE); break;
	case TF_MM10_TARGET:                m_mm10Dest->setCurrentIndex(_synthToIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); break;

	case TF_EFFECT_1:                   m_effect1->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_2:                   m_effect2->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_3:                   m_effect3->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_4:                   m_effect4->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_5:                   m_effect5->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_6:                   m_effect6->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_7:                   m_effect7->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_8:                   m_effect8->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_9:                   m_effect9->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;
	case TF_EFFECT_10:                  m_effect10->setCurrentIndex(_synthToIndex(value, 0, tfIEffect::FX_COUNT-1)); break;

	case TF_DISTORT_AMOUNT:             m_distAmount->setValue(knob); break; 

	case TF_CHORUS_RATE:                m_chorusFreq->setValue(knob); break; 
	case TF_CHORUS_DEPTH:               m_chorusDepth->setValue(knob); break; 
	case TF_CHORUS_GAIN:                m_chorusGain->setValue(knob); break; 

	case TF_DELAY_LEFT:                 m_delayLeft->setValue(knob); break; 
	case TF_DELAY_RIGHT:                m_delayRight->setValue(knob); break; 
	case TF_DELAY_DECAY:                m_delayDecay->setValue(knob); break; 

	case TF_REVERB_ROOMSIZE:            m_revRoomsize->setValue(knob); break; 
	case TF_REVERB_DAMP:                m_revDamp->setValue(knob); break; 
	case TF_REVERB_WET:                 m_revWet->setValue(knob); break; 
	case TF_REVERB_WIDTH:               m_revWidth->setValue(knob); break; 

	case TF_FLANGER_LFO:                m_flangLfo->setValue(knob); break; 
	case TF_FLANGER_FREQUENCY:          m_flangFreq->setValue(knob); break; 
	case TF_FLANGER_AMPLITUDE:          m_flangAmp->setValue(knob); break; 
	case TF_FLANGER_WET:                m_flangWet->setValue(knob); break; 

	case TF_FORMANT_MODE:               
		{
			switch (_synthToIndex(value, 0, 4))
			{
			case 0: m_formantA->setChecked(true); break;
			case 1: m_formantE->setChecked(true); break;
			case 2: m_formantI->setChecked(true); break;
			case 3: m_formantO->setChecked(true); break;
			case 4: m_formantU->setChecked(true); break;
			}
            break;
		}
	case TF_FORMANT_WET:                m_formantWet->setValue(knob); break; 

	case TF_EQ_LOW:                     m_eqLow->setValue(knob); break; 
	case TF_EQ_MID:                     m_eqMid->setValue(knob); break; 
	case TF_EQ_HIGH:                    m_eqHigh->setValue(knob); break; 
	}

    if (m_activeInstr)
        m_activeInstr->setIgnoreParamUI(eFALSE);
}

void eMainWnd::_onSynthClicked(bool checked)
{
	QObject *obj = sender();

	if (obj == m_oscUni1)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(0, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni2)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(1, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni3)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(2, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni4)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(3, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni5)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(4, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni6)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(5, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni7)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(6, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni8)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(7, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni9)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(8, 0, TF_MAXUNISONO-1)); return; }
	if (obj == m_oscUni10)			{ m_activeInstr->setParamUI(TF_OSC_UNISONO, _synthFromIndex(9, 0, TF_MAXUNISONO-1)); return; }

	if (obj == m_oscOctm4)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(8, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOctm3)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(7, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOctm2)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(6, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOctm1)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(5, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOct0)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(4, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOct1)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(3, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOct2)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(2, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOct3)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(1, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_oscOct4)			{ m_activeInstr->setParamUI(TF_OSC_OCTAVE, _synthFromIndex(0, 0, TF_MAXOCTAVES-1)); return; }

	if (obj == m_oscSub0)           { m_activeInstr->setParamUI(TF_OSC_SUBOSC, _synthFromIndex(0, 0, TF_MAXSUBOSC)); return; }
	if (obj == m_oscSub1)           { m_activeInstr->setParamUI(TF_OSC_SUBOSC, _synthFromIndex(1, 0, TF_MAXSUBOSC)); return; }

	if (obj == m_addOctm4)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(8, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOctm3)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(7, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOctm2)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(6, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOctm1)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(5, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOct0)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(4, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOct1)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(3, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOct2)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(2, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOct3)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(1, 0, TF_MAXOCTAVES-1)); return; }
	if (obj == m_addOct4)			{ m_activeInstr->setParamUI(TF_ADD_OCTAVE, _synthFromIndex(0, 0, TF_MAXOCTAVES-1)); return; }

	if (obj == m_addSingle)         { m_activeInstr->setParamUI(TF_ADD_PROFILE, _synthFromIndex(0, 0, TF_ADDSYNTHPROFILES-1)); return; }
	if (obj == m_addDetuned)        { m_activeInstr->setParamUI(TF_ADD_PROFILE, _synthFromIndex(1, 0, TF_ADDSYNTHPROFILES-1)); return; }
	if (obj == m_addGauss)          { m_activeInstr->setParamUI(TF_ADD_PROFILE, _synthFromIndex(3, 0, TF_ADDSYNTHPROFILES-1)); return; }
	if (obj == m_addSpread)         { m_activeInstr->setParamUI(TF_ADD_PROFILE, _synthFromIndex(2, 0, TF_ADDSYNTHPROFILES-1)); return; }

	if (obj == m_lfo1ShapeSine)     { m_activeInstr->setParamUI(TF_LFO1_SHAPE, _synthFromIndex(0, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo1ShapeSawUp)    { m_activeInstr->setParamUI(TF_LFO1_SHAPE, _synthFromIndex(1, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo1ShapeSawDown)  { m_activeInstr->setParamUI(TF_LFO1_SHAPE, _synthFromIndex(2, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo1ShapePulse)    { m_activeInstr->setParamUI(TF_LFO1_SHAPE, _synthFromIndex(3, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo1ShapeNoise)    { m_activeInstr->setParamUI(TF_LFO1_SHAPE, _synthFromIndex(4, 0, TF_LFOSHAPECOUNT)); return; }

	if (obj == m_lfo2ShapeSine)     { m_activeInstr->setParamUI(TF_LFO2_SHAPE, _synthFromIndex(0, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo2ShapeSawUp)    { m_activeInstr->setParamUI(TF_LFO2_SHAPE, _synthFromIndex(1, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo2ShapeSawDown)  { m_activeInstr->setParamUI(TF_LFO2_SHAPE, _synthFromIndex(2, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo2ShapePulse)    { m_activeInstr->setParamUI(TF_LFO2_SHAPE, _synthFromIndex(3, 0, TF_LFOSHAPECOUNT)); return; }
	if (obj == m_lfo2ShapeNoise)    { m_activeInstr->setParamUI(TF_LFO2_SHAPE, _synthFromIndex(4, 0, TF_LFOSHAPECOUNT)); return; }

	if (obj == m_formantA)          { m_activeInstr->setParamUI(TF_FORMANT_MODE, _synthFromIndex(0, 0, 4)); return; }
	if (obj == m_formantE)          { m_activeInstr->setParamUI(TF_FORMANT_MODE, _synthFromIndex(1, 0, 4)); return; }
	if (obj == m_formantI)          { m_activeInstr->setParamUI(TF_FORMANT_MODE, _synthFromIndex(2, 0, 4)); return; }
	if (obj == m_formantO)          { m_activeInstr->setParamUI(TF_FORMANT_MODE, _synthFromIndex(3, 0, 4)); return; }
	if (obj == m_formantU)          { m_activeInstr->setParamUI(TF_FORMANT_MODE, _synthFromIndex(4, 0, 4)); return; }
}

void eMainWnd::_onSynthChanged(double value)
{
    if (m_activeInstr == eNULL)
    {
        return;
    }

    const QObject *obj = sender();
    eASSERT(obj != eNULL);

    if (obj == m_mm1Mod)            { m_activeInstr->setParamUI(TF_MM1_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm2Mod)            { m_activeInstr->setParamUI(TF_MM2_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm3Mod)            { m_activeInstr->setParamUI(TF_MM3_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm4Mod)            { m_activeInstr->setParamUI(TF_MM4_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm5Mod)            { m_activeInstr->setParamUI(TF_MM5_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm6Mod)            { m_activeInstr->setParamUI(TF_MM6_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm7Mod)            { m_activeInstr->setParamUI(TF_MM7_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm8Mod)            { m_activeInstr->setParamUI(TF_MM8_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm9Mod)            { m_activeInstr->setParamUI(TF_MM9_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
    if (obj == m_mm10Mod)           { m_activeInstr->setParamUI(TF_MM10_MOD, (value / TF_MM_MODRANGE) + 0.5f); return; }
}

void eMainWnd::_onSynthChanged(int value)
{
    if (m_activeInstr == eNULL)
    {
        return;
    }

    const QObject *obj = sender();
    eASSERT(obj != eNULL);

    const eF32 knob = (eF32)value/99;

	if (obj == m_gainAmount)        { m_activeInstr->setParamUI(TF_GAIN_AMOUNT, knob); return; }
	if (obj == m_oscPoints)         { m_activeInstr->setParamUI(TF_OSC_POINTCOUNT, _synthFromIndex(value, 0, TF_OSCILLATOR_POINTS-3)); _synthUpdateOscView(); return; }
	if (obj == m_oscVolume)         { m_activeInstr->setParamUI(TF_OSC_VOLUME, knob); return; }
	if (obj == m_oscFreq)           { m_activeInstr->setParamUI(TF_OSC_FREQ, knob); return; }
	if (obj == m_oscPanning)        { m_activeInstr->setParamUI(TF_OSC_PAN, knob); return; }
	if (obj == m_oscDetune)         { m_activeInstr->setParamUI(TF_OSC_DETUNE, knob); return; }
	if (obj == m_oscPolyphony)      { m_activeInstr->setParamUI(TF_OSC_POLYPHONY, _synthFromIndex(value, 0, TF_MAXVOICES-1)); return; }
	if (obj == m_oscSpread)         { m_activeInstr->setParamUI(TF_OSC_SPREAD, knob); return; }
	if (obj == m_oscGlide)          { m_activeInstr->setParamUI(TF_OSC_GLIDE, knob); return; }
	if (obj == m_oscDrive)          { m_activeInstr->setParamUI(TF_OSC_DRIVE, knob); _synthUpdateOscView(); return; }
	if (obj == m_oscSlop)           { m_activeInstr->setParamUI(TF_OSC_SLOP, knob); return; }

	if (obj == m_addVolume)         { m_activeInstr->setParamUI(TF_ADD_VOLUME, knob); return; }
	if (obj == m_addBandwidth)      { m_activeInstr->setParamUI(TF_ADD_BANDWIDTH, knob); return; }
	if (obj == m_addDamp)           { m_activeInstr->setParamUI(TF_ADD_DAMP, knob); return; }
	if (obj == m_addHarmonics)      { m_activeInstr->setParamUI(TF_ADD_HARMONICS, knob); return; }
	if (obj == m_addScale)          { m_activeInstr->setParamUI(TF_ADD_SCALE, knob); return; }
	if (obj == m_addDrive)          { m_activeInstr->setParamUI(TF_ADD_DRIVE, knob); return; }

	if (obj == m_noiseAmount)       { m_activeInstr->setParamUI(TF_NOISE_AMOUNT, knob); return; }
	if (obj == m_noiseFreq)         { m_activeInstr->setParamUI(TF_NOISE_FREQ, knob); return; }
	if (obj == m_noiseBW)           { m_activeInstr->setParamUI(TF_NOISE_BW, knob); return; }

	if (obj == m_lpFreq)            { m_activeInstr->setParamUI(TF_LP_FILTER_CUTOFF, knob); return; }
	if (obj == m_lpRes)             { m_activeInstr->setParamUI(TF_LP_FILTER_RESONANCE, knob); return; }
	if (obj == m_lpOn)              { m_activeInstr->setParamUI(TF_LP_FILTER_ON, value ? 1.0f : 0.0f); return; }

	if (obj == m_hpFreq)            { m_activeInstr->setParamUI(TF_HP_FILTER_CUTOFF, knob); return; }
	if (obj == m_hpRes)             { m_activeInstr->setParamUI(TF_HP_FILTER_RESONANCE, knob); return; }
	if (obj == m_hpOn)              { m_activeInstr->setParamUI(TF_HP_FILTER_ON, value ? 1.0f : 0.0f); return; }

	if (obj == m_bpFreq)            { m_activeInstr->setParamUI(TF_BP_FILTER_CUTOFF, knob); return; }
	if (obj == m_bpQ)               { m_activeInstr->setParamUI(TF_BP_FILTER_Q, knob); return; }
	if (obj == m_bpOn)              { m_activeInstr->setParamUI(TF_BP_FILTER_ON, value ? 1.0f : 0.0f); return; }

	if (obj == m_ntFreq)            { m_activeInstr->setParamUI(TF_NT_FILTER_CUTOFF, knob); return; }
	if (obj == m_ntQ)               { m_activeInstr->setParamUI(TF_NT_FILTER_Q, knob); return; }
	if (obj == m_ntOn)              { m_activeInstr->setParamUI(TF_NT_FILTER_ON, value ? 1.0f : 0.0f); return; }

	if (obj == m_adsr1A)            { m_activeInstr->setParamUI(TF_ADSR1_ATTACK, knob); return; }
	if (obj == m_adsr1D)            { m_activeInstr->setParamUI(TF_ADSR1_DECAY, knob); return; }
	if (obj == m_adsr1S)            { m_activeInstr->setParamUI(TF_ADSR1_SUSTAIN, knob); return; }
	if (obj == m_adsr1R)            { m_activeInstr->setParamUI(TF_ADSR1_RELEASE, knob); return; }
	if (obj == m_adsr1Slope)        { m_activeInstr->setParamUI(TF_ADSR1_SLOPE, knob); return; }

	if (obj == m_adsr2A)            { m_activeInstr->setParamUI(TF_ADSR2_ATTACK, knob); return; }
	if (obj == m_adsr2D)            { m_activeInstr->setParamUI(TF_ADSR2_DECAY, knob); return; }
	if (obj == m_adsr2S)            { m_activeInstr->setParamUI(TF_ADSR2_SUSTAIN, knob); return; }
	if (obj == m_adsr2R)            { m_activeInstr->setParamUI(TF_ADSR2_RELEASE, knob); return; }
	if (obj == m_adsr2Slope)        { m_activeInstr->setParamUI(TF_ADSR2_SLOPE, knob); return; }

	if (obj == m_lfo1Rate)          { m_activeInstr->setParamUI(TF_LFO1_RATE, knob); return; }
	if (obj == m_lfo1Depth)         { m_activeInstr->setParamUI(TF_LFO1_DEPTH, knob); return; }
	if (obj == m_lfo1Sync)          { m_activeInstr->setParamUI(TF_LFO1_SYNC, value ? 1.0f : 0.0f); return; }

	if (obj == m_lfo2Rate)          { m_activeInstr->setParamUI(TF_LFO2_RATE, knob); return; }
	if (obj == m_lfo2Depth)         { m_activeInstr->setParamUI(TF_LFO2_DEPTH, knob); return; }
	if (obj == m_lfo2Sync)          { m_activeInstr->setParamUI(TF_LFO2_SYNC, value ? 1.0f : 0.0f); return; }

	if (obj == m_mm1Src)            { m_activeInstr->setParamUI(TF_MM1_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm1Dest)           { m_activeInstr->setParamUI(TF_MM1_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm2Src)            { m_activeInstr->setParamUI(TF_MM2_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm2Dest)           { m_activeInstr->setParamUI(TF_MM2_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm3Src)            { m_activeInstr->setParamUI(TF_MM3_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm3Dest)           { m_activeInstr->setParamUI(TF_MM3_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm4Src)            { m_activeInstr->setParamUI(TF_MM4_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm4Dest)           { m_activeInstr->setParamUI(TF_MM4_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm5Src)            { m_activeInstr->setParamUI(TF_MM5_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm5Dest)           { m_activeInstr->setParamUI(TF_MM5_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm6Src)            { m_activeInstr->setParamUI(TF_MM6_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm6Dest)           { m_activeInstr->setParamUI(TF_MM6_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm7Src)            { m_activeInstr->setParamUI(TF_MM7_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm7Dest)           { m_activeInstr->setParamUI(TF_MM7_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm8Src)            { m_activeInstr->setParamUI(TF_MM8_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm8Dest)           { m_activeInstr->setParamUI(TF_MM8_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm9Src)            { m_activeInstr->setParamUI(TF_MM9_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm9Dest)           { m_activeInstr->setParamUI(TF_MM9_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }
	if (obj == m_mm10Src)           { m_activeInstr->setParamUI(TF_MM10_SOURCE, _synthFromIndex(value, 0, tfModMatrix::INPUT_COUNT-1)); return; }
	if (obj == m_mm10Dest)          { m_activeInstr->setParamUI(TF_MM10_TARGET, _synthFromIndex(value, 0, tfModMatrix::OUTPUT_COUNT-1)); return; }

	if (obj == m_effect1)           { m_activeInstr->setParamUI(TF_EFFECT_1, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect2)           { m_activeInstr->setParamUI(TF_EFFECT_2, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect3)           { m_activeInstr->setParamUI(TF_EFFECT_3, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect4)           { m_activeInstr->setParamUI(TF_EFFECT_4, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect5)           { m_activeInstr->setParamUI(TF_EFFECT_5, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect6)           { m_activeInstr->setParamUI(TF_EFFECT_6, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect7)           { m_activeInstr->setParamUI(TF_EFFECT_7, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect8)           { m_activeInstr->setParamUI(TF_EFFECT_8, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect9)           { m_activeInstr->setParamUI(TF_EFFECT_9, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }
	if (obj == m_effect10)          { m_activeInstr->setParamUI(TF_EFFECT_10, _synthFromIndex(value, 0, tfIEffect::FX_COUNT-1)); return; }

	if (obj == m_distAmount)        { m_activeInstr->setParamUI(TF_DISTORT_AMOUNT, knob); return; }

	if (obj == m_chorusFreq)        { m_activeInstr->setParamUI(TF_CHORUS_RATE, knob); return; }
	if (obj == m_chorusDepth)       { m_activeInstr->setParamUI(TF_CHORUS_DEPTH, knob); return; }
	if (obj == m_chorusGain)        { m_activeInstr->setParamUI(TF_CHORUS_GAIN, knob); return; }

	if (obj == m_delayLeft)         { m_activeInstr->setParamUI(TF_DELAY_LEFT, knob); return; }
	if (obj == m_delayRight)        { m_activeInstr->setParamUI(TF_DELAY_RIGHT, knob); return; }
	if (obj == m_delayDecay)        { m_activeInstr->setParamUI(TF_DELAY_DECAY, knob); return; }

	if (obj == m_revRoomsize)       { m_activeInstr->setParamUI(TF_REVERB_ROOMSIZE, knob); return; }
	if (obj == m_revDamp)           { m_activeInstr->setParamUI(TF_REVERB_DAMP, knob); return; }
	if (obj == m_revWet)            { m_activeInstr->setParamUI(TF_REVERB_WET, knob); return; }
	if (obj == m_revWidth)          { m_activeInstr->setParamUI(TF_REVERB_WIDTH, knob); return; }

	if (obj == m_flangLfo)          { m_activeInstr->setParamUI(TF_FLANGER_LFO, knob); return; }
	if (obj == m_flangFreq)         { m_activeInstr->setParamUI(TF_FLANGER_FREQUENCY, knob); return; }
	if (obj == m_flangAmp)          { m_activeInstr->setParamUI(TF_FLANGER_AMPLITUDE, knob); return; }
	if (obj == m_flangWet)          { m_activeInstr->setParamUI(TF_FLANGER_WET, knob); return; }

	if (obj == m_formantWet)        { m_activeInstr->setParamUI(TF_FORMANT_WET, knob); return; }

	if (obj == m_eqLow)             { m_activeInstr->setParamUI(TF_EQ_LOW, knob); return; }
	if (obj == m_eqMid)             { m_activeInstr->setParamUI(TF_EQ_MID, knob); return; }
	if (obj == m_eqHigh)            { m_activeInstr->setParamUI(TF_EQ_HIGH, knob); return; }
}

void eMainWnd::updateAfterPreset()
{
	_synthUpdateOscView();
}

void eMainWnd::setPresetValue(eU32 index, eF32 value)
{
	if (m_activeInstr == eNULL)
	{
		return;
	}

	m_activeInstr->setParam(index, value);
	_synthSetParameter(index, value);
}

void eMainWnd::_presetSine(bool checked)
{
	setPresetValue(TF_OSC_POINTCOUNT, 0.66f);

	setPresetValue(TF_OSC_DRIVE, 0.0f);

	setPresetValue(TF_OSC_POINT1_INTERPOLATION, 1.0f);
	setPresetValue(TF_OSC_POINT2_INTERPOLATION, 1.0f);
	setPresetValue(TF_OSC_POINT3_INTERPOLATION, 1.0f);
	setPresetValue(TF_OSC_POINT4_INTERPOLATION, 1.0f);
	setPresetValue(TF_OSC_FINAL_INTERPOLATION, 1.0f);

	setPresetValue(TF_OSC_POINT1_OFFSET, 0.25f);
	setPresetValue(TF_OSC_POINT2_OFFSET, 0.33f);
	setPresetValue(TF_OSC_POINT3_OFFSET, 0.5f);
	setPresetValue(TF_OSC_POINT4_OFFSET, 0.5f);

	setPresetValue(TF_OSC_POINT1_VALUE, 0.0f);
	setPresetValue(TF_OSC_POINT2_VALUE, 0.5f);
	setPresetValue(TF_OSC_POINT3_VALUE, 1.0f);
	setPresetValue(TF_OSC_POINT4_VALUE, 0.5f);

	updateAfterPreset();
}

void eMainWnd::_presetTriangle(bool checked)
{
	setPresetValue(TF_OSC_POINTCOUNT, 0.33f);

	setPresetValue(TF_OSC_DRIVE, 0.0f);

	setPresetValue(TF_OSC_POINT1_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT2_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT3_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT4_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_FINAL_INTERPOLATION, 0.5f);

	setPresetValue(TF_OSC_POINT1_OFFSET, 0.25f);
	setPresetValue(TF_OSC_POINT2_OFFSET, 0.66f);
	setPresetValue(TF_OSC_POINT3_OFFSET, 0.5f);
	setPresetValue(TF_OSC_POINT4_OFFSET, 0.5f);

	setPresetValue(TF_OSC_POINT1_VALUE, 0.0f);
	setPresetValue(TF_OSC_POINT2_VALUE, 1.0f);
	setPresetValue(TF_OSC_POINT3_VALUE, 0.5f);
	setPresetValue(TF_OSC_POINT4_VALUE, 0.5f);

	updateAfterPreset();
}

void eMainWnd::_presetSquare(bool checked)
{
	setPresetValue(TF_OSC_POINTCOUNT, 0.33f);

	setPresetValue(TF_OSC_DRIVE, 0.0f);

	setPresetValue(TF_OSC_POINT1_INTERPOLATION, 0.0f);
	setPresetValue(TF_OSC_POINT2_INTERPOLATION, 0.0f);
	setPresetValue(TF_OSC_POINT3_INTERPOLATION, 0.0f);
	setPresetValue(TF_OSC_POINT4_INTERPOLATION, 0.0f);
	setPresetValue(TF_OSC_FINAL_INTERPOLATION, 0.0f);

	setPresetValue(TF_OSC_POINT1_OFFSET, 0.0f);
	setPresetValue(TF_OSC_POINT2_OFFSET, 0.5f);
	setPresetValue(TF_OSC_POINT3_OFFSET, 0.0f);
	setPresetValue(TF_OSC_POINT4_OFFSET, 0.0f);

	setPresetValue(TF_OSC_POINT1_VALUE, 0.0f);
	setPresetValue(TF_OSC_POINT2_VALUE, 1.0f);
	setPresetValue(TF_OSC_POINT3_VALUE, 0.5f);
	setPresetValue(TF_OSC_POINT4_VALUE, 0.5f);

	updateAfterPreset();
}

void eMainWnd::_presetSawUp(bool checked)
{
	setPresetValue(TF_OSC_POINTCOUNT, 0.33f);

	setPresetValue(TF_OSC_DRIVE, 0.0f);

	setPresetValue(TF_OSC_POINT1_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT2_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT3_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT4_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_FINAL_INTERPOLATION, 0.5f);

	setPresetValue(TF_OSC_POINT1_OFFSET, 0.0f);
	setPresetValue(TF_OSC_POINT2_OFFSET, 1.0f);
	setPresetValue(TF_OSC_POINT3_OFFSET, 0.5f);
	setPresetValue(TF_OSC_POINT4_OFFSET, 0.5f);

	setPresetValue(TF_OSC_POINT1_VALUE, 0.0f);
	setPresetValue(TF_OSC_POINT2_VALUE, 1.0f);
	setPresetValue(TF_OSC_POINT3_VALUE, 0.5f);
	setPresetValue(TF_OSC_POINT4_VALUE, 0.5f);

	updateAfterPreset();
}

void eMainWnd::_presetSawDown(bool checked)
{
	setPresetValue(TF_OSC_POINTCOUNT, 0.33f);

	setPresetValue(TF_OSC_DRIVE, 0.0f);

	setPresetValue(TF_OSC_POINT1_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT2_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT3_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_POINT4_INTERPOLATION, 0.5f);
	setPresetValue(TF_OSC_FINAL_INTERPOLATION, 0.5f);

	setPresetValue(TF_OSC_POINT1_OFFSET, 0.0f);
	setPresetValue(TF_OSC_POINT2_OFFSET, 1.0f);
	setPresetValue(TF_OSC_POINT3_OFFSET, 0.5f);
	setPresetValue(TF_OSC_POINT4_OFFSET, 0.5f);

	setPresetValue(TF_OSC_POINT1_VALUE, 1.0f);
	setPresetValue(TF_OSC_POINT2_VALUE, 0.0f);
	setPresetValue(TF_OSC_POINT3_VALUE, 0.5f);
	setPresetValue(TF_OSC_POINT4_VALUE, 0.5f);

	updateAfterPreset();
}

void eMainWnd::_onSynthInstrumentChanged(int value)
{
    if (m_activeInstr == eNULL)
    {
        return;
    }

    m_synthCurInstr = value;

    const tfInstrument::Data &ap = m_synthInstrs[m_synthCurInstr];

    for (eInt i=0; i<TF_PARAM_COUNT; i++)
    {
        m_activeInstr->setParamUI(i, ap.params[i]);
    }

    _synthUpdateInstrumentName();
    _synthInitParameters();
    _synthUpdateOscView();
}

void eMainWnd::_onSynthInstrNameChanged(const QString &name)
{
    eStrCopy(m_synthInstrs[m_synthCurInstr].name, name.toAscii().constData());
    _synthUpdateInstrumentName();
}

void eMainWnd::_synthUpdateInstrumentName()
{
    if (!m_instrTable->selectedItems().count())
    {
        return;
    }

    QTableWidgetItem *item = m_instrTable->selectedItems().at(0);
    eASSERT(item != eNULL);

    if (m_activeInstr)
    {
        item->setForeground(QBrush(Qt::white));
        item->setText(m_synthInstrs[m_synthCurInstr].name);
    }
    else
    {
        item->setForeground(QBrush(Qt::gray));
        item->setText("<empty>");
    }
}

void eMainWnd::_synthInstrumentsLoadAll()
{
    for (eInt i=0; i<TF_NUM_PRESETS; i++)
    {
        if (!_synthInstrumentLoad(i))
        {
            return;
        }
    }
}

eBool eMainWnd::_synthInstrumentLoad(eU32 index)
{
    const QString path = QApplication::applicationDirPath()+"/tf3programs/program"+QString::number(index)+".txt";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "TF3", "Could not open file!");
        return eFALSE;
    }

    QString name(file.readLine());
    eStrCopy(m_synthInstrs[index].name, name.trimmed().toAscii().constData());

    while (eTRUE)
    {
        const QString line(file.readLine());

        if (line == "")
        {
            file.close();

            if (index == m_synthCurInstr && m_activeInstr)
            {
                // Load new program to into tunefish.
                tfInstrument::Data &ap = m_synthInstrs[m_synthCurInstr];

                for (eInt i=0; i<TF_PARAM_COUNT; i++)
                {
                    m_activeInstr->setParamUI(i, ap.params[i]);
                }
            }

            return eTRUE;
        }

        const QStringList parts = line.split(";");

        if (parts.size() == 2)
        {
            const QString key = parts[0];
            const eF32 value = parts[1].toFloat();

            for (eInt i=0; i<TF_PARAM_COUNT; i++)
            {
                if (key == TF_NAMES[i])
                {
                    m_synthInstrs[index].params[i] = value;
                    break;
                }
            }
        }
    }
}

void eMainWnd::_synthInstrumentLoad()
{
    _synthInstrumentLoad(m_synthCurInstr);
}

void eMainWnd::_synthInstrumentSave()
{
    _synthInstrumentSave(m_synthCurInstr);
}

eBool eMainWnd::_synthInstrumentSave(eU32 index)
{
    const QString path = QApplication::applicationDirPath()+"/tf3programs/e3program"+QString::number(index)+".txt";
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly))
    {
        return eFALSE;
    }

    if (index == m_synthCurInstr && m_activeInstr)
    {
        // Write instrument from Tunefish to
        // instrument list before saving.
        tfInstrument::Data &ap = m_synthInstrs[m_synthCurInstr];

        for (eInt i=0; i<TF_PARAM_COUNT; i++)
        {
            ap.params[i] = m_activeInstr->getParam(i);
        }
    }

    file.write(m_synthInstrs[index].name);
    file.write("\r\n");

    for(eU32 i=0;i<TF_PARAM_COUNT;i++)
    {
        file.write(TF_NAMES[i]);
        file.write(";");
        file.write(QString::number(m_synthInstrs[index].params[i]).toAscii().constData());
        file.write("\r\n");
    }

    file.close();
    return eTRUE;
}

void eMainWnd::_synthInstrumentSaveAll()
{
    for (eInt i=0; i<TF_NUM_PRESETS; i++)
    {
        if (!_synthInstrumentSave(i))
        {
            return;
        }
    }
}

void eMainWnd::_synthInstrumentCopy()
{
}

void eMainWnd::_synthInstrumentPaste()
{
}

eU32 eMainWnd::getActiveOctave()
{
    return m_patternEditor->getActiveOctave();
}

eU32 eMainWnd::getActiveInstrument()
{
    return m_synthCurrentInstrument; 
}

void eMainWnd::_onPatternVScroll(int value)
{
    m_patternEditor->setCursorRow(value);
}

void eMainWnd::_onPatternHScroll(int value)
{
    m_patternEditor->setDisplayTrack(value);
}

void eMainWnd::_createIcons()
{
	const eU32 PIXWIDTH = 14;
	const eU32 PIXHEIGHT = 14;
	const eU32 MAX_X = PIXWIDTH-1;
	const eU32 MAX_Y = PIXHEIGHT-1;
	const QColor bgCol(0, 0, 0, 0);
	const QPen pen(Qt::white);

	m_pixSine = new QPixmap(PIXWIDTH, PIXHEIGHT);
	m_pixSawUp = new QPixmap(PIXWIDTH, PIXHEIGHT);
	m_pixSawDown = new QPixmap(PIXWIDTH, PIXHEIGHT);
	m_pixPulse = new QPixmap(PIXWIDTH, PIXHEIGHT);
	m_pixNoise = new QPixmap(PIXWIDTH, PIXHEIGHT);

	m_pixSine->fill(bgCol);
	m_pixSawUp->fill(bgCol);
	m_pixSawDown->fill(bgCol);
	m_pixPulse->fill(bgCol);
	m_pixNoise->fill(bgCol);

	// noise
	// ------------------------------------------
	QPainter painterSine(m_pixSine);
	painterSine.setPen(pen);
	//painterSine.setRenderHint(QPainter::Antialiasing);
	eU32 old = 0;
	for (eU32 i=0;i<PIXWIDTH;i++)
	{
		eU32 sine = eSin((eF32)i / PIXWIDTH * ePI*2) * PIXHEIGHT/2;
		if (i>0)
			painterSine.drawLine(i-1, old+PIXHEIGHT/2, i, sine+PIXHEIGHT/2);
		old = sine;
	}

	// saw down
	// ------------------------------------------
	QPainter painterSawDown(m_pixSawDown);
	painterSawDown.setPen(pen);
	//painterSawDown.setRenderHint(QPainter::Antialiasing);
	painterSawDown.drawLine(0, 0, MAX_X, MAX_Y);
	painterSawDown.drawLine(0, 0, 0, MAX_Y);

	// saw up
	// ------------------------------------------
	QPainter painterSawUp(m_pixSawUp);
	painterSawUp.setPen(pen);
	//painterSawUp.setRenderHint(QPainter::Antialiasing);
	painterSawUp.drawLine(0, MAX_Y, MAX_X, 0);
	painterSawUp.drawLine(MAX_X, 0, MAX_X, MAX_Y);

	// pulse
	// ------------------------------------------
	QPainter painterPulse(m_pixPulse);
	painterPulse.setPen(pen);
	//painterPulse.setRenderHint(QPainter::Antialiasing);
	painterPulse.drawLine(0, 0, MAX_X/2, 0);
	painterPulse.drawLine(MAX_X/2, MAX_Y, MAX_X, MAX_Y);
	painterPulse.drawLine(MAX_X/2, 0, MAX_X/2, MAX_Y);

	// noise
	// ------------------------------------------
	QPainter painterNoise(m_pixNoise);
	painterNoise.setPen(pen);
	//painterNoise.setRenderHint(QPainter::Antialiasing);
	for (eU32 i=0;i<PIXWIDTH;i++)
	{
		painterNoise.drawLine(i, MAX_Y/2, i, eRandom(0, MAX_Y));
	}

	m_iconSine = new QIcon(*m_pixSine);
	m_iconSawUp = new QIcon(*m_pixSawUp);
	m_iconSawDown = new QIcon(*m_pixSawDown);
	m_iconPulse = new QIcon(*m_pixPulse);
	m_iconNoise = new QIcon(*m_pixNoise);

	m_lfo1ShapeSine->setIcon(*m_iconSine);
	m_lfo2ShapeSine->setIcon(*m_iconSine);
	m_lfo1ShapeSawUp->setIcon(*m_iconSawUp);
	m_lfo2ShapeSawUp->setIcon(*m_iconSawUp);
	m_lfo1ShapeSawDown->setIcon(*m_iconSawDown);
	m_lfo2ShapeSawDown->setIcon(*m_iconSawDown);
	m_lfo1ShapePulse->setIcon(*m_iconPulse);
	m_lfo2ShapePulse->setIcon(*m_iconPulse);
	m_lfo1ShapeNoise->setIcon(*m_iconNoise);
	m_lfo2ShapeNoise->setIcon(*m_iconNoise);

	m_lfo1ShapeSine->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo2ShapeSine->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo1ShapeSawUp->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo2ShapeSawUp->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo1ShapeSawDown->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo2ShapeSawDown->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo1ShapePulse->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo2ShapePulse->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo1ShapeNoise->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
	m_lfo2ShapeNoise->setIconSize(QSize(PIXWIDTH, PIXHEIGHT));
}

void eMainWnd::_freeIcons()
{
	eSAFE_DELETE(m_iconSine);
	eSAFE_DELETE(m_iconSawUp);
	eSAFE_DELETE(m_iconSawDown);
	eSAFE_DELETE(m_iconPulse);
	eSAFE_DELETE(m_iconNoise);

	eSAFE_DELETE(m_pixSine);
	eSAFE_DELETE(m_pixSawUp);
	eSAFE_DELETE(m_pixSawDown);
	eSAFE_DELETE(m_pixPulse);
	eSAFE_DELETE(m_pixNoise);
}

bool eMainWnd::ProfilerTreeItem::operator < (const QTreeWidgetItem &item) const
{
    const eInt sortCol = treeWidget()->sortColumn();  

    if (sortCol == 0)
    {
        return QTreeWidgetItem::operator<(item);
    }
    else
    {
        const eF32 a = text(sortCol).toFloat();  
        const eF32 b = item.text(sortCol).toFloat();  

        return (a < b);
    }
}