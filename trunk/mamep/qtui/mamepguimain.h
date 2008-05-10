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

#include "options.h"
#include "utils.h"
#include "procmgr.h"
#include "gamelist.h"
#include "mameopt.h"

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(const QString &, QWidget *parent = 0);
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
	
	QLineEdit *lineEditSearch;
	QLabel *labelProgress;
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

//	QTextBrowser *textBrowserMAMELog;
//	QTextBrowser *textBrowserFrontendLog;

public slots:
    // game menu
    void on_actionRefresh_activated();
	void on_actionExitStop_activated();
	void on_actionDefaultOptions_activated();
    void log(QString, char logOrigin = 1);
	void poplog(QString);
	void logStatus(QString);
	void init();
	void initSettings();
	void loadLayout();
	void saveLayout();
	void loadSettings();
	void saveSettings();
	
protected:
    void closeEvent(QCloseEvent *event);

private:
	void initHistory(QString);
	Screenshot * initSnap(QString);
};

#define LOG_QMC2	1
#define LOG_MAME	2
#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 6

// external global variables
extern MainWindow *win;
extern Options *dlgOptions;
extern QList<QListWidget *> optCtrlList;

extern QHash<QString, MameOption*> mameOpts;
extern QSettings guisettings;

extern QString flyer_directory,
		cabinet_directory,
		marquee_directory,
		title_directory,
		cpanel_directory,
		pcb_directory,
		icons_directory,
		background_directory,
		mame_binary;

extern QByteArray dlgOptionsGeo;

extern QString currentGame;
extern MameGame *mamegame;
extern Gamelist *gamelist;
extern GameListSortFilterProxyModel *gameListPModel;

extern ProcessManager *procMan;

extern OptionUtils *optUtils;

extern Utils *utils;

#endif
