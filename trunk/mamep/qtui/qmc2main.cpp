#include <QScrollBar>
#include <QProcess>
#include <QTimer>

#include "qmc2main.h"
#include "procmgr.h"
#include "gamelist.h"

// global variables
MainWindow *qmc2MainWindow = NULL;
Gamelist *qmc2Gamelist = NULL;
ProcessManager *qmc2ProcessManager = NULL;
bool qmc2ReloadActive = FALSE;
bool qmc2EarlyReloadActive = FALSE;
bool qmc2GuiReady = FALSE;
bool qmc2IconsPreloaded = FALSE;
bool qmc2StopParser = FALSE;
QStringList qmc2BiosROMs;

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

  qmc2Gamelist = new Gamelist(this);

  QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
  log(LOG_QMC2, "DEBUG: MainWindow::~MainWindow()");
#endif
}

void MainWindow::on_actionReload_activated()
{
	qmc2Gamelist->load();
}

void MainWindow::init()
{
#ifdef QMC2_DEBUG
  log(LOG_QMC2, "DEBUG: MainWindow::init()");
#endif
    on_actionReload_activated();
}

int main(int argc, char *argv[])
 {
	QApplication qmc2App(argc, argv);

  qmc2ProcessManager = new ProcessManager(0);
	qmc2MainWindow = new MainWindow(0);

     qmc2MainWindow->show();

  qmc2GuiReady = TRUE;

     return qmc2App.exec();
 }
