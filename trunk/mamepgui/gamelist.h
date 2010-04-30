#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>

#include "utils.h"

enum
{
	GAME_MISSING = 0,
	GAME_COMPLETE,
	GAME_INCOMPLETE
};

enum
{
	GAMELIST_INIT_FULL = 0,
	GAMELIST_INIT_AUDIT,
	GAMELIST_INIT_DIR,
	GAMELIST_INIT_DRIVER
};

enum
{
	RUNMAME_NORMAL = 0,
	RUNMAME_EXTROM,
	RUNMAME_CMD
};

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
	DOCK_DRIVERINFO,
	DOCK_STORY,
	DOCK_COMMAND,
	DOCK_LAST
};

enum
{
	FOLDER_ALLGAME = 0,
	FOLDER_ALLARC,
	FOLDER_AVAILABLE,
	FOLDER_UNAVAILABLE,
	FOLDER_CONSOLE,
	FOLDER_MANUFACTURER,
	FOLDER_YEAR,
	FOLDER_SOURCE,
	FOLDER_BIOS,
	FOLDER_CPU,
	FOLDER_SND,
	FOLDER_HARDDISK,
	FOLDER_SAMPLES,
	FOLDER_DUMPING,
	FOLDER_WORKING,
	FOLDER_NONWORKING,
	FOLDER_ORIGINALS,
	FOLDER_CLONES,
	FOLDER_RESOLUTION,
	FOLDER_PALETTESIZE,
	FOLDER_REFRESH,
	FOLDER_DISPLAY,
	FOLDER_CONTROLS,
	FOLDER_CHANNELS,
	FOLDER_SAVESTATE,
	/*
	FOLDER_DEFICIENCY,
	FOLDER_ARTWORK,
	*/

	FOLDER_EXT,
	MAX_FOLDERS,

	SORT_STR
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	QString historyText;
	QString mameinfoText;
	QString driverinfoText;
	QString storyText;
	QString commandText;

	QByteArray pmSnapData[DOCK_LAST];

	UpdateSelectionThread(QObject *parent = 0);
	~UpdateSelectionThread();

	void update();

signals:
	void snapUpdated(int);

protected:
	void run();

private:
	QMutex mutex;
	QString gameName;
	bool abort;

	QString getHistory(const QString &, const QString &, int);
	void convertHistory(QString &, const QString &);
	void convertMameInfo(QString &, const QString &);
	void convertCommand(QString &);
	QByteArray getScreenshot(const QString &, const QString &, int);
};

class TreeItem
{
public:
	TreeItem(const QList<QVariant> &data, TreeItem *parent = 0);
	~TreeItem();

	void appendChild(TreeItem *child);

	TreeItem *child(int row);
	int childCount() const;
	int columnCount() const;
	QVariant data(int column) const;
	int row() const;
	TreeItem *parent();

private:
	QList<TreeItem*> childItems;
	QList<QVariant> itemData;
	TreeItem *parentItem;
};

class GameInfo;
class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeModel(QObject *parent = 0);
	~TreeModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int column, TreeItem *childItem) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant displayData(GameInfo *gameInfo, int col) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	void updateRow(const QModelIndex &index);

private:
	TreeItem *rootItem;

	TreeItem *getItem(const QModelIndex &index) const;
	TreeItem *setupModelData(TreeItem *, QString);
};

class GameListTreeView : public QTreeView
{
Q_OBJECT
public:
		GameListTreeView(QWidget * = 0);
		void paintEvent(QPaintEvent *);
		QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers);
		void keyPressEvent(QKeyEvent *);
};

class GameListDelegate : public QItemDelegate
{
Q_OBJECT
public:
	GameListDelegate(QObject * = 0);

	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
	void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
};

class Gamelist : public QObject
{
	Q_OBJECT

public:
	QProcess *loadProc;
	QStringList xmlLines;
	QMenu *menuContext;
	QMenu *headerMenu;
	QString listMode;
	QStringList intFolderNames0, intFolderNames;
	QPixmap pmDeco;
	QRect rectDeco;
	quint16 filterFlags;
	bool autoAudit;

	Gamelist(QObject *parent = 0);
	~Gamelist();

	void loadIcon();
	void disableCtrls();
	void restoreFolderSelection(bool = false);
	void centerGameSelection(QModelIndex);
	bool isAuditConsoleFolder(const QString&);
	bool isConsoleFolder();
	QString getResolution(GameInfo *, int);

public slots:
	void init(bool = true, int = GAMELIST_INIT_AUDIT);	//the default init value is a hack, for connect slots
	void update(int = GAMELIST_INIT_FULL);
	void restoreGameSelection();
	void runMame(int = RUNMAME_NORMAL, QStringList = QStringList());
	QString getViewString(const QModelIndex &index, int column) const;
	GameInfo* getGameInfo (const QModelIndex &, QString&);
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection();
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);

	// external process management
	void runMameFinished(int, QProcess::ExitStatus);
	void runMergedFinished(int, QProcess::ExitStatus);

	void filterFlagsChanged(bool);
	void filterSearchCleared();
	void filterSearchChanged();
	void filterFolderChanged(QTreeWidgetItem * = NULL, QTreeWidgetItem * = NULL);

private:
	bool hasInitd;
	QString currentTempROM;
	QFutureWatcher<void> loadIconWatcher;
	QAbstractItemDelegate *defaultGameListDelegate;
	// interactive threads used by the game list
	UpdateSelectionThread selectionThread;
	QList<QTreeWidgetItem *> intFolderItems;
	QStringList extFolderNames;

	QTimer timerJoy;
	QTime timeJoyRepeatDelay;

	void initFolders();
	int parseExtFolders(const QString &);
	void initExtFolders(const QString &, const QString &);
	void saveExtFolders(const QString &);

	void initMenus();
	void updateDynamicMenu(QMenu *);
	void updateDeleteCfgMenu(const QString &);
	void addDeleteCfgMenu(const QString &, const QString &);
	void loadMMO(int);
	void loadIconWorkder();
	void openJoysticks();
	void closeJoysticks();

private slots:
	void setupSnap(int);
	void showContextMenu(const QPoint &);
	void updateContextMenu();
	void mountDevice();
	void unmountDevice();
	void toggleFolder();
	void showHeaderContextMenu(const QPoint &);
	void updateHeaderContextMenu();
	void deleteCfg();
	void addToExtFolder();
	void removeFromExtFolder();
	void postLoadIcon();
	void processJoyEvents();
};

class GameListSortFilterProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
	QString searchText, filterText;
	QStringList filterList;

	GameListSortFilterProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int, const QModelIndex &) const;
	bool lessThan(const QModelIndex &, const QModelIndex &) const;
};

extern Gamelist *gameList;
extern QString currentGame, currentFolder;
extern QStringList hiddenFolders;
extern QMap<QString, QString> consoleMap;
extern QActionGroup *colSortActionGroup;
extern QList<QAction *> colToggleActions;

#endif
