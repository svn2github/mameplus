#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#include "utils.h"

class MameThread : public QThread
{
	Q_OBJECT

public:
	MyQueue queue;

	MameThread(QObject *parent = 0);
	~MameThread();

	void load();

protected:
	void run();

private:
	QMutex mutex;
	bool abort;
};

class AuditROMThread : public QThread
{
	Q_OBJECT

public:
	void audit();

signals:
	void progressSwitched(int max, QString title = "");
	void progressUpdated(int progress);
	void logUpdated(char, QString);

protected:
	void run();
};

class LoadIconThread : public QThread
{
	Q_OBJECT

public:
	MyQueue iconQueue;

	LoadIconThread(QObject *parent = 0);
	~LoadIconThread();

	void load();

protected:
	void run();

private:
	QMutex mutex;
	bool abort;
};

class UpdateSelectionThread : public QThread
{
	Q_OBJECT

public:
	MyQueue myqueue;
	QString historyText;
	QString mameinfoText;
	QString storyText;

	QPixmap pmSnap;
	QPixmap pmFlyer;
	QPixmap pmCabinet;
	QPixmap pmMarquee;
	QPixmap pmTitle;
	QPixmap pmCPanel;
	QPixmap pmPCB;

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
	QString gamelistBuffer;
	QStringList xmlLines;
	static QStringList phraseTranslatorList;
	QTime loadTimer;
	QTime verifyTimer;
	QTime parseTimer;
	QTime miscTimer;
	QFile romCache;
	QFile gamelistCache;
	QTextStream tsRomCache;
	QTextStream tsGamelistCache;
	int numTotalGames;
	int numGames;
	int numCorrectGames;
	int numMostlyCorrectGames;
	int numIncorrectGames;
	int numNotFoundGames;
	int numUnknownGames;
	int numSearchGames;
	QString mameVersion;
	QString mameTarget;
	bool verifyCurrentOnly;
	bool autoROMCheck;

	AuditROMThread auditThread;
	LoadIconThread iconThread;
	UpdateSelectionThread selectThread;

	Gamelist(QObject *parent = 0);
	~Gamelist();

public slots:
	void load();

	// process management
	void loadStarted();
	void loadFinished(int, QProcess::ExitStatus);
	void loadReadyReadStandardOutput();

	// internal methods
	void parse();
	void updateProgress(int progress);
	void switchProgress(int max, QString title);
	void updateSelection(const QModelIndex & current, const QModelIndex & previous);
	void restoreSelection(QString gameName);
	void restoreSelection();
	void setupIcon();
	void setupAudit();
	void setupHistory();
	void log(char, QString);

	void filterRegExpChanged();
	void filterTimer();
};

class RomInfo : public QObject
{
public:
	QString name, status;
	quint64 size;
	bool available;

	RomInfo(QObject *parent = 0);
	~RomInfo();
};


class GameInfo : public QObject
{
public:
	QString description, year, manufacturer, sourcefile, cloneof, romof, lcDescription, reading;
	QHash<quint32, RomInfo *> crcRomInfoMap;
	int available;
	QIcon icon;
	TreeItem *pModItem;
	QSet<QString> clones;

	GameInfo(QObject *parent = 0);
	~GameInfo();
};

class MameGame : public QObject
{
Q_OBJECT

public:
	QString mameVersion;
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
GameListSortFilterProxyModel(QObject *parent = 0);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
//	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif
