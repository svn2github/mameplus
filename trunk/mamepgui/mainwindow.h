#ifndef _MAMEPGUIMAIN_H_
#define _MAMEPGUIMAIN_H_

#include <QtGui>

#include "ui_mainwindow.h"
#include "gamelist.h"

class RomAuditor;
class MameExeRomAuditor;

class DirsUI;
class PlayOptionsUI;
class AboutUI;
class CmdUI;
class IpsUI;
class M1Core;
class M1UI;
class OptionsUI;
class CsvCfgUI;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void saveSettings();
	bool isDockTabVisible(QString);
	void setVersion();
	void enableCtrls(bool);
	void log(QString);

	RomAuditor *romAuditor;
	MameExeRomAuditor *mameAuditor;

	GameListTreeView *tvGameList;
	QListView *lvGameList;

	QLineEdit *lineEditSearch;
	QToolButton *btnSearch, *btnClearSearch;
	QLabel *labelProgress, *labelGameCount, *labelStatus, *labelEmulation, *labelColor, *labelSound, *labelGraphic, *labelCocktail, *labelProtection, *labelSavestate;
	QWidget *wStatus;
	QProgressBar *progressBarGamelist;
	QSystemTrayIcon *trayIcon;

	DirsUI *dirsUI;
	PlayOptionsUI *playOptionsUI;
	AboutUI *aboutUI;
	CmdUI *cmdUI;
	IpsUI *ipsUI;
	M1UI *m1UI;
	M1Core *m1Core;
	OptionsUI *optionsUI;
	CsvCfgUI *csvCfgUI;

	QStringList dockCtrlNames;
	QDockWidget* dockCtrls[DOCK_LAST];
	QTextBrowser *tbHistory, *tbMameinfo, *tbDriverinfo, *tbStory, *tbCommand;

	QMessageBox *pop;

signals:
	void logUpdated(QString);

public slots:
	void on_actionExitStop_triggered();
	void on_actionRefresh_triggered();
	void on_actionFixDatComplete_triggered();
	void on_actionFixDatAll_triggered();
	void on_actionFixDatIncomplete_triggered();
	void on_actionFixDatMissing_triggered();
	void on_actionEnglish_triggered();
	void on_actionChinese_PRC_triggered();
	void on_actionChinese_Taiwan_triggered();
	void on_actionJapanese_triggered();
	void on_actionSpanish_triggered();
	void on_actionFrench_triggered();
	void on_actionHungarian_triggered();
	void on_actionKorean_triggered();
	void on_actionBrazilian_triggered();
	void on_actionRussian_triggered();
	void on_actionItalian_triggered();
	void on_actionLocalGameList_triggered();
	void on_actionReadme_triggered();
	void on_actionFAQ_triggered();
	void on_actionBoard_triggered();
	void on_actionAbout_triggered();

	void on_actionPlay_triggered();
	void on_actionCommandLine_triggered();
	void on_actionSavestate_triggered();
	void on_actionPlayback_triggered();
	void on_actionRecord_triggered();
	void on_actionMNG_triggered();
	void on_actionAVI_triggered();
	void on_actionWave_triggered();
	void on_actionConfigIPS_triggered();
	void on_actionAudit_triggered();
	void on_actionAuditAll_triggered();
	void on_actionAuditAllSamples_triggered();
	void on_actionProperties_triggered();
	void on_actionSrcProperties_triggered();
	void on_actionDefaultOptions_triggered();
	void on_actionDirectories_triggered();
	void on_actionColSortAscending_triggered();
	void on_actionColSortDescending_triggered();

	void toggleGameListColumn();
	void trayIconActivated(QSystemTrayIcon::ActivationReason);

    void updateLog(QString);
	void poplog(QString);
	void logStatus(QString);
	void logStatus(GameInfo *);
	void init();
	void setDockOptions();
	void setGuiStyle(QString = "");
	void setBgTile();
	void setBgPixmap(QString = "");
	void toggleTrayIcon(int, QProcess::ExitStatus, bool = false);

protected:
	void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *);

private:
	QString background_file;
	QString gui_style;
	QDockWidget *dwHistory;

	void loadGuiSettings();
	void loadLayout();
	bool validateMameBinary();
	QString selectMameBinary();
	void initHistory(int);
	void initSnap(int);
	void showRestartDialog();
	void setTransparentBg(QWidget *);
	void setTransparentStyle(QWidget * w);
	QList<QTabBar *> getSSTabBars();
	void exportFixDat(int);
};

#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 12

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
extern QSettings *pGuiSettings, defaultGuiSettings;
extern QString currentAppDir;
extern QString mame_binary;
extern QString language;
extern bool local_game_list;
extern bool isDarkBg;
extern bool sdlInited;
extern bool isMESS;

extern QStringList validGuiSettings;

#endif
