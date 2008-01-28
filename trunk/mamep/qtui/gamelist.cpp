#include <QTextStream>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QFile>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QBitArray>


#include "gamelist.h"
#include "qmc2main.h"
#include "procmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern ProcessManager *qmc2ProcessManager;
extern bool qmc2ReloadActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2StopParser;
MameGame *mamegame;


RomInfo::RomInfo(QObject *parent)
	: QObject(parent)

{
//	qmc2MainWindow->log(LOG_QMC2, "# RomInfo()");
}

RomInfo::~RomInfo()
{
//    qmc2MainWindow->log(LOG_QMC2, "# ~RomInfo()");
}

GameInfo::GameInfo(QObject *parent)
	: QObject(parent)
{
	pItem = 0;
//	qmc2MainWindow->log(LOG_QMC2, "# GameInfo()");
}

GameInfo::~GameInfo()
{
//  qmc2MainWindow->log(LOG_QMC2, "# ~GameInfo()");
}

MameGame::MameGame(QObject *parent)
	: QObject(parent)
{
  qmc2MainWindow->log(LOG_QMC2, "# MameGame()");

  this->mameVersion = mameVersion;
}

MameGame::~MameGame()
{
  qmc2MainWindow->log(LOG_QMC2, "# ~MameGame()");
}


Gamelist::Gamelist(QObject *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::Gamelist()");
#endif

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = -1;
  loadProc = verifyProc = NULL;
}

Gamelist::~Gamelist()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::~Gamelist()");
#endif

  if ( loadProc )
    loadProc->terminate();
}

void Gamelist::load()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::load()");
#endif

  qmc2ReloadActive = qmc2EarlyReloadActive = TRUE;
  qmc2StopParser = FALSE;

  QStringList args;
  QString command;

  gamelistBuffer = "";

  listXMLCache.setFileName("listXML.cache");

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

    loadTimer.start();
    command = "mamep.exe";
    args << "-listxml";

    listXMLCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !listXMLCache.isOpen() ) {
      qmc2MainWindow->log(LOG_QMC2, tr("WARNING: can't open XML gamelist cache for writing, please check permissions"));
    } else {
      tsListXMLCache.setDevice(&listXMLCache);
      tsListXMLCache.reset();
      tsListXMLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
      tsListXMLCache << "MAME_VERSION\t" + mameVersion + "\n";
    }
    loadProc = qmc2ProcessManager->process(qmc2ProcessManager->start(command, args, FALSE));
    connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
    connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
    connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
    connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
    connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
    connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));

}

void Gamelist::parse()
{
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::prep parse()");

  qmc2MainWindow->treeWidgetHierarchy->hide();
  qmc2MainWindow->treeWidgetHierarchy->clear();
//  qApp->processEvents();

  listXMLHandler handler(qmc2MainWindow->treeWidgetHierarchy);
  QXmlSimpleReader reader;
  reader.setContentHandler(&handler);
  reader.setErrorHandler(&handler);

  QXmlInputSource *pxmlInputSource = new QXmlInputSource();
  pxmlInputSource->setData (gamelistBuffer);
  
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::start parse()");
  reader.parse(*pxmlInputSource);

  gamelistBuffer.clear();

  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::start tree()");
  //start building tree
  buildTree(true);	//parent
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::start tree2()");
  buildTree(false);	//clone
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::end tree()");
  
  delete mamegame;
  qmc2MainWindow->treeWidgetHierarchy->show();
}

void Gamelist::loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadStarted()");
#endif

  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  qmc2MainWindow->progressBarGamelist->reset();
}

void Gamelist::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadFinished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = " + QString::number(exitStatus) + "): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
  qmc2MainWindow->log(LOG_QMC2, tr("done (loading XML gamelist data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->progressBarGamelist->reset();
  qmc2ProcessManager->procMap.remove(proc);
  qmc2ProcessManager->procCount--;
  qmc2EarlyReloadActive = FALSE;
  loadProc = NULL;

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

  parse();
}

void Gamelist::loadReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadReadyReadStandardOutput(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  //mamep: remove windows endl3
  s.replace(QString("\r"), QString(""));

  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<game name="));
  gamelistBuffer += s;

  if ( listXMLCache.isOpen() )
    tsListXMLCache << s;
}

void Gamelist::loadReadyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadReadyReadStandardError(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

}

void Gamelist::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

listXMLHandler::listXMLHandler(QTreeWidget *treeWidget)
    : treeWidget(treeWidget)

{
    item = 0;
	gameinfo = 0;
	
	mamegame = new MameGame();
    metMameTag = false;
}

bool listXMLHandler::startElement(const QString & /* namespaceURI */,
                               const QString & /* localName */,
                               const QString &qName,
                               const QXmlAttributes &attributes)
{
    if (!metMameTag && qName != "mame") {
        return false;
    }

	if (qName == "mame")
	{
	  metMameTag = true;
	}
	else if (qName == "game")
	{
		gameinfo = new GameInfo(mamegame);
		gameinfo->sourcefile = attributes.value("sourcefile");
		gameinfo->cloneof = attributes.value("cloneof");
		mamegame->gamenameGameInfoMap[attributes.value("name")] = gameinfo;
	}
	else if (qName == "rom")
	{
		RomInfo *rominfo = new RomInfo(gameinfo);
		rominfo->name = attributes.value("name");
		rominfo->status = attributes.value("status");
		rominfo->size = attributes.value("size").toLong();

		bool ok;
		gameinfo->crcRomInfoMap[attributes.value("crc").toInt(&ok, 16)] = rominfo;

	 /*
		 long crc = 0, size = 0;
		 String scrc = attributes.value("crc");
		 String ssize = attributes.value("size");
		 if (scrc != null && scrc != null)
		 {
			 try
			 {
				 crc = Long.parseLong(scrc, 16);
				 size = Long.parseLong(ssize);
			 }
			 catch (NumberFormatException e)
			 {
				 System.out.println(gamename + "/" + attributes.value("name") + ": crc: "
						 + attributes.value("crc") + " len:" + attributes.value("size"));
			 }
		 }
		 
		 mameGame.setGameRom(gamename, new Long(crc), attributes.value("name"), new Long(size));
	   */
	}

    currentText.clear();
    return true;
}

bool listXMLHandler::endElement(const QString & /* namespaceURI */,
                             const QString & /* localName */,
                             const QString &qName)
{
    if (qName == "description")
    {
	    gameinfo->description = currentText;
    }
	else if (qName == "manufacturer")
	{
		gameinfo->manufacturer = currentText;
	}
	else if (qName == "year")
	{
		gameinfo->year = currentText;
	}
    return true;
}

bool listXMLHandler::characters(const QString &str)
{
    currentText += str;
    return true;
}

void buildTree(bool isParent)
{
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap.value(gamename);
		if ((gameinfo->cloneof.isEmpty() || isParent) && !(gameinfo->cloneof.isEmpty() && isParent)) //logical XOR
		  continue;

		QTreeWidgetItem *childItem;
		if (isParent)
			//create a new item
			gameinfo->pItem = childItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
		else
		{
			//find parent and add to it
			QTreeWidgetItem *parentItem = mamegame->gamenameGameInfoMap.value(gameinfo->cloneof)->pItem;
			if (parentItem)
				childItem = new QTreeWidgetItem(parentItem);
		}

		childItem->setData(0, Qt::UserRole, gamename);
		childItem->setText(0, gameinfo->description);
		childItem->setText(1, gamename);
		childItem->setText(2, gameinfo->manufacturer);
		childItem->setText(3, gameinfo->sourcefile);
		childItem->setText(5, gameinfo->year);
		childItem->setText(6, gameinfo->cloneof);
	}
}

