#include "mamepguimain.h"

#include <QtPlugin>

// global variables
MainWindow *win;
QSettings guiSettings("mamepgui.ini", QSettings::IniFormat);
QSettings defSettings(":/res/mamepgui.ini", QSettings::IniFormat);

QString list_mode;

void MainWindow::log(QString message, char logOrigin)
{
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	textBrowserFrontendLog->append(msg);
	textBrowserFrontendLog->horizontalScrollBar()->setValue(0);
}

void MainWindow::poplog(QString message)
{
	QMessageBox::critical(this, "Debug", message); 
}

void MainWindow::logStatus(QString message)
{
	labelProgress->setText(message);
}

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
	setupUi(this);

	// View action group
    QActionGroup *viewActions = new QActionGroup(this);
    viewActions->setExclusive(true);
    viewActions->addAction(actionDetails);
    viewActions->addAction(actionGrouped);
    viewActions->addAction(actionLargeIcons);

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
	toolBarSearch->addWidget(lineEditSearch);

	labelProgress = new QLabel(centralwidget);
	statusbar->addWidget(labelProgress);
	labelGameCount = new QLabel(centralwidget);
	statusbar->addPermanentWidget(labelGameCount);

	progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->setMaximumHeight(16);
	progressBarGamelist->hide();

	QAction *actionFolderList = dockWidget_7->toggleViewAction();
	actionFolderList->setIcon(QIcon(":/res/mame32-show-tree.png"));
	
	menuView->insertAction(actionPicture_Area, actionFolderList);
	toolBar->insertAction(actionPicture_Area, actionFolderList);

	//override font for CJK OS
	QFont font;
	font.setPointSize(9);
	setFont(font);
	statusbar->setFont(font);
	menubar->setFont(font);
    menuFile->setFont(font);
    menuView->setFont(font);
    menuOptions->setFont(font);
    menuHelp->setFont(font);

	gamelist = new Gamelist(this);
	optUtils = new OptionUtils(this);
	dlgOptions = new Options(this);

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
		tb = tbHistory = new QTextBrowser(dockWidgetContents);
	else if (title == "MAMEInfo")
		tb = tbMameinfo = new QTextBrowser(dockWidgetContents);
	else// if (title == "Story")
		tb = tbStory = new QTextBrowser(dockWidgetContents);
	
	tb->setObjectName("textBrowser_" + title);
	
	utils->tranaparentBg(tb);

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
	ssSnap = initSnap(QT_TR_NOOP("Snapshot"));
	ssFlyer = initSnap(QT_TR_NOOP("Flyer"));
	ssCabinet = initSnap(QT_TR_NOOP("Cabinet"));
	ssMarquee = initSnap(QT_TR_NOOP("Marquee"));
	ssTitle = initSnap(QT_TR_NOOP("Title"));
	ssCPanel = initSnap(QT_TR_NOOP("Control Panel"));
	ssPCB = initSnap(QT_TR_NOOP("PCB"));

	initHistory(QT_TR_NOOP("History"));
	initHistory(QT_TR_NOOP("MAMEInfo"));
	initHistory(QT_TR_NOOP("Story"));

//	initHistory(textBrowserFrontendLog, "GUI Log");

	initSettings();
	loadSettings();

	// validate mame_binary
	QString mame_binary = guiSettings.value("mame_binary", "mamep.exe").toString();
	QFile mamebin(mame_binary);
	if (!mamebin.exists())
		win->poplog(QString("Could not find %1").arg(mame_binary));

	// must optUtils->initOption() after win, before show()
	optUtils->initOption();

	//show UI
	win->log("win->show()");
	show();
	loadLayout();
	qApp->processEvents();

	// must gamelist->init(true) before loadLayout()
	gamelist->init(true);

	// setup background
	QImage bkground(utils->getPath(guiSettings.value("background_directory", "bkground").toString()) + "bkground.png");
	if (!bkground.isNull())
	{
		QPalette palette;
		palette.setBrush(this->backgroundRole(), QBrush(bkground));
		this->setPalette(palette);
	}

	utils->tranaparentBg(tvGameList);
	utils->tranaparentBg(lvGameList);
	utils->tranaparentBg(treeFolders);

	// connect misc signal and slots
	connect(actionVerticalTabs, SIGNAL(toggled(bool)), this, SLOT(setDockOptions()));
	connect(actionLargeIcons, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));
	connect(actionDetails, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));
	connect(actionGrouped, SIGNAL(toggled(bool)), gamelist, SLOT(init(bool)));

	connect(&gamelist->auditor, SIGNAL(progressSwitched(int, QString)), gamelist, SLOT(switchProgress(int, QString)));
	connect(&gamelist->auditor, SIGNAL(progressUpdated(int)), gamelist, SLOT(updateProgress(int)));
	connect(&gamelist->auditor, SIGNAL(finished()), gamelist->mAuditor, SLOT(init()));

	connect(lineEditSearch, SIGNAL(textChanged(const QString &)), gamelist, SLOT(filterTimer()));	
//	connect(&gamelist->iconThread.iconQueue, SIGNAL(logStatusUpdated(QString)), this, SLOT(logStatus(QString)));

	for (int i = 1; i < optCtrlList.count(); i++)
	{
		connect(optCtrlList[i], SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), 
				optUtils, SLOT(updateModel(QListWidgetItem *)));
	}

	connect(dlgOptions->tabOptions, SIGNAL(currentChanged(int)), optUtils, SLOT(updateModel()));
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::~MainWindow()");
#endif
}

void MainWindow::on_actionRefresh_activated()
{
	gamelist->auditor.audit();
}

void MainWindow::on_actionExitStop_activated()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::on_actionExitStop_activated()");
#endif

	close();
}

void MainWindow::on_actionDefaultOptions_activated()
{
	//prevent crash when list is empty
	if (currentGame.isEmpty())
		return;

	//init ctlrs, 
	for (int i = OPTNFO_GLOBAL; i < OPTNFO_LAST; i++)
		optUtils->updateModel(0, i);

	dlgOptions->exec();
}

void MainWindow::initSettings()
{
    guiSettings.setFallbacksEnabled(false);
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

	actionVerticalTabs->setChecked(guiSettings.value("vertical_tabs").toInt() == 1);

	list_mode = guiSettings.value("list_mode").toString();
	if (list_mode == win->actionGrouped->text())
		win->actionGrouped->setChecked(true);
	else if (list_mode == win->actionLargeIcons->text())
		win->actionLargeIcons->setChecked(true);
	else
		win->actionDetails->setChecked(true);
}

void MainWindow::loadSettings()
{
	currentGame = guiSettings.value("default_game", "pacman").toString();
	option_column_state = guiSettings.value("option_column_state").toByteArray();
}

void MainWindow::saveSettings()
{
	guiSettings.setValue("flyer_directory", mameOpts["flyer_directory"]->globalvalue);
	guiSettings.setValue("cabinet_directory", mameOpts["cabinet_directory"]->globalvalue);
	guiSettings.setValue("marquee_directory", mameOpts["marquee_directory"]->globalvalue);
	guiSettings.setValue("title_directory", mameOpts["title_directory"]->globalvalue);
	guiSettings.setValue("cpanel_directory", mameOpts["cpanel_directory"]->globalvalue);
	guiSettings.setValue("pcb_directory", mameOpts["pcb_directory"]->globalvalue);
	guiSettings.setValue("icons_directory", mameOpts["icons_directory"]->globalvalue);
	guiSettings.setValue("background_directory", mameOpts["background_directory"]->globalvalue);

	guiSettings.setValue("mame_binary", mameOpts["mame_binary"]->globalvalue);

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
	guiSettings.setValue("default_game", currentGame);
	guiSettings.setValue("folder_current", currentFolder);//fixme: rename
	guiSettings.setValue("list_mode", list_mode);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	hide();
	saveSettings();
	mamegame->s11n();
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


Screenshot::Screenshot(const QString & title, QWidget *parent)
: QDockWidget(parent)
{
	setObjectName("dockWidget_" + title);
	setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	dockWidgetContents = new QWidget(this);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	mainLayout = new QGridLayout(dockWidgetContents);
	mainLayout->setObjectName("mainLayout_" + title);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	screenshotLabel = new QLabel(dockWidgetContents);
	screenshotLabel->setObjectName("label_" + title);
	screenshotLabel->setCursor(QCursor(Qt::PointingHandCursor));
    screenshotLabel->setAlignment(Qt::AlignCenter);

//    screenshotLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//so that we can shrink image
	screenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	
//    screenshotLabel->setMinimumSize(320, 240);

	mainLayout->addWidget(screenshotLabel);

	setWidget(dockWidgetContents);
	setWindowTitle(tr(qPrintable(title)));

//	dockWidgetContents->setLayout(mainLayout);


//	resize(300, 200);
}

void Screenshot::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	if (!screenshotLabel->pixmap() || scaledSize != screenshotLabel->pixmap()->size())
		updateScreenshotLabel();
}

//fixme: listen on label
void Screenshot::mousePressEvent(QMouseEvent * event)
{
//	if (event->button() == Qt::LeftButton)
		win->log(objectName());
}

void Screenshot::setPixmap(const QByteArray &pmdata)
{
	QPixmap pm;
	pm.loadFromData(pmdata);
    originalPixmap = pm;
    updateScreenshotLabel();
}

void Screenshot::updateScreenshotLabel()
{
    QSize scaledSize, origSize;
	scaledSize = origSize = originalPixmap.size();
	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	// do not enlarge
	if (scaledSize.width() > origSize.width() || 
		scaledSize.height() > origSize.height())
	{
		scaledSize = origSize;
	}

    screenshotLabel->setPixmap(originalPixmap.scaled(scaledSize,
                                                     Qt::KeepAspectRatio,
                                                     Qt::SmoothTransformation));
}

int main(int argc, char *argv[])
{
	QApplication qmc2App(argc, argv);

	QTranslator appTranslator;

	QString local = guiSettings.value("locale").toString();
	if (local.isEmpty())
		local = QLocale::system().name();
	appTranslator.load(":/lang/mamepgui_" + local);

	qmc2App.installTranslator(&appTranslator);

	procMan = new ProcessManager(0);	
	utils = new Utils(0);
	win = new MainWindow(0);

	return qmc2App.exec();
}

