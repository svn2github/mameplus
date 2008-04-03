#include "qmc2main.h"

// global variables
MainWindow *win;
QSettings guisettings("mamepgui.ini", QSettings::IniFormat);

QString flyer_directory,
		cabinet_directory,
		marquee_directory,
		title_directory,
		cpanel_directory,
		pcb_directory,
		icons_directory;

QString roms_directory;
QString snapshot_directory;

void MainWindow::log(char logOrigin, QString message)
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

	initSnap(QT_TR_NOOP("Snapshot"));
	initSnap(QT_TR_NOOP("Flyer"));
	initSnap(QT_TR_NOOP("Cabinet"));
	initSnap(QT_TR_NOOP("Marquee"));
	initSnap(QT_TR_NOOP("Title"));
	initSnap(QT_TR_NOOP("Control Panel"));
	initSnap(QT_TR_NOOP("PCB"));

	initHistory(QT_TR_NOOP("History"));
	initHistory(QT_TR_NOOP("MAMEInfo"));
	initHistory(QT_TR_NOOP("Story"));

//	initHistory(textBrowserMAMELog, "MAME Log");
//	initHistory(textBrowserFrontendLog, "GUI Log");
	
	progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->setMaximumHeight(16);
	progressBarGamelist->hide();

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
	else if (title == "Story")
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

void MainWindow::initSnap(QString title)
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

	QLabel * lbl;
	if (title == "Snapshot")
		lbl = lblSnap = new QLabel(dockWidgetContents);
	else if (title == "Flyer")
		lbl = lblFlyer = new QLabel(dockWidgetContents);
	else if (title == "Cabinet")
		lbl = lblCabinet = new QLabel(dockWidgetContents);
	else if (title == "Marquee")
		lbl = lblMarquee = new QLabel(dockWidgetContents);
	else if (title == "Title")
		lbl = lblTitle = new QLabel(dockWidgetContents);
	else if (title == "Control Panel")
		lbl = lblCPanel = new QLabel(dockWidgetContents);
	else if (title == "PCB")
		lbl = lblPCB = new QLabel(dockWidgetContents);

	lbl->setObjectName("label_" + title);
	lbl->setCursor(QCursor(Qt::PointingHandCursor));
	lbl->setAlignment(Qt::AlignCenter);
	//so that we can shrink image
	lbl->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

	gridLayout->addWidget(lbl);

	dockWidget->setWidget(dockWidgetContents);
	dockWidget->setWindowTitle(tr(qPrintable(title)));
	addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), dockWidget);

	// create tabbed history widgets
	if (dockWidget0)
		tabifyDockWidget(dockWidget0, dockWidget);
	else
		dockWidget0 = dockWidget;
}

void MainWindow::init()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::init()");
#endif
	initSettings();
	loadSettings();

	// must init after win, before show()
	optUtils->initOption();
	
	on_actionReload_activated();
	show();
	loadLayout();

	optUtils->loadDefault("default.ini");
	optUtils->load();
	optUtils->load(OPTNFO_GLOBAL, "mame.ini");
	optUtils->setupModelData(OPTNFO_GLOBAL);

	connect(gameListPModel,SIGNAL(layoutChanged()), gamelist, SLOT(restoreSelection()));
	connect(lineEditSearch, SIGNAL(textChanged(const QString &)), gamelist, SLOT(filterTimer()));	
	connect(treeViewGameList->selectionModel(),
			SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
			gamelist, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
	connect(&gamelist->iconThread.iconQueue, SIGNAL(logStatusUpdated(QString)), this, SLOT(logStatus(QString)));
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

void MainWindow::on_actionReload_activated()
{
	gamelist->load();
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
//	Ui::dlgOption dialog();
//	dialog.exec();
}

void MainWindow::initSettings()
{
    guisettings.setFallbacksEnabled(false);
}

void MainWindow::loadLayout()
{
	restoreGeometry(guisettings.value("window_geometry").toByteArray());
	restoreState(guisettings.value("window_state").toByteArray());

	gamelist->restoreSelection(guisettings.value("default_game", "pacman").toString());
}

void MainWindow::saveLayout()
{
	guisettings.setValue("window_geometry", saveGeometry());
	guisettings.setValue("window_state", saveState());
	guisettings.setValue("column_state", treeViewGameList->header()->saveState());
	guisettings.setValue("sort_column", treeViewGameList->header()->sortIndicatorSection());
	guisettings.setValue("sort_reverse", (treeViewGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder) ? 0 : 1);
	guisettings.setValue("default_game", currentGame);
}

void MainWindow::loadSettings()
{
	snapshot_directory = guisettings.value("snapshot_directory", "snap").toString();
	roms_directory = guisettings.value("roms_directory", "roms").toString();
	
	flyer_directory = guisettings.value("flyer_directory", "flyers").toString();
	cabinet_directory = guisettings.value("cabinet_directory", "cabinets").toString();
	marquee_directory = guisettings.value("marquee_directory", "marquees").toString();
	title_directory = guisettings.value("title_directory", "titles").toString();
	cpanel_directory = guisettings.value("cpanel_directory", "cpanel").toString();
	pcb_directory = guisettings.value("pcb_directory", "pcb").toString();
	icons_directory = guisettings.value("icons_directory", "icons").toString();
}

void MainWindow::saveSettings()
{
	guisettings.setValue("snapshot_directory", snapshot_directory);
	guisettings.setValue("roms_directory", roms_directory);

	guisettings.setValue("flyer_directory", flyer_directory);
	guisettings.setValue("cabinet_directory", cabinet_directory);
	guisettings.setValue("marquee_directory", marquee_directory);
	guisettings.setValue("title_directory", title_directory);
	guisettings.setValue("cpanel_directory", cpanel_directory);
	guisettings.setValue("pcb_directory", pcb_directory);
	guisettings.setValue("icons_directory", icons_directory);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveSettings();
	saveLayout();
	event->accept();
}

int main(int argc, char *argv[])
{
	QApplication qmc2App(argc, argv);

	QTranslator appTranslator;
//	appTranslator.load("lang/mamepgui_" + QLocale::system().name());
	appTranslator.load(":/lang/mamepgui_zh_CN");

	qmc2App.installTranslator(&appTranslator);
	
	utils = new Utils(0);
	win = new MainWindow(0);
	qmc2ProcessManager = new ProcessManager(win);

	return qmc2App.exec();
}

