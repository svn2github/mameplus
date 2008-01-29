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

#include "qticohandler.h"

#include "gamelist.h"
#include "qmc2main.h"
#include "procmgr.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern ProcessManager *qmc2ProcessManager;
extern bool qmc2ReloadActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2StopParser;
extern bool qmc2IconsPreloaded;
MameGame *mamegame;


#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 1

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

//	if ( verifyProc )
//		verifyProc->terminate();
}

void Gamelist::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::load()");
#endif

	qmc2ReloadActive = qmc2EarlyReloadActive = TRUE;
	qmc2StopParser = FALSE;


	if (!mamegame->des11n())
	{
		qmc2MainWindow->log(LOG_QMC2, "DEBUG: sGamelist::start tree()");
		qmc2MainWindow->treeWidgetHierarchy->clear();
		//start building tree
		buildTree(true);  //parent
		qmc2MainWindow->log(LOG_QMC2, "DEBUG: sGamelist::start tree2()");
		buildTree(false); //clone
		qmc2MainWindow->log(LOG_QMC2, "DEBUG: sGamelist::end tree()");
		qApp->processEvents();
	
		delete mamegame;
		mamegame = 0;
		return;
	}

	QStringList args;
	QString command;

	gamelistBuffer = "";

	loadTimer.start();
	command = "mamep.exe";
	args << "-listxml";

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

  mamegame->s11n();

  delete mamegame;
  mamegame = 0;
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

  parse();
}

void Gamelist::loadReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::loadReadyReadStandardOutput(): proc = 0x" + QString::number((ulong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  //mamep: remove windows endl
  s.replace(QString("\r"), QString(""));

  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<game name="));
  gamelistBuffer += s;
}

void Gamelist::loadReadyReadStandardError()
{
// QProcess *proc = (QProcess *)sender();

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

bool Gamelist::loadIcon(QString gameName, QTreeWidgetItem *item, bool checkOnly, QString *fileName)
{
  static QIcon icon;
  static QPixmap pm;

  if (qmc2IconsPreloaded)
  {
	GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
	icon = gameinfo->icon;
	if (icon.isNull() && !gameinfo->cloneof.isEmpty())
	    icon = mamegame->gamenameGameInfoMap[gameinfo->cloneof]->icon;
	if (!icon.isNull())	
		item->setIcon(0, icon);
    return FALSE;
  }

    // use icon directory
    else {
      QTime preloadTimer, elapsedTime;
      preloadTimer.start();
      QString icoDir = "D:/mame/icons/";
      QDir iconDirectory(icoDir);
      QStringList nameFilter;
      nameFilter << "*.ico";
      QStringList iconFiles = iconDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);
      int iconCount;
	//save progress bar for restoration
      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
      qmc2MainWindow->progressBarGamelist->setRange(0, iconFiles.count());
      qmc2MainWindow->progressBarGamelist->reset();
//    qApp->processEvents();
      for (iconCount = 0; iconCount < iconFiles.count(); iconCount++) {
        qmc2MainWindow->progressBarGamelist->setValue(iconCount);
//      if ( iconCount % 25 == 0 )
//        qApp->processEvents();
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(LOG_QMC2, QString("DEBUG: loading %1").arg(iconFiles[iconCount]));
#endif
          icon = loadWinIco(icoDir + iconFiles[iconCount]);

		QString icofname = iconFiles[iconCount].toLower().remove(".ico");
		if (mamegame->gamenameGameInfoMap.contains(icofname))
			mamegame->gamenameGameInfoMap[icofname]->icon = icon;
/*
        qmc2IconMap[iconFiles[iconCount].toLower().remove(".ico")] = icon;
*/
      }
      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(LOG_QMC2, tr("done (pre-caching icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(LOG_QMC2, tr("%1 icons loaded").arg(iconCount));
      qmc2IconsPreloaded = TRUE;
	  return loadIcon(gameName, item, checkOnly);
    }

  return FALSE;
}

listXMLHandler::listXMLHandler(QTreeWidget *treeWidget)
    : treeWidget(treeWidget)
{
    item = 0;
	gameinfo = 0;
	
    metMameTag = false;

	if (mamegame)
		delete mamegame;
	mamegame = new MameGame();
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
		rominfo->size = attributes.value("size").toULongLong();

		bool ok;
		gameinfo->crcRomInfoMap[attributes.value("crc").toInt(&ok, 16)] = rominfo;
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

void MameGame::s11n()
{
	QFile file("gui.cache");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_3);

	out << mamegame->gamenameGameInfoMap.count();
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gamename];
		out << gamename;
		out << gameinfo->description;
		out << gameinfo->year;
		out << gameinfo->manufacturer;
		out << gameinfo->sourcefile;
		out << gameinfo->cloneof;

		out << gameinfo->crcRomInfoMap.count();
		foreach (quint32 crc, gameinfo->crcRomInfoMap.keys())
		{
			RomInfo *rominfo = gameinfo->crcRomInfoMap.value(crc);
			out << crc;
			out << rominfo->name;
			out << rominfo->status;
			out << rominfo->size;
		}
	}
	file.close();//fixme check
}

int MameGame::des11n()
{
	QFile file("gui.cache");
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);

	// Read and check the header
	quint32 mamepSig;
	in >> mamepSig;
	if (mamepSig != MAMEPLUS_SIG)
		return QDataStream::ReadCorruptData;

	// Read the version
	qint16 version;
	in >> version;
	if (version != S11N_VER)
		return QDataStream::ReadCorruptData;
	
	if (version < 1)
		in.setVersion(QDataStream::Qt_4_2);
	else
		in.setVersion(QDataStream::Qt_4_3);

	//finished checking
	int gamecount;
	in >> gamecount;

	if (gamecount > 0)
	{
		if (mamegame)
			delete mamegame;
		mamegame = new MameGame();
	}
	
	for (int i = 0; i < gamecount; i++)
	{
		GameInfo *gameinfo = new GameInfo(mamegame);
		QString gamename;
		in >> gamename;
		in >> gameinfo->description;
		in >> gameinfo->year;
		in >> gameinfo->manufacturer;
		in >> gameinfo->sourcefile;
		in >> gameinfo->cloneof;
		mamegame->gamenameGameInfoMap[gamename] = gameinfo;

		int romcount;
		in >> romcount;
		for (int j = 0; j < romcount; j++)
		{
			RomInfo *rominfo = new RomInfo(gameinfo);
			quint32 crc;
			in >> crc;
			in >> rominfo->name;
			in >> rominfo->status;
			in >> rominfo->size;
			gameinfo->crcRomInfoMap[crc] = rominfo;
		}
	}

	return in.status();
}

QIcon loadWinIco(const QString & fileName)
{
	QFile *pIcoFile = new QFile(fileName);
	QIcon icon = QIcon();
	if (pIcoFile->open(QIODevice::ReadOnly))
	{
		QList<QImage> imgList = ICOReader::read(pIcoFile);
		if (!imgList.isEmpty())
			icon = QIcon((QPixmap::fromImage(imgList.first())));
		pIcoFile->close();
	}
	delete pIcoFile;
	return icon;
}

void Gamelist::buildTree(bool isParent)
{
//	qmc2MainWindow->treeWidgetHierarchy->setIconSize(QSize(32,32));

	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gamename];
		if ((gameinfo->cloneof.isEmpty() || isParent) && !(gameinfo->cloneof.isEmpty() && isParent)) //logical XOR
		  continue;

		QTreeWidgetItem *childItem;
		if (isParent)
			//create a new item
			gameinfo->pItem = childItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
		else
		{
			//find parent and add to it
			QTreeWidgetItem *parentItem = mamegame->gamenameGameInfoMap[gameinfo->cloneof]->pItem;
			gameinfo->pItem = childItem = new QTreeWidgetItem(parentItem);
		}

		childItem->setData(0, Qt::UserRole, gamename);
		childItem->setText(0, gameinfo->description);
		childItem->setText(1, gamename);
		childItem->setText(2, gameinfo->manufacturer);
		childItem->setText(3, gameinfo->sourcefile);
		childItem->setText(5, gameinfo->year);
		childItem->setText(6, gameinfo->cloneof);

		loadIcon(gamename, childItem);
	}
}

