#include "mamepguimain.h"

#include <QtPlugin>

// global variables
MainWindow *win;
QSettings guisettings("mamepgui.ini", QSettings::IniFormat);

QString flyer_directory,
		cabinet_directory,
		marquee_directory,
		title_directory,
		cpanel_directory,
		pcb_directory,
		icons_directory,
		background_directory,
		mame_binary;

void MainWindow::log(QString message, char logOrigin)
{
	QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

	QString msg = timeString + ": " + message;

	switch ( logOrigin ) {
	case LOG_QMC2:
		textBrowserFrontendLog->append(msg);
		textBrowserFrontendLog->horizontalScrollBar()->setValue(0);
		break;

	case LOG_MAME:
		textBrowserMAMELog->append(msg);
		textBrowserMAMELog->horizontalScrollBar()->setValue(0);
		break;

	default:
		break;
	}
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

	setDockOptions(dockOptions()|QMainWindow::VerticalTabs);

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

//	initHistory(textBrowserMAMELog, "MAME Log");
//	initHistory(textBrowserFrontendLog, "GUI Log");

	initSettings();
	loadSettings();

	// validate mame_binary
	QFile mamebin(mame_binary);
	if (!mamebin.exists())
		win->poplog(QString("Could not find %1").arg(mame_binary));

	// must init after win, before show()
	optUtils->initOption();

	// must gamelist->init() before loadLayout()
	gamelist->init();

	//show UI
	show();
	loadLayout();

	// setup background
	QImage bkground(utils->getPath(background_directory) + "bkground.png");
	if (!bkground.isNull())
	{
		QPalette palette;
		palette.setBrush(this->backgroundRole(), QBrush(bkground));
		this->setPalette(palette);
	}

	utils->tranaparentBg(treeViewGameList);
	utils->tranaparentBg(treeFolders);

	connect(lineEditSearch, SIGNAL(textChanged(const QString &)), gamelist, SLOT(filterTimer()));	
	connect(&gamelist->iconThread.iconQueue, SIGNAL(logStatusUpdated(QString)), this, SLOT(logStatus(QString)));

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
	gamelist->auditThread.audit();
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
	//init ctlrs, 
	for (int i = OPTNFO_GLOBAL; i < OPTNFO_LAST; i++)
		optUtils->updateModel(0, i);

	dlgOptions->exec();
}

void MainWindow::initSettings()
{
    guisettings.setFallbacksEnabled(false);
}

void MainWindow::loadLayout()
{
	restoreGeometry(guisettings.value("window_geometry").toByteArray());
	restoreState(guisettings.value("window_state").toByteArray());
	dlgOptionsGeo = guisettings.value("option_geometry").toByteArray();
}

void MainWindow::saveLayout()
{
	guisettings.setValue("window_geometry", saveGeometry());
	guisettings.setValue("window_state", saveState());
	guisettings.setValue("option_geometry", dlgOptionsGeo);
	guisettings.setValue("column_state", treeViewGameList->header()->saveState());
	guisettings.setValue("sort_column", treeViewGameList->header()->sortIndicatorSection());
	guisettings.setValue("sort_reverse", (treeViewGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder) ? 0 : 1);
	guisettings.setValue("default_game", currentGame);

	QString currentFolder = "";
	if (win->treeFolders->currentItem()->parent())
		currentFolder += win->treeFolders->currentItem()->parent()->text(0);
	currentFolder += "/" + win->treeFolders->currentItem()->text(0);
	

	guisettings.setValue("folder_current", currentFolder);
}

void MainWindow::loadSettings()
{
	flyer_directory = guisettings.value("flyer_directory", "flyers").toString();
	cabinet_directory = guisettings.value("cabinet_directory", "cabinets").toString();
	marquee_directory = guisettings.value("marquee_directory", "marquees").toString();
	title_directory = guisettings.value("title_directory", "titles").toString();
	cpanel_directory = guisettings.value("cpanel_directory", "cpanel").toString();
	pcb_directory = guisettings.value("pcb_directory", "pcb").toString();
	icons_directory = guisettings.value("icons_directory", "icons").toString();
	background_directory = guisettings.value("background_directory", "bkground").toString();

	mame_binary = guisettings.value("mame_binary", "mamep.exe").toString();

	currentGame = guisettings.value("default_game", "pacman").toString();
}

void MainWindow::saveSettings()
{
	guisettings.setValue("flyer_directory", flyer_directory);
	guisettings.setValue("cabinet_directory", "cabinet"/* cabinet_directory*/);
	guisettings.setValue("marquee_directory", marquee_directory);
	guisettings.setValue("title_directory", title_directory);
	guisettings.setValue("cpanel_directory", cpanel_directory);
	guisettings.setValue("pcb_directory", pcb_directory);
	guisettings.setValue("icons_directory", icons_directory);
	guisettings.setValue("background_directory", background_directory);

	guisettings.setValue("mame_binary", mame_binary);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSettings();
	saveLayout();
	event->accept();
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
//	appTranslator.load("lang/mamepgui_" + QLocale::system().name());
//	appTranslator.load(":/lang/mamepgui_zh_CN");

	qmc2App.installTranslator(&appTranslator);

	procMan = new ProcessManager(0);	
	utils = new Utils(0);
	win = new MainWindow(0);

	return qmc2App.exec();
}

