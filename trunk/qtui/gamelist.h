#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#include "audit.h"
#include "utils.h"

enum
{
	GAMELIST_INIT_FULL = 0,
	GAMELIST_INIT_AUDIT,
	GAMELIST_INIT_DIR,
	GAMELIST_INIT_DRIVER
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
	/*
	FOLDER_CPU,
	FOLDER_SND,
	FOLDER_ORIENTATION,
	FOLDER_DEFICIENCY,
	FOLDER_DUMPING,
	FOLDER_WORKING,
	FOLDER_NONWORKING,
	FOLDER_ORIGINAL,
	FOLDER_CLONES,
	FOLDER_RASTER,
	FOLDER_VECTOR,
	FOLDER_RESOLUTION,
	FOLDER_FPS,
	FOLDER_SAVESTATE,
	FOLDER_CONTROL,
	FOLDER_STEREO,
	*/
	FOLDER_HARDDISK,
	/*
	FOLDER_SAMPLES,
	FOLDER_ARTWORK,
	*/
	FOLDER_EXT,
	MAX_FOLDERS
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	MyQueue myqueue;
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
	bool abort;

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

class TreeModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TreeModel(QObject *parent = 0, bool isGroup = true);
	~TreeModel();

	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int column, TreeItem *childItem) const;
	QModelIndex parent(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	void updateRow(const QModelIndex &index);

private:
	TreeItem *rootItem;

	TreeItem *getItem(const QModelIndex &index) const;
	TreeItem *setupModelData(TreeItem *, QString, bool);
};

class GamelistDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	GamelistDelegate(QObject *parent = 0);

	QSize sizeHint ( const QStyleOptionViewItem & option, 
		const QModelIndex & index ) const;

	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index ) const;
};

/*
class XTreeView : public QTreeView
{
Q_OBJECT

public:
    XTreeView(QWidget *parent = 0);
 
protected:
	void paintEvent(QPaintEvent *);
};
//*/

class Gamelist : public QObject
{
	Q_OBJECT

public:
	QProcess *loadProc;
	QStringList xmlLines;
	QMenu *menuContext;
	QMenu *headerMenu;
	QString listMode;

	// interactive threads used by the game list
	UpdateSelectionThread selectionThread;
	bool autoAudit;

	Gamelist(QObject *parent = 0);
	~Gamelist();

	void loadIcon();
	void disableCtrls();
	void restoreFolderSelection(bool = false);
	bool isAuditFolder(QString);
	bool isConsoleFolder();

public slots:
	void init(bool = true, int = GAMELIST_INIT_AUDIT);	//the default init value is a hack, for connect slots
	void update(int = GAMELIST_INIT_FULL);
	void restoreGameSelection();
	void runMame(bool = false, QStringList = QStringList());
	QString getViewString(const QModelIndex &index, int column) const;
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection();
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);
	void setupSnap(int);
	void toggleDelegate(bool);

	// external process management
	void extractMerged(QString, QString);
	void runMameFinished(int, QProcess::ExitStatus);
	void runMergedFinished(int, QProcess::ExitStatus);

	void filterSearchCleared();
	void filterSearchChanged();
	void filterFolderChanged(QTreeWidgetItem * = NULL, QTreeWidgetItem * = NULL);

private:
	bool hasInitd;
	QString currentTempROM;
	QFutureWatcher<void> loadIconWatcher;
	int loadIconStatus;
	QAbstractItemDelegate *defaultGameListDelegate;
	QStringList extFolders;

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

private slots:
	void showContextMenu(const QPoint &);
	void updateContextMenu();
	void mountDevice();
	void unmountDevice();
	void showHeaderContextMenu(const QPoint &);
	void updateHeaderContextMenu();
	void deleteCfg();
	void addToExtFolder();
	void removeFromExtFolder();
	void postLoadIcon();
};

class BiosSet : public QObject
{
public:
	QString description;
	bool isDefault;

	BiosSet(QObject *parent = 0);
};

class RomInfo : public QObject
{
public:
	QString name;
	QString bios;
	quint64 size;
	//quint32 crc is the key
	//md5
	//sha1
	QString merge;
	QString region;
	//offset
	QString status;
	//dispose

	/* internal */
	bool available;

	RomInfo(QObject *parent = 0);
};

class DiskInfo : public QObject
{
public:
	QString name;
	//md5
	//QString sha1 is the key
	QString merge;
	QString region;
	quint8 index;
	QString status;
	//dispose

	/* internal */
	bool available;

	DiskInfo(QObject *parent = 0);
};

class ChipInfo : public QObject
{
public:
	QString name;
	QString tag;
	QString type;
	quint32 clock;

	ChipInfo(QObject *parent = 0);
};

class DisplayInfo : public QObject
{
public:
	QString type;
	QString rotate;
	bool flipx;
	quint16 width;
	quint16 height;
	QString refresh;
//	int pixclock;
	quint16 htotal;
	quint16 hbend;
	quint16 hbstart;
	quint16 vtotal;
	quint16 vbend;
	quint16 vbstart;

	DisplayInfo(QObject *parent = 0);
};

class ControlInfo : public QObject
{
public:
	QString type;
	quint16 minimum;
	quint16 maximum;
	quint16 sensitivity;
	quint16 keydelta;
	bool reverse;

	ControlInfo(QObject *parent = 0);
};

class DeviceInfo : public QObject
{
public:
	QString type;
	QString tag;
	bool mandatory;
	bool isConst;
	QString mountedPath;

//	QString instanceName is the key
	QStringList extensionNames;

	DeviceInfo(QObject *parent = 0);
};

class GameInfo : public QObject
{
public:
	/* game */
	QString sourcefile;
	bool isBios;
//	bool runnable;
	QString cloneof;
	QString romof;
	QString sampleof;
	QString description;
	QString year;
	QString manufacturer;

	/* biosset */
	QHash<QString /*name*/, BiosSet *> biosSets;

	/* rom */
	QHash<quint32 /*crc*/, RomInfo *> roms;

	/* disk */
	QHash<QString /*sha1*/, DiskInfo *> disks;

	/* sample */
	QStringList samples;
	
	/* chip */
	QList<ChipInfo *> chips;

	/* display */
	QList<DisplayInfo *> displays;

	/* sound */
	quint8 channels;

	/* input */
	bool service;
	bool tilt;
	quint8 players;
	quint8 buttons;
	quint8 coins;
	QList<ControlInfo *> controls;

	//dipswitch 

	/* driver, impossible for a game to have multiple drivers */
	quint8 status;
	quint8 emulation;
	quint8 color;
	quint8 sound;
	quint8 graphic;
	quint8 cocktail;
	quint8 protection;
	quint8 savestate;
	quint32 palettesize;

	/* device */
	QMap<QString /* instanceName */, DeviceInfo *> devices;

	/*ramoption */
	QList<quint32> ramOptions;
	quint32 defaultRamOption;

	/* extension */
	QByteArray extraInfo;

	/* internal */
	QString lcDesc;
	QString lcMftr;
	QString reading;

	bool isExtRom;
	bool isCloneAvailable;

	qint8 available;
	QByteArray icondata;
	TreeItem *pModItem;
	QSet<QString> clones;

	GameInfo(QObject *parent = 0);
	~GameInfo();

	QString biosof();
	DeviceInfo *getDevice(QString type, int = 0);
	QString getDeviceInstanceName(QString type, int = 0);
};

class MameGame : public QObject
{
Q_OBJECT

public:
	QString mameVersion;
	QString mameDefaultIni;
	QHash<QString, GameInfo *> games;

	MameGame(QObject *parent = 0);

	void init(int = 0);
	void s11n();
	int completeData();

private:
	QProcess *loadProc;
	int numTotalGames;
	QString mameOutputBuf;

	int des11n();
	void parseListXml();

private slots:
	// external process management
	void loadListXmlReadyReadStandardOutput();
	void loadListXmlFinished(int, QProcess::ExitStatus);
	void loadDefaultIniReadyReadStandardOutput();
	void loadDefaultIniFinished(int, QProcess::ExitStatus);
};

class GameListSortFilterProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
	QString searchText, filterText;
	QStringList filterList;

	GameListSortFilterProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

extern MameGame *mameGame;
extern Gamelist *gameList;
extern QStringList folderList;
extern QString currentGame, currentFolder;

#endif
