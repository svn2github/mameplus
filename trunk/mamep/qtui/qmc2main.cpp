#include <QProcess>
#include <QTimer>
#include <QSettings>
#include <QApplication>

#include "qmc2main.h"
#include "procmgr.h"
#include "gamelist.h"
#include "mameopt.h"


// global variables
MainWindow *qmc2MainWindow = NULL;
Gamelist *qmc2Gamelist = NULL;
MameOptions *mameopts;
QString currentGame;
MameGame *mamegame;

ProcessManager *qmc2ProcessManager = NULL;
bool qmc2ReloadActive = FALSE;
bool qmc2EarlyReloadActive = FALSE;
bool qmc2GuiReady = FALSE;
bool qmc2IconsPreloaded = FALSE;
bool qmc2StopParser = FALSE;
QStringList qmc2BiosROMs;

extern TreeModel *gameListModel;
extern GameListSortFilterProxyModel *gameListPModel;


void MainWindow::log(char logOrigin, QString message)
{
	if ( !qmc2GuiReady )
		return;

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

    labelSnapshot = new QLabel(dockWidgetContents_9);
    labelSnapshot->setCursor(QCursor(Qt::PointingHandCursor));
    labelSnapshot->setAlignment(Qt::AlignCenter);
    gridLayout3->addWidget(labelSnapshot, 0, 0, 1, 1);

	labelProgress = new QLabel(centralwidget);
	statusbar->addWidget(labelProgress);

    progressBarGamelist = new QProgressBar(centralwidget);
	progressBarGamelist->resize(256, 20);
	progressBarGamelist->hide();

	qmc2Gamelist = new Gamelist(this);
	mameopts = new MameOptions(this);

	QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::~MainWindow()");
#endif
}

void MainWindow::on_actionRefresh_activated()
{
	qmc2Gamelist->auditthread.audit();
}

void MainWindow::on_actionReload_activated()
{
	qmc2Gamelist->load();
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
	loadLayout();
}


void MainWindow::init()
{
#ifdef QMC2_DEBUG
	log(LOG_QMC2, "DEBUG: MainWindow::init()");
#endif
	on_actionReload_activated();
	show();
	loadLayout();
	mameopts->load();
}

void MainWindow::saveLayout()
{
	QString fileName = "mamepgui.ini";
	QSettings settings(fileName, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

	settings.setValue("window_geometry", saveGeometry());
	settings.setValue("window_state", saveState());
	settings.setValue("default_game", currentGame);

	//save column widths
	settings.setValue("column_widths", QString("%1,%2,%3,%4,%5,%6,%7")
												.arg(treeViewGameList->columnWidth(0))
												.arg(treeViewGameList->columnWidth(1))
												.arg(treeViewGameList->columnWidth(2))
												.arg(treeViewGameList->columnWidth(3))
												.arg(treeViewGameList->columnWidth(4))
												.arg(treeViewGameList->columnWidth(5))
												.arg(treeViewGameList->columnWidth(6)));
}

void MainWindow::loadLayout()
{
 	QString fileName = "mamepgui.ini";
	QSettings settings(fileName, QSettings::IniFormat);
    settings.setFallbacksEnabled(false);

	restoreGeometry(settings.value("window_geometry").toByteArray());
	restoreState(settings.value("window_state").toByteArray());
/*	
	currentGame = settings.value("default_game").toString();
	if (mamegame->gamenameGameInfoMap.contains(currentGame))
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[currentGame];
		QModelIndex i = gameListModel->index(0, gameinfo->pModItem);
		QModelIndex pi = gameListPModel->mapFromSource(i);

		QMessageBox::warning(qmc2MainWindow->treeViewOption, tr("MAME GUI error"),
					  QString("%1, %2 . %3, %4")
					  .arg(i.column())
					  .arg(i.row())
					  .arg(pi.column())
					  .arg(pi.row()));

		treeViewGameList->setCurrentIndex(i);
		treeViewGameList->setFocus();
        treeViewGameList->selectionModel()->select(pi,
                                            QItemSelectionModel::Select | QItemSelectionModel::Rows );
		treeViewGameList->scrollTo(pi, QAbstractItemView::PositionAtCenter);
	}
*/
	QStringList columnWidths = settings.value("column_widths").toString().split(",");
	for (int col = 0; col < columnWidths.size(); col ++)
		treeViewGameList->setColumnWidth(col, columnWidths[col].toUInt());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	saveLayout();
	event->accept();
}

int main(int argc, char *argv[])
{
	QApplication qmc2App(argc, argv);

	qmc2ProcessManager = new ProcessManager(0);
	qmc2MainWindow = new MainWindow(0);

	qmc2GuiReady = TRUE;

	return qmc2App.exec();
}
