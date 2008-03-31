#ifndef _QMC2_MAIN_H_
#define _QMC2_MAIN_H_

#include "qticohandler.h"
#include "quazip.h"
#include "quazipfile.h"

#include <QApplication>
#include <QtGui>
#include <QtXml>

#include "ui_qmc2main.h"
#include "procmgr.h"
#include "gamelist.h"
#include "utils.h"
#include "mameopt.h"

class MainWindow : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	
	QLineEdit *lineEditSearch;
	QLabel *labelProgress;
	QProgressBar *progressBarGamelist;
	QLabel *labelSnapshot;

public slots:
    // game menu
    void on_actionRefresh_activated();
    void on_actionReload_activated();
	void on_actionExitStop_activated();
	void on_actionDefaultOptions_activated();
    void log(char, QString);
	void init();
	void initSettings();
	void loadLayout();
	void saveLayout();
	void loadSettings();
	void saveSettings();
	
protected:
    void closeEvent(QCloseEvent *event);
};

#define LOG_QMC2	1
#define LOG_MAME	2
#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 4

// external global variables
extern MainWindow *win;
extern QSettings guisettings;

extern QString icons_directory;
extern QString roms_directory;
extern QString snapshot_directory;

extern QString currentGame;
extern MameGame *mamegame;
extern GameListSortFilterProxyModel *gameListPModel;

extern ProcessManager *qmc2ProcessManager;

extern OptionUtils *optUtils;

extern Utils *utils;

#endif
