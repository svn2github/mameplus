#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QString>
#include <QTime>
#include <QProcess>
#include <QIcon>
#include <QFile>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QTreeView>
#include <QVariant>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include <QFileIconProvider>
#include <QThread>

class LoadIconThread : public QThread
{
	Q_OBJECT

public:
    void render();

signals:
	void progressSwitched(int max);
    void progressUpdated(int progress);

protected:
	void run();
};

class AuditROMThread : public QThread
{
	Q_OBJECT

public:
	void audit();

signals:
	void progressSwitched(int max);
    void progressUpdated(int progress);

protected:
	void run();
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
	QIcon icon;

private:
    QList<TreeItem*> childItems;
    QList<QVariant> itemData;
    TreeItem *parentItem;
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
	TreeModel(const QStringList &headers, QObject *parent = 0);
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
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
	bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole);


private:
    void setupModelData(TreeItem *parent, bool isParent);
	TreeItem *getItem(const QModelIndex &index) const;

    TreeItem *rootItem;
};



class Gamelist : public QObject
{
  Q_OBJECT

  public:
    QIcon qmc2SmallGhostImageIcon;
    QIcon qmc2UnknownImageIcon;
    QIcon qmc2UnknownBIOSImageIcon;
    QIcon qmc2CorrectImageIcon;
    QIcon qmc2CorrectBIOSImageIcon;
    QIcon qmc2MostlyCorrectImageIcon;
    QIcon qmc2MostlyCorrectBIOSImageIcon;
    QIcon qmc2IncorrectImageIcon;
    QIcon qmc2IncorrectBIOSImageIcon;
    QIcon qmc2NotFoundImageIcon;
    QIcon qmc2NotFoundBIOSImageIcon;
    QProcess *loadProc;
    QProcess *verifyProc;
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

	AuditROMThread auditthread;
	LoadIconThread iconthread;

    Gamelist(QObject *parent = 0);
    ~Gamelist();

  public slots:
    void load();

    // process management
    void loadStarted();
    void loadFinished(int, QProcess::ExitStatus);
    void loadReadyReadStandardOutput();
    void loadReadyReadStandardError();
    void loadError(QProcess::ProcessError);
    void loadStateChanged(QProcess::ProcessState);

    // internal methods
    void parse();
	void updateProgress(int progress);
	void switchProgress(int max);
	void setupIcon();
	void setupAudit();
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
    QString description, year, manufacturer, sourcefile, cloneof, lcDescription, reading;
    QHash<quint32, RomInfo *> crcRomInfoMap;
	bool available;
	QIcon icon;
	TreeItem *pModItem;
	
	GameInfo(QObject *parent = 0);
    ~GameInfo();
};

class MameGame : public QObject
{
  public:
	QString mameVersion;
	QHash<QString, GameInfo *> gamenameGameInfoMap;

	MameGame(QObject *parent = 0);
	~MameGame();

	void s11n();
	int des11n();
};

class ListXMLHandler : public QXmlDefaultHandler
{
public:
    ListXMLHandler(int d = 0);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);

private:
	GameInfo *gameinfo;
    QString currentText;
    bool metMameTag;
};

bool loadIcon(QString, bool checkOnly = FALSE);
QIcon loadWinIco(const QString & fileName);
void auditROMs();

#endif