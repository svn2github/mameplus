#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QString>
#include <QTime>
#include <QProcess>
#include <QIcon>
#include <QFile>
#include <QTreeWidget>
#include <QTextStream>
#include <QXmlDefaultHandler>

typedef unsigned int						UINT32;


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
    QFile listXMLCache;
    QTextStream tsRomCache;
    QTextStream tsGamelistCache;
    QTextStream tsListXMLCache;
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
    QTreeWidgetItem *checkedItem;
    bool autoROMCheck;

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
};

class RomInfo : public QObject
{
  public:
    QString name, status;
	long size;

	RomInfo(QObject *parent = 0);

    ~RomInfo();
};


class GameInfo : public QObject
{
  public:
    QString description, year, manufacturer, sourcefile, cloneof, lcDescription, reading;
    QHash<UINT32, RomInfo *> crcRomInfoMap;
	QTreeWidgetItem *pItem;
	
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
};

class listXMLHandler : public QXmlDefaultHandler
{
public:
    listXMLHandler(QTreeWidget *treeWidget);

    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool characters(const QString &str);

private:
    QTreeWidget *treeWidget;
    QTreeWidgetItem *item;
	GameInfo *gameinfo;
    QString currentText;
    bool metMameTag;
};

void buildTree(bool isClone);


#endif
