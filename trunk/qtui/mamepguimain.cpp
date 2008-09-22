#include "mamepguimain.h"

#include <QtPlugin>

// global variables
MainWindow *win;
QSettings guiSettings(CFG_PREFIX + "mamepgui.ini", QSettings::IniFormat);
QSettings defSettings(":/res/mamepgui.ini", QSettings::IniFormat);
QString mame_binary;
QString list_mode;
QString language;
QString background_file = NULL;
QString gui_style = NULL;
QActionGroup bgActions(0);
QActionGroup styleActions(0);
bool local_game_list;
bool isDarkBg = false;

QStringList dockCtrlNames;

void MainWindow::log(QString message, char logOrigin)
{
///*
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	textBrowserFrontendLog->append(msg);
	textBrowserFrontendLog->horizontalScrollBar()->setValue(0);
//*/
}

void MainWindow::poplog(QString message)
{
	QMessageBox::critical(this, "Debug", message); 
}

void MainWindow::logStatus(QString message)
{
	labelProgress->setText(message);
}

void MainWindow::logStatus(GameInfo *gameInfo)
{
	//if everything is ok, only show 1 icon
	//show all 6 icons otherwise
	static const QString prefix = ":/res/16x16/";
	static const QString suffix = ".png";
	QString statusBuffer = "";

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

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
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
	   << QT_TR_NOOP("Story"));

	setupUi(this);

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
	langActions->addAction(actionBrazilian);

	QActionGroup *bgStretchActions = new QActionGroup(this);
	bgStretchActions->addAction(actionBgStretch);
	bgStretchActions->addAction(actionBgTile);

	// init controls
    tvGameList = new QTreeView(centralwidget);
    tvGameList->setRootIsDecorated(false);
    tvGameList->setItemsExpandable(false);
	tvGameList->hide();

	lvGameList = new QListView(centralwidget);
	lvGameList->setMovement(QListView::Static);
	lvGameList->setResizeMode(QListView::Adjust);
	lvGameList->setViewMode(QListView::IconMode);
	lvGameList->setUniformItemSizes(true);
	lvGameList->hide();

	lineEditSearch = new QLineEdit(centralwidget);
	lineEditSearch->setStatusTip("type a keyword");
	QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
	lineEditSearch->setSizePolicy(sizePolicy);
	lineEditSearch->setMinimumWidth(240);
	toolBarSearch->addWidget(lineEditSearch);

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

	QAction *actionFolderList = dockWidget_7->toggleViewAction();
	actionFolderList->setIcon(QIcon(":/res/mame32-show-tree.png"));
	
	menuView->insertAction(actionPicture_Area, actionFolderList);
	toolBar->insertAction(actionPicture_Area, actionFolderList);

	gamelist = new Gamelist(this);
	optUtils = new OptionUtils(this);
	dlgOptions = new Options(this);
	dlgAbout = new About(this);
	dlgDirs = new Dirs(this);

	QTimer::singleShot(0, this, SLOT(init()));
}

void MainWindow::initHistory(QString title)
{
	static QDockWidget *dockWidget0 = NULL;
	
	QDockWidget *dockWidget = new QDockWidget(this);

	dockWidget->setObjectName("dockWidget_" + title);
	dockWidget->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	QWidget *dockWidgetContents = new QWidget(dockWidget);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	QGridLayout *gridLayout = new QGridLayout(dockWidgetContents);
	gridLayout->setObjectName("gridLayout_" + title);
	gridLayout->setContentsMargins(0, 0, 0, 0);

	QTextBrowser * tb;
	if (title == "History")
	{
		tb = tbHistory = new QTextBrowser(dockWidgetContents);
		tbHistory->setOpenExternalLinks(true);
	}
	else if (title == "MAMEInfo")
		tb = tbMameinfo = new QTextBrowser(dockWidgetContents);
	else// if (title == "Story")
		tb = tbStory = new QTextBrowser(dockWidgetContents);
	
	tb->setObjectName("textBrowser_" + title);
	
	gridLayout->addWidget(tb);

	dockWidget->setWidget(dockWidgetContents);
	dockWidget->setWindowTitle(tr(qPrintable(title)));
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dockWidget0)
		tabifyDockWidget(dockWidget0, dockWidget);
	else
		dockWidget0 = dockWidget;
}

Screenshot * MainWindow::initSnap(QString title)
{
	static Screenshot *dockWidget0 = NULL;
	
	Screenshot *dockWidget = new Screenshot(title, this);

	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dockWidget0)
		tabifyDockWidget(dockWidget0, dockWidget);
	else
		dockWidget0 = dockWidget;

	return dockWidget;
}

void MainWindow::init()
{
	win->log("win->init()");
	//fixme: should be in constructor
	ssSnap = initSnap(dockCtrlNames[DOCK_SNAP]);
	ssFlyer = initSnap(dockCtrlNames[DOCK_FLYER]);
	ssCabinet = initSnap(dockCtrlNames[DOCK_CABINET]);
	ssMarquee = initSnap(dockCtrlNames[DOCK_MARQUEE]);
	ssTitle = initSnap(dockCtrlNames[DOCK_TITLE]);
	ssCPanel = initSnap(dockCtrlNames[DOCK_CPANEL]);
	ssPCB = initSnap(dockCtrlNames[DOCK_PCB]);

	initHistory(dockCtrlNames[DOCK_HISTORY]);
	initHistory(dockCtrlNames[DOCK_MAMEINFO]);
	initHistory(dockCtrlNames[DOCK_STORY]);
//	initHistory(textBrowserFrontendLog, "GUI Log");

	initSettings();
	loadSettings();

	// validate mame_binary
	mame_binary = guiSettings.value("mame_binary", "mame" EXEC_EXT).toString();
	QFile mamebin(mame_binary);
	if (!mamebin.exists())
	{
		QDir dir(QCoreApplication::applicationDirPath());
		
		QStringList nameFilter;
		nameFilter << "*mame*";

		// iterate all exec files in the path
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Executable);
		for (int i = 0; i < files.count(); i++)
		{
			QFileInfo fi(QCoreApplication::applicationFilePath());
			QFileInfo fi2(files[i]);
			if(fi.fileName() != fi2.fileName())
			{
				mame_binary = files[i];
				break;
			}
		}
	
		QFile mamebin2(mame_binary);
		if (!mamebin2.exists())
		{
			QString filter = "";
#ifdef Q_WS_WIN
			filter.append(tr("Executable files (*" EXEC_EXT ")"));
			filter.append(";;");
#endif
			filter.append(tr("All Files (*)"));
		
			mame_binary = QFileDialog::getOpenFileName(this,
										tr("MAME executable:"),
										QCoreApplication::applicationDirPath(),
										filter
										);

			if (mame_binary.isEmpty())
			{
				win->poplog(QString("Could not find MAME"));
				mame_binary = "";
				//quit the program
				close();
				return;
			}
		}
	}

	// must optUtils->initOption() after win, before show()
	optUtils->initOption();

	//show UI
	win->log("win->show()");

	//apply css
	QFile cssFile(":/res/mamepgui.qss");
	cssFile.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(cssFile.readAll());
	qApp->setStyleSheet(styleSheet);

	show();
	loadLayout();
	setDockOptions();
	qApp->processEvents();

	// must gamelist->init(true) before loadLayout()
	gamelist->init(true, GAMELIST_INIT_FULL);

	// must init app style before background
	if (gui_style.isEmpty())
		gui_style = guiSettings.value("gui_style").toString();

	QStringList styles = QStyleFactory::keys();
	foreach (QString style, styles)
	{
		QAction* act = win->menuGUIStyle->addAction(style);
		act->setCheckable(true);
		if (gui_style == style)
			act->setChecked(true);
		styleActions.addAction(act);
		connect(act, SIGNAL(triggered()), this, SLOT(setGuiStyle()));
	}

	if (!gui_style.isEmpty())
		setGuiStyle(gui_style);

	// init background menu
	QString _dirpath = utils->getPath(guiSettings.value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	
	if (background_file.isEmpty())
		background_file = guiSettings.value("background_file").toString();
	
	if (dir.exists() && bgActions.actions().count() < 1)
	{
		QString dirpath = utils->getPath(_dirpath);
		
		QStringList nameFilter;
		nameFilter << "*.png";
		nameFilter << "*.jpg";
	
		// iterate all files in the path
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		foreach (QString fileName, files)
		{
			QAction* act = win->menuBackground->addAction(fileName);
			act->setCheckable(true);
			if (background_file == fileName)
				act->setChecked(true);
			bgActions.addAction(act);
			connect(act, SIGNAL(triggered()), this, SLOT(setBgPixmap()));
		}
	}

	if (!background_file.isEmpty())
		setBgPixmap(background_file);

	connect(actionBgStretch, SIGNAL(triggered()), this, SLOT(setBgTile()));
	connect(actionBgTile, SIGNAL(triggered()), this, SLOT(setBgTile()));

	// connect misc signal and slots
	// Actions
	connect(actionVerticalTabs, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));
	connect(actionLargeIcons, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));
	connect(actionDetails, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));
	connect(actionGrouped, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));

	// Auditor
	connect(&gamelist->auditor, SIGNAL(progressSwitched(int, QString)), gamelist, SLOT(switchProgress(int, QString)));
	connect(&gamelist->auditor, SIGNAL(progressUpdated(int)), gamelist, SLOT(updateProgress(int)));
	connect(&gamelist->auditor, SIGNAL(finished()), gamelist->mAuditor, SLOT(init()));

	// Game List
	connect(lineEditSearch, SIGNAL(textChanged(const QString &)), gamelist, SLOT(filterTimer()));	

	// Options
	for (int i = 1; i < optCtrls.count(); i++)
	{
		connect(optCtrls[i], SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), 
				optUtils, SLOT(updateModel(QListWidgetItem *)));
	}
	connect(dlgOptions->tabOptions, SIGNAL(currentChanged(int)), optUtils, SLOT(updateModel()));
}

MainWindow::~MainWindow()
{
}

void MainWindow::on_actionPlay_activated()
{
	gamelist->runMame();
}

void MainWindow::on_actionRefresh_activated()
{
	gamelist->auditor.audit();
}

void MainWindow::on_actionProperties_activated()
{
	showOptionsDialog(OPTLEVEL_CURR, 0);
}

void MainWindow::on_actionSrcProperties_activated()
{
	showOptionsDialog(OPTLEVEL_SRC, 0);
}

void MainWindow::on_actionDefaultOptions_activated()
{
	showOptionsDialog(OPTLEVEL_GLOBAL, 1);
}

void MainWindow::on_actionDirectories_activated()
{
	showOptionsDialog(OPTLEVEL_GLOBAL, 0);
}

void MainWindow::showOptionsDialog(int optLevel, int lstRow)
{
	//prevent crash when list is empty
	if (currentGame.isEmpty())
		return;

	//init ctlrs, 
	for (int i = OPTLEVEL_GLOBAL; i < OPTLEVEL_LAST; i++)
		optUtils->updateModel(0, i);

	dlgOptions->tabOptions->setCurrentIndex(optLevel);

	if (lstRow > -1)
		optCtrls[optLevel]->setCurrentRow(lstRow);

	dlgOptions->exec();
	saveSettings();
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
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/showflat.php?Cat=&Number=164054&view=collapsed"));
}

void MainWindow::on_actionBoard_activated()
{
	QDesktopServices::openUrl(QUrl("http://www.mameworld.info/ubbthreads/postlist.php?Cat=&Board=mameplus&view=collapsed"));
}

void MainWindow::on_actionAbout_activated()
{
	dlgAbout->exec();
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

void MainWindow::on_actionBrazilian_activated()
{
	language = "pt_BR";
	showRestartDialog();
}

void MainWindow::on_actionLocalGameList_activated()
{
	local_game_list = actionLocalGameList->isChecked();
}

void MainWindow::on_actionEnforceAspect_activated()
{
	bool isForce = actionEnforceAspect->isChecked();

	win->ssSnap->setAspect(isForce);
	win->ssTitle->setAspect(isForce);
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
    guiSettings.setFallbacksEnabled(false);

	//remove invalid settings
	static const QStringList validSettings = (QStringList() 
		<< "flyer_directory"
		<< "cabinet_directory"
		<< "marquee_directory"
		<< "title_directory"
		<< "cpanel_directory"
		<< "pcb_directory"
		<< "icons_directory"
		<< "background_directory"
		<< "background_file"
		<< "gui_style"
		<< "background_stretch"
		<< "mame_binary"
		<< "history_file"
		<< "story_file"
		<< "mameinfo_file"
		<< "option_geometry"
		<< "sort_column"
		<< "sort_reverse"
		<< "default_game"
		<< "folder_current"
		<< "vertical_tabs"
		<< "enforce_aspect"
		<< "local_game_list"
		<< "list_mode"
		<< "option_column_state"
		<< "window_geometry"
		<< "window_state"
		<< "column_state"
		<< "language");

	QStringList keys = guiSettings.allKeys();
	foreach(QString key, keys)
	{
		if (key.contains("_extra_software") || validSettings.contains(key))
			continue;

		guiSettings.remove(key);
	}

}

void MainWindow::loadLayout()
{
	if (guiSettings.value("window_geometry").isValid())
		restoreGeometry(guiSettings.value("window_geometry").toByteArray());
	else
		restoreGeometry(defSettings.value("window_geometry").toByteArray());

	if (guiSettings.value("window_state").isValid())
		restoreState(guiSettings.value("window_state").toByteArray());
	else
		restoreState(defSettings.value("window_state").toByteArray());
	
	option_geometry = guiSettings.value("option_geometry").toByteArray();

	actionVerticalTabs->setChecked(guiSettings.value("vertical_tabs", "1").toInt() == 1);

	list_mode = guiSettings.value("list_mode").toString();
	if (list_mode == win->actionDetails->objectName().remove("action"))
		actionDetails->setChecked(true);
	else if (list_mode == win->actionLargeIcons->objectName().remove("action"))
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
	else if (language == "pt_BR")
		actionBrazilian->setChecked(true);
	else
		actionEnglish->setChecked(true);

	actionEnforceAspect->setChecked(guiSettings.value("enforce_aspect", "1").toInt() == 1);

	local_game_list = guiSettings.value("local_game_list", "1").toInt() == 1;
	actionLocalGameList->setChecked(local_game_list);

	if (guiSettings.value("background_stretch", "1").toInt() == 0)
		actionBgTile->setChecked(true);
	else
		actionBgStretch->setChecked(true);
	
}

void MainWindow::loadSettings()
{
	currentGame = guiSettings.value("default_game", "pacman").toString();

	if (defSettings.value("option_column_state").isValid())
		option_column_state = defSettings.value("option_column_state").toByteArray();
	else
		option_column_state = guiSettings.value("option_column_state").toByteArray();
}

void MainWindow::saveSettings()
{
	//some guiSettings uses mameOpts mapping for dialog view
	guiSettings.setValue("flyer_directory", mameOpts["flyer_directory"]->globalvalue);
	guiSettings.setValue("cabinet_directory", mameOpts["cabinet_directory"]->globalvalue);
	guiSettings.setValue("marquee_directory", mameOpts["marquee_directory"]->globalvalue);
	guiSettings.setValue("title_directory", mameOpts["title_directory"]->globalvalue);
	guiSettings.setValue("cpanel_directory", mameOpts["cpanel_directory"]->globalvalue);
	guiSettings.setValue("pcb_directory", mameOpts["pcb_directory"]->globalvalue);
	guiSettings.setValue("icons_directory", mameOpts["icons_directory"]->globalvalue);
	guiSettings.setValue("background_directory", mameOpts["background_directory"]->globalvalue);
	guiSettings.setValue("background_file", background_file);
	guiSettings.setValue("gui_style", gui_style);
	guiSettings.setValue("language", language);

	guiSettings.setValue("history_file", mameOpts["history_file"]->globalvalue);
	guiSettings.setValue("story_file", mameOpts["story_file"]->globalvalue);
	guiSettings.setValue("mameinfo_file", mameOpts["mameinfo_file"]->globalvalue);
	if (mameOpts["mame_binary"]->globalvalue != mameOpts["mame_binary"]->defvalue)
		guiSettings.setValue("mame_binary", mameOpts["mame_binary"]->globalvalue);
	else
		guiSettings.setValue("mame_binary", mame_binary);

	//save console dirs
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		if (pMameOpt->guivisible && optName.contains("_extra_software"))
		{
			QString v = mameOpts[optName]->globalvalue;
			if (!v.trimmed().isEmpty())
				guiSettings.setValue(optName, mameOpts[optName]->globalvalue);
		}
	}

	//save layout
	guiSettings.setValue("window_geometry", saveGeometry());
	guiSettings.setValue("window_state", saveState());
	guiSettings.setValue("option_geometry", option_geometry);
	guiSettings.setValue("option_column_state", option_column_state);
	guiSettings.setValue("column_state", tvGameList->header()->saveState());
	guiSettings.setValue("sort_column", tvGameList->header()->sortIndicatorSection());
	guiSettings.setValue("sort_reverse", (tvGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder) ? 0 : 1);
	guiSettings.setValue("vertical_tabs", actionVerticalTabs->isChecked() ? 1 : 0);
	guiSettings.setValue("enforce_aspect", actionEnforceAspect->isChecked() ? 1 : 0);
	guiSettings.setValue("local_game_list", actionLocalGameList->isChecked() ? 1 : 0);
	guiSettings.setValue("background_stretch", actionBgTile->isChecked() ? 0 : 1);
	guiSettings.setValue("default_game", currentGame);
	guiSettings.setValue("folder_current", currentFolder);//fixme: rename
	guiSettings.setValue("list_mode", list_mode);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	if (!mame_binary.isEmpty())
	{
		saveSettings();
		mamegame->s11n();
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
	QColor color, bgColor;
	QPalette palette;

	if (isDarkBg)
	{
		bgColor = QColor(0, 0, 0, 140);
		color = QColor(Qt::white);
	}
	else
	{
		bgColor = QColor(255, 255, 255, 140);
		color = QColor(Qt::black);
	}
	//color: palette(dark); 

	palette.setColor(QPalette::Active, QPalette::Text, color);
	palette.setColor(QPalette::Inactive, QPalette::Text, color);
	palette.setColor(QPalette::Disabled, QPalette::Text, color);
	palette.setColor(QPalette::Active, QPalette::Base, bgColor);
	palette.setColor(QPalette::Inactive, QPalette::Base, bgColor);
	palette.setColor(QPalette::Disabled, QPalette::Base, bgColor);

	w->setPalette(palette);
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

	QString _dirpath = utils->getPath(guiSettings.value("background_directory", "bkground").toString());
	QDir dir(_dirpath);
	QString dirpath = utils->getPath(_dirpath);

	QImage bkgroundImg(_dirpath + fileName);

	// setup mainwindow background
	if (!bkgroundImg.isNull())
	{
		if (actionBgStretch->isChecked())
			bkgroundImg = bkgroundImg.scaled(win->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

		QPalette palette;
		palette.setBrush(win->backgroundRole(), QBrush(bkgroundImg));
		win->setPalette(palette);

		//get the color tone of bg image, set the bg color based on it
		bkgroundImg = bkgroundImg.scaled(1, 1, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
		int grayVal = qGray(bkgroundImg.pixel(0, 0));
		win->log(QString("grayVal: %1").arg(grayVal));
		if (grayVal < 128)
			isDarkBg = true;
		else
			isDarkBg = false;

		// setup background alpha
		setTransparentBg(win->treeFolders);
		setTransparentBg(win->tvGameList);
		setTransparentBg(win->lvGameList);
		setTransparentBg(win->tbHistory);
		setTransparentBg(win->tbMameinfo);
		setTransparentBg(win->tbStory);
//		setTransparentBg(win->tbCommand);
	}
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

void Screenshot::setPixmap(const QByteArray &pmdata, bool forceAspect)
{
	QPixmap pm;
	pm.loadFromData(pmdata);
    originalPixmap = pm;

	this->forceAspect = forceAspect;
	updateScreenshotLabel();
}

void Screenshot::setAspect(bool forceAspect)
{
	this->forceAspect = forceAspect;
	updateScreenshotLabel();
}

void Screenshot::rotateImage()
{
	QString objName =((QWidget* )sender())->objectName();
	objName.remove(0, 6);	//remove "label_"

	//there's no API in Qt to access docked widget tabbar
	QList<QTabBar *> tabs = win->findChildren<QTabBar *>();
	foreach (QTabBar *tab, tabs)
	{
		bool isDock = false;
		for (int i = 0; i < dockCtrlNames.count(); i++)
		{
			if (MainWindow::tr(qPrintable(dockCtrlNames[i])) == tab->tabText(0))
			{
				isDock = true;
				break;
			}
		}
		
		if (isDock && MainWindow::tr(qPrintable(objName)) == tab->tabText(tab->currentIndex()))
		{
			int i = tab->currentIndex();
			if (++i > tab->count() - 1)
				i = 0;
			tab->setCurrentIndex(i);
			win->log(QString("tab: %1, %2")
				.arg(tab->currentIndex())
				.arg(tab->tabText(tab->currentIndex()))
				);
		}
	}
}

void Screenshot::updateScreenshotLabel()
{
    QSize scaledSize, origSize;
	scaledSize = originalPixmap.size();

	if (forceAspect)
	{
		float aspect = 0.75f;
		if (scaledSize.width() < scaledSize.height())
		{
			//vert
			if (scaledSize.height() < scaledSize.width() / aspect)
				// need expand height
				scaledSize.setHeight((int)(scaledSize.width() / aspect));
			else
				// need expand width
				scaledSize.setWidth((int)(scaledSize.height() * aspect));
		}
		else
		{
			//horz
			if (scaledSize.height() < scaledSize.width() * aspect)
				// need expand height
				scaledSize.setHeight((int)(scaledSize.width() * aspect));
			else
				// need expand width
				scaledSize.setWidth((int)(scaledSize.height() / aspect));
		}
	}
//	win->log(QString("sz: %1, %2").arg(scaledSize()));

	origSize = scaledSize;

	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	// do not enlarge
	if (scaledSize.width() > origSize.width() || 
		scaledSize.height() > origSize.height())
	{
		scaledSize = origSize;
	}

	screenshotLabel->setIconSize(scaledSize);
	screenshotLabel->setIcon(originalPixmap.scaled(scaledSize,
                                                     Qt::IgnoreAspectRatio,
                                                     Qt::SmoothTransformation));
}

int main(int argc, char *argv[])
{
	QApplication myApp(argc, argv);

	QTranslator appTranslator;

	language = guiSettings.value("language").toString();
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

