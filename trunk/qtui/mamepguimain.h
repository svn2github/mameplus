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
#include "utils.h"
#include "ips.h"
#include "m1.h"

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
	DOCK_COMMAND,
	DOCK_LAST
};

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(QPixmap pm);
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

	bool isDockTabVisible(QString);
	void setVersion();

	QTreeView *tvGameList;
	QListView *lvGameList;
	
	QLineEdit *lineEditSearch;
	QToolButton *btnSearch, *btnClearSearch;
	QLabel *labelProgress, *labelGameCount, 
		*labelStatus, *labelEmulation, *labelColor, *labelSound, 
		*labelGraphic, *labelCocktail, *labelProtection, *labelSavestate;
	QWidget *wStatus;
	QProgressBar *progressBarGamelist;
	QSystemTrayIcon *trayIcon;
	
	Screenshot *ssSnap, *ssFlyer, *ssCabinet, *ssMarquee, *ssTitle, *ssCPanel, *ssPCB;
	QTextBrowser *tbHistory, *tbMameinfo, *tbStory, *tbCommand;

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
	void on_actionConfigIPS_activated();
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
	void on_trayIconActivated(QSystemTrayIcon::ActivationReason);

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
	void toggleTrayIcon(int, QProcess::ExitStatus, bool = false);
	
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
	void setTransparentStyle(QWidget * w);
	QList<QTabBar *> getSSTabBars();
};

#define LOG_MAME	2
#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 8

#ifdef Q_WS_WIN
#define EXEC_EXT ".exe"
#else
#define EXEC_EXT ""
#endif

// global vars
const float FORCE_ASPECT = 0.75f;
const QString INI_EXT = ".ini";
const QString CFG_PREFIX = 
#ifndef Q_WS_WIN
	QDir::homePath() + "/" + 
#endif
	".mamepgui/";

// external global vars
extern QList<QListWidget *> optCtrls;
extern QStringList consoleGamesL;

//misc class instance
//UI
extern MainWindow *win;
extern Options *optionsUI;
extern Dirs *dirsUI;
extern About *aboutUI;
extern IPS *ipsUI;
extern M1UI *m1UI;
//utils
extern MameGame *mameGame;
extern Gamelist *gameList;
extern QHash<QString, MameOption*> mameOpts;
extern ProcessManager *procMan;
extern OptionUtils *optUtils;
extern Utils *utils;
extern M1 *m1;

//current settings
extern QString currentGame, currentFolder;
extern QSettings guiSettings, defSettings;
extern QByteArray option_column_state;
extern QByteArray option_geometry;
extern QString mame_binary;
extern QString mameIniPath;
extern QString list_mode;
extern QString language;
extern bool local_game_list;
extern bool isDarkBg;
extern bool sdlInited;

#endif
