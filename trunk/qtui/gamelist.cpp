#include "7zCrc.h"
#include "7zFile.h"
#include "7zVersion.h"

#include "7zAlloc.h"
#include "7zExtract.h"
#include "7zIn.h"

#include "gamelist.h"

#include "mamepguimain.h"
#include "mameopt.h"
#include "ips.h"
#include "m1.h"

/* global */
MameGame *mameGame = NULL, *mameGame0 = NULL;
Gamelist *gameList = NULL;
QString currentGame, currentFolder;

//fixme: used in audit
TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;

/* internal */
GamelistDelegate gamelistDelegate(0);
QSet<QString> visibleGames;
static const QString ROOT_FOLDER = "ROOT_FOLDER";

QByteArray defIconDataGreen;
QByteArray defIconDataYellow;
QByteArray defIconDataRed;

QByteArray defMameSnapData;
QByteArray defMessSnapData;

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
private:
	bool metMameTag;

	GameInfo *gameInfo;
	DeviceInfo *deviceInfo;
	QString currentText;
	bool isDefaultRamOption;

public:
	ListXMLHandler(int d = 0)
	{
		gameInfo = 0;
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
		if (!metMameTag && qName != (isMESS ? "mess" : "mame"))
			return false;

		if (qName == (isMESS ? "mess" : "mame"))
		{
			metMameTag = true;
			mameGame->mameVersion = attributes.value("build");
		}
		else if (qName == "game" || qName == "machine")
		{
			//update progress
			static int i;
			gameList->updateProgress(i++);
			qApp->processEvents();
			
			gameInfo = new GameInfo(mameGame);
			gameInfo->sourcefile = attributes.value("sourcefile");
			gameInfo->isBios = attributes.value("isbios") == "yes";
			gameInfo->cloneof = attributes.value("cloneof");
			gameInfo->romof = attributes.value("romof");
			gameInfo->sampleof = attributes.value("sampleof");

			mameGame->games[attributes.value("name")] = gameInfo;
		}
		else if (gameInfo->isBios && qName == "biosset")
		{
			BiosSet *biosSet = new BiosSet(gameInfo);
			biosSet->description = attributes.value("description");
			biosSet->isDefault = attributes.value("default") == "yes";

			gameInfo->biosSets[attributes.value("name")] = biosSet;
		}
		else if (qName == "rom")
		{
			RomInfo *romInfo = new RomInfo(gameInfo);
			romInfo->name = attributes.value("name");
			romInfo->bios = attributes.value("bios");
			romInfo->size = attributes.value("size").toULongLong();
			romInfo->merge = attributes.value("merge");
			romInfo->region = attributes.value("region");
			romInfo->status = attributes.value("status");

			bool ok;
			gameInfo->roms[attributes.value("crc").toUInt(&ok, 16)] = romInfo;
		}
		else if (qName == "disk")
		{
			DiskInfo *diskInfo = new DiskInfo(gameInfo);
			diskInfo->name = attributes.value("name");
			diskInfo->merge = attributes.value("merge");
			diskInfo->region = attributes.value("region");
			diskInfo->index = attributes.value("index").toUShort();
			diskInfo->status = attributes.value("status");

			gameInfo->disks[attributes.value("sha1")] = diskInfo;
		}
		else if (qName == "sample")
		{
			gameInfo->samples << attributes.value("name");
		}
		else if (qName == "chip")
		{
			ChipInfo *chipInfo = new ChipInfo(gameInfo);
			chipInfo->name = attributes.value("name");
			chipInfo->tag = attributes.value("tag");
			chipInfo->type = attributes.value("type");
			chipInfo->clock = attributes.value("clock").toUInt();

			gameInfo->chips.append(chipInfo);
		}
		else if (qName == "display")
		{
			DisplayInfo *displayInfo = new DisplayInfo(gameInfo);
			displayInfo->type = attributes.value("type");
			displayInfo->rotate = attributes.value("rotate");
			displayInfo->flipx = attributes.value("flipx") == "yes";
			displayInfo->width = attributes.value("width").toUShort();
			displayInfo->height = attributes.value("height").toUShort();
			displayInfo->refresh = attributes.value("refresh");
			displayInfo->htotal = attributes.value("htotal").toUShort();
			displayInfo->hbend = attributes.value("hbend").toUShort();
			displayInfo->hbstart = attributes.value("hbstart").toUShort();
			displayInfo->vtotal = attributes.value("vtotal").toUShort();
			displayInfo->vbend = attributes.value("vbend").toUShort();
			displayInfo->vbstart = attributes.value("vbstart").toUShort();

			gameInfo->displays.append(displayInfo);
		}
		else if (qName == "sound")
		{
			gameInfo->channels = attributes.value("channels").toUShort();
		}
		else if (qName == "driver")
		{
			gameInfo->status = utils->getStatus(attributes.value("status"));
			gameInfo->emulation = utils->getStatus(attributes.value("emulation"));
			gameInfo->color = utils->getStatus(attributes.value("color"));
			gameInfo->sound = utils->getStatus(attributes.value("sound"));
			gameInfo->graphic = utils->getStatus(attributes.value("graphic"));
			gameInfo->cocktail = utils->getStatus(attributes.value("cocktail"));
			gameInfo->protection = utils->getStatus(attributes.value("protection"));
			gameInfo->savestate = utils->getStatus(attributes.value("savestate"));
			gameInfo->palettesize = attributes.value("palettesize").toUInt();
		}
		else if (qName == "input")
		{
			gameInfo->service = attributes.value("service") == "yes";
			gameInfo->tilt = attributes.value("tilt") == "yes";
			gameInfo->players = attributes.value("players").toUShort();
			gameInfo->buttons = attributes.value("buttons").toUShort();
			gameInfo->coins = attributes.value("coins").toUShort();
		}
		else if (qName == "control")
		{
			ControlInfo *controlInfo = new ControlInfo(gameInfo);
			controlInfo->type = attributes.value("type");
			controlInfo->minimum = attributes.value("minimum").toUShort();
			controlInfo->maximum = attributes.value("maximum").toUShort();
			controlInfo->sensitivity = attributes.value("sensitivity").toUShort();
			controlInfo->keydelta = attributes.value("keydelta").toUShort();
			controlInfo->reverse = attributes.value("reverse") == "yes";

			gameInfo->controls.append(controlInfo);
		}
		else if (qName == "device")
		{
			deviceInfo = new DeviceInfo(gameInfo);
			deviceInfo->type = attributes.value("type");
			deviceInfo->tag = attributes.value("tag");
			deviceInfo->mandatory = attributes.value("mandatory") == "1";
		}
		else if (deviceInfo && qName == "instance")
		{
			gameInfo->devices.insert(attributes.value("name"), deviceInfo);
		}
		else if (deviceInfo && qName == "extension")
		{
			deviceInfo->extensionNames << attributes.value("name");
		}
		else if (qName == "ramoption")
		{
			if (attributes.value("default") == "1")
				isDefaultRamOption = true;
			else
				isDefaultRamOption = false;
		}

		currentText.clear();
		return true;
	}

	bool endElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName)
	{
		if (qName == "description")
			gameInfo->description = currentText;
		else if (qName == "year")
			gameInfo->year = currentText;
		else if (qName == "manufacturer")
			gameInfo->manufacturer = currentText;
		else if (qName == "ramoption")
		{
			if (isDefaultRamOption)
				gameInfo->defaultRamOption = currentText.toUInt();
			gameInfo->ramOptions << currentText.toUInt();
		}

		return true;
	}

	bool characters(const QString &str)
	{
		currentText += str;
		return true;
	}
};

UpdateSelectionThread::UpdateSelectionThread(QObject *parent)
: QThread(parent)
{
	abort = false;

	QFile icoFile;

	if (defIconDataGreen.isEmpty())
	{
		icoFile.setFileName(":/res/16x16/sqr-g.png");
		icoFile.open(QIODevice::ReadOnly);
		defIconDataGreen = icoFile.readAll();
		icoFile.close();
	}

	if (defIconDataYellow.isEmpty())
	{
		icoFile.setFileName(":/res/16x16/sqr-y.png");
		icoFile.open(QIODevice::ReadOnly);
		defIconDataYellow = icoFile.readAll();
		icoFile.close();
	}

	if (defIconDataRed.isEmpty())
	{
		icoFile.setFileName(":/res/16x16/sqr-r.png");
		icoFile.open(QIODevice::ReadOnly);
		defIconDataRed = icoFile.readAll();
		icoFile.close();
	}

	QFile mameSnapFile(":/res/mamegui/mame.png");
	mameSnapFile.open(QIODevice::ReadOnly);
	defMameSnapData = mameSnapFile.readAll();

	QFile messSnapFile(":/res/mamegui/mess.png");
	messSnapFile.open(QIODevice::ReadOnly);
	defMessSnapData = messSnapFile.readAll();
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

QByteArray UpdateSelectionThread::getScreenshot(const QString &dirpath0, const QString &gameName, int snapType)
{
	QStringList dirpaths = dirpath0.split(";");
	QByteArray snapdata = QByteArray();

	foreach (QString _dirpath, dirpaths)
	{
		QDir dir(_dirpath);
		QString dirpath = utils->getPath(_dirpath);

		// try to load directly	
		QFile snapFile(dirpath + gameName + PNG_EXT);
		if (snapFile.open(QIODevice::ReadOnly))
			snapdata = snapFile.readAll();

		// try to load from built-in names
		if (snapdata.isNull())
		{
			QString zipName;
			switch (snapType)
			{
			case DOCK_SNAP:
				zipName = "snap";
				break;
			case DOCK_FLYER:
				zipName = "flyers";
				break;
			case DOCK_CABINET:
				zipName = "cabinets";
				break;
			case DOCK_MARQUEE:
				zipName = "marquees";
				break;
			case DOCK_TITLE:
				zipName = "titles";
				break;
			case DOCK_CPANEL:
				zipName = "cpanel";
				break;
			case DOCK_PCB:
				zipName = "pcb";
				break;
			}

			QuaZip zip(dirpath + zipName + ZIP_EXT);
			if (zip.open(QuaZip::mdUnzip))
			{
				QuaZipFile zipfile(&zip);
				if (zip.setCurrentFile(gameName + PNG_EXT))
				{
					if (zipfile.open(QIODevice::ReadOnly))
						snapdata = zipfile.readAll();
				}
			}
		}

		// try to add .zip to nearest folder name
		if (snapdata.isNull())
		{
			QuaZip zip(dirpath + dir.dirName() + ZIP_EXT);
			if (zip.open(QuaZip::mdUnzip))
			{
				QuaZipFile zipfile(&zip);
				if (zip.setCurrentFile(gameName + PNG_EXT))
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
		GameInfo *gameInfo = mameGame->games[gameName];
 		if (!gameInfo->cloneof.isEmpty())
			snapdata = getScreenshot(dirpath0, gameInfo->cloneof, snapType);

		// fallback to default image, first getScreenshot() can't reach here
		if (snapdata.isNull())
			snapdata = (isMESS || gameInfo->isExtRom || !gameInfo->devices.isEmpty()) ? 
				defMessSnapData : defMameSnapData;
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

	foreach (QString gameName, mameGame->games.keys())
	{
		GameInfo *gameInfo = mameGame->games[gameName];

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
	GameInfo *gameInfo = mameGame->games[gameName];
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
			{
				if (gameInfo->status == 0)
					icondata = defIconDataRed;
				else if (gameInfo->status == 2)
					icondata = defIconDataYellow;
				else
					icondata = defIconDataGreen;
			}
			else
				icondata = gameInfo->icondata;

			bool isLargeIcon = gameList->listMode == win->actionLargeIcons->objectName().remove("action");
			
			QPixmap pm;
			pm.loadFromData(icondata);

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
	GameInfo *gameInfo = mameGame->games[gameName];

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

BiosSet::BiosSet(QObject *parent)
: QObject(parent)
{
	//	win->log("# BiosSet()");
}

RomInfo::RomInfo(QObject *parent) : 
QObject(parent),
available(false)
{
}

DiskInfo::DiskInfo(QObject *parent) : 
QObject(parent),
available(false)
{
}

ChipInfo::ChipInfo(QObject *parent)
: QObject(parent)
{
}

DisplayInfo::DisplayInfo(QObject *parent)
: QObject(parent)
{
}

ControlInfo::ControlInfo(QObject *parent)
: QObject(parent)
{
}

DeviceInfo::DeviceInfo(QObject *parent) : 
QObject(parent),
isConst(false)
{
	//	win->log("# DeviceInfo()");
}

GameInfo::GameInfo(QObject *parent) :
QObject(parent),
isBios(false),
//hack for displaying status
cocktail(64),
protection(64),
isExtRom(false),
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
		gameInfo = mameGame->games[romof];

		if (!gameInfo->romof.isEmpty())
		{
			biosof = gameInfo->romof;
				if (gameInfo->romof.trimmed()=="")
		win->log("ERR5");
			gameInfo = mameGame->games[gameInfo->romof];
		}
	}
	
	if (gameInfo && gameInfo->isBios)
		return biosof;
	else
		return NULL;
}

QString GameInfo::getDeviceInstanceName(QString type, int)
{
	QMapIterator<QString, DeviceInfo *> it(devices);
	while (it.hasNext())
	{
		it.next();
		DeviceInfo *deviceInfo = it.value();
		QString instanceName = it.key();

		if (deviceInfo->type == type)
			return instanceName;
	}
	return "";
}

DeviceInfo * GameInfo::getDevice(QString type, int)
{
	QMapIterator<QString, DeviceInfo *> it(devices);
	while (it.hasNext())
	{
		it.next();
		DeviceInfo *deviceInfo = it.value();
		QString instanceName = it.key();

		if (deviceInfo->type == type)
			return deviceInfo;
	}
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
	out.setVersion(QDataStream::Qt_4_4);
	out << mameGame->mameVersion;
	out << mameGame->mameDefaultIni;	//default.ini
	out << mameGame->games.count();

	win->log(QString("s11n %1 games").arg(mameGame->games.count()));

	//fixme: should place in thread and use mameGame directly
	gameList->switchProgress(gameList->numTotalGames, tr("Saving listxml"));
	int i = 0;
	foreach (QString gameName, mameGame->games.keys())
	{
		gameList->updateProgress(i++);
		qApp->processEvents();
	
		GameInfo *gameInfo = mameGame->games[gameName];
		out << gameName;

		/* internal */
		out << gameInfo->isExtRom;
		out << gameInfo->available;
		out << gameInfo->extraInfo;

		/* game */
		out << gameInfo->romof;
		out << gameInfo->description;
		out << gameInfo->year;
		out << gameInfo->manufacturer;
		if (!gameInfo->isExtRom)
		{
			out << gameInfo->cloneof;
			out << gameInfo->sourcefile;
			out << gameInfo->isBios;
			out << gameInfo->sampleof;
		}

		/* biosset */
		if (!gameInfo->isExtRom)
		{
			out << gameInfo->biosSets.count();
			foreach (QString name, gameInfo->biosSets.keys())
			{
				BiosSet *biosSet = gameInfo->biosSets[name];
				out << name;
				out << biosSet->description;
				out << biosSet->isDefault;
			}
		}

		/* rom */
		out << gameInfo->roms.count();
		foreach (quint32 crc, gameInfo->roms.keys())
		{
			RomInfo *romInfo = gameInfo->roms[crc];
			out << crc;
			out << romInfo->name;
			out << romInfo->size;
			out << romInfo->status;
			if (!gameInfo->isExtRom)
			{
				out << romInfo->bios;
				out << romInfo->merge;
				out << romInfo->region;
			}
		}

		if (!gameInfo->isExtRom)
		{
			/* disk */
			out << gameInfo->disks.count();
			foreach (QString sha1, gameInfo->disks.keys())
			{
				DiskInfo *diskInfo = gameInfo->disks[sha1];
				out << sha1;
				out << diskInfo->name;
				out << diskInfo->merge;
				out << diskInfo->region;
				out << diskInfo->index;
				out << diskInfo->status;
			}

			/* sample */
			out << gameInfo->samples;

			/* chip */
			out << gameInfo->chips.count();
			foreach (ChipInfo* chipInfo, gameInfo->chips)
			{
				out << chipInfo->name;
				out << chipInfo->tag;
				out << chipInfo->type;
				out << chipInfo->clock;
			}

			/* display */
			out << gameInfo->displays.count();
			foreach (DisplayInfo* displayInfo, gameInfo->displays)
			{
				out << displayInfo->type;
				out << displayInfo->rotate;
				out << displayInfo->flipx;
				out << displayInfo->width;
				out << displayInfo->height;
				out << displayInfo->refresh;
				out << displayInfo->htotal;
				out << displayInfo->hbend;
				out << displayInfo->hbstart;
				out << displayInfo->vtotal;
				out << displayInfo->vbend;
				out << displayInfo->vbstart;
			}

			/* sound */
			out << gameInfo->channels;

			/* input */
			out << gameInfo->service;
			out << gameInfo->tilt;
			out << gameInfo->players;
			out << gameInfo->buttons;
			out << gameInfo->coins;

			out << gameInfo->controls.count();
			foreach (ControlInfo* controlInfo, gameInfo->controls)
			{
				out << controlInfo->type;
				out << controlInfo->minimum;
				out << controlInfo->maximum;
				out << controlInfo->sensitivity;
				out << controlInfo->keydelta;
				out << controlInfo->reverse;
			}

			/* driver */	
			out << gameInfo->status;
			out << gameInfo->emulation;
			out << gameInfo->color;
			out << gameInfo->sound;
			out << gameInfo->graphic;
			out << gameInfo->cocktail;
			out << gameInfo->protection;
			out << gameInfo->savestate;
			out << gameInfo->palettesize;
		}

		/* device */
		bool needSaveDevices = gameInfo->isExtRom ? false : true;
		foreach (DeviceInfo *deviceInfo, gameInfo->devices)
		{
			if (!deviceInfo->mountedPath.isEmpty() && !deviceInfo->isConst)
			{
				needSaveDevices = true;
				break;
			}	
		}

		//hack the devices.size() so that we dont save extroms and only const dev in mountedPath
		int deviceSize = 0;
		if (needSaveDevices)
			deviceSize = gameInfo->devices.size();

		out << deviceSize;

		if (needSaveDevices)
		{
			QMapIterator<QString, DeviceInfo *> it(gameInfo->devices);
			while (it.hasNext())
			{
				it.next();
				out << it.key();
				DeviceInfo *deviceInfo = it.value();

				if (!gameInfo->isExtRom)
				{
					out << deviceInfo->type;
					out << deviceInfo->tag;
					out << deviceInfo->mandatory;
					out << deviceInfo->extensionNames;
				}

				QString mountedPath = deviceInfo->mountedPath;
				if (deviceInfo->isConst)
					mountedPath.clear();
				out << mountedPath;
			}
		}

		/*ramoption */
		if (!gameInfo->isExtRom)
		{
			out << gameInfo->ramOptions;
			out << gameInfo->defaultRamOption;
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
		in.setVersion(QDataStream::Qt_4_4);

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
		GameInfo *gameInfo = new GameInfo(mameGame);
		QString gameName;
		int count;

		in >> gameName;

		/* internal */
		in >> gameInfo->isExtRom;
		in >> gameInfo->available;
		in >> gameInfo->extraInfo;

		/* game */
		in >> gameInfo->romof;
		in >> gameInfo->description;
		in >> gameInfo->year;
		in >> gameInfo->manufacturer;
		if (!gameInfo->isExtRom)
		{
			in >> gameInfo->cloneof;
			in >> gameInfo->sourcefile;
			in >> gameInfo->isBios;
			in >> gameInfo->sampleof;
		}

		/* biosset */
		if (!gameInfo->isExtRom)
		{
			QString name;
			in >> count;
			for (int j = 0; j < count; j++)
			{
				BiosSet *biosSet = new BiosSet(gameInfo);
				in >> name;
				in >> biosSet->description;
				in >> biosSet->isDefault;
				gameInfo->biosSets[name] = biosSet;
			}
		}
		
		/* rom */
		in >> count;
		for (int j = 0; j < count; j++)
		{
			quint32 crc;
			RomInfo *romInfo = new RomInfo(gameInfo);
			in >> crc;
			in >> romInfo->name;
			in >> romInfo->size;
			in >> romInfo->status;
			if (!gameInfo->isExtRom)
			{
				in >> romInfo->bios;
				in >> romInfo->merge;
				in >> romInfo->region;
			}
			gameInfo->roms[crc] = romInfo;
		}

		if (!gameInfo->isExtRom)
		{
			/* disk */
			in >> count;
			for (int j = 0; j < count; j++)
			{
				QString sha1;
				DiskInfo *diskInfo = new DiskInfo(gameInfo);
				in >> sha1;
				in >> diskInfo->name;
				in >> diskInfo->merge;
				in >> diskInfo->region;
				in >> diskInfo->index;
				in >> diskInfo->status;
				gameInfo->disks[sha1] = diskInfo;
			}

			/* sample */
			in >> gameInfo->samples;

			/* chip */
			in >> count;
			for (int j = 0; j < count; j++)
			{
				ChipInfo *chipInfo = new ChipInfo(gameInfo);
				in >> chipInfo->name;
				in >> chipInfo->tag;
				in >> chipInfo->type;
				in >> chipInfo->clock;
				gameInfo->chips.append(chipInfo);
			}

			/* display */
			in >> count;
			for (int j = 0; j < count; j++)
			{
				DisplayInfo *displayInfo = new DisplayInfo(gameInfo);
				in >> displayInfo->type;
				in >> displayInfo->rotate;
				in >> displayInfo->flipx;
				in >> displayInfo->width;
				in >> displayInfo->height;
				in >> displayInfo->refresh;
				in >> displayInfo->htotal;
				in >> displayInfo->hbend;
				in >> displayInfo->hbstart;
				in >> displayInfo->vtotal;
				in >> displayInfo->vbend;
				in >> displayInfo->vbstart;
				gameInfo->displays.append(displayInfo);
			}

			/* sound */
			in >> gameInfo->channels;

			/* input */
			in >> gameInfo->service;
			in >> gameInfo->tilt;
			in >> gameInfo->players;
			in >> gameInfo->buttons;
			in >> gameInfo->coins;

			in >> count;
			for (int j = 0; j < count; j++)
			{
				ControlInfo *controlInfo = new ControlInfo(gameInfo);
				in >> controlInfo->type;
				in >> controlInfo->minimum;
				in >> controlInfo->maximum;
				in >> controlInfo->sensitivity;
				in >> controlInfo->keydelta;
				in >> controlInfo->reverse;
				gameInfo->controls.append(controlInfo);
			}

			/* driver */
			in >> gameInfo->status;
			in >> gameInfo->emulation;
			in >> gameInfo->color;
			in >> gameInfo->sound;
			in >> gameInfo->graphic;
			in >> gameInfo->cocktail;
			in >> gameInfo->protection;
			in >> gameInfo->savestate;
			in >> gameInfo->palettesize;
		}

		/* device */
		in >> count;
		for (int j = 0; j < count; j++)
		{
			DeviceInfo *deviceInfo = new DeviceInfo(gameInfo);
			QString instanceName;
			in >> instanceName;
			if (!gameInfo->isExtRom)
			{
				in >> deviceInfo->type;
				in >> deviceInfo->tag;
				in >> deviceInfo->mandatory;
				in >> deviceInfo->extensionNames;
			}
			in >> deviceInfo->mountedPath;
			gameInfo->devices.insert(instanceName, deviceInfo);
		}

		/*ramoption */
		if (!gameInfo->isExtRom)
		{
			in >> gameInfo->ramOptions;
			in >> gameInfo->defaultRamOption;
		}

		mameGame->games[gameName] = gameInfo;
	}

	win->log(QString("des11n game count %1").arg(gamecount));

	// verify MAME Version
	if (mameGame->mameVersion != mameVersion0)
	{
		win-> log(QString("new mame version: %1 / %2").arg(mameVersion0).arg(mameGame->mameVersion));
		mameGame0 = mameGame;
		return QDataStream::ReadCorruptData;
	}

	completeData();
	return in.status();
}

void MameGame::completeData()
{
	GameInfo *gameInfo, *gameInfo2;

	foreach (QString gameName, mameGame->games.keys())
	{
		gameInfo = mameGame->games[gameName];

		// update mess ext rom info
		if (gameInfo->isExtRom)
		{
			gameInfo2 = mameGame->games[gameInfo->romof];
			gameInfo->sourcefile = gameInfo2->sourcefile;
		}

		// update clone list
		if (!gameInfo->cloneof.isEmpty())
		{
			gameInfo2 = mameGame->games[gameInfo->cloneof];
			gameInfo2->clones.insert(gameName);
			
			if (!gameInfo2->isCloneAvailable && gameInfo->available == 1)
				gameInfo2->isCloneAvailable = true;
		}
	}

}


GamelistDelegate::GamelistDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize GamelistDelegate::sizeHint (const QStyleOptionViewItem & option, 
								  const QModelIndex & index) const
{
	QString gameName = gameList->getViewString(index, COL_NAME);
	GameInfo *gameInfo = mameGame->games[gameName];
	//fixme: combine @ console gamename
	if (!gameInfo->devices.isEmpty())
	{
		QString gameName2 = gameList->getViewString(index, COL_NAME + COL_LAST);

		if (!gameName2.isEmpty())
		{
			gameInfo = mameGame->games[gameName2];
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
	GameInfo *gameInfo = mameGame->games[gameName];

	//fixme: combine @ console gamename
	// override gameName and gameInfo for console roms
	if (!gameInfo->devices.isEmpty())
	{
		QString gameName2 = gameList->getViewString(index, COL_NAME + COL_LAST);
		if (!gameName2.isEmpty())
		{
			gameInfo = mameGame->games[gameName2];
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
			{
				if (gameInfo->status == 0)
					icondata = defIconDataRed;
				else if (gameInfo->status == 2)
					icondata = defIconDataYellow;
				else
					icondata = defIconDataGreen;
			}
			else
				icondata = gameInfo->icondata;
			pm.loadFromData(icondata);

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
menuContext(NULL),
headerMenu(NULL),
autoAudit(false),
loadIconStatus(0),
defaultGameListDelegate(NULL)
{
	connect(&selectionThread, SIGNAL(snapUpdated(int)), this, SLOT(setupSnap(int)));
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
	if (hasInitd && mameGame->games.contains(currentGame))
	{
		selectionThread.myqueue.enqueue(currentGame);
		selectionThread.update();
	}
}

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		//fixme: merge with filter accept
		QString gameName = getViewString(current, COL_NAME);
		if (gameName.isEmpty())
			return;

		QString gameDesc = getViewString(current, COL_DESC);
		GameInfo *gameInfo = mameGame->games[gameName];

		if (!gameInfo->devices.isEmpty())
		{
			QString gameName2 = getViewString(current, COL_NAME + COL_LAST);
			if (!gameName2.isEmpty())
			{
				gameInfo = mameGame->games[gameName2];
				if (gameInfo && gameInfo->isExtRom)
					gameName = gameName2;
			}
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
		currentGame = mameGame->games.keys().first();

//	win->log("currentGame: " + currentGame);
}

void Gamelist::restoreGameSelection()
{
	if (!gameListModel || !gameListPModel)
		return;

	if (!mameGame->games.contains(currentGame))
		return;

	QString gameName = currentGame;
	QModelIndex i, pi;

	// select current game
	GameInfo *gameinfo = mameGame->games[gameName];
	//fixme: should consider other columns
	i = gameListModel->index(COL_DESC, gameinfo->pModItem);
	win->log("restore callback: " + gameName);

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

void Gamelist::disableCtrls()
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
	
	// these are reenabled in gameList->init()
	win->enableCtrls(false);
	
	//delete model
	if (gameListModel)
	{
		delete gameListModel;
		gameListModel = NULL;
	}

	//delete proxy model
	if (gameListPModel)
	{
		delete gameListPModel;
		gameListPModel = NULL;
	}
}

void Gamelist::init(bool toggleState, int initMethod)
{
	//filter toggled(false) SIGNAL from button
	if (!toggleState)
		return;

	folderList.clear();
	folderList
		<< tr("All Games")
		<< (isMESS ? tr("All Systems") : tr("All Arcades"))
		<< (isMESS ? tr("Available Systems") : tr("Available Arcades"))
		<< (isMESS ? tr("Unavailable Systems") : tr("Unavailable Arcades"))
		<< (isMESS ? tr("Softwares") : tr("Consoles"))
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
		//validate currentGame
		if (!mameGame->games.contains(currentGame))
			currentGame = mameGame->games.keys().first();
	
		disableCtrls();

		//init the model
		gameListModel = new TreeModel(win, isGroup);
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

			// load mame.ini overrides
			optUtils->loadIni(OPTLEVEL_GLOBAL, (isMESS ? "mess" INI_EXT : "mame" INI_EXT));

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

//		if (autoAudit)
//		{
//			win->romAuditor.audit(true);
//			return;
//		}

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
		if (initMethod == GAMELIST_INIT_FULL || initMethod == GAMELIST_INIT_AUDIT)
		{
			QByteArray column_state;

			// restore view column state, needed on first init and after auditing, but not for folder switching
			if (guiSettings.value("column_state").isValid())
				column_state = guiSettings.value("column_state").toByteArray();
			else
				column_state = defSettings.value("column_state").toByteArray();
			
			win->tvGameList->header()->restoreState(column_state);
			qApp->processEvents();
			restoreFolderSelection();
		}

		//sorting 
		win->tvGameList->setSortingEnabled(true);

		if (isLView)
			win->lvGameList->setFocus();
		else
			win->tvGameList->setFocus();

		//fixme: hack to update snapshot_directory for non-Windows
		optUtils->preUpdateModel(NULL, OPTLEVEL_GLOBAL);

		// attach menus
		initMenus();

		// everything is done, enable ctrls now
		win->enableCtrls(true);

		// load icon in a background thread
		loadIcon();

		hasInitd = true;

		//for re-init list from folders
		restoreGameSelection();

		win->log(QString("init'd game count %1").arg(mameGame->games.count()));
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
			nameFilter << "*" ICO_EXT;
			
			// iterate all files in the path
			QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
			for (int i = 0; i < files.count(); i++)
			{
				QString gameName = files[i].toLower().remove(ICO_EXT);
				if (mameGame->games.contains(gameName))
				{
					gameInfo = mameGame->games[gameName];
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
			QuaZip zip(dirpath + "icons" ZIP_EXT);

			if(!zip.open(QuaZip::mdUnzip))
				continue;

			QuaZipFileInfo info;
			QuaZipFile zipFile(&zip);
			for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
			{
				if(!zip.getCurrentFileInfo(&info))
					continue;

				QString gameName = info.name.toLower().remove(ICO_EXT);
				if (mameGame->games.contains(gameName))
				{
					gameInfo = mameGame->games[gameName];
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
			foreach (QString gameName, mameGame->games.keys())
			{
				gameInfo = mameGame->games[gameName];

				// get clone icons from parent
				if (!gameInfo->isExtRom && gameInfo->icondata.isNull() && !gameInfo->cloneof.isEmpty())
				{
					gameInfo2 = mameGame->games[gameInfo->cloneof];
					if (!gameInfo2->icondata.isNull())
					{
						gameInfo->icondata = gameInfo2->icondata;
//						emit icoUpdated(gameName);
					}
				}

				// get ext rom icons from system
				if (gameInfo->isExtRom && gameInfo->icondata.isNull())
				{
					gameInfo2 = mameGame->games[gameInfo->romof];
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
	if (hasLanguage)
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

	foreach (QString gameName, mameGame->games.keys())
	{
		GameInfo *gameInfo = mameGame->games[gameName];
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

	// init context menuContext, we don't need to init it twice
	if (menuContext == NULL)
	{
		menuContext = new QMenu(win);
	
		menuContext->addAction(win->actionPlay);
		menuContext->addAction(win->actionRecord);
		menuContext->addSeparator();
		menuContext->addAction(win->actionAudit);
		menuContext->addSeparator();
		menuContext->addAction(win->actionSrcProperties);
		menuContext->addAction(win->actionProperties);
	}

	QWidget *w;

	if (isLView)
		w = win->lvGameList;
	else
		w = win->tvGameList;

	w->setContextMenuPolicy(Qt::CustomContextMenu);
	disconnect(w, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));
	connect(w, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint &)));

	disconnect(menuContext, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
	connect(menuContext, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

	disconnect(win->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));
	connect(win->menuFile, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

	//init tvGameList header context menuContext
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
    menuContext->popup(win->tvGameList->mapToGlobal(p));
}

void Gamelist::updateContextMenu()
{
	if (!mameGame->games.contains(currentGame))
		return;

	QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];

	QPixmap pm;
	pm.loadFromData(gameInfo->icondata, "ico");
	QIcon icon(pm);

	win->actionPlay->setEnabled(true);
	win->actionPlay->setIcon(icon);
    win->actionPlay->setText(tr("Play %1").arg(gameInfo->description));

	win->actionSrcProperties->setText(tr("Properties for %1").arg(gameInfo->sourcefile));

	updateDynamicMenu(win->menuFile);
	updateDynamicMenu(menuContext);
}

void Gamelist::updateDynamicMenu(QMenu *rootMenu)
{
	const QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];

	//update IPS menu
	rootMenu->removeAction(win->actionConfigIPS);
	if (hasIPS && ipsUI->checkAvailable(gameName))
		rootMenu->insertAction(win->actionSrcProperties, win->actionConfigIPS);

	//remove existing device menus
	QList<QAction *>rootMenuActions = rootMenu->actions();
	foreach (QAction *action, rootMenuActions)
	{
		if (action->objectName().startsWith("actionDevice_"))
		{
			rootMenu->removeAction(action);
			delete action;
		}
	}

	//construct devices for ext roms
	if (gameInfo->isExtRom)
	{
		//only one const device possible
		bool constDeviceFound = false;

		const QMap<QString, DeviceInfo *> systemDevices = mameGame->games[gameInfo->romof]->devices;
		QMapIterator<QString, DeviceInfo *> it(systemDevices);
		while (it.hasNext())
		{
			it.next();
			DeviceInfo *systemDeviceInfo = it.value();
			QString instanceName = it.key();

			DeviceInfo *deviceInfo;

			if (!gameInfo->devices.isEmpty() && gameInfo->devices.contains(instanceName))
				deviceInfo = gameInfo->devices[instanceName];
			else
				deviceInfo = new DeviceInfo(gameInfo);
			deviceInfo->type = systemDeviceInfo->type;
			deviceInfo->tag = systemDeviceInfo->tag;
			deviceInfo->mandatory = systemDeviceInfo->mandatory;
			deviceInfo->extensionNames = systemDeviceInfo->extensionNames;
			gameInfo->devices.insert(instanceName, deviceInfo);

			//set const device that is loaded by GUI
			if (!constDeviceFound)
			{
				foreach (QString extension, deviceInfo->extensionNames)
				{
					if (gameName.endsWith("." + extension, Qt::CaseInsensitive))
					{
						deviceInfo->mountedPath = gameName;
						deviceInfo->isConst = true;
						constDeviceFound = true;
						break;
					}
				}
			}
		}
	}

	//dont append device menu when there's no device
	if (gameInfo->devices.isEmpty())
		return;

	//dont append device menu when there's only 1 const device
	if (gameInfo->isExtRom && gameInfo->devices.size() == 1)
		return;

	//test if the device need a submenu
	QStringList deviceTypes, deviceTypesSubmenu;
	foreach (DeviceInfo *deviceInfo, gameInfo->devices)
	{
		if (deviceTypes.contains(deviceInfo->type))
		{
			if (!deviceTypesSubmenu.contains(deviceInfo->type))
				deviceTypesSubmenu.append(deviceInfo->type);
		}
		else
			deviceTypes.append(deviceInfo->type);
	}

	//append device menus
	QMapIterator<QString, DeviceInfo *> it(gameInfo->devices);
	while (it.hasNext())
	{
		it.next();
		DeviceInfo *deviceInfo = it.value();
		QString instanceName = it.key();
		QString strDeviceType = QString("actionDevice_type_%1").arg(deviceInfo->type);

		QMenu *menuDeviceType = NULL;
		QAction *actionDeviceType = NULL;

		//find existing menu to place submenus
		rootMenuActions = rootMenu->actions();
		foreach (QAction *action, rootMenuActions)
		{
			if (action->objectName() == strDeviceType)
			{
				actionDeviceType = action;
				menuDeviceType = actionDeviceType->menu();
				break;
			}
		}

		//otherwise, create new submenu
		if (actionDeviceType == NULL && deviceTypesSubmenu.contains(deviceInfo->type))
		{
			menuDeviceType = new QMenu(rootMenu);
			menuDeviceType->setTitle(utils->capitalizeStr(deviceInfo->type));
			
			actionDeviceType = menuDeviceType->menuAction();
			actionDeviceType->setObjectName(strDeviceType);
			rootMenu->insertAction(win->actionSrcProperties, actionDeviceType);
		}

		//insert submenu items
		//fixme parent
		QMenu *menuDevice = new QMenu(menuDeviceType);

		//generate device string
		QString strDevice = "[Empty slot]";
		if (!deviceInfo->mountedPath.isEmpty())
		{
			if (deviceInfo->isConst)
				strDevice = gameInfo->description;
			else
			{
				QFileInfo fi(deviceInfo->mountedPath);
				strDevice = fi.completeBaseName();
			}
		}

		menuDevice->setTitle(QString("%1%2: %3")
			.arg(utils->capitalizeStr(instanceName))
			.arg(deviceInfo->mandatory ? " *" : "")
			.arg(strDevice)
			);

		
		QAction *actionDevice = menuDevice->menuAction();
		actionDevice->setObjectName(QString("actionDevice_%1").arg(instanceName));

		if (deviceTypesSubmenu.contains(deviceInfo->type))
			menuDeviceType->addAction(actionDevice);
		else
			rootMenu->insertAction(win->actionSrcProperties, actionDevice);

		QAction *actionMount = new QAction("Mount...", actionDevice);
		QAction *actionUnmount = new QAction("Unmount...", actionDevice);
		menuDevice->addAction(actionMount);
		menuDevice->addAction(actionUnmount);
		if (deviceInfo->isConst)
		{
			actionMount->setEnabled(false);
			actionUnmount->setEnabled(false);
		}
		else
		{
			if (deviceInfo->mountedPath.isEmpty())
				actionUnmount->setEnabled(false);

			connect(actionMount, SIGNAL(triggered()), this, SLOT(mountDevice()));
			connect(actionUnmount, SIGNAL(triggered()), this, SLOT(unmountDevice()));
		}

#if 1
		//see MESS device.c for all device types
		QAction *actionDeviceIcon;
		if (deviceTypesSubmenu.contains(deviceInfo->type))
			actionDeviceIcon = actionDeviceType;
		else
			actionDeviceIcon = actionDevice;

		if (deviceInfo->type ==  "floppydisk")
			actionDeviceIcon->setIcon(QIcon(":/res/16x16/media-floppy.png"));
		else if (deviceInfo->type ==  "harddisk")
			actionDeviceIcon->setIcon(QIcon(":/res/16x16/drive-harddisk.png"));
		else if (deviceInfo->type ==  "printer")
			actionDeviceIcon->setIcon(QIcon(":/res/16x16/printer.png"));
		else if (deviceInfo->type ==  "cdrom")
			actionDeviceIcon->setIcon(QIcon(":/res/16x16/media-optical.png"));
#endif
	}

	//fixme: parent
	QAction *actionSeparator = new QAction(0);
	actionSeparator->setObjectName("actionDevice_separator");
	actionSeparator->setSeparator(true);
	rootMenu->insertAction(win->actionSrcProperties, actionSeparator);
}

void Gamelist::mountDevice()
{
	QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];
	QString instanceName = ((QAction*)sender())->parent()->objectName().remove("actionDevice_");
	DeviceInfo *deviceInfo = NULL;

	//find the currentDevice
	foreach (QString key, gameInfo->devices.keys())
	{
		if (key == instanceName)
		{
			deviceInfo = gameInfo->devices[instanceName];
			break;
		}
	}

	QStringList nameFilter;
	foreach (QString ext, deviceInfo->extensionNames)
		nameFilter << "*." + ext;
	nameFilter << "*" ZIP_EXT;

	QString filter;
	filter.append(tr("Common image types") + " (" +  nameFilter.join(" ") + ")");
	filter.append(";;");
	filter.append(tr("All Files") + " (*)");

	if (gameInfo->isExtRom)
		gameName = gameInfo->romof;

	QString _dirpath = mameOpts[gameName + "_extra_software"]->globalvalue;
	QString fileName = QFileDialog::getOpenFileName(0, tr("File name:"), _dirpath, filter);
	
	if (deviceInfo == NULL || fileName.isEmpty())
		return;

	deviceInfo->mountedPath = fileName;
}

void Gamelist::unmountDevice()
{
	QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];
	QString instanceName = ((QAction*)sender())->parent()->objectName().remove("actionDevice_");

	foreach (QString key, gameInfo->devices.keys())
	{
		if (key == instanceName)
		{
			DeviceInfo *deviceInfo = gameInfo->devices[instanceName];
			deviceInfo->mountedPath.clear();
			break;
		}
	}

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

	autoAudit = true;

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

//	optUtils->loadDefault(mameGame->mameDefaultIni);

	//reload gameList. this is a chained call from loadListXmlFinished()
	init(true, GAMELIST_INIT_FULL);
//	win->log("end of gameList->loadDefFin()");
}

// extract a rom from the merged file
void Gamelist::extractMerged(QString mergedFileName, QString fileName)
{
	currentTempROM = QDir::tempPath() + "/" + fileName;

//fixme: wrap to a class
//fixme: break the loop when file extracted


	CFileInStream archiveStream;
	CLookToRead lookStream;
	CSzArEx db;
	SRes res;
	ISzAlloc allocImp;
	ISzAlloc allocTempImp;

	if (InFile_Open(&archiveStream.file,  qPrintable(mergedFileName)))
	{
		win->log("can not open: " + mergedFileName);
		return;
	}

	FileInStream_CreateVTable(&archiveStream);
	LookToRead_CreateVTable(&lookStream, False);

	lookStream.realStream = &archiveStream.s;
	LookToRead_Init(&lookStream);

	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	CrcGenerateTable();

	SzArEx_Init(&db);
	res = SzArEx_Open(&db, &lookStream.s, &allocImp, &allocTempImp);
	if (res == SZ_OK)
	{
		UInt32 i;
		
		/*
		if you need cache, use these 3 variables.
		if you use external function, you can make these variable as static.
		*/
		UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
		Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
		size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
		
		for (i = 0; i < db.db.NumFiles; i++)
		{
			size_t offset;
			size_t outSizeProcessed;
			CSzFileItem *f = db.db.Files + i;

			if (f->IsDir)
				continue;

			if (f->Name != fileName)
				continue;

			res = SzAr_Extract(&db, &lookStream.s, i,
				&blockIndex, &outBuffer, &outBufferSize,
				&offset, &outSizeProcessed,
				&allocImp, &allocTempImp);
		  
			if (res != SZ_OK)
				break;

			CSzFile outFile;
			size_t processedSize;
			  
			if (OutFile_Open(&outFile, qPrintable(QDir::tempPath() + "/" + fileName)))
			{
				//PrintError("can not open output file");
				res = SZ_ERROR_FAIL;
				break;
			}

			processedSize = outSizeProcessed;
			if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 ||
				processedSize != outSizeProcessed)
			{
				// PrintError("can not write output file");
				res = SZ_ERROR_FAIL;
				break;
			}
			
			if (File_Close(&outFile))
			{
				//PrintError("can not close output file");
				res = SZ_ERROR_FAIL;
				break;
			}

			//success
			break;
		}
		IAlloc_Free(&allocImp, outBuffer);
	}
	SzArEx_Free(&db, &allocImp);
	File_Close(&archiveStream.file);

	runMame(true);
}

// delete the extracted rom
void Gamelist::runMergedFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	QFile tmpMergedFile(currentTempROM);
	tmpMergedFile.remove();
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

	GameInfo *gameInfo, *gameInfo0;
	foreach (QString gameName, mameGame->games.keys())
	{
		gameInfo = mameGame->games[gameName];

		// restore previous audit results
		if (mameGame0 && mameGame0->games.contains(gameName))
		{
			gameInfo0 = mameGame0->games[gameName];
			gameInfo->available = gameInfo0->available;
		}
	}

	mameGame->completeData();

	// restore previous console audit
	if (mameGame0)
	{
		foreach (QString gameName, mameGame0->games.keys())
		{
			gameInfo0 = mameGame0->games[gameName];

			if (gameInfo0->isExtRom && 
				//the console is supported by current mame version
				mameGame->games.contains(gameInfo0->romof))
			{
				gameInfo = new GameInfo(mameGame);
				gameInfo->description = gameInfo0->description;
				gameInfo->isExtRom = true;
				gameInfo->romof = gameInfo0->romof;
				gameInfo->sourcefile = gameInfo0->sourcefile;
				gameInfo->available = 1;
				mameGame->games[gameName] = gameInfo;
			}
		}

		delete mameGame0;
	}

	delete pxmlInputSource;
	mameOutputBuf.clear();
}

void Gamelist::filterSearchCleared()
{
	if (win->lineEditSearch->text().count() < 1)
		return;

	win->lineEditSearch->setText("");
	qApp->processEvents();
	filterSearchChanged();
}

//apply searchText
void Gamelist::filterSearchChanged()
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

	// update Refresh menuContext text
	QString folder;
	if (isConsoleFolder())
		folder = currentFolder;
	else
		folder = folderList[FOLDER_ALLARC];
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
		{
			//if we are in an empty MESS system folder, assign currentGame to the system
//			currentGame = filterText;
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS);	//hack for console subfolders
		}
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
	QStringList consoleList, mftrList, yearList, srcList, biosList;
	GameInfo *gameInfo;
	foreach (QString gameName, mameGame->games.keys())
	{
		gameInfo = mameGame->games[gameName];
		if (!gameInfo->devices.isEmpty())
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
	
	win->treeFolders->clear();
	for (int i = 0; i < folderList.size(); i++)
	{
		items.append(new QTreeWidgetItem(win->treeFolders, QStringList(folderList[i])));

		win->treeFolders->addTopLevelItems(items);
		items[i]->setIcon(0, icoFolder);

		if (i == FOLDER_ALLGAME)
		{
//			if (!isMAMEPlus)
				items[i]->setHidden(true);
		}
		else if (i == FOLDER_CONSOLE)
		{
			foreach (QString consoleName, consoleList)
			{
				QTreeWidgetItem *subItem = new QTreeWidgetItem(items[i], QStringList(consoleName));
				items[i]->addChild(subItem);

				QString path = consoleName + "_extra_software";
				if (!guiSettings.contains(path) || guiSettings.value(path).toString().isEmpty())
					subItem->setHidden(true);
			}
		}

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
				GameInfo *gameInfo = mameGame->games[name];
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

		QStringList folderFiles = dir.entryList((QStringList() << "*" INI_EXT), QDir::Files | QDir::Readable);
		
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

				if (!mameGame->games.contains(gameName))
					continue;

				//append parent for standalone clones
				GameInfo *gameInfo = mameGame->games[gameName];
				if (!gameInfo->cloneof.isEmpty() && !gameListPModel->filterList.contains(gameInfo->cloneof))
					gameListPModel->filterList.append(gameInfo->cloneof);
			}
		}
	}
}

void Gamelist::restoreFolderSelection(bool isForce)
{
	//if currentFolder has been set, it's a folder switching call
	if(!currentFolder.isEmpty() && !isForce)
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
						win->log(QString("treeb.gamecount %1").arg(mameGame->games.count()));
						return;
					}
				}
			}
		}
	}
	//fallback
	win->treeFolders->setCurrentItem(rootItem->child(FOLDER_ALLARC));
}

bool Gamelist::isAuditFolder(QString consoleName)
{
	QStringList paths = currentFolder.split("/");
	if (paths.size() == 2)
	{
		if (paths[1] == gameList->folderList[FOLDER_CONSOLE])
			return true;

		else if(paths[1] == consoleName)
			return true;
	}

	return false;		
}

bool Gamelist::isConsoleFolder()
{
	QStringList paths = currentFolder.split("/");
	if (paths.size() == 2)
	{
		if (paths[1] == gameList->folderList[FOLDER_CONSOLE])
			return true;

		else if (mameGame->games.contains(paths[1]))
		{
			GameInfo *gameInfo = mameGame->games[paths[1]];
			if (!gameInfo->devices.isEmpty())
				return true;
		}
	}

	return false;
}

void Gamelist::runMame(bool hasTempRom, QStringList playArgs)
{
	//block multi mame session for now
	//if (procMan->procCount > 0)
	//	return;

	QStringList args;
	args << playArgs;

	if (hasLanguage)
	{
		QString langpath = utils->getPath(mameOpts["langpath"]->globalvalue);
		args << "-langpath" << langpath;
		args << "-language" << language;
	}

	//force update ext rom fields
	updateDynamicMenu(win->menuFile);

	const QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];

	if (gameInfo->devices.isEmpty())
	{
		//MAME game
		args << gameName;
	}
	else
	// run MESS roms, add necessary params
	{
		// extract merged rom
		if (!hasTempRom && gameName.contains(SZIP_EXT "/"))
		{
			QStringList paths = gameName.split(SZIP_EXT "/");
			extractMerged(paths[0] + SZIP_EXT, paths[1]);
			return;
		}

		// MESS system
		QString systemName = gameName;
		if (gameInfo->isExtRom)
			systemName = gameInfo->romof;
		args << systemName;

		// MESS device
		bool tempRomLoaded = false;
		QString warnings = "";
		foreach (QString instanceName, gameInfo->devices.keys())
		{
			DeviceInfo *deviceInfo = gameInfo->devices[instanceName];

			if (!deviceInfo->mountedPath.isEmpty())
			{
				args << "-" + instanceName;
				if (hasTempRom && !tempRomLoaded)
				{
					args << currentTempROM;
					tempRomLoaded = true;
				}
				else
					args << deviceInfo->mountedPath;
			}
			else
			{
				if (deviceInfo->mandatory)
					warnings.append(instanceName + "\n");
			}
		}

		if (warnings.size() > 0)
		{
			win->poplog(tr("%1 requires that these device(s)\nmust be mounted:\n\n")
				.arg(systemName) + warnings + "\ncouldn't start MESS.");
			return;
		}
	}

//	win->poplog(args.join(" "));

	loadProc = procMan->process(procMan->start(mame_binary, args, !hasTempRom));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMameFinished(int, QProcess::ExitStatus)));
	// delete the extracted rom
	if (hasTempRom)
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMergedFinished(int, QProcess::ExitStatus)));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), win, SLOT(toggleTrayIcon(int, QProcess::ExitStatus)));
	win->toggleTrayIcon(0, QProcess::NormalExit, true);
}

void Gamelist::runMameFinished(int, QProcess::ExitStatus)
{
	QFile tmpNvFile(utils->getPath(QDir::tempPath()) + currentGame + ".nv");
	tmpNvFile.remove();
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
	QString gameNameExtRom = srcModel->data(indexGameName, Qt::UserRole).toString();
	//must use desc from view value for ext roms
	QString gameDesc = srcModel->data(indexGameDesc).toString();

	GameInfo *gameInfo = mameGame->games[gameName];
	if (!gameInfo->devices.isEmpty() && !gameNameExtRom.isEmpty())
		gameInfo = mameGame->games[gameNameExtRom];

	bool isSFZCH = gameName == "sfzch";
	bool isConsole = isSFZCH || !gameInfo->devices.isEmpty();
	bool isBIOS = gameInfo->isBios;
	bool isExtRom = gameInfo->isExtRom;

	// apply search filter
	if (!searchText.isEmpty())
	{
		QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
		QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);

		result = gameName.contains(regExpSearch)|| 
				 gameDesc.contains(regExpSearch);

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
		result = result && !isBIOS && !isExtRom;
		if (!isMESS)
			result = result && !isConsole;
		break;

	case Qt::UserRole + FOLDER_AVAILABLE:
		result = result && !isBIOS && !isExtRom
			&& (gameInfo->available == 1 || gameInfo->isCloneAvailable);
		if (!isMESS)
			result = result && !isConsole;		
		break;

	case Qt::UserRole + FOLDER_UNAVAILABLE:
		result = result && !isBIOS && !isExtRom
			&& (gameInfo->available != 1);
		if (!isMESS)
			result = result && !isConsole;
		break;

	case Qt::UserRole + FOLDER_CONSOLE:
		result = result && !isExtRom && isConsole;
		break;

	case Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS:	//hack for console subfolders
		result = result && isExtRom && gameName == filterText;
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
			visibleGames.insert(gameNameExtRom);
		else
			visibleGames.insert(gameName);
	}

	return result;
}
