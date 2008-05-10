#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#include "utils.h"

class AuditROMThread : public QThread
{
	Q_OBJECT

public:
	~AuditROMThread();
	void audit();

signals:
	void progressSwitched(int max, QString title = "");
	void progressUpdated(int progress);
	void logUpdated(char, QString);

protected:
	void run();

private:
	QMutex mutex;
};

class LoadIconThread : public QThread
{
	Q_OBJECT

public:
	MyQueue iconQueue;

//	LoadIconThread(QObject *parent = 0);
	~LoadIconThread();

	void load();

signals:
	void icoUpdated(QString);

protected:
	void run();

private:
	bool done;
	bool restart;
	QMutex mutex;
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	MyQueue myqueue;
	QString historyText;
	QString mameinfoText;
	QString storyText;

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
	TreeModel(QObject *parent = 0);
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
	void setupModelData(TreeItem *parent, bool isParent);
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

	QStringList folderList, mftrList, yearList, srcList, biosList;

	AuditROMThread auditThread;
	LoadIconThread iconThread;
	UpdateSelectionThread selectThread;

	Gamelist(QObject *parent = 0);
	~Gamelist();

public slots:
	void init();
	void loadDefaultIni();

	void initFolders();

	void runMame();

	// process management
	void loadListXmlStarted();
	void loadListXmlReadyReadStandardOutput();
	void loadListXmlFinished(int, QProcess::ExitStatus);
	void loadDefaultIniReadyReadStandardOutput();
	void loadDefaultIniFinished(int, QProcess::ExitStatus);

	// internal methods
	void parse();
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);
	void restoreSelection();
	void setupIcon(QString);
	void setupAudit();
	void setupHistory();

	void filterTimer();
	void filterRegExpChanged();
	void filterRegExpChanged2(QTreeWidgetItem *, QTreeWidgetItem *previous = 0);
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
	~BiosInfo();
};

class GameInfo : public QObject
{
public:
	QString description, year, manufacturer, sourcefile, cloneof, romof, lcDescription, reading;
	bool isbios;
	QHash<quint32, RomInfo *> crcRomInfoMap;
	QHash<QString, BiosInfo *> nameBiosInfoMap;
	int available;
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
	QHash<QString, GameInfo *> gamenameGameInfoMap;

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
