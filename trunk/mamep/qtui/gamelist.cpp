#include <QTextStream>
#include <QHeaderView>
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

#include "quazip.h"
#include "quazipfile.h"

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
TreeModel *model;
static QIcon defIcon;

#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 2

void LoadIconThread::render()
{
    if (!isRunning())
        start(LowPriority);
}

void LoadIconThread::run()
{
	QTime preloadTimer, elapsedTime;
	preloadTimer.start();
	QString icoDir = "D:/mame/icons/";
	QDir iconDirectory(icoDir);
	QStringList nameFilter;
	nameFilter << "*.ico";
	QStringList iconFiles = iconDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);

	emit progressSwitched(iconFiles.count());

	//scan icon dir, assign icons to gameinfo	
	for (int iconCount = 0; iconCount < iconFiles.count(); iconCount++)
	{
		if ( iconCount % 100 == 0 )
			emit progressUpdated(iconCount);

		QIcon icon = loadWinIco(icoDir + iconFiles[iconCount]);

		QString icofname = iconFiles[iconCount].toLower().remove(".ico");
		if (mamegame->gamenameGameInfoMap.contains(icofname))
		{
			GameInfo *gameinfo = mamegame->gamenameGameInfoMap[icofname];
			gameinfo->icon = icon;
		}
	}
	elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());

	emit progressSwitched(-1);
}

void AuditROMThread::audit()
{
    if (!isRunning())
        start(LowPriority);
}

void AuditROMThread::run()
{
	QString romDir = "D:/mame/roms/";
	QDir romDirectory(romDir);
	QStringList nameFilter;
	nameFilter << "*.zip";

	QStringList romFiles = romDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);

	qmc2MainWindow->log(LOG_QMC2, "audit 0");
	emit progressSwitched(romFiles.count());

	for (int i = 0; i < romFiles.count(); i++)
	{
		if ( i % 100 == 0 )
			emit progressUpdated(i);
	
		QString gamename = romFiles[i].toLower().remove(".zip");

//		qmc2MainWindow->log(LOG_QMC2, QString("auditing zip: " + gamename));

		if (mamegame->gamenameGameInfoMap.contains(gamename))
		{
			QuaZip zip(romDir + romFiles[i]);
			if(!zip.open(QuaZip::mdUnzip))
			{
		//	  qWarning("testRead(): zip.open(): %d", zip.getZipError());
			  continue;
			}
		//	printf("%d entries\n", zip.getEntriesCount());
			QuaZipFileInfo info;
			QuaZipFile zipFile(&zip);
			GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gamename];

			for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
			{
			  if(!zip.getCurrentFileInfo(&info))
			  {
//				qmc2MainWindow->log(LOG_QMC2, QString("zipfile err: " + zip.getZipError()));
				continue;
			  }

			  quint32 crc = info.crc;
			  if (gameinfo->crcRomInfoMap.contains(info.crc))
				gameinfo->crcRomInfoMap[crc]->available = true; 
			}
		}
	}

	qmc2MainWindow->log(LOG_QMC2, "audit 1");
	//see if any rom of a game is not available
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gamename];
		foreach (quint32 crc, gameinfo->crcRomInfoMap.keys())
		{
			RomInfo *rominfo = gameinfo->crcRomInfoMap.value(crc);
			if (!rominfo->available)
			{
				gameinfo->available = false;
				break;
			}
		}
	}
	qmc2MainWindow->log(LOG_QMC2, "audit 2");
	emit progressSwitched(-1);
}

TreeItem::TreeItem(const QList<QVariant> &data, TreeItem *parent)
{
    parentItem = parent;
    itemData = data;
}

TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}

void TreeItem::appendChild(TreeItem *item)
{
    childItems.append(item);
}

TreeItem *TreeItem::child(int row)
{
    return childItems.value(row);
}

int TreeItem::childCount() const
{
    return childItems.count();
}

int TreeItem::columnCount() const
{
    return itemData.count();
}

QVariant TreeItem::data(int column) const
{
    return itemData.value(column);
}

int TreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}

TreeItem *TreeItem::parent()
{
    return parentItem;
}

bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData.size())
        return false;

    itemData[column] = value;
    return true;
}

TreeModel::TreeModel(const QStringList &headers, QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
	
    foreach (QString header, headers)
        rootData << header;
	
    rootItem = new TreeItem(rootData);
    setupModelData(rootItem, true);
	setupModelData(rootItem, false);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
	return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::TextColorRole)
	{
		return qVariantFromValue(QColor(Qt::black));
	}
	
    if (!index.isValid())
        return QVariant();

    TreeItem *item = getItem(index);

	if (role == Qt::DecorationRole && index.column() == 0)
	{
		if (item->icon.isNull())
		{
			return defIcon;
		}
		return item->icon;
	}

    if (role != Qt::DisplayRole)
        return QVariant();

    return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
	{
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
			return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::index(int column, TreeItem *childItem) const
{
	if (childItem)
		return createIndex(childItem->row(), column, childItem);
    else
        return QModelIndex();

}


QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value,
                        int role)
{
    TreeItem *item = getItem(index);

	if (index.isValid())
	{
		switch (role)
		{
		case Qt::DisplayRole:
			if (item->setData(index.column(), value))
				emit dataChanged(index, index);
				return true;
			break;

		case Qt::DecorationRole:
			const QIcon icon(qvariant_cast<QIcon>(value));
			if(!icon.isNull())
			{
				item->icon = icon;
				emit dataChanged(index, index);
				return true;
			}
			break;
		}
	}
	return false;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    return rootItem->setData(section, value);
}

void TreeModel::setupModelData(TreeItem *parent, bool buildParent)
{
	GameInfo *gameinfo;
	QList<QVariant> columnData;

	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];

		if ((gameinfo->cloneof.isEmpty() ^ buildParent))
			continue;

		columnData.clear();
		columnData << gameinfo->description;
		columnData << gamename;
		columnData << "N/A";
		columnData << gameinfo->manufacturer;
		columnData << gameinfo->sourcefile;
		columnData << gameinfo->year;
		columnData << gameinfo->cloneof;

		if (!buildParent)
			//find parent and add to it
			parent = mamegame->gamenameGameInfoMap[gameinfo->cloneof]->pModItem;

		// Append a new item to the current parent's list of children.
		gameinfo->pModItem = new TreeItem(columnData, parent);
		parent->appendChild(gameinfo->pModItem);
	}
}


RomInfo::RomInfo(QObject *parent)
: QObject(parent)
{
	available = false;
	//	qmc2MainWindow->log(LOG_QMC2, "# RomInfo()");
}

RomInfo::~RomInfo()
{
	//    qmc2MainWindow->log(LOG_QMC2, "# ~RomInfo()");
}

GameInfo::GameInfo(QObject *parent)
: QObject(parent)
{
	available = true;
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
	connect(&iconthread, SIGNAL(progressSwitched(int)), this, SLOT(switchProgress(int)));
	connect(&iconthread, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
	connect(&iconthread, SIGNAL(finished()), this, SLOT(setupIcon()));

	connect(&auditthread, SIGNAL(progressSwitched(int)), this, SLOT(switchProgress(int)));
	connect(&auditthread, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
	connect(&auditthread, SIGNAL(finished()), this, SLOT(setupAudit()));

	numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = -1;
	loadProc = verifyProc = NULL;
	defIcon = loadWinIco(":/res/win_roms.ico");
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

void Gamelist::updateProgress(int progress)
{
	qmc2MainWindow->progressBarGamelist->setValue(progress);
}

void Gamelist::switchProgress(int max)
{
	static int prevmax;
	if (max != -1)
	{
		prevmax = qmc2MainWindow->progressBarGamelist->maximum();
		qmc2MainWindow->progressBarGamelist->setRange(0, max);
	}
	else
		qmc2MainWindow->progressBarGamelist->setRange(0, prevmax);

	qmc2MainWindow->progressBarGamelist->reset();
}

void Gamelist::setupIcon()
{
	GameInfo *gameinfo;
	QIcon icon;
	
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];

		//complete clone gameinfo with parent icons
		icon = gameinfo->icon; 
		if (icon.isNull() && !gameinfo->cloneof.isEmpty())
			icon = mamegame->gamenameGameInfoMap[gameinfo->cloneof]->icon;
		model->setData(model->index(0, gameinfo->pModItem), icon, Qt::DecorationRole);
	}
}

void Gamelist::setupAudit()
{
	GameInfo *gameinfo;
	RomInfo *rominfo;
	
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		model->setData(model->index(2, gameinfo->pModItem), 
				gameinfo->available ? "Yes" : "No", Qt::DisplayRole);
	}
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
		QStringList headers;
		headers << "Description" << "Name" << "Available" << "Manufacturer" << "Driver" << "Year" << "Clone of";

		model = new TreeModel(headers, qmc2MainWindow);
		qmc2MainWindow->treeViewGameList->setModel(model);

//		auditthread.audit();
		iconthread.render();


//fixme		delete mamegame;
//		mamegame = 0;

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

	ListXMLHandler handler(0);
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
//	buildTree(true);	//parent
	qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::start tree2()");
//	buildTree(false);	//clone
	qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::end tree()");

	mamegame->s11n();

	delete mamegame;
	mamegame = 0;
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

ListXMLHandler::ListXMLHandler(int d)
{
	gameinfo = 0;
	metMameTag = false;

	if (mamegame)
		delete mamegame;
	mamegame = new MameGame();
}

bool ListXMLHandler::startElement(const QString & /* namespaceURI */,
								  const QString & /* localName */,
								  const QString &qName,
								  const QXmlAttributes &attributes)
{
	if (!metMameTag && qName != "mame")
		return false;

	if (qName == "mame")
		metMameTag = true;
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
		gameinfo->crcRomInfoMap[attributes.value("crc").toUInt(&ok, 16)] = rominfo;
	}

	currentText.clear();
	return true;
}

bool ListXMLHandler::endElement(const QString & /* namespaceURI */,
								const QString & /* localName */,
								const QString &qName)
{
	if (qName == "description")
		gameinfo->description = currentText;
	else if (qName == "manufacturer")
		gameinfo->manufacturer = currentText;
	else if (qName == "year")
		gameinfo->year = currentText;
	return true;
}

bool ListXMLHandler::characters(const QString &str)
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
	if (icon.isNull() && !defIcon.isNull())
		icon = defIcon;
	delete pIcoFile;
	return icon;
}
/*
void Gamelist::buildTree(bool isParent)
{
	static int gameCount = 0;
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

		childItem->setText(11, QString::number(gameinfo->crcRomInfoMap.count()));

		childItem->setIcon(0, defIcon);
		childItem->setSizeHint(0, QSize(200, 17));
//		if (!gameinfo->available)
//			childItem->setHidden(true);
//		else
			gameCount++;
	}
	qmc2MainWindow->log(LOG_QMC2, QString("DEBUG: available games %1").arg(gameCount));
}
*/


