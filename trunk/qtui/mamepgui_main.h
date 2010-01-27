#ifndef _MAMEPGUIMAIN_H_
#define _MAMEPGUIMAIN_H_

//common
#include "quazip.h"
#include "quazipfile.h"

#include <QtGui>
#include <QApplication>
#include <QtXml>

#include "ui_mamepgui_main.h"
#include "mamepgui_types.h"
#include "gamelist.h"
#include "audit.h"

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(QPixmap pm);
	void setPixmap(const QByteArray &, bool);
    void updateScreenshotLabel(bool = false);

protected:
    void resizeEvent(QResizeEvent *);


private slots:
	void rotateImage();

private:
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
	void enableCtrls(bool);
	void log(QString);

	RomAuditor romAuditor;
	MameExeRomAuditor *mameAuditor;

	GameListTreeView *tvGameList;
	QListView *lvGameList;
	
	QLineEdit *lineEditSearch;
	QToolButton *btnSearch, *btnClearSearch;
	QLabel *labelProgress, *labelGameCount, 
		*labelStatus, *labelEmulation, *labelColor, *labelSound, 
		*labelGraphic, *labelCocktail, *labelProtection, *labelSavestate;
	QWidget *wStatus;
	QProgressBar *progressBarGamelist;
	QSystemTrayIcon *trayIcon;
	
	QStringList dockCtrlNames;
	QDockWidget* dockCtrls[DOCK_LAST];
	QTextBrowser *tbHistory, *tbMameinfo, *tbDriverinfo, *tbStory, *tbCommand;

	QMessageBox *pop;

signals:
	void logUpdated(QString);

public slots:
	void on_actionExitStop_activated();
	void on_actionRefresh_activated();
	void on_actionFixDatComplete_activated();
	void on_actionFixDatAll_activated();
	void on_actionFixDatIncomplete_activated();
	void on_actionFixDatMissing_activated();
	void on_actionEnglish_activated();
	void on_actionChinese_PRC_activated();
	void on_actionChinese_Taiwan_activated();
	void on_actionJapanese_activated();
	void on_actionFrench_activated();
	void on_actionHungarian_activated();
	void on_actionKorean_activated();
	void on_actionBrazilian_activated();
	void on_actionRussian_activated();
	void on_actionLocalGameList_activated();
	void on_actionReadme_activated();
	void on_actionFAQ_activated();
	void on_actionBoard_activated();
	void on_actionAbout_activated();

	void on_actionPlay_activated();
	void on_actionCommandLine_activated();
	void on_actionSavestate_activated();
	void on_actionPlayback_activated();
	void on_actionRecord_activated();
	void on_actionMNG_activated();
	void on_actionAVI_activated();
	void on_actionWave_activated();
	void on_actionConfigIPS_activated();
	void on_actionAudit_activated();
	void on_actionAuditAll_activated();
	void on_actionAuditAllSamples_activated();
	void on_actionProperties_activated();
	void on_actionSrcProperties_activated();
	void on_actionDefaultOptions_activated();
	void on_actionDirectories_activated();

	void on_actionColSortAscending_activated();
	void on_actionColSortDescending_activated();
	void toggleGameListColumn();
	void on_trayIconActivated(QSystemTrayIcon::ActivationReason);

    void updateLog(QString);
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
	QString background_file;
	QString gui_style;
	
	void initHistory(int);
	void initSnap(int);
	void showRestartDialog();
	void setTransparentBg(QWidget *);
	void setTransparentStyle(QWidget * w);
	QList<QTabBar *> getSSTabBars();
	void exportFixDat(int);
};

#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 11

// global vars
#define ZIP_EXT ".zip"
#define ICO_EXT ".ico"
#define INI_EXT ".ini"
#define STA_EXT ".sta"
#define INP_EXT ".inp"
#define PNG_EXT ".png"
#define MNG_EXT ".mng"
#define AVI_EXT ".avi"
#define WAV_EXT ".wav"
#define SZIP_EXT ".7z"
#ifdef Q_WS_WIN
#define EXEC_EXT ".exe"
#else
#define EXEC_EXT ""
#endif

extern QString CFG_PREFIX;

extern MainWindow *win;
extern QSettings *pGuiSettings, defSettings;
extern QString currentAppDir;
extern QString mame_binary;
extern QString language;
extern bool local_game_list;
extern bool isDarkBg;
extern bool sdlInited;
extern bool isMESS;

extern QStringList validGuiSettings;

#endif
