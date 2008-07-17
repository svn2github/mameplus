#ifndef _QMC2_MAIN_H_
#define _QMC2_MAIN_H_

#include "quazip.h"
#include "quazipfile.h"

#include <QApplication>
#include <QtGui>
#include <QtXml>

Q_IMPORT_PLUGIN(qico)
//Q_IMPORT_PLUGIN(qmng)

#include "ui_mamepguimain.h"

#include "audit.h"
#include "gamelist.h"
#include "mameopt.h"
#include "dialogs.h"
#include "procmgr.h"
#include "utils.h"

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(const QByteArray &);

protected:
    void resizeEvent(QResizeEvent *);
	void mousePressEvent(QMouseEvent *);

/*
private slots:
    void newScreenshot();
    void saveScreenshot();
    void shootScreen();
    void updateCheckBox();
*/
private:
    void updateScreenshotLabel();

	QLabel *screenshotLabel;
    QPixmap originalPixmap;
	QGridLayout *mainLayout;
	QWidget *dockWidgetContents;
};

class MainWindow : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	QTreeView *tvGameList;
	QListView *lvGameList;
	
	QLineEdit *lineEditSearch;
	QLabel *labelProgress, *labelGameCount;
	QProgressBar *progressBarGamelist;
	
	Screenshot *ssSnap;
	Screenshot *ssFlyer;
	Screenshot *ssCabinet;
	Screenshot *ssMarquee;
	Screenshot *ssTitle;
	Screenshot *ssCPanel;
	Screenshot *ssPCB;

	QTextBrowser *tbHistory;
	QTextBrowser *tbMameinfo;
	QTextBrowser *tbStory;
	QTextBrowser *tbCommand;

//	QTextBrowser *textBrowserFrontendLog;

public slots:
    // game menu
    void on_actionRefresh_activated();
	void on_actionExitStop_activated();
	void on_actionDefaultOptions_activated();
	void on_actionAbout_activated();
    void log(QString, char logOrigin = 1);
	void poplog(QString);
	void logStatus(QString);
	void init();
	void initSettings();
	void loadLayout();
	void loadSettings();
	void saveSettings();
	void setDockOptions();
	
protected:
    void closeEvent(QCloseEvent *event);

private:
	void initHistory(QString);
	Screenshot * initSnap(QString);
};

#define LOG_QMC2	1
#define LOG_MAME	2
#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 7

// external global variables
extern MainWindow *win;
extern Options *dlgOptions;
extern About *dlgAbout;

extern QList<QListWidget *> optCtrls;

extern QHash<QString, MameOption*> mameOpts;
extern QSettings guiSettings, defSettings;
extern QByteArray option_column_state;

extern QString list_mode;

extern QByteArray option_geometry;

extern QString currentGame, currentFolder;
extern MameGame *mamegame;
extern Gamelist *gamelist;
extern QStringList consoleGamesL;

extern ProcessManager *procMan;

extern OptionUtils *optUtils;

extern Utils *utils;

#endif
