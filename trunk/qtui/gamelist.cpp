#include "gamelist.h"

#include "mamepguimain.h"
#include "mameopt.h"
#include "ips.h"
#include "m1.h"

/* global */
MameGame *mameGame = NULL, *mameGame0 = NULL;
Gamelist *gameList = NULL;
QString currentGame, currentFolder;
QStringList consoleGamesL;

//fixme: used in audit
TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;

/* internal */
GamelistDelegate gamelistDelegate(0);
QSet<QString> visibleGames;
static const QString ROOT_FOLDER = "ROOT_FOLDER";
QByteArray defIconData;
QByteArray defSnapData;

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
	FOLDER_EXT,
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
	COL_DESC = 0,
	COL_NAME,
	COL_ROM,
	COL_MFTR,
	COL_SRC,
	COL_YEAR,
	COL_CLONEOF,
	COL_LAST
};

/* to support mameplus .mmo translation */
enum {
	UI_LANG_EN_US = 0,
	UI_LANG_ZH_CN,
	UI_LANG_ZH_TW,
	UI_LANG_FR_FR,
	UI_LANG_DE_DE,
	UI_LANG_IT_IT,
	UI_LANG_JA_JP,
	UI_LANG_KO_KR,
	UI_LANG_ES_ES,
	UI_LANG_CA_ES,
	UI_LANG_VA_ES,
	UI_LANG_PL_PL,
	UI_LANG_PT_PT,
	UI_LANG_PT_BR,
	UI_LANG_HU_HU,
	UI_LANG_MAX
};

enum {
	UI_MSG_MAME = 0,
	UI_MSG_LIST,
	UI_MSG_READINGS,
	UI_MSG_MANUFACTURE,
	UI_MSG_OSD0,
	UI_MSG_OSD1,
	UI_MSG_OSD2,
	UI_MSG_OSD3,
	UI_MSG_OSD4,
	UI_MSG_OSD5,
	UI_MSG_MAX = 31
};

class ListXMLHandler : public QXmlDefaultHandler
{
public:
	ListXMLHandler(int d = 0)
	{
		gameinfo = 0;
		metMameTag = false;

// fixme: cannot delete it when new mame version
//		if (mameGame)
//			delete mameGame;
		mameGame = new MameGame(win);
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
			mameGame->mameVersion = attributes.value("build");
		}
		else if (qName == "game")
		{
			//update progress
			static int i;
			gameList->updateProgress(i++);
			qApp->processEvents();
			
			gameinfo = new GameInfo(mameGame);
			currentDevice = "";
			gameinfo->sourcefile = attributes.value("sourcefile");
			gameinfo->isBios = attributes.value("isbios") == "yes";
			gameinfo->isExtRom = false;
			gameinfo->cloneof = attributes.value("cloneof");
			gameinfo->romof = attributes.value("romof");

			mameGame->nameInfoMap[attributes.value("name")] = gameinfo;
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
		else if (qName == "driver")
		{
			gameinfo->status = utils->getStatus(attributes.value("status"));
			gameinfo->emulation = utils->getStatus(attributes.value("emulation"));
			gameinfo->color = utils->getStatus(attributes.value("color"));
			gameinfo->sound = utils->getStatus(attributes.value("sound"));
			gameinfo->graphic = utils->getStatus(attributes.value("graphic"));
			gameinfo->cocktail = utils->getStatus(attributes.value("cocktail"));
			gameinfo->protection = utils->getStatus(attributes.value("protection"));
			gameinfo->savestate = utils->getStatus(attributes.value("savestate"));
			gameinfo->palettesize = attributes.value("palettesize").toUInt();
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

	bool endElement(const QString & /* namespaceURI */,
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

	bool characters(const QString &str)
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

UpdateSelectionThread::UpdateSelectionThread(QObject *parent)
: QThread(parent)
{
	abort = false;

	QFile icoFile(":/res/win_roms.ico");
	icoFile.open(QIODevice::ReadOnly);
	defIconData = icoFile.readAll();

	QFile snapFile(":/res/mamegui/mame.png");
	snapFile.open(QIODevice::ReadOnly);
	defSnapData = snapFile.readAll();
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
		start(IdlePriority);
}

void UpdateSelectionThread::run()
{
	while (!myqueue.isEmpty() && !abort)
	{
		QString gameName = myqueue.dequeue();

		for (int snapType = DOCK_SNAP; snapType <= DOCK_PCB; snapType ++)
		{
			if (!abort && win->dockCtrls[snapType]->isVisible() && win->isDockTabVisible(win->dockCtrlNames[snapType]))
		{
				pmSnapData[snapType] = getScreenshot(mameOpts[validGuiSettings[snapType]]->globalvalue, gameName, snapType);
				emit snapUpdated(snapType);
		}
		}
//		static QMovie movie( "xxx.mng" );
//		win->lblPCB->setMovie( &movie );

		QString path;
		if (!abort && win->tbHistory->isVisible() && win->isDockTabVisible("History"))
		{
			path = "history.dat";
			if (mameOpts.contains("history_file"))
				path = mameOpts["history_file"]->globalvalue;

			historyText = utils->getHistory(path, gameName, 1);

			emit snapUpdated(DOCK_HISTORY);
		}
		if (!abort && win->tbMameinfo->isVisible() && win->isDockTabVisible("MAMEInfo"))
		{
			path = "mameinfo.dat";
			if (mameOpts.contains("mameinfo_file"))
				path = mameOpts["mameinfo_file"]->globalvalue;
		
			mameinfoText = utils->getHistory(path, gameName);
			emit snapUpdated(DOCK_MAMEINFO);
		}
		if (!abort && win->tbStory->isVisible() && win->isDockTabVisible("Story"))
		{
			path = "story.dat";
			if (mameOpts.contains("story_file"))
				path = mameOpts["story_file"]->globalvalue;
		
			storyText = utils->getHistory(path, gameName);
			emit snapUpdated(DOCK_STORY);
		}
		if (!abort && win->tbCommand->isVisible() && win->isDockTabVisible("Command"))
		{
			path = "command.dat";
			if (mameOpts.contains("command_file"))
				path = mameOpts["command_file"]->globalvalue;

			commandText = utils->getHistory(path, gameName);

			// command.dat parsing
			commandText.replace(QRegExp("<br>\\s+"), "<br>");
			/* directions */
			//generate dup dirs
			commandText.replace("_2_1_4_1_2_3_6", "_2_1_4_4_1_2_3_6");
			commandText.replace("_2_3_6_3_2_1_4", "_2_3_6_6_3_2_1_4");
			commandText.replace("_4_1_2_3_6", "<img src=\":/res/16x16/dir-hcf.png\" />");
			commandText.replace("_6_3_2_1_4", "<img src=\":/res/16x16/dir-hcb.png\" />");
			commandText.replace("_2_3_6", "<img src=\":/res/16x16/dir-qdf.png\" />");
			commandText.replace("_2_1_4", "<img src=\":/res/16x16/dir-qdb.png\" />");
			commandText.replace(QRegExp("_(\\d)"), "<img src=\":/res/16x16/dir-\\1.png\" />");
			// buttons
			commandText.replace(QRegExp("_([A-DGKNPS\\+])"), "<img src=\":/res/16x16/btn-\\1.png\" />");
			commandText.replace(QRegExp("_([a-f])"), "<img src=\":/res/16x16/btn-n\\1.png\" />");
			//------
			commandText.replace(QRegExp("<br>[\\x2500-]{8,}<br>"), "<hr>");
			//special moves, starts with <br> || <hr>
			commandText.replace(QRegExp(">\\x2605"), "><img src=\":/res/16x16/star_gold.png\" />");
			commandText.replace(QRegExp(">\\x2606"), "><img src=\":/res/16x16/star_silver.png\" />");
			commandText.replace(QRegExp(">\\x25B2"), "><img src=\":/res/16x16/tri-r.png\" />");
			commandText.replace(QRegExp(">\\x25CB"), "><img src=\":/res/16x16/cir-y.png\" />");
			commandText.replace(QRegExp(">\\x25CE"), "><img src=\":/res/16x16/cir-r.png\" />");			
			commandText.replace(QRegExp(">\\x25CF"), "><img src=\":/res/16x16/cir-g.png\" />");
			commandText.replace(QChar(0x2192), "<img src=\":/res/16x16/blank.png\" /><img src=\":/res/16x16/arrow-r.png\" />");
//			commandText.replace(QChar(0x3000), "<img src=\":/res/16x16/blank.png\" />");

			/* colors
			Y: +45
			G: +120 0 -28
			B: -150 0 -20
			C: -32
			P: -90
			*/

			///*
			if (language.startsWith("zh_") || language.startsWith("ja_"))
			{
				QFont font;
				font.setFamily("MS Gothic");
				font.setFixedPitch(true);
				win->tbCommand->setFont(font);
//				win->tbCommand->setLineWrapMode(QTextEdit::NoWrap);
			}
//*/
			emit snapUpdated(DOCK_COMMAND);
		}
	}
}

QByteArray UpdateSelectionThread::getScreenshot(const QString &dirpath0, const QString &gameName, int)
{
	QStringList dirpaths = dirpath0.split(";");
	QByteArray snapdata = QByteArray();

	foreach (QString _dirpath, dirpaths)
	{
		QDir dir(_dirpath);
		QString dirpath = utils->getPath(_dirpath);

		// try to load directly	
		QFile snapFile(dirpath + gameName + ".png");
		if (snapFile.open(QIODevice::ReadOnly))
			snapdata = snapFile.readAll();

		// try to add .zip to nearest folder name
		if (snapdata.isNull())
		{
			QuaZip zip(dirpath + dir.dirName() + ".zip");
			if (zip.open(QuaZip::mdUnzip))
			{
				QuaZipFile zipfile(&zip);
				if (zip.setCurrentFile(gameName + ".png"))
				{
					if (zipfile.open(QIODevice::ReadOnly))
						snapdata = zipfile.readAll();
				}
			}
		}

		if (!snapdata.isNull())
			break;
	}

	// recursively load parent image
	if (snapdata.isNull())
	{
		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
 		if (!gameInfo->cloneof.isEmpty())
			snapdata = getScreenshot(dirpath0, gameInfo->cloneof, 0);

		// fallback to default image, first getScreenshot() can't reach here
		if (snapdata.isNull())
			snapdata = defSnapData;
	}

	return snapdata;
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

TreeModel::TreeModel(QObject *parent, bool isGroup)
: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;

	static const QStringList columnList = (QStringList() 
		<< QT_TR_NOOP("Description") 
		<< QT_TR_NOOP("Name") 
		<< QT_TR_NOOP("ROMs") 
		<< QT_TR_NOOP("Manufacturer") 
		<< QT_TR_NOOP("Driver") 
		<< QT_TR_NOOP("Year") 
		<< QT_TR_NOOP("Clone of"));

	foreach (QString header, columnList)
		rootData << tr(qPrintable(header));

	rootItem = new TreeItem(rootData);

	foreach (QString gameName, mameGame->nameInfoMap.keys())
	{
		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];

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

TreeModel::~TreeModel()
{
	delete rootItem;
}

//mandatory
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

//mandatory
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

//mandatory
int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem = getItem(parent);

	return parentItem->childCount();
}

//mandatory
int TreeModel::columnCount(const QModelIndex &parent) const
{
	return rootItem->columnCount();
}

//mandatory
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeItem *item = getItem(index);
	const QString gameName = item->data(1).toString();
	GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
	int col = index.column();

	switch (role)
	{
	case Qt::ForegroundRole:
		if (gameInfo->emulation == 0 && !gameInfo->isExtRom)
			return qVariantFromValue(QColor(isDarkBg ? QColor(255, 96, 96) : Qt::darkRed));
		else
			//fixme: use palette color
			return qVariantFromValue(QColor((isDarkBg) ? Qt::white : Qt::black));

	case Qt::DecorationRole:
		if (col == COL_DESC)
		{
			QByteArray icondata;
			if (gameInfo->icondata.isNull())
				icondata = defIconData;
			else
				icondata = gameInfo->icondata;

			bool isLargeIcon = gameList->listMode == win->actionLargeIcons->objectName().remove("action");
			
			QPixmap pm;
			pm.loadFromData(icondata, "ico");

			//scale down the icon
			if (!isLargeIcon)
				pm = pm.scaled(QSize(16, 16), Qt::KeepAspectRatio, Qt::SmoothTransformation);

			if(!gameInfo->available)
			{
				QPainter p;
				p.begin(&pm);
				if(isLargeIcon)
					p.drawPixmap(24, 24, QPixmap(":/res/status-na.png"));
				else
					p.drawPixmap(8, 8, QPixmap(":/res/status-na.png"));
				p.end();
			}
			
			return QIcon(pm);
		}
		break;

 	case Qt::UserRole + FOLDER_BIOS:
		return gameInfo->biosof();
		
	case Qt::UserRole + FOLDER_CONSOLE:
		return gameInfo->isExtRom ? true : false;

	//convert 'Name' column for Console
	case Qt::UserRole:
		if (col == COL_NAME && gameInfo->isExtRom)
			return item->data(col);
		break;
		
	case Qt::DisplayRole:
		if (col == COL_DESC && local_game_list && !gameInfo->lcDesc.isEmpty())
			return gameInfo->lcDesc;

		else if (col == COL_MFTR && local_game_list && !gameInfo->lcMftr.isEmpty())
			return gameInfo->lcMftr;

		else if (col == COL_YEAR && gameInfo->year.isEmpty())
			return "?";

		else if (col == COL_NAME && gameInfo->isExtRom)
			return gameInfo->romof;
	
		//convert 'ROMs' column
		else if (col == COL_ROM)
		{
			int status = item->data(COL_ROM).toInt();
			switch (status)
			{
			case -1:
				return "";
				
			case 0:
				return "No";

			case 1:
				return "Yes";
			}
		}

		return item->data(col);
	}
	return QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
							   int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

void TreeModel::updateRow(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 0);
	QModelIndex j = index.sibling(index.row(), columnCount() - 1);

	emit dataChanged(i, j);
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

TreeItem * TreeModel::buildItem(TreeItem *parent, QString gameName, bool isGroup)
{
	GameInfo *gameInfo = mameGame->nameInfoMap[gameName];

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

GameInfo::GameInfo(QObject *parent) :
QObject(parent),
cocktail(64),
protection(64),
isCloneAvailable(false),
available(0)
{
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
		gameInfo = mameGame->nameInfoMap[romof];

		if (!gameInfo->romof.isEmpty())
		{
			biosof = gameInfo->romof;
				if (gameInfo->romof.trimmed()=="")
		win->log("ERR5");
			gameInfo = mameGame->nameInfoMap[gameInfo->romof];
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

	QDir().mkpath(CFG_PREFIX + "cache");
	QFile file(CFG_PREFIX + "cache/gamelist.cache");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_3);
	out << mameGame->mameVersion;
	out << mameGame->mameDefaultIni;	//default.ini
	out << mameGame->nameInfoMap.count();

	win->log(QString("s11n %1 games").arg(mameGame->nameInfoMap.count()));

	//fixme: should place in thread and use mameGame directly
	gameList->switchProgress(gameList->numTotalGames, tr("Saving listxml"));
	int i = 0;
	foreach (QString gameName, mameGame->nameInfoMap.keys())
	{
		gameList->updateProgress(i++);
		qApp->processEvents();
	
		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
		out << gameName;
		out << gameInfo->description;
		out << gameInfo->year;
		out << gameInfo->manufacturer;
		out << gameInfo->sourcefile;
		out << gameInfo->isBios;
		out << gameInfo->isExtRom;
		out << gameInfo->cloneof;
		out << gameInfo->romof;
		out << gameInfo->available;

		out << gameInfo->status;
		out << gameInfo->emulation;
		out << gameInfo->color;
		out << gameInfo->sound;
		out << gameInfo->graphic;
		out << gameInfo->cocktail;
		out << gameInfo->protection;
		out << gameInfo->savestate;
		out << gameInfo->palettesize;

		out << gameInfo->clones.count();
		foreach (QString clonename, gameInfo->clones)
			out << clonename;

		out << gameInfo->crcRomInfoMap.count();
		foreach (quint32 crc, gameInfo->crcRomInfoMap.keys())
		{
			RomInfo *romInfo = gameInfo->crcRomInfoMap[crc];
			out << crc;
			out << romInfo->name;
			out << romInfo->status;
			out << romInfo->size;
		}

		out << gameInfo->nameBiosInfoMap.count();
		foreach (QString name, gameInfo->nameBiosInfoMap.keys())
		{
			BiosInfo *biosInfo = gameInfo->nameBiosInfoMap[name];
			out << name;
			out << biosInfo->description;
			out << biosInfo->isdefault;
		}

		out << gameInfo->nameDeviceInfoMap.count();
		foreach (QString name, gameInfo->nameDeviceInfoMap.keys())
		{
			DeviceInfo *deviceInfo = gameInfo->nameDeviceInfoMap[name];
			out << name;
			out << deviceInfo->mandatory;
			out << deviceInfo->extension;
		}
	}
	gameList->switchProgress(-1, "");
	
	file.close();
}

int MameGame::des11n()
{
	QFile file(CFG_PREFIX + "cache/gamelist.cache");
	file.open(QIODevice::ReadOnly);
	QDataStream in(&file);

	// Read and check the header
	quint32 mamepSig;
	in >> mamepSig;
	if (mamepSig != MAMEPLUS_SIG)
	{
		win->log(tr("Cache signature error."));
		return QDataStream::ReadCorruptData;
	}

	// Read the version
	qint16 version;
	in >> version;
	if (version != S11N_VER)
	{
		win->log(tr("Cache version has been updated. A full refresh is required."));
		return QDataStream::ReadCorruptData;
	}

	if (version < 1)
		in.setVersion(QDataStream::Qt_4_2);
	else
		in.setVersion(QDataStream::Qt_4_3);

	//finished checking
	if (mameGame)
		delete mameGame;
	mameGame = new MameGame(win);

	// MAME Version
	mameGame->mameVersion = utils->getMameVersion();
	QString mameVersion0;
	in >> mameVersion0;

	// default mame.ini text
	in >> mameGame->mameDefaultIni;
	
	int gamecount;
	in >> gamecount;

	for (int i = 0; i < gamecount; i++)
	{
		GameInfo *gameinfo = new GameInfo(mameGame);
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

		in >> gameinfo->status;
		in >> gameinfo->emulation;
		in >> gameinfo->color;
		in >> gameinfo->sound;
		in >> gameinfo->graphic;
		in >> gameinfo->cocktail;
		in >> gameinfo->protection;
		in >> gameinfo->savestate;
		in >> gameinfo->palettesize;

		mameGame->nameInfoMap[gamename] = gameinfo;

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

	win->log(QString("des11n game count %1").arg(gamecount));

	// verify MAME Version
	if (mameGame->mameVersion != mameVersion0)
	{
		win-> log(QString("new mame version: %1 / %2").arg(mameVersion0).arg(mameGame->mameVersion));
		mameGame0 = mameGame;
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
	QString gameName = gameList->getViewString(index, COL_NAME);
	GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
	//fixme: combine @ console gamename
	if (!gameInfo->nameDeviceInfoMap.isEmpty())
	{
		QString gameName2 = gameList->getViewString(index, COL_NAME + COL_LAST);

		if (!gameName2.isEmpty())
		{
			gameInfo = mameGame->nameInfoMap[gameName2];
			if (gameInfo && gameInfo->isExtRom)
				gameName = gameName2;
		}
	}

	//fixme: should not use hardcoded values?
	if (currentGame == gameName)
		return QSize(1,33);
	else
		return QSize(1,17);
}

void GamelistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							 const QModelIndex &index ) const
{
	QString gameName = gameList->getViewString(index, COL_NAME);
	GameInfo *gameInfo = mameGame->nameInfoMap[gameName];

	//fixme: combine @ console gamename
	// override gameName and gameInfo for console roms
	if (!gameInfo->nameDeviceInfoMap.isEmpty())
	{
		QString gameName2 = gameList->getViewString(index, COL_NAME + COL_LAST);
		if (!gameName2.isEmpty())
		{
			gameInfo = mameGame->nameInfoMap[gameName2];
			if (gameInfo && gameInfo->isExtRom)
				gameName = gameName2;
		}
	}

	if (currentGame == gameName)
	{
		static QPixmap pmSelBarLite(":/res/mamegui/selected_bar_light.png");
		static QPixmap pmSelBarDark(":/res/mamegui/selected_bar_dark.png");
		QRect rc = option.rect;
		QPoint pt;
		QString text;

		if (index.column() == COL_DESC)
		{
			QString gameDesc = gameList->getViewString(index, COL_DESC);

			//draw big icon
			pt = rc.topLeft();
			pt.setX(pt.x() + 2);
			rc = QRect(pt, rc.bottomRight());

			QByteArray icondata;
			QPixmap pm;
			if (gameInfo->icondata.isNull())
				icondata = defIconData;
			else
				icondata = gameInfo->icondata;
			pm.loadFromData(icondata, "ico");

			// paint the unavailable icon on top of original icon
			if(!gameInfo->available)
			{
				QPainter p;
				p.begin(&pm);
				p.drawPixmap(24, 24, QPixmap(":/res/status-na.png"));
				p.end();
			}
			QApplication::style()->drawItemPixmap (painter, rc, Qt::AlignLeft | Qt::AlignVCenter, pm);

			// calc text rect
			pt = rc.topLeft();
			pt.setX(pt.x() + 34);	//32px + 2px left padding
			rc = QRect(pt, rc.bottomRight());

			text = gameDesc;
		}
		else
			text = gameList->getViewString(index, index.column());

		// set bold font for selected items
		QFont boldFont(option.font);
		boldFont.setBold(true);
		painter->setFont(boldFont);

		//elide the text within bounding rect
		QFontMetrics fm(boldFont);
		text = fm.elidedText(text, option.textElideMode, rc.width() - 5);	//3px + 2px right padding

		if (option.state & QStyle::State_Selected)
		{
			//draw text bg
			painter->drawPixmap(rc, isDarkBg ? pmSelBarDark : pmSelBarLite);
//			painter->fillRect(rc, option.palette.highlight());

			//draw text
			pt = rc.topLeft();
			pt.setX(pt.x() + 3);
			rc = QRect(pt, rc.bottomRight());
			QStyleOptionViewItem myoption = option;
			//override foreground, black doesnt look nice
			painter->setPen(Qt::white);
//			painter->drawText(rc, text, QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
			QApplication::style()->drawItemText(painter, rc, Qt::AlignLeft | Qt::AlignVCenter, myoption.palette, true, text);
			return;
		}
	}

	QItemDelegate::paint(painter, option, index);
	return;
}

/*
XTreeView::XTreeView(QWidget *parent)
: QTreeView(parent)
{
}

void XTreeView::paintEvent(QPaintEvent *event)
{
//	QTreeView::paintEvent(event);
	QPainter p(this);

	p.begin(this);
	p.drawPixmap(0, 32, QPixmap(":/res/32x32/input-gaming.png"));
	p.end();
}
//*/

Gamelist::Gamelist(QObject *parent) : 
QObject(parent),
hasInitd(false),
loadProc(NULL),
numTotalGames(-1),
menu(NULL),
headerMenu(NULL),
loadIconStatus(0)
{
	connect(&selectionThread, SIGNAL(snapUpdated(int)), this, SLOT(setupSnap(int)));
	
	mAuditor = new MergedRomAuditor(parent);
}

Gamelist::~Gamelist()
{
//	win->log("DEBUG: Gamelist::~Gamelist()");
	if (loadProc)
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
	bool isConvExtRom = false;
	if (column >= COL_LAST)
	{
		column -= COL_LAST;
		isConvExtRom = true;
	}

	QModelIndex j = index.sibling(index.row(), column);
	//fixme: sometime model's NULL...
	if (!index.model())
		return "";

	if (isConvExtRom)
		return index.model()->data(j, Qt::UserRole).toString();
	else
		return index.model()->data(j, Qt::DisplayRole).toString();
}

void Gamelist::updateSelection()
{
	if (hasInitd && mameGame->nameInfoMap.contains(currentGame))
	{
		selectionThread.myqueue.enqueue(currentGame);
		selectionThread.update();
	}
}

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		QString gameName = getViewString(current, COL_NAME);
		if (gameName.isEmpty())
			return;

		QString gameDesc = getViewString(current, COL_DESC);

		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];

		if (!gameInfo->nameDeviceInfoMap.isEmpty())
		{
			QString gameName2 = getViewString(current, COL_NAME + COL_LAST);
			if (!gameName2.isEmpty())
			{
				gameInfo = mameGame->nameInfoMap[gameName2];
				if (gameInfo && gameInfo->isExtRom)
					gameName = gameName2;
			}

			/*
			DeviceInfo *deviceinfo = gameInfo->nameDeviceInfoMap["cartridge"];
			if (deviceinfo)
				win->log(deviceinfo->extension.join(", "));
			*/
		}

		currentGame = gameName;
		
		//update statusbar
		win->logStatus(gameDesc);
		win->logStatus(gameInfo);

		selectionThread.myqueue.enqueue(currentGame);
		selectionThread.update();

#ifdef Q_OS_WIN
		//fixme: move to thread!
		if (m1 != NULL && m1->available)
			m1->updateList();
#endif /* Q_OS_WIN */

		//update selected rows, fixme: performance bottleneck!
		gameListModel->updateRow(gameListPModel->mapToSource(current));
		gameListModel->updateRow(gameListPModel->mapToSource(previous));
	}
	else
		currentGame = "";
}

void Gamelist::restoreGameSelection()
{
	if (!gameListModel || !gameListPModel)
		return;

	QString gameName = currentGame;
	QModelIndex i, pi;

	// select current game
	if (mameGame->nameInfoMap.contains(gameName))
	{
		GameInfo *gameinfo = mameGame->nameInfoMap[gameName];
		//fixme: should consider other columns
		i = gameListModel->index(COL_DESC, gameinfo->pModItem);
		win->log("restore callback: " + gameName);
	}

	if (i.isValid())
		pi = gameListPModel->mapFromSource(i);

	if (!pi.isValid())
	{
		// select first row otherwise
		pi = gameListPModel->index(0, 0, QModelIndex());
	}

	if (!pi.isValid())
		return;

	win->tvGameList->setCurrentIndex(pi);
	win->lvGameList->setCurrentIndex(pi);

	win->tvGameList->scrollTo(pi, QAbstractItemView::PositionAtCenter);
	win->lvGameList->scrollTo(pi, QAbstractItemView::PositionAtCenter);

	win->labelGameCount->setText(tr("%1 games").arg(visibleGames.count()));

	//auto collapse
	QString folderName;
	QTreeWidgetItem *item = win->treeFolders->currentItem();
	QTreeWidgetItemIterator it(win->treeFolders);

	if (item->parent() == NULL)
		folderName = item->text(0);
	else
		folderName = item->parent()->text(0);

	while (*it)
	{
		if ((*it)->parent() == NULL && (*it)->isExpanded() && 
			(*it)->text(0) != folderName)
		{
			win->log("co: " + (*it)->text(0) + ", " + folderName);
			win->treeFolders->collapseItem(*it);
		}
		++it;
	}
}

// must update GUI in main thread
void Gamelist::setupSnap(int snapType)
{
	switch (snapType)
	{
	case DOCK_SNAP:
	case DOCK_TITLE:
	case DOCK_FLYER:
	case DOCK_CABINET:
	case DOCK_MARQUEE:
	case DOCK_CPANEL:
	case DOCK_PCB:
		((Screenshot*)win->dockCtrls[snapType])->setPixmap(selectionThread.pmSnapData[snapType], win->actionEnforceAspect->isChecked());
		break;
	case DOCK_HISTORY:
		win->tbHistory->setHtml(selectionThread.historyText);
		break;
	case DOCK_MAMEINFO:
		win->tbMameinfo->setHtml(selectionThread.mameinfoText);
		break;
	case DOCK_STORY:
		win->tbStory->setHtml(selectionThread.storyText);
		break;
	case DOCK_COMMAND:
		win->tbCommand->setHtml(selectionThread.commandText);
		break;
#if 0
//draw in memory
		QPixmap pm(100, 100);
		QPainter p(&pm);

		QRect rc(0, 0, 100, 100);
		QString text = selectionThread.storyText;
		text.replace("<br>", "\n");
		rc = p.boundingRect(rc, Qt::AlignLeft, text);
//		win->log(QString("rc: %1, %2, %3").arg(rc.width()).arg(rc.height()).arg(selectionThread.storyText.count()));

		QPixmap pm2(rc.width(), 300);
		QPainter p2;
		QFont font;
		win->log(p2.font().family());
		pm2.fill();

		p2.begin(&pm2);
		font.setFamily("MS Gothic");
//		font.setPointSize(8);
		p2.setFont(font);
		p2.drawPixmap(24, 24, QPixmap(":/res/status-na.png"));
		p2.drawText(rc, Qt::AlignLeft | Qt::TextWordWrap, text);
		p2.end();
		
		win->ssPCB->setPixmap(pm2);
#endif
	default:
		break;
	}
}

void Gamelist::init(bool toggleState, int initMethod)
{
	//filter button's toggled(false) SIGNAL
	if (!toggleState)
		return;

	bool isGroup = true, isLView = false;

	// get current game list mode
	if (win->actionDetails->isChecked())
	{
		listMode = win->actionDetails->objectName().remove("action");
		isGroup = false;
	}
	else if (win->actionLargeIcons->isChecked())
	{
		listMode = win->actionLargeIcons->objectName().remove("action");
		isLView = true;
		isGroup = false;
	}
	else
		listMode = win->actionGrouped->objectName().remove("action");

	int des11n_status = QDataStream::Ok;

	//fixme: illogical call before mameGame init
	// des11n on app start
	if (!mameGame)
		des11n_status = mameGame->des11n();

	if (des11n_status == QDataStream::Ok)
	{
		//disable sorting before insertion for better performance
		win->tvGameList->setSortingEnabled(false);

		// disable ctrl updating before deleting its model
		disconnect(win->tvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));
		disconnect(win->lvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));

		win->lvGameList->hide();
		win->layMainView->removeWidget(win->lvGameList);

		win->tvGameList->hide();
		win->layMainView->removeWidget(win->tvGameList);

		//delete the model
		if (gameListModel)
			delete gameListModel;

		//init the model
		gameListModel = new TreeModel(win, isGroup);

		//delete/init proxy model
		if (gameListPModel)
			delete gameListPModel;
		gameListPModel = new GameListSortFilterProxyModel(win);

		gameListPModel->setSourceModel(gameListModel);
		gameListPModel->setSortCaseSensitivity(Qt::CaseInsensitive);

		if (isLView)
		{
			win->layMainView->addWidget(win->lvGameList);
			win->lvGameList->show();
			win->lvGameList->setModel(gameListPModel);
		}
		else
		{
			win->layMainView->addWidget(win->tvGameList);
			win->tvGameList->show();
			win->tvGameList->setModel(gameListPModel);
			if (defaultGameListDelegate == NULL)
				defaultGameListDelegate = win->tvGameList->itemDelegate();

			if (win->actionRowDelegate->isChecked())
				win->tvGameList->setItemDelegate(&gamelistDelegate);
		}

		if (initMethod == GAMELIST_INIT_FULL)
		{
			/* init everything else here after we have mameGame */
			// init folders
			initFolders();

			//fixme: something here should be moved to opt
			// init options from default mame.ini
			optUtils->loadDefault(mameGame->mameDefaultIni);
			// assign option type, defvalue, min, max, etc. from template
			optUtils->loadTemplate();
			// load mame.ini overrides
			optUtils->loadIni(OPTLEVEL_GLOBAL, "mame.ini");

			foreach (QString gameName, mameGame->nameInfoMap.keys())
			{
				GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
				// add GUI MESS extra software paths
				if (!gameInfo->nameDeviceInfoMap.isEmpty())
				{
					MameOption *pMameOpt = new MameOption(0);	//fixme parent
					pMameOpt->guivisible = true;
					mameOpts[gameName + "_extra_software"] = pMameOpt;
					//save a list of console system for later use
					consoleGamesL << gameName;
				}

				// isCloneAvailable: see if parent should be shown in a treeview even its available != 1
				if (!gameInfo->cloneof.isEmpty())
				{
					GameInfo *gameInfo2 = mameGame->nameInfoMap[gameInfo->cloneof];
					if (gameInfo->available == 1)
						gameInfo2->isCloneAvailable = true;
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

			// we're ready to set version info
			win->setVersion();
		}

		loadMMO(UI_MSG_LIST);
		loadMMO(UI_MSG_MANUFACTURE);

		// connect gameListModel/gameListPModel signals after the view init completed
		// connect gameListModel/gameListPModel signals after mameOpts init
		if (isLView)
		{
			connect(win->lvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));

			disconnect(win->lvGameList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
			connect(win->lvGameList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
		}
		else
		{
			connect(win->tvGameList, SIGNAL(activated(const QModelIndex &)), this, SLOT(runMame()));

			disconnect(win->tvGameList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
			connect(win->tvGameList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));
		}
		//refresh current list
		filterFolderChanged(win->treeFolders->currentItem());

		// restore game list column state
		if (initMethod != GAMELIST_INIT_DIR)
		{
			QByteArray column_state;

			// restore view column state, needed on first init and after auditing, but not for folder switching
			if (guiSettings.value("column_state").isValid())
				column_state = guiSettings.value("column_state").toByteArray();
			else
				column_state = defSettings.value("column_state").toByteArray();

			win->tvGameList->header()->restoreState(column_state);
			restoreFolderSelection();
		}

		//sorting 
		win->tvGameList->setSortingEnabled(true);

		if (isLView)
			win->lvGameList->setFocus();
		else
			win->tvGameList->setFocus();

		// attach menus
		initMenus();

		// everything is done, enable ctrls now
		win->treeFolders->setEnabled(true);
		win->actionLargeIcons->setEnabled(true);
		win->actionDetails->setEnabled(true);
		win->actionGrouped->setEnabled(true);
		win->actionRefresh->setEnabled(true);
		win->actionDirectories->setEnabled(true);
		win->actionProperties->setEnabled(true);
		win->actionSrcProperties->setEnabled(true);
		win->actionDefaultOptions->setEnabled(true);
		win->actionPlay->setEnabled(true);
		win->lineEditSearch->setEnabled(true);

		// load icon in a background thread
		loadIcon();

		hasInitd = true;

		//for re-init list from folders
		restoreGameSelection();

		win->log(QString("init'd game count %1").arg(mameGame->nameInfoMap.count()));
	}
	else
	{
		//we'll call init() again later
		mameOutputBuf = "";
		QStringList args;

		args << "-listxml";
		loadTimer.start();
		loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));

		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadListXmlReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadListXmlFinished(int, QProcess::ExitStatus)));
	}
}

void Gamelist::loadIcon()
{
	// load icons
	disconnect(&loadIconWatcher, SIGNAL(finished()), this, SLOT(postLoadIcon()));
	connect(&loadIconWatcher, SIGNAL(finished()), this, SLOT(postLoadIcon()));
	QFuture<void> future = QtConcurrent::run(this, &Gamelist::loadIconWorkder);
	loadIconWatcher.setFuture(future);
}
	
void Gamelist::loadIconWorkder()
{
	bool done = false;
	bool cancel = false;

	GameInfo *gameInfo, *gameInfo2;

//	win->log(QString("ico count: %1").arg(mameGame->gamenameGameInfoMap.count()));

	while(!done)
	{
		// iterate split dirpath
		QStringList dirpaths = mameOpts["icons_directory"]->globalvalue.split(";");
		foreach (QString _dirpath, dirpaths)
		{
			QDir dir(_dirpath);
			QString dirpath = utils->getPath(_dirpath);
		
			QStringList nameFilter;
			nameFilter << "*" + ICO_EXT;
			
			// iterate all files in the path
			QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
			for (int i = 0; i < files.count(); i++)
			{
				QString gameName = files[i].toLower().remove(ICO_EXT);
				if (mameGame->nameInfoMap.contains(gameName))
				{
					gameInfo = mameGame->nameInfoMap[gameName];
					if (gameInfo->icondata.isNull())
					{
						QFile icoFile(dirpath + gameName + ICO_EXT);
						if (icoFile.open(QIODevice::ReadOnly))
						{
							gameInfo->icondata = icoFile.readAll();
							loadIconStatus++;
						}
					}
				}

				if (cancel)
					break;
			}

			if (cancel)
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

				QString gameName = info.name.toLower().remove(ICO_EXT);
				if (mameGame->nameInfoMap.contains(gameName))
				{
					gameInfo = mameGame->nameInfoMap[gameName];
					if (gameInfo->icondata.isNull())
					{
						QuaZipFile icoFile(&zip);
						if (icoFile.open(QIODevice::ReadOnly))
						{
							gameInfo->icondata = icoFile.readAll();
							loadIconStatus++;
						}
					}
				}
				/*
				else if (gameName == "warning")
				{
				}
				//*/
				if (cancel)
					break;
			}
			if (cancel)
				break;
		}

		if (!cancel)
		{
			// get clone icons from parent
			foreach (QString gameName, mameGame->nameInfoMap.keys())
			{
				gameInfo = mameGame->nameInfoMap[gameName];
				if (!gameInfo->isExtRom && gameInfo->icondata.isNull() && !gameInfo->cloneof.isEmpty())
				{
					gameInfo2 = mameGame->nameInfoMap[gameInfo->cloneof];
					if (!gameInfo2->icondata.isNull())
					{
						gameInfo->icondata = gameInfo2->icondata;
//						emit icoUpdated(gameName);
					}
				}
			}
			done = true;
		}
		cancel = false;
	}
}

void Gamelist::postLoadIcon()
{
	win->lvGameList->update(win->lvGameList->rect());
	win->tvGameList->update(win->tvGameList->rect());
}

void Gamelist::loadMMO(int msgCat)
{
	static const QStringList msgFileName = (QStringList() 
		<< "mame"
		<< "lst"
		<< "readings"
		<< "manufact");

	QString dirpath;
	//only mameplus contains this option for now
	if (mameOpts.contains("langpath"))
		dirpath = utils->getPath(mameOpts["langpath"]->globalvalue);
	else
		dirpath = "lang/";
	QFile file( dirpath + language + "/" + msgFileName[msgCat] + ".mmo");
	if (!file.exists())
	{
		win->log("not exist: " + dirpath + language + "/" + msgFileName[msgCat] + ".mmo");
		return;
	}

	struct mmo_header
	{
		int dummy;
		int version;
		int num_msg;
	};
	
	struct mmo_data
	{
		const unsigned char *uid;
		const unsigned char *ustr;
		const void *wid;
		const void *wstr;
	};
	
	struct mmo {
		enum {
			MMO_NOT_LOADED,
			MMO_NOT_FOUND,
			MMO_READY
		} status;
	
		struct mmo_header header;
		struct mmo_data *mmo_index;
		char *mmo_str;
	};
	
	struct mmo _mmo;
	struct mmo *pMmo = &_mmo;
	QHash<QString, QString> mmohash;
	int size = sizeof pMmo->header;

	if (!file.open(QIODevice::ReadOnly))
		goto mmo_readerr;

	if (file.read((char*)&pMmo->header, size) != size)
		goto mmo_readerr;

	if (pMmo->header.dummy)
		goto mmo_readerr;

	if (pMmo->header.version != 3)
		goto mmo_readerr;

	pMmo->mmo_index = (mmo_data*)malloc(pMmo->header.num_msg * sizeof(pMmo->mmo_index[0]));
	if (!pMmo->mmo_index)
		goto mmo_readerr;

	size = pMmo->header.num_msg * sizeof(pMmo->mmo_index[0]);
	if (file.read((char*)pMmo->mmo_index, size) != size)
		goto mmo_readerr;

	int str_size;
	size = sizeof(str_size);
	if (file.read((char*)&str_size, size) != size)
		goto mmo_readerr;

	pMmo->mmo_str = (char*)malloc(str_size);
	if (!pMmo->mmo_str)
		goto mmo_readerr;

	if (file.read((char*)pMmo->mmo_str, str_size) != str_size)
		goto mmo_readerr;

	for (int i = 0; i < pMmo->header.num_msg; i++)
	{
		QString name((char*)((unsigned char*)pMmo->mmo_str + (unsigned long)pMmo->mmo_index[i].uid));
		QString localName = QString::fromUtf8((char*)((unsigned char*)pMmo->mmo_str + (unsigned long)pMmo->mmo_index[i].ustr));
		mmohash[name] = localName;
	}

	foreach (QString gameName, mameGame->nameInfoMap.keys())
	{
		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
		switch(msgCat)
		{
			case UI_MSG_LIST:
				if (mmohash.contains(gameInfo->description))
					gameInfo->lcDesc = mmohash[gameInfo->description];				
				break;
			case UI_MSG_MANUFACTURE:
				if (mmohash.contains(gameInfo->manufacturer))
					gameInfo->lcMftr = mmohash[gameInfo->manufacturer];
				break;
		}
	}

mmo_readerr:
	if (pMmo->mmo_str)
	{
		free(pMmo->mmo_str);
		pMmo->mmo_str = NULL;
	}

	if (pMmo->mmo_index)
	{
		free(pMmo->mmo_index);
		pMmo->mmo_index = NULL;
	}

	file.close();
}

void Gamelist::initMenus()
{
	bool isLView = false;
	if (win->actionLargeIcons->isChecked())
		isLView = true;

	// init context menu, we don't need to init it twice
	if (menu == NULL)
	{
		menu = new QMenu(win);
	
		menu->addAction(win->actionPlay);
//		menu->addAction(win->actionPlayInp);
#ifdef Q_OS_WIN
		menu->addAction(win->actionConfigIPS);
#endif /* Q_OS_WIN */
		menu->addSeparator();
//		menu->addAction(win->actionAudit);
//		menu->addSeparator();
		menu->addAction(win->actionSrcProperties);
		menu->addAction(win->actionProperties);
	}

	QWidget *w;

	if (isLView)
		w = win->lvGameList;
	else
		w = win->tvGameList;

	w->setContextMenuPolicy(Qt::CustomContextMenu);
	disconnect(w, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
	connect(w, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

	disconnect(menu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

	disconnect(win->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
	connect(win->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

	//init tvGameList header context menu
	if (headerMenu == NULL)
	{
		headerMenu = new QMenu(win->tvGameList);
	
		headerMenu->addAction(win->actionColSortAscending);
		headerMenu->addAction(win->actionColSortDescending);
		headerMenu->addSeparator();
		headerMenu->addAction(win->actionColDescription);
		headerMenu->addAction(win->actionColName);
		headerMenu->addAction(win->actionColROMs);
		headerMenu->addAction(win->actionColManufacturer);
		headerMenu->addAction(win->actionColDriver);
		headerMenu->addAction(win->actionColYear);
		headerMenu->addAction(win->actionColCloneOf);
	
		// add sorting action to an exclusive group
		QActionGroup *sortingActions = new QActionGroup(headerMenu);
		sortingActions->addAction(win->actionColSortAscending);
		sortingActions->addAction(win->actionColSortDescending);
	}

	QHeaderView *header = win->tvGameList->header();

	header->setContextMenuPolicy (Qt::CustomContextMenu);

	disconnect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showHeaderContextMenu(const QPoint &)));
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showHeaderContextMenu(const QPoint &)));

	disconnect(header, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(restoreGameSelection()));
	connect(header, SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(restoreGameSelection()));

	disconnect(headerMenu, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));
	connect(headerMenu, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));

	disconnect(win->menuCustomizeFields, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));
	connect(win->menuCustomizeFields, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));
}

void Gamelist::showContextMenu(const QPoint &p)
{
    menu->popup(win->tvGameList->mapToGlobal(p));
}

void Gamelist::updateContextMenu()
{
	if (currentGame.isEmpty())
	{
		win->actionPlay->setEnabled(false);
		return;
	}

#ifdef Q_OS_WIN
	win->actionConfigIPS->setEnabled(ipsUI->checkAvailable(currentGame));
#endif /* Q_OS_WIN */

	QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->nameInfoMap[gameName];

	QPixmap pm;
	pm.loadFromData(gameInfo->icondata, "ico");
	QIcon icon(pm);

	win->actionPlay->setEnabled(true);
	win->actionPlay->setIcon(icon);
    win->actionPlay->setText(tr("Play %1").arg(gameInfo->description));

	win->actionSrcProperties->setText(tr("Properties for %1").arg(gameInfo->sourcefile));
}

void Gamelist::showHeaderContextMenu(const QPoint &p)
{
	QHeaderView *header = win->tvGameList->header();

	int currCol = header->logicalIndexAt(p);
	int sortCol = header->sortIndicatorSection();

	if (currCol != sortCol)
	{
		win->actionColSortAscending->setChecked(false);
		win->actionColSortDescending->setChecked(false);
	}
	else
	{
		if (header->sortIndicatorOrder() == Qt::AscendingOrder)
			win->actionColSortAscending->setChecked(true);
		else
			win->actionColSortDescending->setChecked(true);
	}
	
    headerMenu->popup(win->tvGameList->mapToGlobal(p));
}

void Gamelist::updateHeaderContextMenu()
{
	QHeaderView *header = win->tvGameList->header();

	win->actionColDescription->setChecked(header->isSectionHidden(0) ? false : true);
	win->actionColName->setChecked(header->isSectionHidden(1) ? false : true);
	win->actionColROMs->setChecked(header->isSectionHidden(2) ? false : true);
	win->actionColManufacturer->setChecked(header->isSectionHidden(3) ? false : true);
	win->actionColDriver->setChecked(header->isSectionHidden(4) ? false : true);
	win->actionColYear->setChecked(header->isSectionHidden(5) ? false : true);
	win->actionColCloneOf->setChecked(header->isSectionHidden(6) ? false : true);
}

void Gamelist::toggleDelegate(bool isHilite)
{
	if (isHilite)
		win->tvGameList->setItemDelegate(&gamelistDelegate);
	else if (defaultGameListDelegate != NULL)
		win->tvGameList->setItemDelegate(defaultGameListDelegate);
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

void Gamelist::loadListXmlFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	parse();

	//chain
	loadDefaultIni();
}

void Gamelist::loadDefaultIni()
{
	mameGame->mameDefaultIni = "";
	QStringList args;
	args << "-showconfig" << "-noreadconfig";

	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));
	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadDefaultIniReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadDefaultIniFinished(int, QProcess::ExitStatus)));
}

void Gamelist::loadDefaultIniReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString buf = proc->readAllStandardOutput();
	
	mameGame->mameDefaultIni += buf;
}

void Gamelist::loadDefaultIniFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	optUtils->loadDefault(mameGame->mameDefaultIni);
	optUtils->loadTemplate();

	//reload gameList. this is a chained call from loadListXmlFinished()
	init(true, GAMELIST_INIT_FULL);
//	win->log("end of gameList->loadDefFin()");
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
void Gamelist::extractMergedFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	runMame(true);
}

// delete the extracted rom
void Gamelist::runMergedFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	QFile file(currentTempROM);
	file.remove();
	currentTempROM.clear();
}

void Gamelist::parse()
{
	ListXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QXmlInputSource *pxmlInputSource = new QXmlInputSource();
	pxmlInputSource->setData(mameOutputBuf);

	win->log("DEBUG: Gamelist::start parse()");
	
	switchProgress(numTotalGames, tr("Parsing listxml"));
	reader.parse(*pxmlInputSource);
	switchProgress(-1, "");

	GameInfo *gameInfo, *gameInfo0, *gameInfo2;
	foreach (QString gameName, mameGame->nameInfoMap.keys())
	{
		gameInfo = mameGame->nameInfoMap[gameName];

		// restore previous audit results
		if (mameGame0 && mameGame0->nameInfoMap.contains(gameName))
		{
			gameInfo0 = mameGame0->nameInfoMap[gameName];
			gameInfo->available = gameInfo0->available;
		}

		// update clone list
		if (!gameInfo->cloneof.isEmpty())
		{
			gameInfo2 = mameGame->nameInfoMap[gameInfo->cloneof];
			gameInfo2->clones.insert(gameName);
		}
	}

	// restore previous console audit
	if (mameGame0)
	{
		foreach (QString gameName, mameGame0->nameInfoMap.keys())
		{
			gameInfo0 = mameGame0->nameInfoMap[gameName];

			if (gameInfo0->isExtRom && 
				//the console is supported by current mame version
				mameGame->nameInfoMap.contains(gameInfo0->romof))
			{
				gameInfo = new GameInfo(mameGame);
				gameInfo->description = gameInfo0->description;
				gameInfo->isBios = false;
				gameInfo->isExtRom = true;
				gameInfo->romof = gameInfo0->romof;
				gameInfo->sourcefile = gameInfo0->sourcefile;
				gameInfo->available = 1;
				mameGame->nameInfoMap[gameName] = gameInfo;
			}
		}

		delete mameGame0;
	}

	delete pxmlInputSource;
	mameOutputBuf.clear();
}

void Gamelist::filterRegExpCleared()
{
	if (win->lineEditSearch->text().count() < 1)
		return;

	win->lineEditSearch->setText("");
	qApp->processEvents();
	filterRegExpChanged();
}

//apply searchText
void Gamelist::filterRegExpChanged()
{
	// multiple space-separated keywords
	QString text = win->lineEditSearch->text();
	// do not search less than 2 Latin chars
	if (text.count() == 1 && text.at(0).unicode() < 0x3000 /* CJK symbols start */)
		return;

	visibleGames.clear();
	text.replace(spaceRegex, "*");

	//fixme: doesnt use filterregexp
	static QRegExp emptyRegex("");
	gameListPModel->searchText = text;
	// set it for a callback to refresh the list
	gameListPModel->setFilterRegExp(emptyRegex);
	qApp->processEvents();
	win->tvGameList->expandAll();
	restoreGameSelection();
}

//apply folder switching
void Gamelist::filterFolderChanged(QTreeWidgetItem *_current, QTreeWidgetItem *previous)
{
	QTreeWidgetItem *current = _current;

	if (!current)
		current = win->treeFolders->currentItem();

	// the selection could be empty
	if (!current)
		return;

	QString filterText;

	currentFolder.clear();
	if (win->treeFolders->currentItem()->parent() != NULL)
		currentFolder.append(win->treeFolders->currentItem()->parent()->text(0));
	currentFolder.append("/" + win->treeFolders->currentItem()->text(0));

	/* hack to speed up sorting and filtering, don't know why. need more investigation
	  * filtering game desc fleld is extremely lengthy
	  * _current == NULL means this call is from a signal, only deal with it
	  */
	if (_current == NULL &&
		(currentFolder == "/" + folderList[FOLDER_ALLGAME] || 
		currentFolder == "/" + folderList[FOLDER_ALLARC] || 
		currentFolder == "/" + folderList[FOLDER_AVAILABLE] || 
		currentFolder == "/" + folderList[FOLDER_UNAVAILABLE] || 
		currentFolder == "/" + folderList[FOLDER_CONSOLE] ||
		visibleGames.count() > 10000))
	{
		win->log("hack to reinit list");
		init(true, GAMELIST_INIT_DIR);
		return;
	}

	visibleGames.clear();

	// update Refresh menu text
	QString folder;
	if (utils->isConsoleFolder())
		folder = currentFolder;
	else
		folder = tr("All Arcades");
	win->actionRefresh->setText(tr("Refresh").append(": ").append(folder));

	gameListPModel->filterList.clear();

	QString folderName;
	//root folder
	if (current->parent() == NULL)
	{
		folderName = current->text(0);
		filterText.clear();

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
		else if (folderName == folderList[FOLDER_BIOS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS);
		else if (extFolders.contains(folderName))
		{
			initExtFolders(folderName, ROOT_FOLDER);
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_EXT);
		}
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLARC);
	}
	//sub folder
	else
	{
		folderName = current->parent()->text(0);
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
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS + MAX_FOLDERS);	//hack for bios subfolders
		else if (extFolders.contains(folderName)) 
		{
			initExtFolders(folderName, filterText);
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_EXT);
		}
		//fixme
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLARC);
	}

	static QRegExp regExp("");
	// set it for a callback to refresh the list
	gameListPModel->filterText = filterText;	// must set before regExp
	gameListPModel->setFilterRegExp(regExp);

	//fixme: must have this, otherwise the list cannot be expanded properly
	qApp->processEvents();
	win->tvGameList->expandAll();
	restoreGameSelection();
}

void Gamelist::initFolders()
{
	folderList
		<< tr("All Games")
		<< tr("All Arcades")
		<< tr("Available Arcades")
		<< tr("Unavailable Arcades")
		<< tr("Consoles")
		<< tr("Manufacturer")
		<< tr("Year")
		<< tr("Driver")
		<< tr("BIOS")
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
	foreach (QString gameName, mameGame->nameInfoMap.keys())
	{
		gameInfo = mameGame->nameInfoMap[gameName];
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
			consoleList << gameName;

		if (gameInfo->isBios)
			biosList << gameName;

		if (!mftrList.contains(gameInfo->manufacturer))
			mftrList << gameInfo->manufacturer;

		QString year = gameInfo->year;
		if (year.isEmpty())
			year = "?";
		if (!yearList.contains(year))
			yearList << year;

		if (!srcList.contains(gameInfo->sourcefile))
			srcList << gameInfo->sourcefile;
	}

	consoleList.sort();
	mftrList.sort();
	yearList.sort();
	srcList.sort();
	biosList.sort();

	static QIcon icoFolder(":/res/32x32/folder.png");
	QList<QTreeWidgetItem *> items;
	for (int i = 0; i < folderList.size(); i++)
	{
		items.append(new QTreeWidgetItem(win->treeFolders, QStringList(folderList[i])));

		win->treeFolders->addTopLevelItems(items);
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
			{
				GameInfo *gameInfo = mameGame->nameInfoMap[name];
/*				QPixmap pm;
				pm.loadFromData(gameInfo->icondata, "ico");
				QIcon icon(pm);
*/
				QTreeWidgetItem *item = new QTreeWidgetItem(items[i], QStringList(name));
				item->setToolTip(0, gameInfo->description);
//				item->setIcon(0, icon);

				items[i]->addChild(item);
			}
	}

	//init ext folders

	QString folderPath = utils->getPath(guiSettings.value("folder_directory", "folders").toString());
	QStringList dirPaths = folderPath.split(";");

	extFolders.clear();
	foreach (QString _dirPath, dirPaths)
	{
		QDir dir(_dirPath);

		QStringList folderFiles = dir.entryList((QStringList() << "*" + INI_EXT), QDir::Files | QDir::Readable);
		
		foreach (QString folderFile, folderFiles)
		{
			QFile f(folderPath + folderFile);
			QFileInfo fi(f);
			extFolders.append(fi.completeBaseName());
		}
	}

	foreach (QString extFolder, extFolders)
		initExtFolders(extFolder, NULL);

	disconnect(win->treeFolders, SIGNAL(itemSelectionChanged()), this, SLOT(filterFolderChanged()));
	connect(win->treeFolders, SIGNAL(itemSelectionChanged()), this, SLOT(filterFolderChanged()));
}

void Gamelist::initExtFolders(const QString &folderName, const QString &subFolderName)
{
	QString folderPath = utils->getSinglePath(guiSettings.value("folder_directory", "folders").toString(), folderName + INI_EXT);
	QFile inFile(folderPath);

	//start parsing folder .ini
	if (inFile.open(QFile::ReadOnly | QFile::Text))
	{
		QString line;
		QString key;
		QMultiMap<QString, QString> extFolderMap;

		QTextStream in(&inFile);
		in.setCodec("UTF-8");

		//fill in extFolderMap
		do
		{
			line = in.readLine().trimmed();
			if (!line.isEmpty())
			{
				if (line.startsWith("[") && line.endsWith("]") && 
					line != "[FOLDER_SETTINGS]")
				{
				 
					if (line == "[" + ROOT_FOLDER + "]")
						key = ROOT_FOLDER;
					else
						key = line.mid(1, line.size() - 2);
				}
				else if (!key.isEmpty())
					extFolderMap.insert(key, line);
			}
		}
		while (!line.isNull());

		//build GUI tree
		if (subFolderName.isEmpty())
		{
			static QIcon icoFolder(":/res/32x32/folder.png");

			QList<QString> keys = extFolderMap.uniqueKeys();
			if (!keys.isEmpty())
			{
				QTreeWidgetItem *rootFolderItem = new QTreeWidgetItem(win->treeFolders, QStringList(folderName));
				rootFolderItem->setIcon(0, icoFolder);
			
				foreach (QString key, keys)
				{
					if (key == ROOT_FOLDER)
						continue;

					QTreeWidgetItem *folderItem = new QTreeWidgetItem(rootFolderItem, QStringList(key));
				}
			}
		}
		//apply the filter
		else
		{
			gameListPModel->filterList.clear();

			//also add parent
			foreach (QString gameName, extFolderMap.values(subFolderName))
			{
				gameListPModel->filterList.append(gameName);

				if (!mameGame->nameInfoMap.contains(gameName))
					continue;

				//append parent for standalone clones
				GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
				if (!gameInfo->cloneof.isEmpty() && !gameListPModel->filterList.contains(gameInfo->cloneof))
					gameListPModel->filterList.append(gameInfo->cloneof);
			}
		}
	}
}

void Gamelist::restoreFolderSelection()
{
	//if currentFolder has been set, it's a folder switching call
	if(!currentFolder.isEmpty())
		return;

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
						win->log(QString("treeb.gamecount %1").arg(mameGame->nameInfoMap.count()));
						return;
					}
				}
			}
		}
	}
	//fallback
	win->treeFolders->setCurrentItem(rootItem->child(FOLDER_ALLARC));
}

void Gamelist::runMame(bool runMerged)
{
	//block multi mame session for now
	//if (procMan->procCount > 0)
	//	return;

	if (currentGame.isEmpty())
		return;

	QStringList args;

	GameInfo *gameInfo = mameGame->nameInfoMap[currentGame];

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
			loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));
			// delete the extracted rom
			connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMergedFinished(int, QProcess::ExitStatus)));
			connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), win, SLOT(toggleTrayIcon(int, QProcess::ExitStatus)));
			win->toggleTrayIcon(0, QProcess::NormalExit, true);
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

	if (mameOpts.contains("language"))
		args << "-language" << language;

	loadProc = procMan->process(procMan->start(mame_binary, args));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), win, SLOT(toggleTrayIcon(int, QProcess::ExitStatus)));
	win->toggleTrayIcon(0, QProcess::NormalExit, true);
}


GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool GameListSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool result = true;
	QModelIndex indexGameDesc, indexGameName, index2;
	const QAbstractItemModel *srcModel = sourceModel();

	indexGameDesc = srcModel->index(sourceRow, COL_DESC, sourceParent);
	indexGameName = srcModel->index(sourceRow, COL_NAME, sourceParent);

	//it's safe to use model functions cuz gameName is not translated
	QString gameName = srcModel->data(indexGameName).toString();

	GameInfo *gameInfo = NULL;
	if (mameGame->nameInfoMap.contains(gameName))
		gameInfo = mameGame->nameInfoMap[gameName];

	bool isSFZCH = gameName == "sfzch";
	bool isConsole = isSFZCH || gameInfo && !gameInfo->nameDeviceInfoMap.isEmpty();
	bool isBIOS = gameInfo->isBios;
	//fixme: cannot use gameInfo->isExtRom
	bool isExtRom = srcModel->data(indexGameDesc, Qt::UserRole + FOLDER_CONSOLE).toBool();

	// filter out BIOS and Console systems
	if (isConsole && !isExtRom && !isSFZCH)
		return false;

	// apply search filter
	if (!searchText.isEmpty())
	{
		QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
		QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);

		result = gameName.contains(regExpSearch)|| 
			utils->getDesc(gameName).contains(regExpSearch);

		// also true if any of a parent's clone matches
		if (!isExtRom && !gameInfo->clones.isEmpty())
		{
			foreach (QString gameName2, gameInfo->clones)
			{
				if (gameName2.contains(regExpSearch) ||
					utils->getDesc(gameName2).contains(regExpSearch))
				{
					result = true;
					break;
				}
			}
		}
	}

	const int role = filterRole();
	switch (role)
	{
	// apply folder filter
	case Qt::UserRole + FOLDER_ALLARC:
		result = result && !isBIOS && !isExtRom && !isConsole;
		break;

	case Qt::UserRole + FOLDER_AVAILABLE:
		result = result && !isBIOS && !isExtRom && !isConsole
			&& (gameInfo->available == 1 || gameInfo->isCloneAvailable);
		break;

	case Qt::UserRole + FOLDER_UNAVAILABLE:
		result = result && !isBIOS && !isExtRom && !isConsole
			&& (gameInfo->available != 1);
		break;

	case Qt::UserRole + FOLDER_CONSOLE:
		result = result && !isBIOS && isExtRom;
		break;

	case Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS:	//hack for console subfolders
		result = result && !isBIOS && gameName == filterText;
		break;

	case Qt::UserRole + FOLDER_MANUFACTURER:
		result = result && !isBIOS && gameInfo->manufacturer == filterText;
		break;

	case Qt::UserRole + FOLDER_YEAR:
	{
		QString year = gameInfo->year;
		if (year.isEmpty())
			year = "?";
		result = result && !isBIOS && year == filterText;
		break;
	}
	case Qt::UserRole + FOLDER_SOURCE:
		result = result && gameInfo->sourcefile == filterText;
		break;

	case Qt::UserRole + FOLDER_BIOS:
		result = result && isBIOS;
		break;

	case Qt::UserRole + FOLDER_BIOS + MAX_FOLDERS:	//hack for bios subfolders
		result = result && !isBIOS && 
			srcModel->data(indexGameDesc, Qt::UserRole + FOLDER_BIOS).toString() == filterText;
		break;

	case Qt::UserRole + FOLDER_EXT:
		result = result && !isBIOS && filterList.contains(gameName);
		break;

	case Qt::UserRole + FOLDER_ALLGAME:
		result = result && !isBIOS;
		break;

	default:
		// empty list otherwise
		result = false;
	}

	//add the games to counter
	if (result)
	{
		if (isExtRom)
			visibleGames.insert(srcModel->data(indexGameName, Qt::UserRole).toString());
		else
			visibleGames.insert(gameName);
	}

	return result;
}

