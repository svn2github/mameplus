#include "mamepguimain.h"

MameGame *mamegame = NULL, *mamegame0 = NULL;

QString currentGame, currentFolder;
Gamelist *gamelist = NULL;
QStringList consoleGamesL;
TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;

QTimer *searchTimer = NULL;
GamelistDelegate gamelistDelegate(0);

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
	FOLDER_HARDDISK,
	FOLDER_SAMPLES,
	FOLDER_ARTWORK,*/
	MAX_FOLDERS
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
	DOCK_STORY,
	DOCK_LAST
};

enum
{
	COL_DESC = 0,
	COL_NAME,
	COL_ROM,
	COL_MFTR,
	COL_SRC,
	COL_YEAR,
	COL_CLONEOF,
	COL_LAST
};

static QStringList columnList = (QStringList() 
	<< "Description" 
	<< "Name" 
	<< "ROMs" 
	<< "Manufacturer" 
	<< "Driver" 
	<< "Year" 
	<< "Clone of");

class ListXMLHandler : public QXmlDefaultHandler
{
public:
	ListXMLHandler(int d = 0)
	{
		gameinfo = 0;
		metMameTag = false;

// fixme: cannot delete it when new mame version
//		if (mamegame)
//			delete mamegame;
		mamegame = new MameGame(win);
	}

	bool startElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName,
		const QXmlAttributes &attributes)
	{
		if (!metMameTag && qName != "mame")
			return false;

		if (qName == "mame")
		{
			metMameTag = true;
			mamegame->mameVersion = attributes.value("build");
		}
		else if (qName == "game")
		{
			//update progress
			static int i;
			gamelist->updateProgress(i++);
			qApp->processEvents();
			
			gameinfo = new GameInfo(mamegame);
			currentDevice = "";
			gameinfo->sourcefile = attributes.value("sourcefile");
			gameinfo->isBios = attributes.value("isbios") == "yes";
			gameinfo->isExtRom = false;
			gameinfo->cloneof = attributes.value("cloneof");
			gameinfo->romof = attributes.value("romof");
			//fixme: is it possible?
			if (attributes.value("name").isEmpty())
				win->poplog("Error: empty 1");

			mamegame->gamenameGameInfoMap[attributes.value("name")] = gameinfo;
		}
		else if (qName == "rom")
		{
			RomInfo *rominfo = new RomInfo(gameinfo);
			rominfo->name = attributes.value("name");
			rominfo->bios = attributes.value("bios");
			rominfo->size = attributes.value("size").toULongLong();
			rominfo->status = attributes.value("status");

			bool ok;
			gameinfo->crcRomInfoMap[attributes.value("crc").toUInt(&ok, 16)] = rominfo;
		}
		else if (gameinfo->isBios && qName == "biosset")
		{
			BiosInfo *biosinfo = new BiosInfo(gameinfo);
			biosinfo->description = attributes.value("description");
			biosinfo->isdefault = attributes.value("default") == "yes";

			gameinfo->nameBiosInfoMap[attributes.value("name")] = biosinfo;
		}
		else if (qName == "device")
		{
			DeviceInfo *deviceinfo = new DeviceInfo(gameinfo);
			deviceinfo->type = attributes.value("type");
			currentDevice = deviceinfo->type;
			deviceinfo->mandatory = attributes.value("mandatory") == "1";

			gameinfo->nameDeviceInfoMap[currentDevice] = deviceinfo;
		}
		else if (!currentDevice.isEmpty() && qName == "extension")
		{
			DeviceInfo *deviceinfo = gameinfo->nameDeviceInfoMap[currentDevice];
			deviceinfo->extension << attributes.value("name");
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

private:
	GameInfo *gameinfo;
	QString currentText;
	QString currentDevice;
	bool metMameTag;
};

LoadIconThread::~LoadIconThread()
{
	done = true;
	wait();
}

void LoadIconThread::load()
{
	QMutexLocker locker(&mutex);

	if (isRunning())
	{
		restart = true;
	}
	else
	{
		restart = false;
		done = false;
		
		start(LowPriority);
	}
}

void LoadIconThread::run()
{
	GameInfo *gameInfo, *gameInfo2;

//	win->log(QString("ico count: %1").arg(mamegame->gamenameGameInfoMap.count()));

	while(!done)
	{
		// iteratre split dirpath
		QStringList dirpaths = mameOpts["icons_directory"]->globalvalue.split(";");
		foreach (QString _dirpath, dirpaths)
		{
			QDir dir(_dirpath);
			QString dirpath = utils->getPath(_dirpath);
		
			QStringList nameFilter;
			nameFilter << "*.ico";
			
			// iterate all files in the path
			QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
			for (int i = 0; i < files.count(); i++)
			{
				QString gameName = files[i].toLower().remove(".ico");

				if (mamegame->gamenameGameInfoMap.contains(gameName))
				{
					gameInfo = mamegame->gamenameGameInfoMap[gameName];

					if (iconQueue.contains(gameName) && gameInfo->icondata.isNull())
					{
						QFile icoFile(dirpath + gameName + ".ico");
						if (icoFile.open(QIODevice::ReadOnly))
						{
							gameInfo->icondata = icoFile.readAll();
							emit icoUpdated(gameName);
		//					win->log(QString("icoUpdated f: %1").arg(gameName));
						}
					}
				}

				if (restart)
					break;
			}

			if (restart)
				break;

			// iterate all files in the zip
			QuaZip zip(dirpath + "icons.zip");

			if(!zip.open(QuaZip::mdUnzip))
				continue;
			
			QuaZipFileInfo info;
			QuaZipFile zipFile(&zip);
			for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
			{
				if(!zip.getCurrentFileInfo(&info))
					continue;

				QString gameName = info.name.toLower().remove(".ico");

				if (mamegame->gamenameGameInfoMap.contains(gameName))
				{
					gameInfo = mamegame->gamenameGameInfoMap[gameName];
					// fixme: read cloneof first
					if (iconQueue.contains(gameName) &&
						gameInfo->icondata.isNull())
					{
						QuaZipFile icoFile(&zip);
						if (icoFile.open(QIODevice::ReadOnly))
						{
							gameInfo->icondata = icoFile.readAll();
							emit icoUpdated(gameName);
		//					win->log(QString("icoUpdated z: %1").arg(gameName));
						}
					}
				}
				if (restart)
					break;
			}
			if (restart)
				break;
		}
///*
		// get clone icons from parent
		mutex.lock();
		for (int i = 0; i < iconQueue.count(); i++)
		{
			QString gameName = iconQueue.value(i);

			if (mamegame->gamenameGameInfoMap.contains(gameName))
			{
				gameInfo = mamegame->gamenameGameInfoMap[gameName];
				if (!gameInfo->isExtRom && gameInfo->icondata.isNull() && !gameInfo->cloneof.isEmpty())
				{
					gameInfo2 = mamegame->gamenameGameInfoMap[gameInfo->cloneof];
					if (!gameInfo2->icondata.isNull())
					{
						gameInfo->icondata = gameInfo2->icondata;
						emit icoUpdated(gameName);
		//				win->log(QString("icoUpdated c: %1").arg(gameName));
					}
				}
			}
//			else
//				win->log(QString("errico: %1 %2").arg(gameName).arg(mamegame->gamenameGameInfoMap.count()));
		}
		mutex.unlock();
//*/
		if (!restart)
			done = true;

		restart = false;
	}
}

UpdateSelectionThread::UpdateSelectionThread(QObject *parent)
: QThread(parent)
{
	abort = false;
}

UpdateSelectionThread::~UpdateSelectionThread()
{
	abort = true;
	wait();
}

void UpdateSelectionThread::update()
{
	QMutexLocker locker(&mutex);

	if (!isRunning())
		start(NormalPriority);
}

void UpdateSelectionThread::run()
{
	while (!myqueue.isEmpty() && !abort)
	{
		QString gameName = myqueue.dequeue();

		//fixme: do not update tabbed docks

		if (win->ssSnap->isVisible())
		{
			pmdataSnap = utils->getScreenshot(mameOpts["snapshot_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_SNAP);
		}
		if (win->ssFlyer->isVisible())
		{
			pmdataFlyer = utils->getScreenshot(mameOpts["flyer_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_FLYER);
		}
		if (win->ssCabinet->isVisible())
		{
			pmdataCabinet = utils->getScreenshot(mameOpts["cabinet_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_CABINET);
		}
		if (win->ssMarquee->isVisible())			
		{
			pmdataMarquee = utils->getScreenshot(mameOpts["marquee_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_MARQUEE);
		}
		if (win->ssTitle->isVisible())
		{
			pmdataTitle = utils->getScreenshot(mameOpts["title_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_TITLE);
		}
		if (win->ssCPanel->isVisible())
		{
			pmdataCPanel = utils->getScreenshot(mameOpts["cpanel_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_CPANEL);
		}
		if (win->ssPCB->isVisible())
		{
			pmdataPCB = utils->getScreenshot(mameOpts["pcb_directory"]->globalvalue, gameName);
			emit snapUpdated(DOCK_PCB);
		}
//		static QMovie movie( "xxx.mng" );
//		win->lblPCB->setMovie( &movie );

		if (win->tbHistory->isVisible())
		{
			historyText = utils->getHistory(gameName, "history.dat");
			emit snapUpdated(DOCK_HISTORY);
		}
		if (win->tbMameinfo->isVisible())
		{
			mameinfoText = utils->getHistory(gameName, "mameinfo.dat");
			emit snapUpdated(DOCK_MAMEINFO);
		}
		if (win->tbStory->isVisible())
		{
			storyText = utils->getHistory(gameName, "story.dat");
			emit snapUpdated(DOCK_STORY);
		}
	}
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
	return childItems[row];
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
	return itemData[column];
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

TreeModel::TreeModel(QObject *parent, bool isGroup)
: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;

	foreach (QString header, columnList)
		rootData << header;

	rootItem = new TreeItem(rootData);

	foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
	{
		if (gameName.trimmed()=="")
		win->log("ERR1");
		GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];

		// build parent
		if (!isGroup || gameInfo->cloneof.isEmpty())
		{
			TreeItem *parent = buildItem(rootItem, gameName, isGroup);

			// build clones
			if (isGroup)
			foreach (QString cloneName, gameInfo->clones)
				buildItem(parent, cloneName, isGroup);
		}
	}
}

TreeItem * TreeModel::buildItem(TreeItem *parent, QString gameName, bool isGroup)
{
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];

	if (gameName.trimmed()=="")
		win->log("ERR2");

	QList<QVariant> columnData;
	columnData << gameInfo->description;
	columnData << gameName;
	columnData << gameInfo->available;
	columnData << gameInfo->manufacturer;
	columnData << gameInfo->sourcefile;
	columnData << gameInfo->year;
	columnData << gameInfo->cloneof;

	// Append a new item to the current parent's list of children
//	if (gameInfo->pModItem)
//		delete gameInfo->pModItem;
	gameInfo->pModItem = new TreeItem(columnData, parent);
	parent->appendChild(gameInfo->pModItem);
	return gameInfo->pModItem;
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
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextColorRole)
	{
		return qVariantFromValue(QColor(Qt::black));
	}

	TreeItem *item = getItem(index);
	QString gameName = item->data(1).toString();
	if (gameName.trimmed()=="")
		win->poplog("ERR3");

	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
	
	if (role == Qt::DecorationRole && index.column() == COL_DESC)
	{
		QByteArray icondata;
		if (gameInfo->icondata.isNull())
		{
			gamelist->iconThread.iconQueue.setSize(win->tvGameList->viewport()->height() / 17 + 2);
//			gamelist->iconThread.iconQueue.setSize(64);
			gamelist->iconThread.iconQueue.enqueue(gameName);
			gamelist->iconThread.load();
			icondata = utils->deficondata;
		}
		else
			icondata = gameInfo->icondata;
		
		QPixmap pm;
		pm.loadFromData(icondata, "ico");
		return QIcon(pm);
	}
	
	if (role == Qt::UserRole + FOLDER_BIOS)
		return gameInfo->biosof();

	if (role == Qt::UserRole + FOLDER_CONSOLE)
		return gameInfo->isExtRom ? true : false;

	//convert 'Name' column for Console
	if (index.column() == COL_NAME && gameInfo->isExtRom)
	{
		if (role == Qt::UserRole)
			return item->data(index.column());
	}

	if (role != Qt::DisplayRole)
		return QVariant();
	if (index.column() == COL_NAME && gameInfo->isExtRom)
	{
		return gameInfo->romof;
	}

	//convert 'ROMs' column
	if (index.column() == COL_ROM)
	{
		int s = item->data(COL_ROM).toInt();
		if (s == -1)
			return "";
		if (s == 0)
			return "No";
		if (s == 1)
			return "Yes";
	}

	return item->data(index.column());
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem * TreeModel::getItem(const QModelIndex &index) const
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

void TreeModel::rowChanged(const QModelIndex &index)
{

	QModelIndex i = index.sibling(index.row(), 0);
	QModelIndex j = index.sibling(index.row(), columnCount() - 1);

	emit dataChanged(i, j);
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
			if(!value.isNull())
			{
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


RomInfo::RomInfo(QObject *parent)
: QObject(parent)
{
	available = false;
	//	win->log("# RomInfo()");
}

RomInfo::~RomInfo()
{
	//    win->log("# ~RomInfo()");
}

BiosInfo::BiosInfo(QObject *parent)
: QObject(parent)
{
	//	win->log("# BiosInfo()");
}

DeviceInfo::DeviceInfo(QObject *parent)
: QObject(parent)
{
	//	win->log("# DeviceInfo()");
}

GameInfo::GameInfo(QObject *parent)
: QObject(parent)
{
	available = 0;
	//	win->log("# GameInfo()");
}

GameInfo::~GameInfo()
{
	available = -1;
//	win->log("# ~GameInfo()");
}

QString GameInfo::biosof()
{
	QString biosof;
	GameInfo *gameInfo = NULL;

	if (!romof.isEmpty())
	{
		biosof = romof;
			if (romof.trimmed()=="")
		win->log("ERR4");
		gameInfo = mamegame->gamenameGameInfoMap[romof];

		if (!gameInfo->romof.isEmpty())
		{
			biosof = gameInfo->romof;
				if (gameInfo->romof.trimmed()=="")
		win->log("ERR5");
			gameInfo = mamegame->gamenameGameInfoMap[gameInfo->romof];
		}
	}
	
	if (gameInfo && gameInfo->isBios)
		return biosof;
	else
		return NULL;
}

MameGame::MameGame(QObject *parent)
: QObject(parent)
{
	win->log("# MameGame()");

	this->mameVersion = mameVersion;
}

MameGame::~MameGame()
{
	win->log("# ~MameGame()");
}

void MameGame::s11n()
{
	win->log("start s11n()");

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_3);
	out << mamegame->mameVersion;
	out << mamegame->mameDefaultIni;	//default.ini
	out << mamegame->gamenameGameInfoMap.count();

	win->log(QString("xxx1 %1").arg(mamegame->gamenameGameInfoMap.count()));

	//fixme: should place in thread and use mamegame directly
	gamelist->switchProgress(gamelist->numTotalGames, tr("Saving listxml"));
	int i = 0;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gamelist->updateProgress(i++);
		qApp->processEvents();
	
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gamename];
		out << gamename;
		out << gameinfo->description;
		out << gameinfo->year;
		out << gameinfo->manufacturer;
		out << gameinfo->sourcefile;
		out << gameinfo->isBios;
		out << gameinfo->isExtRom;
		out << gameinfo->cloneof;
		out << gameinfo->romof;
		out << gameinfo->available;

		out << gameinfo->clones.count();
		foreach (QString clonename, gameinfo->clones)
			out << clonename;

		out << gameinfo->crcRomInfoMap.count();
		foreach (quint32 crc, gameinfo->crcRomInfoMap.keys())
		{
			RomInfo *rominfo = gameinfo->crcRomInfoMap[crc];
			out << crc;
			out << rominfo->name;
			out << rominfo->status;
			out << rominfo->size;
		}

		out << gameinfo->nameBiosInfoMap.count();
		foreach (QString name, gameinfo->nameBiosInfoMap.keys())
		{
			BiosInfo *biosinfo = gameinfo->nameBiosInfoMap[name];
			out << name;
			out << biosinfo->description;
			out << biosinfo->isdefault;
		}

		out << gameinfo->nameDeviceInfoMap.count();
		foreach (QString name, gameinfo->nameDeviceInfoMap.keys())
		{
			DeviceInfo *deviceinfo = gameinfo->nameDeviceInfoMap[name];
			out << name;
			out << deviceinfo->mandatory;
			out << deviceinfo->extension;
		}
	}
	gamelist->switchProgress(-1, "");
	
	guiSettings.setValue("list_cache", qCompress(ba, 9));
}

int MameGame::des11n()
{
	QByteArray ba = qUncompress(guiSettings.value("list_cache").toByteArray());
	QDataStream in(ba);

	// Read and check the header
	quint32 mamepSig;
	in >> mamepSig;
	if (mamepSig != MAMEPLUS_SIG)
	{
		win->log("sig err");
		return QDataStream::ReadCorruptData;
	}

	// Read the version
	qint16 version;
	in >> version;
	if (version != S11N_VER)
	{
		win->log("ver err");
		return QDataStream::ReadCorruptData;
	}

	if (version < 1)
		in.setVersion(QDataStream::Qt_4_2);
	else
		in.setVersion(QDataStream::Qt_4_3);

	//finished checking
	if (mamegame)
		delete mamegame;
	mamegame = new MameGame(win);

	// MAME Version
	mamegame->mameVersion = utils->getMameVersion();
	QString mameVersion0;
	in >> mameVersion0;

	// default mame.ini text
	in >> mamegame->mameDefaultIni;
	win->log(QString("mamegame->mameDefaultIni.count %1").arg(mamegame->mameDefaultIni.count()));
	
	int gamecount;
	in >> gamecount;

	for (int i = 0; i < gamecount; i++)
	{
		GameInfo *gameinfo = new GameInfo(mamegame);
		QString gamename;
		in >> gamename;
		in >> gameinfo->description;
		in >> gameinfo->year;
		in >> gameinfo->manufacturer;
		in >> gameinfo->sourcefile;
		in >> gameinfo->isBios;
		in >> gameinfo->isExtRom;
		in >> gameinfo->cloneof;
		in >> gameinfo->romof;
		in >> gameinfo->available;
		mamegame->gamenameGameInfoMap[gamename] = gameinfo;

		int count;
		QString clone;
		in >> count;
		for (int j = 0; j < count; j++)
		{
			in >> clone;
			gameinfo->clones.insert(clone);
		}

		quint32 crc;
		in >> count;
		for (int j = 0; j < count; j++)
		{
			RomInfo *rominfo = new RomInfo(gameinfo);
			in >> crc;
			in >> rominfo->name;
			in >> rominfo->status;
			in >> rominfo->size;
			gameinfo->crcRomInfoMap[crc] = rominfo;
		}

		QString name;
		in >> count;
		for (int j = 0; j < count; j++)
		{
			BiosInfo *biosinfo = new BiosInfo(gameinfo);
			in >> name;
			in >> biosinfo->description;
			in >> biosinfo->isdefault;
			gameinfo->nameBiosInfoMap[name] = biosinfo;
		}

		in >> count;
		for (int j = 0; j < count; j++)
		{
			DeviceInfo *deviceinfo = new DeviceInfo(gameinfo);
			in >> name;
			in >> deviceinfo->mandatory;
			in >> deviceinfo->extension;
			gameinfo->nameDeviceInfoMap[name] = deviceinfo;
		}
	}

	win->log(QString("des11n2.gamecount %1").arg(gamecount));

	// verify MAME Version
	if (mamegame->mameVersion != mameVersion0)
	{
		win-> log(QString("new mame version: %1 / %2").arg(mameVersion0).arg(mamegame->mameVersion));
		mamegame0 = mamegame;
		return QDataStream::ReadCorruptData;
	}

	return in.status();
}


GamelistDelegate::GamelistDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize GamelistDelegate::sizeHint (const QStyleOptionViewItem & option, 
								  const QModelIndex & index) const
{
	QString gameName = gamelist->getViewString(index, COL_NAME);
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
//fixme: combine @ console gamename
	if (!gameInfo->nameDeviceInfoMap.isEmpty())
	{
		QString gameName2 = gamelist->getViewString(index, COL_NAME + COL_LAST);

		if (!gameName2.isEmpty())
		{
			gameInfo = mamegame->gamenameGameInfoMap[gameName2];
			if (gameInfo && gameInfo->isExtRom)
				gameName = gameName2;
		}
	}

	if (currentGame == gameName)
		return QSize(1,33);
	else
		return QSize(1,17);
}

void GamelistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							 const QModelIndex &index ) const
{
	QString gameName = gamelist->getViewString(index, COL_NAME);
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
//fixme: combine @ console gamename
	if (!gameInfo->nameDeviceInfoMap.isEmpty())
	{
		QString gameName2 = gamelist->getViewString(index, COL_NAME + COL_LAST);
		if (!gameName2.isEmpty())
		{
			gameInfo = mamegame->gamenameGameInfoMap[gameName2];
			if (gameInfo && gameInfo->isExtRom)
				gameName = gameName2;
		}
	}
	
	QString gameDesc = gamelist->getViewString(index, COL_DESC);
	
	if (currentGame == gameName && index.column() == COL_DESC)
	{
		//draw big icon
		QRect rc = option.rect;
		QPoint p = rc.topLeft();
		p.setX(p.x() + 2);
		rc = QRect(p, rc.bottomRight());

		if (gameName.trimmed()=="")
			win->log("ERRb");

		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];

		QByteArray icondata;
		if (gameinfo->icondata.isNull())
		{
			icondata = utils->deficondata;
//			win->log("paint def ico: " + gameName);
		}
		else
		{
			icondata = gameinfo->icondata;
//			win->log("paint game ico: " + gameName);
		}
		
		QPixmap pm;
		pm.loadFromData(icondata, "ico");

		QApplication::style()->drawItemPixmap (painter, rc, Qt::AlignLeft | Qt::AlignVCenter, pm);

		// calc text rect
		p = rc.topLeft();
		p.setX(p.x() + 34);
		rc = QRect(p, rc.bottomRight());

		//draw text bg
		if (option.state & QStyle::State_Selected)
			painter->fillRect(rc, option.palette.highlight());

		//draw text
		p = rc.topLeft();
		p.setX(p.x() + 2);
		rc = QRect(p, rc.bottomRight());
		QApplication::style()->drawItemText(painter, rc, Qt::AlignLeft | Qt::AlignVCenter, option.palette, true, 
											gameDesc, QPalette::HighlightedText);
		return;
	}

	QItemDelegate::paint(painter, option, index);
	return;
}


Gamelist::Gamelist(QObject *parent)
: QObject(parent)
{
	inited = false;

	connect(&iconThread, SIGNAL(icoUpdated(QString)), this, SLOT(setupIcon(QString)));
	connect(&selectThread, SIGNAL(snapUpdated(int)), this, SLOT(setupSnap(int)));
	
	if (!searchTimer)
		searchTimer = new QTimer(this);
	connect(searchTimer, SIGNAL(timeout()), this, SLOT(filterRegExpChanged()));

	mAuditor = new MergedRomAuditor(parent);

	numTotalGames = -1;
	loadProc = NULL;
}

Gamelist::~Gamelist()
{
//	win->log("DEBUG: Gamelist::~Gamelist()");

	if ( loadProc )
		loadProc->terminate();
}

void Gamelist::updateProgress(int progress)
{
	win->progressBarGamelist->setValue(progress);
}

void Gamelist::switchProgress(int max, QString title)
{
	win->logStatus(title);

	if (max != -1)
	{
		win->statusbar->addWidget(win->progressBarGamelist);
		win->progressBarGamelist->setRange(0, max);
		win->progressBarGamelist->reset();
		win->progressBarGamelist->show();
	}
	else
	{
		win->statusbar->removeWidget(win->progressBarGamelist);
	}
}

QString Gamelist::getViewString(const QModelIndex &index, int column) const
{
	bool isUserValue = false;
	if (column >= COL_LAST)
	{
		column -= COL_LAST;
		isUserValue = true;
	}

	QModelIndex j = index.sibling(index.row(), column);
	//fixme: sometime model's NULL...
	if (!index.model())
		return "";

	if (isUserValue)
		return index.model()->data(j, Qt::UserRole).toString();
	else
		return index.model()->data(j, Qt::DisplayRole).toString();
}

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		QString gameName = getViewString(current, COL_NAME);
		if (gameName.isEmpty())
			return;

		GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
			if (gameName.trimmed()=="")
		win->log("ERRc");
		QString gameDesc = gameInfo->description;
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
		{
			QString gameName2 = getViewString(current, COL_NAME + COL_LAST);
			if (!gameName2.isEmpty())
			{
					if (gameName2.trimmed()=="")
				win->log("ERRd");

				gameInfo = mamegame->gamenameGameInfoMap[gameName2];
				if (gameInfo && gameInfo->isExtRom)
				{
					gameName = gameName2;
					gameDesc = gameInfo->description;
				}
			}
		}

		currentGame = gameName;
		
		//update statusbar
		win->logStatus(gameDesc);
		win->log(QString("%1").arg(currentGame));
/*
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
		{
			DeviceInfo *deviceinfo = gameInfo->nameDeviceInfoMap["cartridge"];
			if (deviceinfo)
				win->log(deviceinfo->extension.join(", "));
		}
*/
		selectThread.myqueue.enqueue(currentGame);
		selectThread.update();

		//update selected rows
		gameListModel->rowChanged(gameListPModel->mapToSource(current));
		gameListModel->rowChanged(gameListPModel->mapToSource(previous));
	}
	else
		currentGame = "";
}

void Gamelist::restoreSelection()
{
	//fixme: hack?
	if (!gameListModel || !gameListPModel)
		return;

	QString gameName = currentGame;
	QModelIndex i;

	// select current game
//	win->log("restore callback: " + gameName);
	if (mamegame->gamenameGameInfoMap.contains(gameName))
	{
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
		i = gameListModel->index(0, gameinfo->pModItem);
//		win->log("R: " + gameName);
	}

	QModelIndex pi = gameListPModel->mapFromSource(i);

	if (!pi.isValid())
	{
		// select first row otherwise
		pi = gameListPModel->index(0, 0, QModelIndex());
//		win->log("R: 0");
	}

	win->tvGameList->setCurrentIndex(pi);
	win->lvGameList->setCurrentIndex(pi);
	//fixme: PositionAtCenter doesnt work well (auto scroll right)
	win->tvGameList->scrollTo(pi, QAbstractItemView::PositionAtTop);
	win->lvGameList->scrollTo(pi, QAbstractItemView::PositionAtTop);

	win->labelGameCount->setText(QString("%1 Games").arg(gameListPModel->rowCount()));
}

void Gamelist::setupIcon(QString gameName)
{
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];

	gameListModel->setData(gameListModel->index(0, gameInfo->pModItem), 
		gameInfo->icondata, Qt::DecorationRole);

//	win->log(QString("setupico %1 %2").arg(gameName).arg(gameInfo->icondata.isNull()));
}

void Gamelist::setupSnap(int snapType)
{
	switch (snapType)
	{
	case DOCK_SNAP:
		win->ssSnap->setPixmap(selectThread.pmdataSnap);
		break;
	case DOCK_FLYER:
		win->ssFlyer->setPixmap(selectThread.pmdataFlyer);
		break;
	case DOCK_CABINET:
		win->ssCabinet->setPixmap(selectThread.pmdataCabinet);
		break;
	case DOCK_MARQUEE:
		win->ssMarquee->setPixmap(selectThread.pmdataMarquee);
		break;
	case DOCK_TITLE:
		win->ssTitle->setPixmap(selectThread.pmdataTitle);
		break;
	case DOCK_CPANEL:
		win->ssCPanel->setPixmap(selectThread.pmdataCPanel);
		break;
	case DOCK_PCB:
		win->ssPCB->setPixmap(selectThread.pmdataPCB);
		break;

	case DOCK_HISTORY:
		win->tbHistory->setText(selectThread.historyText);
		break;
	case DOCK_MAMEINFO:
		win->tbMameinfo->setText(selectThread.mameinfoText);
		break;
	case DOCK_STORY:
		win->tbStory->setText(selectThread.storyText);
	}
}

void Gamelist::init(bool toggleState)
{
	if (!toggleState)
		return;

	// get current game list mode
	if (win->actionDetails->isChecked())
		list_mode = win->actionDetails->objectName().remove("action");
	else if (win->actionLargeIcons->isChecked())
		list_mode = win->actionLargeIcons->objectName().remove("action");
	else
		list_mode = win->actionGrouped->objectName().remove("action");

	int des11n_status = QDataStream::Ok;

	// only des11n on app start
	//fixme: illogical call before mamegame init
	if (!mamegame)
		des11n_status = mamegame->des11n();

//	win->log(QString("des11n status: %1").arg(des11n_status));

	if (des11n_status == QDataStream::Ok)
	{
		// disable ctrl updating before deleting its model
		win->lvGameList->hide();
		win->layMainView->removeWidget(win->lvGameList);
		win->tvGameList->hide();
		win->layMainView->removeWidget(win->tvGameList);

		if (gameListModel)
			delete gameListModel;

		// Grouped
		if (list_mode == win->actionGrouped->objectName().remove("action"))
		{
			gameListModel = new TreeModel(win, true);

			win->layMainView->addWidget(win->tvGameList);		
//			win->tvGameList->show();
		}
		// Large Icons
		else if (list_mode == win->actionLargeIcons->objectName().remove("action"))
		{
			gameListModel = new TreeModel(win, false);

			win->layMainView->addWidget(win->lvGameList);		
//			win->lvGameList->show();
		}
		// Details
		else
		{
			gameListModel = new TreeModel(win, false);

			win->layMainView->addWidget(win->tvGameList);		
//			win->tvGameList->show();
		}

		if (gameListPModel)
			delete gameListPModel;
		gameListPModel = new GameListSortFilterProxyModel(win);

		gameListPModel->setSourceModel(gameListModel);
		gameListPModel->setSortCaseSensitivity(Qt::CaseInsensitive);

		if (list_mode == win->actionLargeIcons->text())
			win->lvGameList->setModel(gameListPModel);
		else
		{
			win->tvGameList->setModel(gameListPModel);
			win->tvGameList->setItemDelegate(&gamelistDelegate);
		}

		if (!inited)
		{
			/* init everything else here after we have mamegame */
			// init folders
			gamelist->initFolders();

			// init options from default mame.ini
			optUtils->loadDefault(mamegame->mameDefaultIni);
			// assign option type, defvalue, min, max, etc. from template
			optUtils->loadTemplate();
			// load mame.ini overrides
			optUtils->loadIni(OPTLEVEL_GLOBAL, "mame.ini");
			
			// add GUI MESS extra software paths
			foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
			{
				GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
				if (!gameInfo->nameDeviceInfoMap.isEmpty())
				{
					MameOption *pMameOpt = new MameOption(0);	//fixme parent
					pMameOpt->guivisible = true;
					mameOpts[gameName + "_extra_software"] = pMameOpt;
					//save a list of console system for later use
					consoleGamesL << gameName;
				}
			}
			consoleGamesL.sort();

			// load GUI path overrides
			foreach (QString optName, mameOpts.keys())
			{
				MameOption *pMameOpt = mameOpts[optName];
			
				if (!pMameOpt->guivisible)
					continue;

				if (guiSettings.contains(optName))
					pMameOpt->globalvalue = guiSettings.value(optName).toString();
			}

			// misc win init
			win->setWindowTitle(win->windowTitle() + " - " + mamegame->mameVersion);
			win->actionDefaultOptions->setEnabled(true);

			connect(win->tvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));
			connect(win->lvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));
		}

		// connect gameListModel/gameListPModel signals after the view init completed
		// connect gameListModel/gameListPModel signals after mameOpts init
		connect(gameListPModel, SIGNAL(layoutChanged()), this, SLOT(restoreSelection()));
		connect(win->tvGameList->selectionModel(),
				SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
				this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
		connect(win->lvGameList->selectionModel(),
				SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
				this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
		
		// restore view column state
		if (guiSettings.value("column_state").isValid())
			win->tvGameList->header()->restoreState(guiSettings.value("column_state").toByteArray());
		else
			win->tvGameList->header()->restoreState(defSettings.value("column_state").toByteArray());
		
		win->log("sorting");
		
		win->tvGameList->setSortingEnabled(true);
		win->tvGameList->sortByColumn(guiSettings.value("sort_column").toInt(), 
			(guiSettings.value("sort_reverse").toInt() == 0) ? Qt::AscendingOrder : Qt::DescendingOrder);
		win->tvGameList->setFocus();

		//refresh current list
		filterRegExpChanged2(win->treeFolders->currentItem());

		inited = true;

		// Grouped
		if (list_mode == win->actionGrouped->text())
			win->tvGameList->show();
		// Large Icons
		else if (list_mode == win->actionLargeIcons->text())
			win->lvGameList->show();
		// Details
		else
			win->tvGameList->show();

		win->treeFolders->setEnabled(true);
		win->actionRefresh->setEnabled(true);

		win->log(QString("inited.gamecount %1").arg(mamegame->gamenameGameInfoMap.count()));
	}
	else
	{
		mameOutputBuf = "";
		QString command = guiSettings.value("mame_binary", "mamep.exe").toString();
		QStringList args;

		args << "-listxml";
		loadTimer.start();
		loadProc = procMan->process(procMan->start(command, args, FALSE));

		connect(loadProc, SIGNAL(started()), this, SLOT(loadListXmlStarted()));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadListXmlReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadListXmlFinished(int, QProcess::ExitStatus)));
	}
}

void Gamelist::loadListXmlStarted()
{
	win->log("DEBUG: Gamelist::loadListXmlStarted()");
}

void Gamelist::loadListXmlReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString buf = proc->readAllStandardOutput();
	
	//mamep: remove windows endl
	buf.replace(QString("\r"), QString(""));
	
	numTotalGames += buf.count("<game name=");
	mameOutputBuf += buf;

	win->logStatus(QString(tr("Loading listxml: %1 games")).arg(numTotalGames));
}

void Gamelist::loadListXmlFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();
	QTime elapsedTime;
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	parse();

	//chain
	loadDefaultIni();
	win->log("end of gamelist->loadFin()");
}

void Gamelist::loadDefaultIni()
{
	mamegame->mameDefaultIni = "";
	QString command = guiSettings.value("mame_binary", "mamep.exe").toString();
	QStringList args;
	args << "-showconfig" << "-norc";

	loadProc = procMan->process(procMan->start(command, args, FALSE));
	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadDefaultIniReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadDefaultIniFinished(int, QProcess::ExitStatus)));
}

void Gamelist::loadDefaultIniReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString buf = proc->readAllStandardOutput();
	
	mamegame->mameDefaultIni += buf;
}

void Gamelist::loadDefaultIniFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	optUtils->loadDefault(mamegame->mameDefaultIni);
	optUtils->loadTemplate();

//	mamegame->s11n();
	
	//reload gamelist
	init(true);
	win->log("end of gamelist->loadDefFin()");
}

// extract a rom from the merged file
void Gamelist::extractMerged(QString mergedFileName, QString fileName)
{
	QString command = "bin/7z";
	QStringList args;
	args << "e" << "-y" << mergedFileName << fileName <<"-o" + QDir::tempPath();
	currentTempROM = QDir::tempPath() + "/" + fileName;

	loadProc = procMan->process(procMan->start(command, args, FALSE));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(extractMergedFinished(int, QProcess::ExitStatus)));
}

// call the emulator after the rom has been extracted
void Gamelist::extractMergedFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	runMame(true);
}

// delete the extracted rom
void Gamelist::runMergedFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	QFile file(currentTempROM);
	file.remove();
	currentTempROM.clear();
}

void Gamelist::parse()
{
	win->log("DEBUG: Gamelist::prep parse()");

	ListXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QXmlInputSource *pxmlInputSource = new QXmlInputSource();
	pxmlInputSource->setData (mameOutputBuf);

	win->log("DEBUG: Gamelist::start parse()");
	
	switchProgress(numTotalGames, tr("Parsing listxml"));
	reader.parse(*pxmlInputSource);
	switchProgress(-1, "");

	GameInfo *gameInfo, *gameInfo0, *gameInfo2;
	foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
	{
		gameInfo = mamegame->gamenameGameInfoMap[gameName];

		// restore previous audit results
		if (mamegame0 && mamegame0->gamenameGameInfoMap.contains(gameName))
		{
			gameInfo0 = mamegame0->gamenameGameInfoMap[gameName];
			gameInfo->available = gameInfo0->available;
		}

		// update clone list
		if (!gameInfo->cloneof.isEmpty())
		{
			gameInfo2 = mamegame->gamenameGameInfoMap[gameInfo->cloneof];
			gameInfo2->clones.insert(gameName);
		}
	}

	// restore previous console audit
	if (mamegame0)
	{
		foreach (QString gameName, mamegame0->gamenameGameInfoMap.keys())
		{
			gameInfo0 = mamegame0->gamenameGameInfoMap[gameName];

			if (gameInfo0->isExtRom)
			{
				gameInfo = new GameInfo(mamegame);
				gameInfo->description = gameInfo0->description;
				gameInfo->isBios = false;
				gameInfo->isExtRom = true;
				gameInfo->romof = gameInfo0->romof;
				gameInfo->sourcefile = gameInfo0->sourcefile;
				gameInfo->available = 1;
				mamegame->gamenameGameInfoMap[gameName] = gameInfo;
			}
		}

		delete mamegame0;
	}

	delete pxmlInputSource;
	mameOutputBuf.clear();
}

void Gamelist::filterTimer()
{
	searchTimer->start(200);
	searchTimer->setSingleShot(true);
}

void Gamelist::filterRegExpChanged()
{
	// multiple space-separated keywords
	QString text = win->lineEditSearch->text();
	// do not search less than 2 Latin chars
	if (text.count() == 1 && text.at(0).unicode() < 0x3000 /* CJK symbols start */)
		return;

	text.replace(utils->spaceRegex, "*");

	//fixme: doesnt use filterregexp
	static QRegExp regExp("");
	// set it for a callback to refresh the list
	gameListPModel->searchText = text;	// must set before regExp
	gameListPModel->setFilterRegExp(regExp);
	win->tvGameList->expandAll();
	restoreSelection();
}

void Gamelist::filterRegExpChanged2(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	QString filterText;

	// the selection could be empty
	if (!current)
		return;

	currentFolder= "";
	if (win->treeFolders->currentItem()->parent())
		currentFolder += win->treeFolders->currentItem()->parent()->text(0);
	currentFolder += "/" + win->treeFolders->currentItem()->text(0);

	win->actionRefresh->setText(tr("Refresh") + " " + currentFolder);

	if (!current->parent())
	{
		QString folderName = current->text(0);
		filterText = "";

		if (folderName == folderList[FOLDER_ALLGAME])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAME);
		else if (folderName == folderList[FOLDER_ALLARC])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLARC);
		else if (folderName == folderList[FOLDER_AVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_AVAILABLE);
		else if (folderName == folderList[FOLDER_UNAVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_UNAVAILABLE);
		else if (folderName == folderList[FOLDER_CONSOLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONSOLE);
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAME);
	}
	else
	{
		QString folderName = current->parent()->text(0);
		filterText = current->text(0);

		if (folderName == folderList[FOLDER_CONSOLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS);	//hack for console subfolders
		else if (folderName == folderList[FOLDER_MANUFACTURER])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_MANUFACTURER);
		else if (folderName == folderList[FOLDER_YEAR])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_YEAR);
		else if (folderName == folderList[FOLDER_SOURCE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SOURCE);
		else if (folderName == folderList[FOLDER_BIOS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS);
		//fixme
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAME);
	}

	static QRegExp regExp("");
	// set it for a callback to refresh the list
	gameListPModel->filterText = filterText;	// must set before regExp
	gameListPModel->setFilterRegExp(regExp);

	//fixme: must have this, otherwise the list cannot be expanded properly
	qApp->processEvents();
	win->tvGameList->expandAll();
	restoreSelection();
}

void Gamelist::initFolders()
{
	folderList
		<< QT_TR_NOOP("All Games")
		<< QT_TR_NOOP("All Arcades")
		<< QT_TR_NOOP("Available Arcades")
		<< QT_TR_NOOP("Unavailable Arcades")
		<< QT_TR_NOOP("Consoles")
		<< QT_TR_NOOP("Manufacturer")
		<< QT_TR_NOOP("Year")
		<< QT_TR_NOOP("Driver")
		<< QT_TR_NOOP("BIOS")
		/*
		<< QT_TR_NOOP("CPU")
		<< QT_TR_NOOP("Sound")
		<< QT_TR_NOOP("Orientation")
		<< QT_TR_NOOP("Emulation Status")
		<< QT_TR_NOOP("Dumping Status")
		<< QT_TR_NOOP("Working")
		<< QT_TR_NOOP("Not working")
		<< QT_TR_NOOP("Orignals")
		<< QT_TR_NOOP("Clones")
		<< QT_TR_NOOP("Raster")
		<< QT_TR_NOOP("Vector")
		<< QT_TR_NOOP("Resolution")
		<< QT_TR_NOOP("FPS")
		<< QT_TR_NOOP("Save State")
		<< QT_TR_NOOP("Control Type")
		<< QT_TR_NOOP("Stereo")
		<< QT_TR_NOOP("CHD")
		<< QT_TR_NOOP("Samples")
		<< QT_TR_NOOP("Artwork")*/
		;

	QStringList consoleList, mftrList, yearList, srcList, biosList;
	GameInfo *gameInfo;
	foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
	{
		gameInfo = mamegame->gamenameGameInfoMap[gameName];
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
			consoleList << gameName;
		if (gameInfo->isBios)
			biosList << gameName;
		if (!mftrList.contains(gameInfo->manufacturer))
			mftrList << gameInfo->manufacturer;
		if (!yearList.contains(gameInfo->year))
			yearList << gameInfo->year;
		if (!srcList.contains(gameInfo->sourcefile))
			srcList << gameInfo->sourcefile;
	}

	consoleList.sort();
	mftrList.sort();
	yearList.sort();
	srcList.sort();
	biosList.sort();
	
	QIcon icoFolder(":/res/32x32/folder.png");
	QList<QTreeWidgetItem *> items;
	for (int i = 0; i < folderList.size(); i++)
	{
		items.append(new QTreeWidgetItem(win->treeFolders, QStringList(folderList[i])));

		win->treeFolders->insertTopLevelItems(0, items);
		items[i]->setIcon(0, icoFolder);

		if (i == FOLDER_CONSOLE)
			foreach (QString name, consoleList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		else if (i == FOLDER_MANUFACTURER)
			foreach (QString name, mftrList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		else if (i == FOLDER_YEAR)
			foreach (QString name, yearList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		else if (i == FOLDER_SOURCE)
			foreach (QString name, srcList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		else if (i == FOLDER_BIOS)
			foreach (QString name, biosList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));
	}

	connect(win->treeFolders, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this, SLOT(filterRegExpChanged2(QTreeWidgetItem *, QTreeWidgetItem *)));

	// restore folder selection
	currentFolder = guiSettings.value("folder_current", "/" + folderList[0]).toString();
	int sep = currentFolder.indexOf("/");
	QString parentFolder = currentFolder.left(sep);
	QString subFolder = currentFolder.right(currentFolder.size() - sep - 1);

	if (parentFolder.isEmpty())
	{
		parentFolder = subFolder;
		subFolder = "";
	}

	QTreeWidgetItem *rootItem = win->treeFolders->invisibleRootItem();

	for (int i = 0; i < rootItem->childCount(); i++)
	{
		QTreeWidgetItem *subItem = rootItem->child(i);
		if (subItem->text(0) == parentFolder)
		{
		win->log(QString("tree lv1.gamecount %1").arg(mamegame->gamenameGameInfoMap.count()));
			if (subFolder.isEmpty())
			{
				win->treeFolders->setCurrentItem(subItem);
				return;
			}
			else
			{
				subItem->setExpanded(true);
				for (int j = 0; j < subItem->childCount(); j++)
				{
					QTreeWidgetItem *subsubItem = subItem->child(j);
					if (subsubItem->text(0) == subFolder)
					{
						win->treeFolders->setCurrentItem(subsubItem);
						win->log(QString("treeb.gamecount %1").arg(mamegame->gamenameGameInfoMap.count()));
						return;
					}
				}
			}
		}
	}
	//fallback
	win->treeFolders->setCurrentItem(rootItem->child(0));
}

void Gamelist::runMame(bool runMerged)
{
	//block multi mame session for now
	//if (procMan->procCount > 0)
	//	return;

	QString command = mameOpts["mame_binary"]->globalvalue;
	QStringList args;

	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[currentGame];

	// run console roms, add necessary params
	if (gameInfo->isExtRom)
	{
		// console system
		args << gameInfo->romof;
		// console device
		args << "-cart";

		QStringList paths = currentGame.split(".7z/");
		// run extracted rom
		if (runMerged)
		{
			// use temp rom name instead
			args << currentTempROM;
			loadProc = procMan->process(procMan->start(command, args, FALSE));
			// delete the extracted rom
			connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMergedFinished(int, QProcess::ExitStatus)));
			return;
		}

		// extract merged rom
		if (currentGame.contains(".7z/"))
		{
			extractMerged(paths[0] + ".7z", paths[1]);
			return;
		}
	}

	args << currentGame;
	procMan->process(procMan->start(command, args));
}


GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool GameListSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool result = true;
	bool isExtRom, isConsole;
	QModelIndex indexGameDesc, indexGameName, index2;
	const QAbstractItemModel *srcModel = sourceModel();

	indexGameDesc = srcModel->index(sourceRow, COL_DESC, sourceParent);
	indexGameName = srcModel->index(sourceRow, COL_NAME, sourceParent);
	
	GameInfo *gameInfo = NULL;
	QString gameName = srcModel->data(indexGameName).toString();
	if (gameName.trimmed() != "")
		gameInfo = mamegame->gamenameGameInfoMap[gameName];

	if (!searchText.isEmpty())
	{
//		win->log(QString("search: %1").arg(searchText));
	
		QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
		QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);

		// apply search filter
		result = (srcModel->data(indexGameDesc).toString().contains(regExpSearch)
				|| gameName.contains(regExpSearch));
	}

	isConsole = gameName == "sfzch" || gameInfo && !gameInfo->nameDeviceInfoMap.isEmpty();
	isExtRom = srcModel->data(indexGameDesc, Qt::UserRole + FOLDER_CONSOLE).toBool();

	// apply folder filter
	if (filterRole() == Qt::UserRole + FOLDER_ALLGAME)
	{
		return result;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_ALLARC)
	{
		return result && !isExtRom && !isConsole;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_AVAILABLE)
	{
		index2 = srcModel->index(sourceRow, COL_ROM, sourceParent);
		return result && srcModel->data(index2).toString() == "Yes"
						&& !isExtRom && !isConsole;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_UNAVAILABLE)
	{
		index2 = srcModel->index(sourceRow, COL_ROM, sourceParent);
		return result && srcModel->data(index2).toString() != "Yes"
						&& srcModel->data(indexGameDesc, Qt::UserRole + FOLDER_CONSOLE).toString() != "Yes"
						&& !isExtRom && !isConsole;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_CONSOLE)
	{
		return result && isExtRom;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS)	//hack for console subfolders
	{
		return result && srcModel->data(indexGameName).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_MANUFACTURER)
	{
		index2 = srcModel->index(sourceRow, COL_MFTR, sourceParent);
		return result && srcModel->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_YEAR)
	{
		index2 = srcModel->index(sourceRow, COL_YEAR, sourceParent);
		return result && srcModel->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_SOURCE)
	{
		index2 = srcModel->index(sourceRow, COL_SRC, sourceParent);
		return result && srcModel->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_BIOS)
	{
		return result && srcModel->data(indexGameDesc, Qt::UserRole + FOLDER_BIOS).toString() == filterText;
	}

	// empty list otherwise
	return false;
}

