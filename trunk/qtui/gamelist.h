#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#include "audit.h"
#include "utils.h"

enum
{
	GAMELIST_INIT_FULL = 0,
	GAMELIST_INIT_AUDIT,
	GAMELIST_INIT_DIR
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	MyQueue myqueue;
	QString historyText;
	QString mameinfoText;
	QString storyText;
	QString commandText;

	QByteArray pmdataSnap;
	QByteArray pmdataFlyer;
	QByteArray pmdataCabinet;
	QByteArray pmdataMarquee;
	QByteArray pmdataTitle;
	QByteArray pmdataCPanel;
	QByteArray pmdataPCB;

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
	bool setData(int column, const QVariant &value);

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

	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const;
	QModelIndex index(int row, int column,
		const QModelIndex &parent = QModelIndex()) const;
	QModelIndex index(int column, TreeItem *childItem) const;
	QModelIndex parent(const QModelIndex &index) const;
	int rowCount(const QModelIndex &parent = QModelIndex()) const;
	void rowChanged(const QModelIndex &index);
	int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool setData(const QModelIndex &index, const QVariant &value,
		int role = Qt::EditRole);
	bool setHeaderData(int section, Qt::Orientation orientation,
		const QVariant &value, int role = Qt::EditRole);

	TreeItem *getItem(const QModelIndex &index) const;
	
	TreeItem *rootItem;

private:
	TreeItem * buildItem(TreeItem *, QString, bool);
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
	QString mameOutputBuf;
	QStringList xmlLines;
	QTime loadTimer;
	int numTotalGames;
	QString mameVersion;
	QMenu *menu;
	QMenu *headerMenu;

	QStringList folderList;

	// interactive threads used by the game list
	RomAuditor auditor;
	UpdateSelectionThread selectionThread;

	MergedRomAuditor *mAuditor;

	Gamelist(QObject *parent = 0);
	~Gamelist();

	void loadIcon();
	void restoreGameSelection();

public slots:
	void init(bool, int = GAMELIST_INIT_AUDIT);	//the default init value is a hack, for connect slots

	void showContextMenu(const QPoint &);
	void updateContextMenu();
	void showHeaderContextMenu(const QPoint &);
	void updateHeaderContextMenu();

	void loadDefaultIni();
	void runMame(bool = false);
	QString getViewString(const QModelIndex &index, int column) const;
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection();
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);
	void setupSnap(int);


	// external process management
	void loadListXmlReadyReadStandardOutput();
	void loadListXmlFinished(int, QProcess::ExitStatus);
	void loadDefaultIniReadyReadStandardOutput();
	void loadDefaultIniFinished(int, QProcess::ExitStatus);
	void extractMerged(QString, QString);
	void extractMergedFinished(int, QProcess::ExitStatus);
	void runMergedFinished(int, QProcess::ExitStatus);

	void filterRegExpChanged();
	void filterRegExpCleared();
	void filterRegExpChanged2(QTreeWidgetItem *, QTreeWidgetItem *previous = NULL);

private:
	QString currentTempROM;
	QFutureWatcher<void> loadIconWatcher;
	int loadIconStatus;
	QAbstractItemDelegate *defaultGameListDelegate;

	void initFolders();
	void initMenus();
	void loadMMO(int);
	void loadIconWorkder();
	void parse();
	void restoreFolderSelection();

private slots:
	void postLoadIcon();
};

class RomInfo : public QObject
{
public:
	QString name, bios, status;
	quint64 size;
	bool available;

	RomInfo(QObject *parent = 0);
	~RomInfo();
};

class BiosInfo : public QObject
{
public:
	QString description;
	bool isdefault;

	BiosInfo(QObject *parent = 0);
};

class DeviceInfo : public QObject
{
public:
	QString type;
	QStringList extension;
	bool mandatory;

	DeviceInfo(QObject *parent = 0);
};

class GameInfo : public QObject
{
public:
	QString description, year, manufacturer, sourcefile, cloneof, romof, lcDesc, lcMftr, reading;
	quint8 status, emulation, color, sound, graphic, cocktail, protection, savestate;
	quint32 palettesize;

	bool isBios, isExtRom,isCloneAvailable;

	QHash<quint32, RomInfo *> crcRomInfoMap;
	QHash<QString, BiosInfo *> nameBiosInfoMap;
	QHash<QString, DeviceInfo *> nameDeviceInfoMap;
	
	qint8 available;
	QByteArray icondata;
	TreeItem *pModItem;
	QSet<QString> clones;

	GameInfo(QObject *parent = 0);
	~GameInfo();
	QString biosof();
};

class MameGame : public QObject
{
Q_OBJECT

public:
	QString mameVersion;
	QString mameDefaultIni;
	QHash<QString, GameInfo *> nameInfoMap;

	MameGame(QObject *parent = 0);
	~MameGame();

	void s11n();
	int des11n();
};

class GameListSortFilterProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
	QString searchText, filterText;

	GameListSortFilterProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif
