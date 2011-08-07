#include <QtPlugin>

#include "7zVersion.h"

#include "mainwindow.h"
#include "prototype.h"
#include "utils.h"
#include "processmanager.h"
#include "gamelist.h"
#include "screenshot.h"
#include "audit.h"
#include "mameopt.h"
#include "dialogs.h"
#include "ips.h"
#include "m1.h"

#ifdef USE_SDL
#undef main
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
QSettings defaultGuiSettings(":/res/mamepgui" INI_EXT, QSettings::IniFormat);
QString currentAppDir;
QString mame_binary;
QString language;
bool local_game_list;
bool isDarkBg = false;
bool sdlInited = false;
bool isMESS = false;

QStringList validGuiSettings;

#define MPGUI_VER "1.5.2"

void MainWindow::log(QString message)
{
	emit logUpdated(message);
}

void MainWindow::updateLog(QString message)
{
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	tbGUILog->append(msg);
	tbGUILog->horizontalScrollBar()->setValue(0);
}

void MainWindow::poplog(QString message)
{
	//explicitly declare the dialog so that it can be closed by joystick
	pop->setText(message);
	pop->exec(); 
}

void MainWindow::logStatus(QString message)
{
	if (currentAppDir != QDir::currentPath())
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
		gameInfo = pMameDat->games[gameInfo->romof];

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
	wStatus->setToolTip(statusBuffer);

//	setText(QString("E: %1").arg(status));
}

MainWindow::MainWindow(QWidget *parent) : 
	QMainWindow(parent),
	dwHistory(NULL)
{
	setupUi(this);

	currentAppDir = QDir::currentPath();

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

#ifdef Q_OS_MAC
	actionDefaultOptions->setText(tr("Preferences..."));

	//macx font hack
	QFont font = QApplication::font();
	font.setPixelSize(13);
	qApp->setFont(font);
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
	langActions->addAction(actionRussian);
	langActions->addAction(actionItalian);

	QActionGroup *bgStretchActions = new QActionGroup(this);
	bgStretchActions->addAction(actionBgStretch);
	bgStretchActions->addAction(actionBgTile);

	// init controls
    tvGameList = new GameListTreeView(centralwidget);
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

	pop = new QMessageBox(this);
	pop->setObjectName("msgWarning");

	QAction *actionFolderList = dwFolderList->toggleViewAction();
	actionFolderList->setIcon(QIcon(":/res/mame32-show-tree.png"));
	menuShowFolders->addAction(actionFolderList);
	menuShowFolders->addSeparator();
	toolBar->insertAction(actionLargeIcons, actionFolderList);

	romAuditor = new RomAuditor(this);
	mameAuditor = new MameExeRomAuditor(this);

	pMameDat = new MameDat(0, 0);
	gameList = new Gamelist(0);
	optUtils = new OptionUtils(0);
	dirsUI = new DirsUI(this);
	playOptionsUI = new PlayOptionsUI(this);
	optionsUI = new OptionsUI(this);
	csvCfgUI = new CsvCfgUI(this);
	aboutUI = new AboutUI(this);
	ipsUI = new IpsUI(this);
	cmdUI = new CmdUI(this);
#ifdef Q_OS_WIN
	m1Core = new M1Core(0);
	m1UI = new M1UI(this);
#endif /* Q_OS_WIN */

#ifdef USE_SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_NOPARACHUTE) >= 0)
		sdlInited = true;
#endif /* USE_SDL */

	QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef USE_SDL
//	if (joystick == NULL)
//		;
//	else
//		SDL_JoystickClose(joystick);

	SDL_Quit();
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
	// Log
	connect(this, SIGNAL(logUpdated(QString)), this, SLOT(updateLog(QString)));

	enableCtrls(false);

	for (int i = DOCK_SNAP; i <= DOCK_PCB; i ++)
		initSnap(i);

	for (int i = DOCK_HISTORY; i <= DOCK_COMMAND; i ++)
		initHistory(i);

	ipsUI->init();

	// must call optUtils->init() after win, before show()
	optUtils->init();

#ifdef Q_OS_WIN
	//fixme: win font hack
	if (language.startsWith("zh_") || language.startsWith("ja_"))
	{
		QFont font;
		font.setFamily("MS Gothic");
		font.setFixedPitch(true);
		tbCommand->setFont(font);
//		tbCommand->setLineWrapMode(QTextEdit::NoWrap);
	}

	//rearrange docks
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
		poplog("Current user has no sufficient privilege to read/write:\n" + warnings + "\n\ncouldn't save GUI settings.");
		//quit the program
		close();
		return;
	}

	loadGuiSettings();
	
	if (!validateMameBinary())
	{
			//quit the program
			close();
			return;
	}

	QIcon mamepIcon(":/res/mamep_256.png");
	qApp->setWindowIcon(mamepIcon);
	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setIcon(mamepIcon);

	// must init app style before background
	if (gui_style.isEmpty())
		gui_style = pGuiSettings->value("gui_style").toString();

	QStringList styles = QStyleFactory::keys();
	QActionGroup *styleActions = new QActionGroup(this);
	foreach (QString style, styles)
	{
		QAction* act = menuGUIStyle->addAction(style);
		act->setCheckable(true);
		if (gui_style == style)
			act->setChecked(true);
		styleActions->addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(setGuiStyle()));
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
			QAction* act = menuBackground->addAction(fileName);
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

	int des11n_status = pMameDat->load();
	if (des11n_status == QDataStream::Ok)
	{
		gameList->init(true, GAMELIST_INIT_FULL);
		setVersion();
	}
	else
	//des11n() failed, construct a new pMameDat
	{
		show();
		pTempDat = pMameDat;
		pMameDat = new MameDat(0, 1);
	}

	// connect misc signals and slots

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
//	connect(actionLargeIcons, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionDetails, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));
	connect(actionGrouped, SIGNAL(toggled(bool)), gameList, SLOT(init(bool)));

	connect(actionStretchSshot, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));
	connect(actionEnforceAspect, SIGNAL(toggled(bool)), gameList, SLOT(updateSelection()));

	connect(actionFilterClones, SIGNAL(toggled(bool)), gameList, SLOT(filterFlagsChanged(bool)));
	connect(actionFilterNonWorking, SIGNAL(toggled(bool)), gameList, SLOT(filterFlagsChanged(bool)));
	connect(actionFilterUnavailable, SIGNAL(toggled(bool)), gameList, SLOT(filterFlagsChanged(bool)));

	// Auditor
	connect(romAuditor, SIGNAL(progressSwitched(int, QString)), gameList, SLOT(switchProgress(int, QString)));
	connect(romAuditor, SIGNAL(progressUpdated(int)), gameList, SLOT(updateProgress(int)));
	connect(romAuditor, SIGNAL(finished()), gameList, SLOT(init()));

	// Game List
	connect(lineEditSearch, SIGNAL(returnPressed()), gameList, SLOT(filterSearchChanged()));
	connect(btnSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchChanged()));
	connect(btnClearSearch, SIGNAL(clicked()), gameList, SLOT(filterSearchCleared()));

	// Tray Icon
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

//	gameList->restoreGameSelection();
//	gameList->updateSelection();
}

bool MainWindow::validateMameBinary()
{
	// validate mame_binary
	mame_binary = pGuiSettings->value("mame_binary", "mamep.exe").toString();
	pMameDat->mameVersion = utils->getMameVersion();
	QFileInfo mamebin(mame_binary);

	// if no valid exec was found
	if (mame_binary.isEmpty() ||
		mamebin.absoluteFilePath() == QCoreApplication::applicationFilePath() ||
		pMameDat->mameVersion.isEmpty())
	{
		mame_binary = selectMameBinary();
		mamebin.setFile(mame_binary);
		pMameDat->mameVersion = utils->getMameVersion();

		if (mame_binary.isEmpty() ||
			mamebin.absoluteFilePath() == QCoreApplication::applicationFilePath() ||
			pMameDat->mameVersion.isEmpty())
		{
			poplog(QString("Could not find valid MAME/MESS."));
			mame_binary = "";
			return false;
		}
	}

	//save the new mame_binary value now, it will be accessed later in option module
	pGuiSettings->setValue("mame_binary", mame_binary);
	if (QFileInfo(mame_binary).baseName().contains("mess", Qt::CaseInsensitive))
		isMESS = true;

	return true;
}

QString MainWindow::selectMameBinary()
{
	QString filter = "";
#ifdef Q_WS_WIN
	filter.append(tr("Executable files") + " (*" EXEC_EXT ")");
	filter.append(";;");
#endif
	filter.append(tr("All Files (*)"));

	return QFileDialog::getOpenFileName(this,
								tr("MAME/MESS executable:"),
								QCoreApplication::applicationDirPath(),
								filter);
}

void MainWindow::setVersion()
{
	//set version info

	QString mameString;
	QString m1Ver = "";
	QString m1VerString = "";
	QString sdlVerString = "";

	if (!isMESS)
		mameString.append(QString("<a href=\"http://mamedev.org\">M.A.M.E.</a> %1 - Multiple Arcade Machine Emulator &copy; Nicola Salmoria and the MAME Team<br>").arg(pMameDat->mameVersion));
	if (hasDevices)
		mameString.append(QString("<a href=\"http://www.mess.org\">M.E.S.S.</a> %1 - Multi Emulator Super System &copy; the MESS Team<br>").arg(pMameDat->mameVersion));

#ifdef Q_OS_WIN
	if (m1Core != NULL && m1Core->available)
	{
		m1Ver = m1Core->version;
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
		"<strong>M+GUI</strong> %1 &copy; 2008-2010 <a href=\"http://mameicons.free.fr/mame32p/\">MAME Plus!</a> Team<br>"
		"A Qt implementation of <a href=\"http://mameui.classicgaming.gamespy.com\">MameUI</a><br>"
		"Source code released under <a href=\"http://sam.zoy.org/wtfpl/COPYING\">WTFPL</a>"
		"<hr>"
		"%2%3%4%5%6"
		"</body>"
		"</html>")
		.arg(MPGUI_VER)
		.arg(mameString)
		.arg(m1VerString)
		.arg("<a href=\"http://qt.nokia.com\">Qt</a> " QT_VERSION_STR " &copy; Nokia Corporation<br>")
		.arg(sdlVerString)
		.arg("<a href=\"http://www.7-zip.org\">LZMA SDK</a> " MY_VERSION_COPYRIGHT_DATE)
		;

	aboutUI->tbVersion->setHtml(strVersion);
#ifdef Q_OS_WIN
	if (!m1Ver.isEmpty())
		m1UI->setWindowTitle("M1 - " + m1Ver);
#endif /* Q_OS_WIN */

	QFileInfo fi(mame_binary);

	setWindowTitle(QString("%1 - %2 %3")
		.arg("M+GUI " MPGUI_VER)
		.arg(fi.baseName().toUpper())
		.arg(pMameDat->mameVersion));
}

void MainWindow::enableCtrls(bool isEnabled)
{
	treeFolders->setEnabled(isEnabled);
	actionLargeIcons->setEnabled(false);
	actionDetails->setEnabled(isEnabled);
	actionGrouped->setEnabled(isEnabled);
	actionRefresh->setEnabled(isEnabled);
	actionDirectories->setEnabled(isEnabled);
	actionProperties->setEnabled(isEnabled);
	actionSrcProperties->setEnabled(isEnabled);
	actionDefaultOptions->setEnabled(isEnabled);
	actionPlay->setEnabled(isEnabled);
	menuPlayWith->setEnabled(isEnabled);
	menuAudit->setEnabled(isEnabled);
	menuArrangeIcons->setEnabled(isEnabled);
	menuCustomizeFields->setEnabled(isEnabled);
	menuCustomFilters->setEnabled(isEnabled);
	lineEditSearch->setEnabled(isEnabled);
	btnSearch->setEnabled(isEnabled);
	btnClearSearch->setEnabled(isEnabled);
}

void MainWindow::on_actionPlay_triggered()
{
	gameList->runMame();
}

void MainWindow::on_actionCommandLine_triggered()
{
	gameList->runMame(RUNMAME_CMD);
}

void MainWindow::on_actionSavestate_triggered()
{
	playOptionsUI->initSavestate();
	playOptionsUI->exec();
}
void MainWindow::on_actionPlayback_triggered()
{
	playOptionsUI->initPlayback();
	playOptionsUI->exec();
}

void MainWindow::on_actionRecord_triggered()
{
	playOptionsUI->initRecord();
	playOptionsUI->exec();
}

void MainWindow::on_actionMNG_triggered()
{
	playOptionsUI->initMNG();
	playOptionsUI->exec();
}

void MainWindow::on_actionAVI_triggered()
{
	playOptionsUI->initAVI();
	playOptionsUI->exec();
}

void MainWindow::on_actionWave_triggered()
{
	playOptionsUI->initWave();
	playOptionsUI->exec();
}

void MainWindow::on_actionConfigIPS_triggered()
{
	ipsUI->updateList();
	ipsUI->exec();
}

void MainWindow::on_actionRefresh_triggered()
{
	romAuditor->audit();
}

void MainWindow::on_actionFixDatComplete_triggered()
{
	exportFixDat(AUDIT_EXPORT_COMPLETE);
}

void MainWindow::on_actionFixDatAll_triggered()
{
	exportFixDat(AUDIT_EXPORT_ALL);
}

void MainWindow::on_actionFixDatIncomplete_triggered()
{
	exportFixDat(AUDIT_EXPORT_INCOMPLETE);
}

void MainWindow::on_actionFixDatMissing_triggered()
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
		romAuditor->audit(false, method, fileName);
}

void MainWindow::on_actionAudit_triggered()
{
	mameAuditor->audit(VERIFY_CURRENT_ROMS);
}

void MainWindow::on_actionAuditAll_triggered()
{
	mameAuditor->audit(VERIFY_ALL_ROMS);
}

void MainWindow::on_actionAuditAllSamples_triggered()
{
	mameAuditor->audit(VERIFY_ALL_SAMPLES);
}

void MainWindow::on_actionSrcProperties_triggered()
{
	if (!pMameDat->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_SRC, 0);
	optionsUI->exec();
}

void MainWindow::on_actionProperties_triggered()
{
	if (!pMameDat->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_CURR, 0);
	optionsUI->exec();	
}

void MainWindow::on_actionDefaultOptions_triggered()
{
	if (!pMameDat->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_GLOBAL, 0);
	optionsUI->exec();
}

void MainWindow::on_actionDirectories_triggered()
{
	if (!pMameDat->games.contains(currentGame))
		return;

	optionsUI->init(OPTLEVEL_GUI, 0);
	optionsUI->exec();
}

void MainWindow::on_actionExitStop_triggered()
{
	close();
}

void MainWindow::on_actionReadme_triggered()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=158710&view=collapsed"));
}

void MainWindow::on_actionFAQ_triggered()
{
	if (language.startsWith("zh_"))
		QDesktopServices::openUrl(QUrl("http://bbs.wisestudio.org/viewthread.php?tid=504"));
	else
		QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=164054&view=collapsed"));
}

void MainWindow::on_actionBoard_triggered()
{
	if (language.startsWith("zh_"))
		QDesktopServices::openUrl(QUrl("http://bbs.wisestudio.org/forum-16-1.html"));
	else
		QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/postlist.php?Cat=&Board=mameplus&view=collapsed"));
}

void MainWindow::on_actionAbout_triggered()
{
	aboutUI->exec();
}

void MainWindow::toggleGameListColumn()
{
	int col = colToggleActions.indexOf((QAction *)sender());

	if (col == -1)
		return;

	if (tvGameList->header()->isSectionHidden(col))
		tvGameList->header()->setSectionHidden (col, false);
	else
		tvGameList->header()->setSectionHidden (col, true);
}

void MainWindow::on_actionColSortAscending_triggered()
{
	int col = colSortActionGroup->actions().indexOf((QAction *)sender());

	if (col == -1)
		col = tvGameList->header()->sortIndicatorSection();

	tvGameList->sortByColumn(col, Qt::AscendingOrder);
}

void MainWindow::on_actionColSortDescending_triggered()
{
	tvGameList->sortByColumn(tvGameList->header()->sortIndicatorSection(), Qt::DescendingOrder);
}

void MainWindow::on_actionEnglish_triggered()
{
	language = "en_US";
	showRestartDialog();
}

void MainWindow::on_actionChinese_PRC_triggered()
{
	language = "zh_CN";
	showRestartDialog();
}

void MainWindow::on_actionChinese_Taiwan_triggered()
{
	language = "zh_TW";
	showRestartDialog();
}

void MainWindow::on_actionJapanese_triggered()
{
	language = "ja_JP";
	showRestartDialog();
}

void MainWindow::on_actionSpanish_triggered()
{
	language = "es_ES";
	showRestartDialog();
}

void MainWindow::on_actionFrench_triggered()
{
	language = "fr_FR";
	showRestartDialog();
}

void MainWindow::on_actionHungarian_triggered()
{
	language = "hu_HU";
	showRestartDialog();
}

void MainWindow::on_actionKorean_triggered()
{
	language = "ko_KR";
	showRestartDialog();
}

void MainWindow::on_actionBrazilian_triggered()
{
	language = "pt_BR";
	showRestartDialog();
}

void MainWindow::on_actionRussian_triggered()
{
	language = "ru_RU";
	showRestartDialog();
}

void MainWindow::on_actionItalian_triggered()
{
	language = "it_IT";
	showRestartDialog();
}

void MainWindow::on_actionLocalGameList_triggered()
{
	local_game_list = actionLocalGameList->isChecked();
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
	case QSystemTrayIcon::DoubleClick:
		if (isVisible())
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

void MainWindow::loadGuiSettings()
{
    pGuiSettings->setFallbacksEnabled(false);

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

		//m1
		<< "m1_directory"
		<< "m1_language"
		//ips
		<< "ips_language"
		<< "ips_relationship"

		//ui path
		<< "mame_binary"

		//ui mainwindow
		<< "background_file"
		<< "background_stretch"
		<< "gui_style"

		//ui gamelist
		<< "zoom_icon"
		<< "list_mode"
		<< "local_game_list"
		<< "sort_column"
		<< "sort_reverse"
		<< "default_game"

		//ui folder
		<< "default_folder"
		<< "hide_folders"
		<< "folder_flag"

		//ui snapshot
		<< "stretch_screenshot_larger"
		<< "enforce_aspect"
		<< "vertical_tabs"

		<< "window_geometry"
		<< "window_state"
		<< "column_state"
		<< "option_geometry"
		<< "option_column_state"
		<< "language");

	//remove invalid settings
	QStringList keys = pGuiSettings->allKeys();
	foreach(QString key, keys)
	{
		//fixme: ignore mess for now
		if (key.endsWith("_extra_software") || validGuiSettings.contains(key))
			continue;

		pGuiSettings->remove(key);
	}

	//assign settings to local variables
	//fixme: move loadLayout and other similar methods
	currentGame = pGuiSettings->value("default_game").toString();

	//fixme: remove default by setting it programatically
	if (pGuiSettings->value("option_column_state").isValid())
		option_column_state = pGuiSettings->value("option_column_state").toByteArray();
	else
		option_column_state = defaultGuiSettings.value("option_column_state").toByteArray();
}

void MainWindow::loadLayout()
{
	if (pGuiSettings->value("window_geometry").isValid())
		restoreGeometry(pGuiSettings->value("window_geometry").toByteArray());

	if (pGuiSettings->value("window_state").isValid())
		restoreState(pGuiSettings->value("window_state").toByteArray());
	
	option_geometry = pGuiSettings->value("option_geometry").toByteArray();

	actionVerticalTabs->setChecked(pGuiSettings->value("vertical_tabs", "1").toInt() == 1);
	actionRowDelegate->setChecked(pGuiSettings->value("zoom_icon", "1").toInt() == 1);

	gameList->listMode = pGuiSettings->value("list_mode").toString();
	if (gameList->listMode == actionDetails->objectName().remove("action"))
		actionDetails->setChecked(true);
	else if (gameList->listMode == actionLargeIcons->objectName().remove("action"))
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
	else if (language == "es_ES")
		actionSpanish->setChecked(true);
	else if (language == "fr_FR")
		actionFrench->setChecked(true);
	else if (language == "hu_HU")
		actionHungarian->setChecked(true);
	else if (language == "ko_KR")
		actionKorean->setChecked(true);
	else if (language == "pt_BR")
		actionBrazilian->setChecked(true);
	else if (language == "ru_RU")
		actionRussian->setChecked(true);
	else if (language == "it_IT")
		actionItalian->setChecked(true);
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

	QList<QTreeWidgetItem *> softwaresItems = treeFolders->findItems(gameList->intFolderNames[FOLDER_CONSOLE], Qt::MatchFixedString);
	QTreeWidgetItem *softwaresItem = NULL;
	if (!softwaresItems.isEmpty())
		softwaresItem = softwaresItems.first();

	//save console dirs
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
						const QString sysDesc = softwaresItem->child(i)->text(0);

						if (consoleMap.contains(sysDesc) && 
							consoleMap[sysDesc] == sysName)
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
						const QString sysDesc = softwaresItem->child(i)->text(0);

						if (consoleMap.contains(sysDesc) && 
							consoleMap[sysDesc] == sysName)
						{
							softwaresItem->child(i)->setHidden(true);
							break;
						}
					}
					
				}
			}
		}
	}

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
	pGuiSettings->setValue("zoom_icon", actionRowDelegate->isChecked() ? 1 : 0);
	pGuiSettings->setValue("local_game_list", actionLocalGameList->isChecked() ? 1 : 0);
	pGuiSettings->setValue("background_stretch", actionBgTile->isChecked() ? 0 : 1);
	pGuiSettings->setValue("default_game", currentGame);
	pGuiSettings->setValue("default_folder", defalutFolder);
	pGuiSettings->setValue("list_mode", gameList->listMode);
	pGuiSettings->setValue("hide_folders", hiddenFolders);
	pGuiSettings->setValue("folder_flag", gameList->filterFlags);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	if (!mame_binary.isEmpty())
	{
		saveSettings();
		pMameDat->save();
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
	QImage bkgroundImg(_dirpath + fileName);

	// setup mainwindow background
	if (!bkgroundImg.isNull())
	{
		if (actionBgStretch->isChecked())
			bkgroundImg = bkgroundImg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

		//fixme: must init before stylesheet applied
		QPalette palette = this->palette();
		palette.setBrush(backgroundRole(), QBrush(bkgroundImg));
//		setPalette(palette);

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
	trayIcon->setVisible(isTrayIconVisible);
	if (isTrayIconVisible)
		trayIcon->setToolTip(windowTitle());

	setVisible(!isTrayIconVisible);
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
		for (int i = 0; i < dockCtrlNames.size(); i++)
		{
			//iterate tabs in a tab bar
			for (int t = 0; t < tabBar->count(); t ++)
			{
				//if the tab contains any known screenshot/history dock names
				QString tabName = tabBar->tabText(t);
				if (tr(qPrintable(dockCtrlNames[i])) == tabName 
#ifdef Q_OS_WIN
					|| tabName == "M1"
#endif /* Q_OS_WIN */
					)
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

//fixme: dockCtrls[snapType]->isVisible() && isDockTabVisible(dockCtrlNames[snapType])
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

int main(int argc, char *argv[])
{
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
}
