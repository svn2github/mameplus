#ifndef _MAMEPGUIMAIN_H_
#define _MAMEPGUIMAIN_H_

#include "quazip.h"
#include "quazipfile.h"

#include <QApplication>
#include <QtGui>
#include <QtXml>

//static qt works with windows version
Q_IMPORT_PLUGIN(qico)
Q_IMPORT_PLUGIN(qjpeg)
//Q_IMPORT_PLUGIN(qmng)

#include "ui_mamepguimain.h"

#include "audit.h"
#include "gamelist.h"
#include "mameopt.h"
#include "dialogs.h"
#include "procmgr.h"
#include "utils.h"

enum
{
	DOCK_SNAP,
	DOCK_FLYER,
	DOCK_CABINET,
	DOCK_MARQUEE,
	DOCK_TITLE,
	DOCK_CPANEL,
	DOCK_PCB,
	
	DOCK_HISTORY,
	DOCK_MAMEINFO,
	DOCK_STORY,
	DOCK_LAST
};


class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(const QByteArray &, bool = false);
	void setAspect(bool);

protected:
    void resizeEvent(QResizeEvent *);


private slots:
	void rotateImage();

private:
    void updateScreenshotLabel();

	QPushButton *screenshotLabel;
    QPixmap originalPixmap;
	QGridLayout *mainLayout;
	QWidget *dockWidgetContents;
	bool forceAspect;

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
	QLabel *labelProgress, *labelGameCount, 
	*labelStatus, *labelEmulation, *labelColor, *labelSound, 
	*labelGraphic, *labelCocktail, *labelProtection, *labelSavestate;
	QWidget *wStatus;
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
	void on_actionExitStop_activated();
	void on_actionRefresh_activated();
	void on_actionEnglish_activated();
	void on_actionChinese_PRC_activated();
	void on_actionChinese_Taiwan_activated();
	void on_actionJapanese_activated();
	void on_actionBrazilian_activated();
	void on_actionLocalGameList_activated();
	void on_actionEnforceAspect_activated();
	void on_actionReadme_activated();
	void on_actionFAQ_activated();
	void on_actionBoard_activated();
	void on_actionAbout_activated();

	void on_actionPlay_activated();
	void on_actionProperties_activated();
	void on_actionSrcProperties_activated();
	void on_actionDefaultOptions_activated();
	void on_actionDirectories_activated();

	void on_actionColDescription_activated();
	void on_actionColName_activated();
	void on_actionColROMs_activated();
	void on_actionColManufacturer_activated();
	void on_actionColDriver_activated();
	void on_actionColYear_activated();
	void on_actionColCloneOf_activated();

    void log(QString, char logOrigin = 1);
	void poplog(QString);
	void logStatus(QString);
	void logStatus(GameInfo *);
	void init();
	void initSettings();
	void loadLayout();
	void loadSettings();
	void saveSettings();
	void setDockOptions();
	void setGuiStyle(QString = NULL);
	void setBgTile();
	void setBgPixmap(QString = NULL);
	
protected:
	void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

private:
	void toggleGameListColumn(int);
	void initHistory(QString);
	Screenshot * initSnap(QString);
	void showOptionsDialog(int, int = -1);
	void showRestartDialog();
	void setTransparentBg(QWidget *);
};

#define LOG_MAME	2
#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 8

#ifdef Q_WS_WIN
#define EXEC_EXT ".exe"
#else
#define EXEC_EXT ""
#endif


// external global variables
extern MainWindow *win;
extern Options *dlgOptions;
extern About *dlgAbout;
extern Dirs *dlgDirs;

extern QList<QListWidget *> optCtrls;

extern const QString CFG_PREFIX;
extern QHash<QString, MameOption*> mameOpts;
extern QSettings guiSettings, defSettings;
extern QByteArray option_column_state;
extern QString mame_binary;
extern QString list_mode;
extern QString language;
extern bool enforce_aspect;
extern bool local_game_list;
extern bool isDarkBg;

extern QByteArray option_geometry;

extern QString currentGame, currentFolder;
extern MameGame *mamegame;
extern Gamelist *gamelist;
extern QStringList consoleGamesL;

extern ProcessManager *procMan;

extern OptionUtils *optUtils;

extern Utils *utils;

#endif
