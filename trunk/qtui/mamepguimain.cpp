#include <QtPlugin>

#include "7zVersion.h"

#include "mamepguimain.h"

#include "mameopt.h"
#include "utils.h"
#include "dialogs.h"
#include "ips.h"
#include "m1.h"

#ifdef USE_SDL
#include "SDL.h"
#undef main
#endif /* USE_SDL */

//static Qt plugins
#ifdef USE_STATIC
Q_IMPORT_PLUGIN(qico)
Q_IMPORT_PLUGIN(qjpeg)
//Q_IMPORT_PLUGIN(qmng)
#endif /* USE_STATIC */

/* global */
QString CFG_PREFIX = 
#ifndef Q_WS_WIN
	QDir::homePath() + "/" + 
#endif
	".mamepgui/";

MainWindow *win;
QSettings *pGuiSettings;
QSettings defSettings(":/res/mamepgui" INI_EXT, QSettings::IniFormat);
QString currentDir;
QString mame_binary;
QString language;
bool local_game_list;
bool isDarkBg = false;
bool sdlInited = false;
bool isMESS = false;

QStringList validGuiSettings;

/* internal */
QDockWidget *dwHistory = NULL;

void MainWindow::log(QString message, char logOrigin)
{
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	tbGUILog->append(msg);
	tbGUILog->horizontalScrollBar()->setValue(0);
}

void MainWindow::poplog(QString message)
{
	QMessageBox::critical(this, "Warning", message); 
}

void MainWindow::logStatus(QString message)
{
	if (currentDir != QDir::currentPath())
		return;

	labelProgress->setText(message);
}

void MainWindow::logStatus(GameInfo *gameInfo)
{
	//if everything is ok, only show 1 icon
	//show all 6 icons otherwise
	static const QString prefix = ":/res/16x16/";
	static const QString suffix = ".png";
	QString statusBuffer = "";

	if (gameInfo->isExtRom)
		gameInfo = mameGame->games[gameInfo->romof];

	QString statusText = utils->getStatusString (gameInfo->status);
	QString statusName = QT_TR_NOOP("status");
	labelStatus->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->emulation);
	statusName = QT_TR_NOOP("emulation");
	labelEmulation->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->color);
	statusName = QT_TR_NOOP("color");
	labelColor->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->sound);
	statusName = QT_TR_NOOP("sound");
	labelSound->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->graphic);
	statusName = QT_TR_NOOP("graphic");
	labelGraphic->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	statusText = utils->getStatusString (gameInfo->savestate, true);
	statusName = QT_TR_NOOP("savestate");
	labelSavestate->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
	statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";

	//show 2 more icons if applicable, hide otherwise
	labelCocktail->hide();
	wStatus->layout()->removeWidget(labelCocktail);
	if (gameInfo->cocktail != 64)
	{
		wStatus->layout()->addWidget(labelCocktail);
		labelCocktail->show();
		statusText = utils->getStatusString (gameInfo->cocktail);
		statusName = QT_TR_NOOP("cocktail");
		labelCocktail->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
		statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";
	}

	labelProtection->hide();
	wStatus->layout()->removeWidget(labelProtection);
	if (gameInfo->protection != 64)
	{
		wStatus->layout()->addWidget(labelProtection);
		labelProtection->show();
		statusText = utils->getStatusString (gameInfo->protection);
		statusName = QT_TR_NOOP("protection");
		labelProtection->setPixmap(QPixmap(prefix + statusName + "_" + statusText + suffix));
		statusBuffer +=  tr(qPrintable(statusName)) + ": " + Utils::tr(qPrintable(statusText)) + "\n";
	}

	statusBuffer.chop(1);
	win->wStatus->setToolTip(statusBuffer);

	
//	setText(QString("E: %1").arg(status));
}

MainWindow::MainWindow(QWidget *parent) : 
QMainWindow(parent)
{
	currentDir = QDir::currentPath();

	dockCtrlNames = (QStringList() 
	   << QT_TR_NOOP("Snapshot")
	   << QT_TR_NOOP("Flyer")
	   << QT_TR_NOOP("Cabinet")
	   << QT_TR_NOOP("Marquee")
	   << QT_TR_NOOP("Title")
	   << QT_TR_NOOP("Control Panel")
	   << QT_TR_NOOP("PCB")
	
	   << QT_TR_NOOP("History")
	   << QT_TR_NOOP("MAMEInfo")
	   << QT_TR_NOOP("DriverInfo")
	   << QT_TR_NOOP("Story")
	   << QT_TR_NOOP("Command"));

	setupUi(this);

#ifdef Q_OS_MAC
	actionDefaultOptions->setText(tr("Preferences..."));
#endif

	//setExclusive(true) for some actions
    QActionGroup *viewActions = new QActionGroup(this);
    viewActions->addAction(actionDetails);
    viewActions->addAction(actionGrouped);
    viewActions->addAction(actionLargeIcons);

	QActionGroup *langActions = new QActionGroup(this);
	langActions->addAction(actionEnglish);
	langActions->addAction(actionChinese_PRC);
	langActions->addAction(actionChinese_Taiwan);
	langActions->addAction(actionJapanese);
	langActions->addAction(actionHungarian);
	langActions->addAction(actionKorean);
	langActions->addAction(actionBrazilian);

	QActionGroup *bgStretchActions = new QActionGroup(this);
	bgStretchActions->addAction(actionBgStretch);
	bgStretchActions->addAction(actionBgTile);

	// init controls
    tvGameList = new QTreeView(centralwidget);
    tvGameList->setRootIsDecorated(false);
    tvGameList->setItemsExpandable(false);
	tvGameList->setFrameShape(QFrame::NoFrame);
	tvGameList->hide();

	lvGameList = new QListView(centralwidget);
	lvGameList->setMovement(QListView::Static);
	lvGameList->setResizeMode(QListView::Adjust);
	lvGameList->setViewMode(QListView::IconMode);
//	lvGameList->setGridSize(QSize(96, 64));
	lvGameList->setUniformItemSizes(true);
	lvGameList->setWordWrap(true);
//	lvGameList->setTextElideMode(Qt::TextDontClip | Qt::TextWordWrap);
	lvGameList->setFrameShape(QFrame::NoFrame);
	lvGameList->hide();

	lineEditSearch = new QLineEdit(centralwidget);
	QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
	lineEditSearch->setSizePolicy(sizePolicy);
	lineEditSearch->setMinimumWidth(240);
	toolBarSearch->addWidget(lineEditSearch);

	btnSearch = new QToolButton(centralwidget);
	btnSearch->setIcon(QIcon(":/res/16x16/system-search.png"));
	btnSearch->setFixedWidth(24);
	btnSearch->setToolTip(tr("Search"));
	toolBarSearch->addWidget(btnSearch);
	
	btnClearSearch = new QToolButton(centralwidget);
	btnClearSearch->setIcon(QIcon(":/res/16x16/status_cross.png"));
	btnClearSearch->setFixedWidth(24);
	btnClearSearch->setToolTip(tr("Clear"));
	toolBarSearch->addWidget(btnClearSearch);

	labelProgress = new QLabel(statusbar);
	statusbar->addWidget(labelProgress);
	
	wStatus = new QWidget(statusbar);
	wStatus->setMaximumHeight(18);
	QHBoxLayout *layout = new QHBoxLayout(wStatus);
	layout->setMargin(0);
	layout->setSpacing(1);
	wStatus->setLayout(layout);
	statusbar->addPermanentWidget(wStatus);

	labelStatus = new QLabel(statusbar);
	layout->addWidget(labelStatus);
	labelEmulation = new QLabel(statusbar);
	layout->addWidget(labelEmulation);
	labelColor = new QLabel(statusbar);
	layout->addWidget(labelColor);
	labelSound = new QLabel(statusbar);
	layout->addWidget(labelSound);
	labelGraphic = new QLabel(statusbar);
	layout->addWidget(labelGraphic);
	labelSavestate = new QLabel(statusbar);
	layout->addWidget(labelSavestate);
	labelCocktail = new QLabel(statusbar);
	layout->addWidget(labelCocktail);
	labelProtection = new QLabel(statusbar);
	layout->addWidget(labelProtection);

	labelGameCount = new QLabel(statusbar);
	statusbar->addPermanentWidget(labelGameCount);

	progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->setMaximumHeight(16);
	progressBarGamelist->hide();

	QAction *actionFolderList = dwFolderList->toggleViewAction();
	actionFolderList->setIcon(QIcon(":/res/mame32-show-tree.png"));
	menuView->insertAction(actionVerticalTabs, actionFolderList);
	toolBar->insertAction(actionLargeIcons, actionFolderList);

	mameAuditor = new MameExeRomAuditor(this);

	mameGame = new MameGame(0);
	gameList = new Gamelist(0);
	optUtils = new OptionUtils(0);
	dirsUI = new Dirs(this);
	playOptionsUI = new PlayOptions(this);
	optionsUI = new Options(this);
	csvCfgUI = new CsvCfg(this);
	aboutUI = new About(this);
	ipsUI = new IPS(this);
#ifdef Q_OS_WIN
	m1UI = new M1UI(this);
#endif /* Q_OS_WIN */

	QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef USE_SDL
#ifdef Q_OS_WIN
	SDL_Quit();
#endif
#endif /* USE_SDL */
}

void MainWindow::initHistory(int snapType)
{
	QString title = dockCtrlNames[snapType];
	QDockWidget *dockWidget = new QDockWidget(this);

	dockWidget->setObjectName("dockWidget_" + title);
	dockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	QWidget *dockWidgetContents = new QWidget(dockWidget);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	QGridLayout *gridLayout = new QGridLayout(dockWidgetContents);
	gridLayout->setObjectName("gridLayout_" + title);
	gridLayout->setContentsMargins(0, 0, 0, 0);

	QTextBrowser * tb;
	if (snapType == DOCK_HISTORY)
	{
		tb = tbHistory = new QTextBrowser(dockWidgetContents);
		tbHistory->setOpenExternalLinks(true);
	}
	else if (snapType == DOCK_MAMEINFO)
		tb = tbMameinfo = new QTextBrowser(dockWidgetContents);
	else if (snapType == DOCK_DRIVERINFO)
		tb = tbDriverinfo = new QTextBrowser(dockWidgetContents);
	else if (snapType == DOCK_STORY)
		tb = tbStory = new QTextBrowser(dockWidgetContents);
	else
		tb = tbCommand = new QTextBrowser(dockWidgetContents);
	
	tb->setObjectName("textBrowser_" + title);
	tb->setFrameShape(QFrame::NoFrame);

	gridLayout->addWidget(tb);

	dockWidget->setWidget(dockWidgetContents);
	dockWidget->setWindowTitle(tr(qPrintable(title)));
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dwHistory)
		tabifyDockWidget(dwHistory, dockWidget);
	else
		dwHistory = dockWidget;

	menuDocuments->addAction(dockWidget->toggleViewAction());
	dockCtrls[snapType] = dockWidget;
}

void MainWindow::initSnap(int snapType)
{
	static Screenshot *dockWidget0 = NULL;
	
	Screenshot *dockWidget = new Screenshot(dockCtrlNames[snapType], this);

	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dockWidget0)
		tabifyDockWidget(dockWidget0, dockWidget);
	else
		dockWidget0 = dockWidget;

	menuPictures->addAction(dockWidget->toggleViewAction());
	dockCtrls[snapType] = dockWidget;
}

void MainWindow::init()
{
	enableCtrls(false);

	for (int i = DOCK_SNAP; i <= DOCK_PCB; i ++)
		initSnap(i);

	for (int i = DOCK_HISTORY; i <= DOCK_COMMAND; i ++)
		initHistory(i);

	ipsUI->init();

	// must call optUtils->init() after win, before show()
	optUtils->init();

	//rearrange docks
#ifdef Q_OS_WIN
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::LeftDockWidgetArea), m1UI);
	tabifyDockWidget(dwFolderList, m1UI);
#endif /* Q_OS_WIN */
	tabifyDockWidget(dwHistory, dwGUILog);

	//hide non popular docks by default
	dwGUILog->hide();

	for (int i = DOCK_SNAP; i < DOCK_LAST; i ++)
	{
		if (i != DOCK_SNAP
		 && i != DOCK_TITLE
		 && i != DOCK_HISTORY
		 && i != DOCK_MAMEINFO)
			dockCtrls[i]->hide();
	}

	dwFolderList->raise();
	dockCtrls[DOCK_SNAP]->raise();
	dockCtrls[DOCK_HISTORY]->raise();

	/* test ini readable/writable */
	// mkdir for individual game settings
	QDir().mkpath(CFG_PREFIX);

	QString warnings = "";
	QFile iniFile(CFG_PREFIX + "mamepgui" INI_EXT);
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append(QFileInfo(iniFile).absoluteFilePath());
	iniFile.close();

	if (warnings.size() > 0)
	{
		win->poplog("Current user has no sufficient privilege to read/write:\n" + warnings + "\n\ncouldn't save GUI settings.");
		//quit the program
		close();
		return;
	}

	initSettings();
	loadSettings();
	
	// validate mame_binary
	mame_binary = pGuiSettings->value("mame_binary", "mamep.exe").toString();
	QFileInfo mamebin(mame_binary);

	// if no valid exec was found, popup a dialog
	if (!mamebin.exists() || mamebin.absoluteFilePath() == QCoreApplication::applicationFilePath())
	{
		QString filter = "";
#ifdef Q_WS_WIN
		filter.append(tr("Executable files") + " (*" EXEC_EXT ")");
		filter.append(";;");
#endif
		filter.append(tr("All Files (*)"));
	
		mame_binary = QFileDialog::getOpenFileName(this,
									tr("MAME/MESS executable:"),
									QCoreApplication::applicationDirPath(),
									filter);

		mamebin.setFile(mame_binary);
		if (mame_binary.isEmpty() || mamebin.absoluteFilePath() == QCoreApplication::applicationFilePath())
		{
			win->poplog(QString("Could not find MAME/MESS."));
			mame_binary = "";
			//quit the program
			close();
			return;
		}
	}

	//save the new mame_binary value now, it will be accessed later in option module
	pGuiSettings->setValue("mame_binary", mame_binary);
	if (QFileInfo(mame_binary).baseName().contains("mess", Qt::CaseInsensitive))
		isMESS = true;

	QIcon mamepIcon(":/res/mamep_256.png");
	qApp->setWindowIcon(mamepIcon);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(mamepIcon);

	// must init app style before background
	if (gui_style.isEmpty())
		gui_style = pGuiSettings->value("gui_style").toString();

#ifdef Q_WS_MAC
	//fixme: temp hack, aqua theme is buggy
	if (gui_style.isEmpty() || gui_style.startsWith("macintosh", Qt::CaseInsensitive))
		gui_style = "Plastique";
#endif

	QStringList styles = QStyleFactory::keys();
	QActionGroup *styleActions = new QActionGroup(this);
	foreach (QString style, styles)
	{
		QAction* act = win->menuGUIStyle->addAction(style);
		act->setCheckable(true);
		if (gui_style == style)
			act->setChecked(true);
		styleActions->addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(setGuiStyle()));

#ifdef Q_WS_MAC
		//fixme: temp hack, aqua theme is buggy
		if (style.startsWith("macintosh", Qt::CaseInsensitive))
			act->setEnabled(false);
#endif
	}

	if (!gui_style.isEmpty())
		setGuiStyle(gui_style);

	// init background menu
	QString _dirpath = utils->getPath(pGuiSettings->value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	
	if (background_file.isEmpty())
		background_file = pGuiSettings->value("background_file", "bkground.png").toString();

	QActionGroup *bgActions = new QActionGroup(this);
	if (dir.exists())
	{
		QString dirpath = utils->getPath(_dirpath);
		
		QStringList nameFilter;
		nameFilter << "*" PNG_EXT;
		nameFilter << "*.jpg";
	
		// iterate all files in the path
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		foreach (QString fileName, files)
		{
			QAction* act = win->menuBackground->addAction(fileName);
			act->setCheckable(true);
			if (background_file == fileName)
				act->setChecked(true);
			bgActions->addAction(act);
			connect(act, SIGNAL(triggered()), this, SLOT(setBgPixmap()));
		}
	}

	//show UI
	loadLayout();
	setDockOptions();

	//after loadLayout(), so we can check background stretch option
	if (!background_file.isEmpty())
		setBgPixmap(background_file);

	mameGame->init();
	show();

	// connect misc signal and slots

	// Docked snapshots
	QList<QTabBar *> tabBars = getSSTabBars();
	foreach (QTabBar *tabBar, tabBars)
	{
		connect(tabBar, SIGNAL(currentChanged(int)), gameList, SLOT(updateSelection()));
		tabBar->setMovable(true);
	}

	connect(actionBgStretch, SIGNAL(triggered()), this, SLOT(setBgTile()));
	connect(actionBgTile, SIGNAL(triggered()), this, SLOT(setBgTile()));

	// Actions
	connect(actionVerticalTabs, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));
	connect(actionLargeIcons, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionDetails, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionGrouped, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));

	connect(actionRowDelegate, SIGNAL(toggled(bool)), gameList, SLOT(toggleDelegate(bool)));
	connect(actionStretchSshot, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));
	connect(actionEnforceAspect, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));

	// Auditor
	connect(&romAuditor, SIGNAL(progressSwitched(int, QString)), gameList, SLOT(switchProgress(int, QString)));
	connect(&romAuditor, SIGNAL(progressUpdated(int)), gameList, SLOT(updateProgress(int)));
	connect(&romAuditor, SIGNAL(finished()), &mergedAuditor, SLOT(audit()));

	connect(&mergedAuditor, SIGNAL(progressSwitched(int, QString)), gameList, SLOT(switchProgress(int, QString)));
	connect(&mergedAuditor, SIGNAL(progressUpdated(int)), gameList, SLOT(updateProgress(int)));
	connect(&mergedAuditor, SIGNAL(finished()), gameList, SLOT(init()));

	// Game List
	connect(lineEditSearch, SIGNAL(returnPressed()), gameList, SLOT(filterSearchChanged()));
	connect(btnSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchChanged()));
	connect(btnClearSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchCleared()));

	// Tray Icon
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(on_trayIconActivated(QSystemTrayIcon::ActivationReason)));

#ifdef Q_OS_WIN
	//init M1 in a background thread
	m1 = new M1(this);
	m1->init();
#endif /* Q_OS_WIN */

//	gameList->restoreGameSelection();
//	gameList->updateSelection();
}

void MainWindow::setVersion()
{
	//set version info

	QString mameString;
	QString m1Ver = "";
	QString m1VerString = "";
	QString sdlVerString = "";

	if (!isMESS)
		mameString = QString("<a href=\"http://mamedev.org\">M.A.M.E.</a> %1 - Multiple Arcade Machine Emulator &copy; Nicola Salmoria and the MAME Team<br>").arg(mameGame->mameVersion);
	else
		mameString = QString("<a href=\"http://www.mess.org\">M.E.S.S.</a> %1 - Multi Emulator Super System &copy; The MESS Team<br>").arg(mameGame->mameVersion);

#ifdef Q_OS_WIN
	if (m1 != NULL && m1->available)
	{
		m1Ver = m1->version;
		m1VerString = QString("<a href=\"http://rbelmont.mameworld.info/?page_id=223\">M1</a> %1 multi-platform arcade music emulator &copy; R. Belmont<br>")
						.arg(m1Ver);
	}
#endif /* Q_OS_WIN */

#ifdef USE_SDL
	sdlVerString = QString("<a href=\"http://www.libsdl.org\">SDL</a> %1.%2.%3-%4 - Simple DirectMedia Layer<br>")
					.arg(SDL_MAJOR_VERSION)
					.arg(SDL_MINOR_VERSION)
					.arg(SDL_PATCHLEVEL)
					.arg(SDL_REVISION)
					;
#endif /* USE_SDL */

	QString strVersion = QString(
		"<html>"
		"<head>"
		"<style type=\"text/css\">"
		"a {color: #006d9f;text-decoration: underline;}"
		"</style>"
		"</head>"
		"<body>"
		"<strong>M+GUI</strong> %1 &copy; 2008-2009 <a href=\"http://mameicons.free.fr/mame32p/\">MAME Plus!</a> Team<br>"
		"A Qt implementation of <a href=\"http://mameui.classicgaming.gamespy.com\">MameUI</a>"
		"<hr>"
		"%2"
		"<a href=\"http://trolltech.com\">Qt</a> %3 &copy; Nokia Corporation<br>"
		"%4"
		"%5"
		"%6"
		"</body>"
		"</html>")
		.arg("1.4.5a")
		.arg(mameString)
		.arg(QT_VERSION_STR)
		.arg(sdlVerString)
		.arg(m1VerString)
		.arg("<a href=\"http://www.7-zip.org\">LZMA SDK</a> " MY_VERSION_COPYRIGHT_DATE)
		;

	aboutUI->tbVersion->setHtml(strVersion);
#ifdef Q_OS_WIN
	m1UI->setWindowTitle("M1 - " + m1Ver);
#endif /* Q_OS_WIN */

	QFileInfo fi(mame_binary);

	win->setWindowTitle(QString("%1 - %2 %3")
		.arg(win->windowTitle())
		.arg(fi.baseName().toUpper())
		.arg(mameGame->mameVersion));
}

void MainWindow::enableCtrls(bool isEnabled)
{
	win->treeFolders->setEnabled(isEnabled);
	win->actionLargeIcons->setEnabled(isEnabled);
	win->actionDetails->setEnabled(isEnabled);
	win->actionGrouped->setEnabled(isEnabled);
	win->actionRefresh->setEnabled(isEnabled);
	win->actionDirectories->setEnabled(isEnabled);
	win->actionProperties->setEnabled(isEnabled);
	win->actionSrcProperties->setEnabled(isEnabled);
	win->actionDefaultOptions->setEnabled(isEnabled);
	win->actionPlay->setEnabled(isEnabled);
	win->menuPlayWith->setEnabled(isEnabled);
	win->menuSaveFixdat->setEnabled(isEnabled);
	win->lineEditSearch->setEnabled(isEnabled);
	win->btnSearch->setEnabled(isEnabled);
	win->btnClearSearch->setEnabled(isEnabled);
}

void MainWindow::on_actionPlay_activated()
{
	gameList->runMame();
}

void MainWindow::on_actionSavestate_activated()
{
	playOptionsUI->initSavestate();
	playOptionsUI->exec();
}
void MainWindow::on_actionPlayback_activated()
{
	playOptionsUI->initPlayback();
	playOptionsUI->exec();
}

void MainWindow::on_actionRecord_activated()
{
	playOptionsUI->initRecord();
	playOptionsUI->exec();
}

void MainWindow::on_actionMNG_activated()
{
	playOptionsUI->initMNG();
	playOptionsUI->exec();
}

void MainWindow::on_actionAVI_activated()
{
	playOptionsUI->initAVI();
	playOptionsUI->exec();
}

void MainWindow::on_actionWave_activated()
{
	playOptionsUI->initWave();
	playOptionsUI->exec();
}

void MainWindow::on_actionConfigIPS_activated()
{
	ipsUI->updateList();
	ipsUI->exec();
}

void MainWindow::on_actionRefresh_activated()
{
	romAuditor.audit();
}

void MainWindow::on_actionFixDatAll_activated()
{
	exportFixDat(AUDIT_EXPORT_ALL);
}

void MainWindow::on_actionFixDatIncomplete_activated()
{
	exportFixDat(AUDIT_EXPORT_INCOMPLETE);
}

void MainWindow::on_actionFixDatMissing_activated()
{
	exportFixDat(AUDIT_EXPORT_MISSING);
}

void MainWindow::exportFixDat(int method)
{
	QString filter = "";
	filter.append(tr("Dat files") + " (*.dat)");
	filter.append(";;");
	filter.append(tr("All Files (*)"));

	QFileInfo mamebin(mame_binary);
	
	QString fileName = QFileDialog::getSaveFileName
		(0, tr("File name:"), mamebin.absolutePath(), filter);	

	if (!fileName.isEmpty())
		romAuditor.audit(false, method, fileName);
}

void MainWindow::on_actionAudit_activated()
{
	mameAuditor->audit();
}

void MainWindow::on_actionAuditAll_activated()
{
	mameAuditor->audit(true);
}

void MainWindow::on_actionSrcProperties_activated()
{
	if (!mameGame->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_SRC, 0);
	optionsUI->exec();
}

void MainWindow::on_actionProperties_activated()
{
	if (!mameGame->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_CURR, 0);
	optionsUI->exec();	
}

void MainWindow::on_actionDefaultOptions_activated()
{
	if (!mameGame->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_GLOBAL, 0);
	optionsUI->exec();
}

void MainWindow::on_actionDirectories_activated()
{
	if (!mameGame->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_GUI, 0);
	optionsUI->exec();
}

void MainWindow::on_actionExitStop_activated()
{
	close();
}

void MainWindow::on_actionReadme_activated()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=158710&view=collapsed"));
}

void MainWindow::on_actionFAQ_activated()
{
	if (language.startsWith("zh_"))
		QDesktopServices::openUrl(QUrl("http://bbs.wisestudio.org/viewthread.php?tid=504"));
	else
		QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=164054&view=collapsed"));
}

void MainWindow::on_actionBoard_activated()
{
	if (language.startsWith("zh_"))
		QDesktopServices::openUrl(QUrl("http://bbs.wisestudio.org/forum-16-1.html"));
	else
		QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/postlist.php?Cat=&Board=mameplus&view=collapsed"));
}

void MainWindow::on_actionAbout_activated()
{
	aboutUI->exec();
}

void MainWindow::toggleGameListColumn(int logicalIndex)
{
	if (win->tvGameList->header()->isSectionHidden(logicalIndex))
		win->tvGameList->header()->setSectionHidden (logicalIndex, false);
	else
		win->tvGameList->header()->setSectionHidden (logicalIndex, true);
}

void MainWindow::on_actionColDescription_activated()
{
	toggleGameListColumn(0);
}

void MainWindow::on_actionColName_activated()
{
	toggleGameListColumn(1);
}

void MainWindow::on_actionColROMs_activated()
{
	toggleGameListColumn(2);
}

void MainWindow::on_actionColManufacturer_activated()
{
	toggleGameListColumn(3);
}

void MainWindow::on_actionColDriver_activated()
{
	toggleGameListColumn(4);
}

void MainWindow::on_actionColYear_activated()
{
	toggleGameListColumn(5);
}

void MainWindow::on_actionColCloneOf_activated()
{
	toggleGameListColumn(6);
}

void MainWindow::on_actionEnglish_activated()
{
	language = "en_US";
	showRestartDialog();
}

void MainWindow::on_actionChinese_PRC_activated()
{
	language = "zh_CN";
	showRestartDialog();
}

void MainWindow::on_actionChinese_Taiwan_activated()
{
	language = "zh_TW";
	showRestartDialog();
}

void MainWindow::on_actionJapanese_activated()
{
	language = "ja_JP";
	showRestartDialog();
}

void MainWindow::on_actionHungarian_activated()
{
	language = "hu_HU";
	showRestartDialog();
}

void MainWindow::on_actionKorean_activated()
{
	language = "ko_KR";
	showRestartDialog();
}

void MainWindow::on_actionBrazilian_activated()
{
	language = "pt_BR";
	showRestartDialog();
}

void MainWindow::on_actionLocalGameList_activated()
{
	local_game_list = actionLocalGameList->isChecked();
}

void MainWindow::on_trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		if (win->isVisible())
			hide();
		else
			show();
		break;
	default:
		;
	}
}

void MainWindow::showRestartDialog()
{
	if (QMessageBox::Yes == QMessageBox::question(this, 
								tr("Restart"), 
								tr("Changing this option requires a restart to take effect.\nDo you wish to continue?"), 
								QMessageBox::Yes | QMessageBox::No))
		close();
}

void MainWindow::initSettings()
{
    pGuiSettings->setFallbacksEnabled(false);

	//remove invalid settings
	validGuiSettings = (QStringList() 
		<< "snapshot_directory"	//fixme: core
		<< "flyer_directory"
		<< "cabinet_directory"
		<< "marquee_directory"
		<< "title_directory"
		<< "cpanel_directory"
		<< "pcb_directory"
		
		<< "history_file"
		<< "mameinfo_file"
		<< "story_file"
		<< "command_file"	//fixme: core

		<< "icons_directory"
		<< "background_directory"
		<< "folder_directory"
		<< "background_file"
		<< "m1_directory"
		<< "m1_language"
		<< "ips_language"
		<< "ips_relationship"
		<< "gui_style"
		<< "background_stretch"
		<< "mame_binary"
		<< "option_geometry"
		<< "sort_column"
		<< "sort_reverse"
		<< "default_game"
		<< "default_folder"
		<< "hide_folders"
		<< "vertical_tabs"
		<< "stretch_screenshot_larger"
		<< "enforce_aspect"
		<< "game_list_delegate"
		<< "local_game_list"
		<< "list_mode"
		<< "option_column_state"
		<< "window_geometry"
		<< "window_state"
		<< "column_state"
		<< "language");

	QStringList keys = pGuiSettings->allKeys();
	foreach(QString key, keys)
	{
		if (key.endsWith("_extra_software") || validGuiSettings.contains(key))
			continue;

		pGuiSettings->remove(key);
	}

}

void MainWindow::loadLayout()
{
	if (pGuiSettings->value("window_geometry").isValid())
		restoreGeometry(pGuiSettings->value("window_geometry").toByteArray());

	if (pGuiSettings->value("window_state").isValid())
		restoreState(pGuiSettings->value("window_state").toByteArray());
	
	option_geometry = pGuiSettings->value("option_geometry").toByteArray();

	actionVerticalTabs->setChecked(pGuiSettings->value("vertical_tabs", "1").toInt() == 1);
	actionRowDelegate->setChecked(pGuiSettings->value("game_list_delegate", "0").toInt() == 1);

	gameList->listMode = pGuiSettings->value("list_mode").toString();
	if (gameList->listMode == win->actionDetails->objectName().remove("action"))
		actionDetails->setChecked(true);
	else if (gameList->listMode == win->actionLargeIcons->objectName().remove("action"))
		actionLargeIcons->setChecked(true);
	else
		actionGrouped->setChecked(true);

	//fixme: use built-in locale name string?
	if (language == "zh_CN")
		actionChinese_PRC->setChecked(true);
	else if (language == "zh_TW")
		actionChinese_Taiwan->setChecked(true);
	else if (language == "ja_JP")
		actionJapanese->setChecked(true);
	else if (language == "hu_HU")
		actionHungarian->setChecked(true);
	else if (language == "ko_KR")
		actionKorean->setChecked(true);
	else if (language == "pt_BR")
		actionBrazilian->setChecked(true);
	else
		actionEnglish->setChecked(true);

	actionStretchSshot->setChecked(pGuiSettings->value("stretch_screenshot_larger", "0").toInt() == 1);
	actionEnforceAspect->setChecked(pGuiSettings->value("enforce_aspect", "1").toInt() == 1);

	local_game_list = pGuiSettings->value("local_game_list", "1").toInt() == 1;
	actionLocalGameList->setChecked(local_game_list);

	if (pGuiSettings->value("background_stretch", "1").toInt() == 0)
		actionBgTile->setChecked(true);
	else
		actionBgStretch->setChecked(true);
	
}

void MainWindow::loadSettings()
{
	currentGame = pGuiSettings->value("default_game").toString();

	if (defSettings.value("option_column_state").isValid())
		option_column_state = defSettings.value("option_column_state").toByteArray();
	else
		option_column_state = pGuiSettings->value("option_column_state").toByteArray();
}

void MainWindow::saveSettings()
{
	//some guiSettings uses mameOpts mapping for dialog view
	pGuiSettings->setValue("cabinet_directory", mameOpts["cabinet_directory"]->globalvalue);
	pGuiSettings->setValue("cpanel_directory", mameOpts["cpanel_directory"]->globalvalue);
	pGuiSettings->setValue("flyer_directory", mameOpts["flyer_directory"]->globalvalue);
	pGuiSettings->setValue("marquee_directory", mameOpts["marquee_directory"]->globalvalue);
	pGuiSettings->setValue("pcb_directory", mameOpts["pcb_directory"]->globalvalue);
	pGuiSettings->setValue("title_directory", mameOpts["title_directory"]->globalvalue);
	pGuiSettings->setValue("icons_directory", mameOpts["icons_directory"]->globalvalue);
	pGuiSettings->setValue("background_directory", mameOpts["background_directory"]->globalvalue);
	pGuiSettings->setValue("folder_directory", mameOpts["folder_directory"]->globalvalue);

	pGuiSettings->setValue("background_file", background_file);
	pGuiSettings->setValue("m1_directory", mameOpts["m1_directory"]->globalvalue);
#ifdef Q_OS_WIN
	pGuiSettings->setValue("m1_language", m1UI->cmbLang->currentText());
#endif /* Q_OS_WIN */
	pGuiSettings->setValue("ips_language", ipsUI->cmbLang->currentText());
	pGuiSettings->setValue("ips_relationship", ipsUI->chkRelation->isChecked() ? 1 : 0);
	pGuiSettings->setValue("gui_style", gui_style);
	pGuiSettings->setValue("language", language);

	pGuiSettings->setValue("history_file", mameOpts["history_file"]->globalvalue);
	pGuiSettings->setValue("story_file", mameOpts["story_file"]->globalvalue);
	pGuiSettings->setValue("mameinfo_file", mameOpts["mameinfo_file"]->globalvalue);
	if (mameOpts["mame_binary"]->globalvalue != mameOpts["mame_binary"]->defvalue)
		pGuiSettings->setValue("mame_binary", mameOpts["mame_binary"]->globalvalue);
	else
		pGuiSettings->setValue("mame_binary", mame_binary);

	QList<QTreeWidgetItem *> softwaresItems = win->treeFolders->findItems(gameList->intFolderNames[FOLDER_CONSOLE], Qt::MatchFixedString);
	QTreeWidgetItem *softwaresItem = NULL;
	if (!softwaresItems.isEmpty())
		softwaresItem = softwaresItems.first();

	//save console dirs
//	int iNext = 0;
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		if (pMameOpt->guivisible && optName.endsWith("_extra_software"))
		{
			QString sysName = optName;
			sysName.remove("_extra_software");

			QString v = mameOpts[optName]->globalvalue;
			if (!v.trimmed().isEmpty())
			{
				pGuiSettings->setValue(optName, mameOpts[optName]->globalvalue);

				if (softwaresItem != NULL)
					for (int i = 0; i < softwaresItem->childCount(); i++)
					{
						if (sysName == softwaresItem->child(i)->text(0))
						{
							softwaresItem->child(i)->setHidden(false);
							break;
						}
					}
			}
			else
			{
				pGuiSettings->remove(optName);

				if (softwaresItem != NULL)
				{
					for (int i = 0; i < softwaresItem->childCount(); i++)
					{
						if (sysName == softwaresItem->child(i)->text(0))
						{
							softwaresItem->child(i)->setHidden(true);
/*							iNext = i + 1;

							if (iNext >= softwaresItem->childCount())
								iNext = i - 1;

							if (iNext < 0)
								iNext = 0;
*/
							break;
						}
					}
					
				}
			}
		}
	}
//	win->treeFolders->setCurrentItem(softwaresItem->child(iNext));

	//prepare default_folder
	QString folderName, subFolderName, defalutFolder;
	QStringList strlist = utils->split2Str(currentFolder, "/");

	folderName = strlist.first();
	subFolderName = strlist.last();

	if (folderName.isEmpty())
	{
		folderName = strlist.last();
		subFolderName.clear();
	}

	int p = gameList->intFolderNames.indexOf(folderName);
	if (p >= 0)
		folderName = gameList->intFolderNames0[p];

	if (subFolderName.isEmpty())
		defalutFolder = "/" + folderName;
	else
		defalutFolder = folderName + "/" + subFolderName;

	//save layout
	pGuiSettings->setValue("window_geometry", saveGeometry());
	pGuiSettings->setValue("window_state", saveState());
	pGuiSettings->setValue("option_geometry", option_geometry);
	pGuiSettings->setValue("option_column_state", option_column_state);
	pGuiSettings->setValue("column_state", tvGameList->header()->saveState());
	pGuiSettings->setValue("sort_column", tvGameList->header()->sortIndicatorSection());
	pGuiSettings->setValue("sort_reverse", (tvGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder) ? 0 : 1);
	pGuiSettings->setValue("vertical_tabs", actionVerticalTabs->isChecked() ? 1 : 0);
	pGuiSettings->setValue("stretch_screenshot_larger", actionStretchSshot->isChecked() ? 1 : 0);
	pGuiSettings->setValue("enforce_aspect", actionEnforceAspect->isChecked() ? 1 : 0);
	pGuiSettings->setValue("game_list_delegate", actionRowDelegate->isChecked() ? 1 : 0);
	pGuiSettings->setValue("local_game_list", actionLocalGameList->isChecked() ? 1 : 0);
	pGuiSettings->setValue("background_stretch", actionBgTile->isChecked() ? 0 : 1);
	pGuiSettings->setValue("default_game", currentGame);
	pGuiSettings->setValue("default_folder", defalutFolder);
	pGuiSettings->setValue("list_mode", gameList->listMode);
	pGuiSettings->setValue("hide_folders", hiddenFolders);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	if (!mame_binary.isEmpty())
	{
		saveSettings();
		mameGame->s11n();
	}
	event->accept();
}

void MainWindow::setDockOptions()
{
	DockOptions opts = dockOptions();
    if (actionVerticalTabs->isChecked())
	    opts |= VerticalTabs;
    else
	    opts &= ~VerticalTabs;
	QMainWindow::setDockOptions(opts);
}

void MainWindow::setTransparentBg(QWidget * w)
{
	QPalette palette;
	QBrush brush, bgBrush;

	brush.setStyle(Qt::SolidPattern);
	bgBrush.setStyle(Qt::SolidPattern);

	if (isDarkBg)
	{
		bgBrush.setColor(QColor(0, 0, 0, 128));
		brush.setColor(QColor(Qt::white));
	}
	else
	{
		bgBrush.setColor(QColor(255, 255, 255, 128));
		brush.setColor(QColor(Qt::black));
	}

	palette.setBrush(QPalette::Active, QPalette::Base, bgBrush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, bgBrush);
	palette.setBrush(QPalette::Disabled, QPalette::Base, bgBrush);

	palette.setBrush(QPalette::Active, QPalette::Text, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
	palette.setBrush(QPalette::Disabled, QPalette::Text, brush);

	w->setPalette(palette);
}

void MainWindow::setTransparentStyle(QWidget * w)
{
	QString style;
	if (isDarkBg)
		style =
			"color: rgba(255, 255, 255, 196);"
			"background-color: rgba(0, 0, 0, 128);"
			;
	else
		style =
			"color: rgba(0, 0, 0, 196);"
			"background-color: rgba(255, 255, 255, 128);"
			;
	
	w->setStyleSheet(style);
}

void MainWindow::resizeEvent(QResizeEvent * /* event */)
{
//	setBgPixmap(background_file);
}

void MainWindow::setGuiStyle(QString style)
{
	if (style.isEmpty())
		style = ((QAction *)sender())->text();

	gui_style = style;

	QApplication::setStyle(QStyleFactory::create(gui_style));

	//have to chain setBgPixmap() otherwise bgcolor isn't set properly
	if (!background_file.isEmpty())
		setBgPixmap(background_file);
}

void MainWindow::setBgTile()
{
	setBgPixmap(background_file);
}

void MainWindow::setBgPixmap(QString fileName)
{
	if (fileName.isEmpty())
		fileName = ((QAction *)sender())->text();

	background_file = fileName;

	QString _dirpath = utils->getPath(pGuiSettings->value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	QString dirpath = utils->getPath(_dirpath);

	QImage bkgroundImg(_dirpath + fileName);

	// setup mainwindow background
	if (!bkgroundImg.isNull())
	{
		if (actionBgStretch->isChecked())
			bkgroundImg = bkgroundImg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

		//fixme: must init before stylesheet applied
		QPalette palette = win->palette();
		palette.setBrush(backgroundRole(), QBrush(bkgroundImg));
//		win->setPalette(palette);

		//get the color tone of bg image
		bkgroundImg = bkgroundImg.scaled(1, 1, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		int grayVal = qGray(bkgroundImg.pixel(0, 0));
		if (grayVal < 128)
			isDarkBg = true;
		else
			isDarkBg = false;

		QString TEXT_COLOR = QString("color:") + (isDarkBg ? "white" : "black") + "; ";
		QString TEXT_BGCOLOR = isDarkBg ? "background-color: rgba(0, 0, 0, 128)" : "background-color: rgba(255, 255, 255, 128)";
		static const QString STATIC_STYLE = 
			" QToolBar{background-color:palette(window);}"
			" QStatusBar::item, QDockWidget::title {border-width:1px; border-style:solid; border-color:palette(dark);}"
			" QStatusBar QLabel {padding:0 1px;}"
			" QDockWidget::title {padding:1px 2px; margin:2px 0;}"
			;

		// setup background alpha
		setTransparentBg(treeFolders);
		setTransparentBg(tvGameList);
		setTransparentBg(lvGameList);
		setTransparentBg(tbHistory);
		setTransparentBg(tbMameinfo);
		setTransparentBg(tbDriverinfo);
		setTransparentBg(tbStory);
		setTransparentBg(tbCommand);
#ifdef Q_OS_WIN
		setTransparentBg(m1UI->twSongList);
		setTransparentStyle(m1UI->groupBox);
		setTransparentStyle(m1UI->lcdNumber);
#endif /* Q_OS_WIN */

///*
		qApp->setStyleSheet( STATIC_STYLE
			+ " QDockWidget, QStatusBar QLabel {" + TEXT_COLOR + "}"
			+ " QStatusBar::item, QDockWidget::title {" + TEXT_BGCOLOR + "}"
			);
//*/

		setPalette(palette);
	}
}

void MainWindow::toggleTrayIcon(int, QProcess::ExitStatus, bool isTrayIconVisible)
{
	win->trayIcon->setVisible(isTrayIconVisible);
	if (isTrayIconVisible)
		win->trayIcon->setToolTip(win->windowTitle());

	win->setVisible(!isTrayIconVisible);
}

QList<QTabBar *> MainWindow::getSSTabBars()
{
	//there's no API in Qt to access docked widget tabbar, we have to do it on our own
	QList<QTabBar *> tabBars = findChildren<QTabBar *>();
	QList<QTabBar *> tabBars2;

	//iterate all tab ctrls
	foreach (QTabBar *tabBar, tabBars)
	{
		bool isSSDocked = false;

		//iterate known screenshot/history dock names
		for (int i = 0; i < dockCtrlNames.count(); i++)
		{
			//iterate tabs in a tab bar
			for (int t = 0; t < tabBar->count(); t ++)
			{
				//if the tab contains any known screenshot/history dock names
				if (tr(qPrintable(dockCtrlNames[i])) == tabBar->tabText(t))
				{
					isSSDocked = true;
					break;
				}
			}
			if (isSSDocked)
				break;
		}

		if (isSSDocked)
			tabBars2.append(tabBar);
	}

	return tabBars2;
}

bool MainWindow::isDockTabVisible(QString objName)
{
	bool isSSTabbed = false;
	QList<QTabBar *> tabBars = getSSTabBars();
	objName = tr(qPrintable(objName));

	//iterate all tab ctrls
	foreach (QTabBar *tabBar, tabBars)
	{
		//tab bar's current index matches $objName
		if (objName == tabBar->tabText(tabBar->currentIndex()))
			return true;

		//iterate tabs in a tab bar
		for (int t = 0; t < tabBar->count(); t ++)
		{
			//if $objName is tabified in any of the tab ctrls
			if (objName == tabBar->tabText(t))
			{
				isSSTabbed = true;
				break;
			}
		}
	}

	// if the dock area contains only one SS widget
	if (!isSSTabbed)
		return true;

	return false;
}

Screenshot::Screenshot(QString title, QWidget *parent)
: QDockWidget(parent),
forceAspect(false)
{
	setObjectName("dockWidget_" + title);
	setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	dockWidgetContents = new QWidget(this);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	mainLayout = new QGridLayout(dockWidgetContents);
	mainLayout->setObjectName("mainLayout_" + title);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	screenshotLabel = new QPushButton(dockWidgetContents);
	screenshotLabel->setObjectName("label_" + title);
	screenshotLabel->setCursor(QCursor(Qt::PointingHandCursor));
	screenshotLabel->setFlat(true);

	//so that we can shrink image
	screenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	
	mainLayout->addWidget(screenshotLabel);

	setWidget(dockWidgetContents);
	setWindowTitle(MainWindow::tr(qPrintable(title)));

	connect(screenshotLabel, SIGNAL(clicked()), this, SLOT(rotateImage()));
}

void Screenshot::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	updateScreenshotLabel();
}

void Screenshot::setPixmap(QPixmap pm)
{
	originalPixmap = pm;
	forceAspect = false;
	updateScreenshotLabel();
}

void Screenshot::setPixmap(const QByteArray &pmdata, bool _forceAspect)
{
	QPixmap pm;
	pm.loadFromData(pmdata);
	originalPixmap = pm;

	forceAspect = _forceAspect;
	updateScreenshotLabel();
}

//click screenshot area to rotate dockwidgets
void Screenshot::rotateImage()
{
	QString objName = ((QWidget* )sender())->objectName();
	objName.remove(0, 6);	//remove "label_"

	//there's no API in Qt to access docked widget tabbar
	QList<QTabBar *> tabs = win->findChildren<QTabBar *>();
	foreach (QTabBar *tab, tabs)
	{
		bool isDock = false;

		// if the dock widget contains any of screenshot/history widgets
		for (int i = 0; i < win->dockCtrlNames.count(); i++)
		{
			if (MainWindow::tr(qPrintable(win->dockCtrlNames[i])) == tab->tabText(0))
			{
				isDock = true;
				break;
			}
		}

		// select the next tab
		if (isDock && MainWindow::tr(qPrintable(objName)) == tab->tabText(tab->currentIndex()))
		{
			int i = tab->currentIndex();
			if (++i > tab->count() - 1)
				i = 0;
			tab->setCurrentIndex(i);
			break;
		}
	}
}

void Screenshot::updateScreenshotLabel()
{
    QSize scaledSize = utils->getScaledSize(originalPixmap.size(), screenshotLabel->size(), forceAspect);

	screenshotLabel->setIconSize(scaledSize);
	screenshotLabel->setIcon(originalPixmap.scaled(scaledSize,
			Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

int main(int argc, char *argv[])
{
#ifdef USE_SDL
	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) >= 0)
		sdlInited = true;
#endif /* USE_SDL */

	for (int i = 1; i < argc; i++)
	{
		if (QString(argv[i]) == "-configpath" && i + 1 < argc)
		{
			CFG_PREFIX = utils->getPath(argv[i + 1]);
			break;
		}
	}

	pGuiSettings = new QSettings(CFG_PREFIX + "mamepgui" INI_EXT, QSettings::IniFormat);

	QApplication myApp(argc, argv);

	QTranslator appTranslator;

	language = pGuiSettings->value("language").toString();
	if (language.isEmpty())
		language = QLocale::system().name();
	appTranslator.load(":/lang/mamepgui_" + language);

	myApp.installTranslator(&appTranslator);

	if (language.startsWith("zh_") || language.startsWith("ja_"))
	{
		QFont font;
		font.setPointSize(9);
		myApp.setFont(font);
	}
	
	procMan = new ProcessManager(0);	
	utils = new Utils(0);
	win = new MainWindow(0);

	return myApp.exec();

#ifdef USE_SDL
	if(SDL_WasInit(SDL_INIT_VIDEO) != 0)
	{
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
	}
#endif /* USE_SDL */
}

