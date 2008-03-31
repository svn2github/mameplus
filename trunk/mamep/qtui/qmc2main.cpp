#include "qmc2main.h"

// global variables
MainWindow *win;
QSettings guisettings("mamepgui.ini", QSettings::IniFormat);

QString icons_directory;
QString roms_directory;
QString snapshot_directory;

static Gamelist *gamelist = NULL;

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

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
	setupUi(this);

	lineEditSearch = new QLineEdit(centralwidget);
	lineEditSearch->setStatusTip("type a keyword");
	QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lineEditSearch->sizePolicy().hasHeightForWidth());
	lineEditSearch->setSizePolicy(sizePolicy);
	toolBarSearch->addWidget(lineEditSearch);

	labelSnapshot = new QLabel(snapWidget);
	labelSnapshot->setCursor(QCursor(Qt::PointingHandCursor));
	labelSnapshot->setAlignment(Qt::AlignCenter);
	snapWidget->layout()->addWidget(labelSnapshot);

	labelProgress = new QLabel(centralwidget);
	statusbar->addWidget(labelProgress);

	progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->resize(256, 20);
	progressBarGamelist->hide();

	gamelist = new Gamelist(this);
	optUtils = new OptionUtils(this);

	QTimer::singleShot(0, this, SLOT(init()));
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
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::~MainWindow()");
#endif
}

void MainWindow::on_actionRefresh_activated()
{
	gamelist->auditthread.audit();
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
	icons_directory = guisettings.value("icons_directory", "icons").toString();
	roms_directory = guisettings.value("roms_directory", "roms").toString();
}

void MainWindow::saveSettings()
{
	guisettings.setValue("snapshot_directory", snapshot_directory);
	guisettings.setValue("icons_directory", icons_directory);
	guisettings.setValue("roms_directory", roms_directory);
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
	
	utils = new Utils(0);
	win = new MainWindow(0);
	qmc2ProcessManager = new ProcessManager(win);

	return qmc2App.exec();
}

