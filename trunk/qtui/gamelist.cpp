#include "gamelist.h"

#include "mamepguimain.h"
#include "mameopt.h"
#include "ips.h"
#include "m1.h"

/* global */
MameGame *mameGame = NULL;
Gamelist *gameList = NULL;
QString currentGame, currentFolder;
QStringList hiddenFolders;
QMap<QString, QString> consoleMap;


//fixme: used in audit
TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;

/* internal */
MameGame *mameGame0 = NULL;
GamelistDelegate gamelistDelegate(0);
QSet<QString> visibleGames;
QMultiMap<QString, QString> extFolderMap;
QStringList deleteCfgFiles;
QMap<QString, QString> biosMap;

QByteArray defIconDataGreen;
QByteArray defIconDataYellow;
QByteArray defIconDataRed;

QByteArray defMameSnapData;
QByteArray defMessSnapData;

#define ROOT_FOLDER "ROOT_FOLDER"
#define EXTFOLDER_MAGIC "**00_"
#define STR_DELCFG "actionDelCfg_"
#define STR_TOGGLE_FOLDER "actionToggleFolder_"
#define STR_EXTSFOLDER "actionExtSubFolder_"
#define STR_EXTFOLDER "actionExtFolder_"

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
	// construct dockInfo	
	struct DockInfo
	{
		QString optName;
		QString fileName;
		int type;
		QString title;
		QString *buffer;
	};

	static const DockInfo _dockInfoList[] =
	{
		{ "history_file",	"history.dat",	DOCK_HISTORY,	"History",		&historyText },
		{ "mameinfo_file",	"mameinfo.dat", DOCK_MAMEINFO,	"MAMEInfo",		&mameinfoText },
		{ "mameinfo_file",	"mameinfo.dat", DOCK_DRIVERINFO,"DriverInfo",	&driverinfoText },
		{ "story_file", 	"story.dat",	DOCK_STORY, 	"Story",		&storyText },
		{ "command_file",	"command.dat",	DOCK_COMMAND,	"Command",		&commandText },
		{ NULL }
	};

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

		QString path, localPath;

		const DockInfo *dockInfoList = _dockInfoList;

		/* loop over entries until we hit a NULL name */
		for ( ; dockInfoList->optName != NULL; dockInfoList++)
		{
			if (!abort && win->tbHistory->isVisible() && win->isDockTabVisible(dockInfoList->title))
			{
				if (hasLanguage)
					localPath = utils->getPath(mameOpts["langpath"]->globalvalue);

				dockInfoList->buffer->clear();
			
				path = dockInfoList->fileName;

				if (!localPath.isEmpty())
				{
					localPath = localPath + language + "/" + path;

					*dockInfoList->buffer = getHistory(localPath, gameName, dockInfoList->type + DOCK_LAST /*hack for local*/);
					if (!dockInfoList->buffer->isEmpty())
						dockInfoList->buffer->append("<hr>");
				}

				if (mameOpts.contains(dockInfoList->optName))
					path = mameOpts[dockInfoList->optName]->globalvalue;

				//we don't want to display the same dat twice
				if (localPath != path)
					dockInfoList->buffer->append(getHistory(path, gameName, dockInfoList->type));

				//special handling for command
				if (dockInfoList->type == DOCK_COMMAND)
				{
					convertCommand(commandText);
/*
					//fixme: font hack, should be removed
					if (language.startsWith("zh_") || language.startsWith("ja_"))
					{
						QFont font;
						font.setFamily("MS Gothic");
						font.setFixedPitch(true);
						win->tbCommand->setFont(font);
		//				win->tbCommand->setLineWrapMode(QTextEdit::NoWrap);
					}
*/
				}

				emit snapUpdated(dockInfoList->type);
			}
		}
	}
}

QString UpdateSelectionThread::getHistory(const QString &fileName, const QString &gameName, int method)
{
	QString buf = "";

	QString searchTag = gameName;
	GameInfo *gameInfo = mameGame->games[searchTag];
	if (gameInfo->isExtRom)
	{
		searchTag = gameInfo->romof;
//		gameInfo = mameGame->games[searchTag];
	}
	if (method == DOCK_DRIVERINFO)
		searchTag = gameInfo->sourcefile;

	QFileInfo fileInfo(fileName);
	QStringList paths = utils->split2Str(fileInfo.absoluteFilePath(), "/", true);

	QHash<QString, MameFileInfo *> mameFileInfoList = 
		utils->iterateMameFile(paths.first(), "", paths.last(), MAMEFILE_READ);

	if (mameFileInfoList.size() > 0)
	{
		QTextStream in(mameFileInfoList[mameFileInfoList.keys().first()]->data);
		in.setCodec("UTF-8");

		bool isFound, recData = false;
		QString line;

		do
		{
			line = in.readLine();
			if (!line.startsWith("#"))
			{
				if (line.startsWith("$"))
				{
					if (line.startsWith("$info="))
					{
						isFound = false;
						line.remove(0, 6);	//remove $info=
						QStringList tags = line.split(',');

						foreach (QString tag, tags)
						{
							//found the entry, start recording
							if (tag == searchTag)
							{
								recData = true;
								isFound = true;
								break;
							}
						}

						// reach another entry, stop recording
						if (!isFound && recData)
						{
							recData = false;
							//finished
							break;
						}
					}
					else if (recData && line.startsWith("$<a href="))
					{
						line.remove(0, 1);	//remove $
						line.replace("<a href=", QString("<a style=\"color:") + (isDarkBg ? "#00a0e9" : "#006d9f") + "\" href=");
						buf += line;
						buf += "<br>";
					}
//					else if (recData)
//						buf += "<br>";

				}
				else if (recData)
				{
					buf += line;
					buf += "<br>";
				}
			}
		}
		while (!line.isNull());
	}

	utils->clearMameFileInfoList(mameFileInfoList);

	buf = buf.trimmed();

	if (buf.isEmpty() && mameGame->games.contains(searchTag))
	{
		gameInfo = mameGame->games[searchTag];
		if (!gameInfo->cloneof.isEmpty())
			buf = getHistory(fileName, gameInfo->cloneof, method);
	}
	else if (method == DOCK_HISTORY)
		buf.prepend(QString("<a style=\"color:") + (isDarkBg ? "#00a0e9" : "#006d9f") + 
			"\" href=\"http://maws.mameworld.info/maws/romset/" + searchTag + "\">View information at MAWS</a><br>");

	//post process redundant break lines
	while (buf.startsWith("<br>"))
		buf.remove(0, 4);

	while (buf.endsWith("<br>"))
		buf.remove(buf.size() - 4, 4);

	return buf;
}

void UpdateSelectionThread::convertCommand(QString &commandText)
{
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
}

QByteArray UpdateSelectionThread::getScreenshot(const QString &_dirPaths, const QString &gameName, int snapType)
{
	QByteArray snapdata = QByteArray();

	// prepare built-in names
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

	zipName = zipName.append(";.");
	QString dirPaths = _dirPaths;
	QString fileNameFilters = gameName + PNG_EXT;
	// try to load from patterns
	if (snapType == DOCK_SNAP && mameOpts.contains("snapname"))
	{
		QString pattern = mameOpts["snapname"]->currvalue;

		pattern.replace("%g", gameName);
		pattern.replace("%i", "0000");

		QStringList dirPathList = _dirPaths.split(";");
		foreach (QString _dirPath, dirPathList)
		{
			QDir dir(_dirPath);
			QString dirPath = utils->getPath(_dirPath);
			QFileInfo fileInfo(dirPath + pattern + PNG_EXT);
			dirPaths.append(";" + fileInfo.absolutePath());
			fileNameFilters.append(";" + fileInfo.fileName());
		}
	}

	QHash<QString, MameFileInfo *> mameFileInfoList = 
		utils->iterateMameFile(dirPaths, 
		zipName, 
		fileNameFilters,
		MAMEFILE_READ);

	if (mameFileInfoList.size() > 0)
		snapdata = mameFileInfoList[mameFileInfoList.keys().first()]->data;

	utils->clearMameFileInfoList(mameFileInfoList);

	if (!snapdata.isNull())
		return snapdata;

	// recursively load parent image
	GameInfo *gameInfo = mameGame->games[gameName];
		if (!gameInfo->cloneof.isEmpty())
		snapdata = getScreenshot(_dirPaths, gameInfo->cloneof, snapType);

	// fallback to default image, first getScreenshot() can't reach here
	if (snapdata.isNull())
		snapdata = (isMESS || gameInfo->isExtRom || !gameInfo->devices.isEmpty()) ? 
			defMessSnapData : defMameSnapData;

	return snapdata;
}


/* a copy of Qt example itemviews/simpletreemodel */
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
			TreeItem *parent = setupModelData(rootItem, gameName, isGroup);

			// build clones
			if (isGroup)
				foreach (QString cloneName, gameInfo->clones)
					setupModelData(parent, cloneName, isGroup);
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
	if (!hasIndex(row, column, parent))
		return QModelIndex();
	
	TreeItem *parentItem;
	
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());
	
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

	TreeItem *childItem = static_cast<TreeItem*>(index.internalPointer());
	TreeItem *parentItem = childItem->parent();
	
	if (parentItem == rootItem)
		return QModelIndex();
	
	return createIndex(parentItem->row(), 0, parentItem);
}

//mandatory
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeItem *item = getItem(index);
	const QString gameName = item->data(COL_NAME).toString();
	GameInfo *gameInfo = mameGame->games[gameName];
	int col = index.column();

	switch (role)
	{
	case Qt::ForegroundRole:
		if (gameInfo->emulation == 0 && !gameInfo->isExtRom)
			return qVariantFromValue(QColor(isDarkBg ? QColor(255, 96, 96) : Qt::darkRed));
		else
			return qVariantFromValue(QColor((isDarkBg) ? Qt::white : Qt::black));

	case Qt::DecorationRole:
		if (col == COL_DESC)
		{
			QByteArray icondata;

			if (gameInfo->icondata.isNull())
			{
				if (gameInfo->isExtRom || gameInfo->status == 1)
					icondata = defIconDataGreen;
				else if (gameInfo->status == 2)
					icondata = defIconDataYellow;
				else
					icondata = defIconDataRed;
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

	//convert 'Name' column for ext roms
	case Qt::UserRole:
		if (col == COL_NAME && gameInfo->isExtRom)
			return item->data(col);
		break;

	case Qt::DisplayRole:
		switch (col)
		{
		case COL_DESC:
			if (local_game_list && !gameInfo->lcDesc.isEmpty())
				return gameInfo->lcDesc;
			break;

		case COL_MFTR:
			if (local_game_list && !gameInfo->lcMftr.isEmpty())
				return gameInfo->lcMftr;
			break;

		case COL_YEAR:
			if (gameInfo->year.isEmpty())
				return "?";
			break;

		case COL_NAME:
			if (gameInfo->isExtRom)
				return gameInfo->romof;
			break;

		//convert 'ROMs' column
		case COL_ROM:
			switch (item->data(COL_ROM).toInt())
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

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
		return rootItem->data(section);

	return QVariant();
}

//mandatory
int TreeModel::rowCount(const QModelIndex &parent) const
{
	TreeItem *parentItem;
	if (parent.column() > 0)
		return 0;
	
	if (!parent.isValid())
		parentItem = rootItem;
	else
		parentItem = static_cast<TreeItem*>(parent.internalPointer());
	
	return parentItem->childCount();
}

//mandatory
int TreeModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
	else
		return rootItem->columnCount();
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

TreeItem * TreeModel::setupModelData(TreeItem *parent, QString gameName, bool isGroup)
{
	GameInfo *gameInfo = mameGame->games[gameName];

	if (gameName.trimmed() == "")
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
isHorz(true),
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
//		if (romof.trimmed() == "")
//			win->log("ERR4");
		gameInfo = mameGame->games[romof];

		if (!gameInfo->romof.isEmpty())
		{
			biosof = gameInfo->romof;
//			if (gameInfo->romof.trimmed() == "")
//				win->log("ERR5");
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


MameGame::MameGame(QObject *parent) : 
QObject(parent),
loadProc(NULL),
numTotalGames(-1)
{
//	win->log("MameGame()");
}

void MameGame::init(int method)
{
	int des11n_status = QDataStream::Ok;

	//des11n() is called only on startup, mameGame has been constructed in MainWindow()
	static bool hasDes11n = false;
	if (!hasDes11n)
	{
		des11n_status = des11n();
		hasDes11n = true;
	}

	//des11n() failed, construct a new mameGame
	//fixme: not good to use global mameGame ptr here
	if (des11n_status != QDataStream::Ok || method != 0)
	{
		//save a copy of des11n mameGame
		mameGame0 = mameGame;
		mameGame = new MameGame(0);

		QStringList args;
		args << "-listxml";

		loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));

		connect(loadProc, SIGNAL(readyReadStandardOutput()), mameGame, SLOT(loadListXmlReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), mameGame, SLOT(loadListXmlFinished(int, QProcess::ExitStatus)));
	}
	else
		gameList->init(true, GAMELIST_INIT_FULL);
}

void MameGame::s11n()
{
//	win->log("start s11n()");

	QDir().mkpath(CFG_PREFIX + "cache");
	QFile file(CFG_PREFIX + "cache/gamelist.cache");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_4);
	out << mameVersion;
	out << mameDefaultIni;	//default.ini
	out << games.count();

	win->log(QString("s11n %1 games").arg(games.count()));

	gameList->switchProgress(numTotalGames, tr("Saving listxml"));
	int i = 0;
	foreach (QString gameName, games.keys())
	{
		gameList->updateProgress(i++);
		qApp->processEvents();
	
		GameInfo *gameInfo = games[gameName];
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

	// MAME Version
	mameVersion = utils->getMameVersion();
	QString mameVersion0;
	in >> mameVersion0;

	// default mame.ini text
	in >> mameDefaultIni;
	
	int gamecount;
	in >> gamecount;

	for (int i = 0; i < gamecount; i++)
	{
		GameInfo *gameInfo = new GameInfo(this);
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

		games.insert(gameName, gameInfo);
	}

	win->log(QString("des11n %1 games from cache.").arg(gamecount));

	// verify MAME Version
	if (mameVersion != mameVersion0)
	{
		win-> log(QString("new MAME version: %1 vs %2").arg(mameVersion0).arg(mameVersion));
		return QDataStream::ReadCorruptData;
	}

	return completeData() | in.status();
}

void MameGame::parseListXml()
{
	ListXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QXmlInputSource *pXmlInputSource = new QXmlInputSource();
	pXmlInputSource->setData(mameOutputBuf);

//	win->log("DEBUG: Gamelist::start parseListXml()");
	
	gameList->switchProgress(numTotalGames, tr("Parsing listxml"));
	reader.parse(*pXmlInputSource);
	gameList->switchProgress(-1, "");

	GameInfo *_gameInfo, *gameInfo0;
	bool noAutoAudit = false;

	// restore previous audit results and other info
	foreach (QString gameName, games.keys())
	{
		_gameInfo = games[gameName];

		if (mameGame0 != NULL && mameGame0->games.contains(gameName))
		{
			gameInfo0 = mameGame0->games[gameName];

			_gameInfo->available = gameInfo0->available;
			if (_gameInfo->available)
				noAutoAudit = true;
			
			if (!gameInfo0->icondata.isEmpty())
				_gameInfo->icondata = gameInfo0->icondata;
			if (!gameInfo0->lcDesc.isEmpty())
				_gameInfo->lcDesc = gameInfo0->lcDesc;
			if (!gameInfo0->lcMftr.isEmpty())
				_gameInfo->lcMftr = gameInfo0->lcMftr;
		}
	}

	gameList->autoAudit = !noAutoAudit;

	completeData();

	// restore previous audit results for ext roms
	if (mameGame0 != NULL)
	{
		foreach (QString gameName, mameGame0->games.keys())
		{
			gameInfo0 = mameGame0->games[gameName];

			if (gameInfo0->isExtRom && 
				//the console is supported by current mame version
				games.contains(gameInfo0->romof))
			{
				_gameInfo = new GameInfo(this);
				_gameInfo->description = gameInfo0->description;
				_gameInfo->isExtRom = true;
				_gameInfo->romof = gameInfo0->romof;
				_gameInfo->sourcefile = gameInfo0->sourcefile;
				_gameInfo->available = 1;
				games[gameName] = _gameInfo;
			}
		}
		delete mameGame0;
		mameGame0 = NULL;
	}

	delete pXmlInputSource;
	mameOutputBuf.clear();
}

int MameGame::completeData()
{
	GameInfo *_gameInfo, *gameInfo2;

	foreach (QString gameName, games.keys())
	{
		_gameInfo = games[gameName];

		// update mess ext rom info
		if (_gameInfo->isExtRom)
		{
			gameInfo2 = games[_gameInfo->romof];
			_gameInfo->sourcefile = gameInfo2->sourcefile;
		}

		// update clone list
		if (!_gameInfo->cloneof.isEmpty())
		{
			if (!games.contains(_gameInfo->cloneof))
				return QDataStream::ReadCorruptData;
		
			gameInfo2 = games[_gameInfo->cloneof];
			gameInfo2->clones.insert(gameName);
			
			if (!gameInfo2->isCloneAvailable && _gameInfo->available == 1)
				gameInfo2->isCloneAvailable = true;
		}
		
		// update horz/vert
		if (_gameInfo->isExtRom)
			gameInfo2 = games[_gameInfo->romof];
		else
			gameInfo2 = _gameInfo;

		if (!gameInfo2->displays.isEmpty() && 
			(gameInfo2->displays.first()->rotate == "90" || gameInfo2->displays.first()->rotate == "270"))
			_gameInfo->isHorz = false;
	}

	return QDataStream::Ok;
}

void MameGame::loadListXmlReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString buf = proc->readAllStandardOutput();
	
	//mamep: remove windows endl
	buf.replace(QString("\r"), QString(""));
	
	numTotalGames += buf.count("<game name=");
	mameOutputBuf += buf;

	win->logStatus(QString(tr("Loading listxml: %1 games")).arg(numTotalGames));
}

void MameGame::loadListXmlFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	parseListXml();

	QStringList args;
	args << "-showconfig" << "-noreadconfig";

	mameDefaultIni.clear();
	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));
	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadDefaultIniReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadDefaultIniFinished(int, QProcess::ExitStatus)));
	//reload gameList. this is a chained call from loadListXmlFinished()
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), gameList, SLOT(update()));
}

void MameGame::loadDefaultIniReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	//hack for Mac OS X, readAllStandardOutput() may not readAll
	mameDefaultIni.append(proc->readAllStandardOutput());
}

void MameGame::loadDefaultIniFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);
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
				if (gameInfo->isExtRom || gameInfo->status == 1)
					icondata = defIconDataGreen;
				else if (gameInfo->status == 2)
					icondata = defIconDataYellow;
				else
					icondata = defIconDataRed;
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
loadProc(NULL),
menuContext(NULL),
headerMenu(NULL),
autoAudit(false),
hasInitd(false),
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
	if (gameListModel == NULL || gameListPModel == NULL || !hasInitd)
		return;

	if (!mameGame->games.contains(currentGame))
		return;

	//fixme: should consider other columns
	// select current game
	GameInfo *gameInfo = mameGame->games[currentGame];

	QModelIndex i, pi;
	i = gameListModel->index(COL_DESC, gameInfo->pModItem);

	if (i.isValid())
		pi = gameListPModel->mapFromSource(i);

	// select first row otherwise
	if (!pi.isValid())
		pi = gameListPModel->index(0, 0, QModelIndex());

	if (!pi.isValid())
		return;

//	win->log("restore callback: " + currentGame);

	bool isLView = false;
	if (win->actionLargeIcons->isChecked())
		isLView = true;

	//fixme: time consuming
	if (isLView)
	{
		win->lvGameList->setCurrentIndex(pi);
		win->lvGameList->scrollTo(pi, QAbstractItemView::PositionAtCenter);
		win->lvGameList->setFocus();
	}
	else
	{
		win->tvGameList->setCurrentIndex(pi);
		win->tvGameList->scrollTo(pi, QAbstractItemView::PositionAtCenter);
		win->tvGameList->setFocus();
	}

	win->labelGameCount->setText(tr("%1 games").arg(visibleGames.count()));

	//auto collapse other folders
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
//			win->log("co: " + (*it)->text(0) + ", " + folderName);
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

	case DOCK_DRIVERINFO:
		win->tbDriverinfo->setHtml(selectionThread.driverinfoText);
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

void Gamelist::update(int initMethod)
{
	init(true, initMethod);
}

void Gamelist::init(bool toggleState, int initMethod)
{
	//filter toggled(false) SIGNAL from button
	if (!toggleState)
		return;

	//have to init here instead of in the constructor, after isMESS has been assigned
	if (!hasInitd)
		intFolderNames0
			<< QT_TR_NOOP("All Games")
			<< (isMESS ? QT_TR_NOOP("All Systems") : QT_TR_NOOP("All Arcades"))
			<< (isMESS ? QT_TR_NOOP("Available Systems") : QT_TR_NOOP("Available Arcades"))
			<< (isMESS ? QT_TR_NOOP("Unavailable Systems") : QT_TR_NOOP("Unavailable Arcades"))
			<< (isMESS ? QT_TR_NOOP("Softwares") : QT_TR_NOOP("Consoles"))
			<< QT_TR_NOOP("Manufacturer")
			<< QT_TR_NOOP("Year")
			<< QT_TR_NOOP("Driver")
			<< QT_TR_NOOP("BIOS")
			<< QT_TR_NOOP("CPU")
			<< QT_TR_NOOP("Sound")
			<< QT_TR_NOOP("CHD")
			<< QT_TR_NOOP("Samples")
			<< QT_TR_NOOP("Dumping Status")
			<< QT_TR_NOOP("Working")
			<< QT_TR_NOOP("Not working")
			<< QT_TR_NOOP("Originals")
			<< QT_TR_NOOP("Clones")
			<< QT_TR_NOOP("Resolution")
			<< QT_TR_NOOP("Colors")
			<< QT_TR_NOOP("Refresh Rate")
			<< QT_TR_NOOP("Display")
			<< QT_TR_NOOP("Control Type")
			<< QT_TR_NOOP("Channels")
			<< QT_TR_NOOP("Save State")
			/*
			<< QT_TR_NOOP("Emulation Status")
			<< QT_TR_NOOP("Artwork")
			*/
			;

	foreach (QString folderName, intFolderNames0)
		intFolderNames << tr(qPrintable(folderName));

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
	win->show();

	if (initMethod == GAMELIST_INIT_FULL)
	{
		/* init everything else here after we have mameGame */

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

			if (pGuiSettings->contains(optName))
				pMameOpt->globalvalue = pGuiSettings->value(optName).toString();
		}

		// we're ready to set version info
		if (!hasInitd)
			win->setVersion();
	}

	//localization must be loaded after init of options so that proper lang directory can be located
	if (!hasInitd || initMethod == GAMELIST_INIT_DRIVER)
	{
		loadMMO(UI_MSG_LIST);
		loadMMO(UI_MSG_MANUFACTURE);
	}

	// init folders must be called after init of localization so that folder names are translated
	if (initMethod == GAMELIST_INIT_FULL && !hasInitd)
		initFolders();

	// auto audit will shortcircuit gameList->init() and must be the last thing in gameList->init()
	if (autoAudit)
	{
		win->romAuditor.audit(true);
		return;
	}

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
		if (pGuiSettings->value("column_state").isValid())
			column_state = pGuiSettings->value("column_state").toByteArray();
		else
			column_state = defSettings.value("column_state").toByteArray();
		
		win->tvGameList->header()->restoreState(column_state);
		restoreFolderSelection();
	}

	//sorting
	win->tvGameList->setSortingEnabled(true);

	//fixme: hack to update snapshot_directory for non-Windows
	optUtils->preUpdateModel(NULL, OPTLEVEL_GLOBAL);

	if (!hasInitd)
	{
		// attach menus
		initMenus();

		// load icon in a background thread
		loadIcon();
	}

	// everything is done, enable ctrls now
	win->enableCtrls(true);

	//save fixdat
	win->romAuditor.exportDat();

	hasInitd = true;
	win->log(QString("init'd %1 games").arg(mameGame->games.count()));

	//for re-init list from folders
	restoreGameSelection();
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
	GameInfo *gameInfo, *gameInfo2;

	QHash<QString, MameFileInfo *> mameFileInfoList = 
		utils->iterateMameFile(mameOpts["icons_directory"]->globalvalue, "icons", "*" ICO_EXT, MAMEFILE_READ);

	foreach (QString key, mameFileInfoList.keys())
	{
		QString gameName = key;
		gameName.chop(4 /* sizeof ICO_EXT */);
		if (mameGame->games.contains(gameName))
		{
			gameInfo = mameGame->games[gameName];
			gameInfo->icondata = mameFileInfoList[key]->data;
		}
	}

	utils->clearMameFileInfoList(mameFileInfoList);

	//complete data
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
//				emit icoUpdated(gameName);
			}
		}

		// get ext rom icons from system
		if (gameInfo->isExtRom && gameInfo->icondata.isNull())
		{
			gameInfo2 = mameGame->games[gameInfo->romof];
			if (!gameInfo2->icondata.isNull())
			{
				gameInfo->icondata = gameInfo2->icondata;
//				emit icoUpdated(gameName);
			}
		}					
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
//		win->log("not exist: " + dirpath + language + "/" + msgFileName[msgCat] + ".mmo");
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
		menuContext->addMenu(win->menuDeleteCfg);
		menuContext->addSeparator();
		menuContext->addMenu(win->menuAddtoFolder);
		menuContext->addAction(win->actionRemoveFromFolder);
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

	//play menu
	QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];

	QPixmap pm;
	pm.loadFromData(gameInfo->icondata, "ico");
	QIcon icon(pm);

	win->actionPlay->setIcon(icon);
    win->actionPlay->setText(tr("Play %1")
		.arg(gameInfo->isExtRom ? gameInfo->description : gameName));
    
	//remove cfg menu
	updateDeleteCfgMenu (gameName);

	//ext folder menu
	QString extFolderName, extSubFolderName;
	QStringList paths = utils->split2Str(currentFolder, "/");
	
	if (paths.first().isEmpty())
	{
		extFolderName = paths.last();
		extSubFolderName = ROOT_FOLDER;
	}
	else
	{
		extFolderName = paths.first();
		extSubFolderName = paths.last();
	}

	QString folderPath = utils->getSinglePath(pGuiSettings->value("folder_directory", "folders").toString(), extFolderName + INI_EXT);
	QFile inFile(folderPath);
//	win->log(folderPath);
	const bool isAccessable = inFile.exists() && inFile.permissions() & QFile::WriteUser;

	win->actionRemoveFromFolder->setText(tr("Remove From \"%1%2\"")
		.arg(extFolderName)
		.arg((extSubFolderName != ROOT_FOLDER) ? "/" + extSubFolderName : ""));
	win->actionRemoveFromFolder->setEnabled(isAccessable);

	//prop menu
	win->actionSrcProperties->setText(tr("Properties for %1").arg(gameInfo->sourcefile));

	updateDynamicMenu(win->menuFile);
	updateDynamicMenu(menuContext);
}

void Gamelist::updateDynamicMenu(QMenu *rootMenu)
{
	const QString gameName = currentGame;
	GameInfo *gameInfo = mameGame->games[gameName];

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

	//update custom folder menu

	//update IPS menu
	rootMenu->removeAction(win->actionConfigIPS);
	if (hasIPS && ipsUI->checkAvailable(gameName))
		rootMenu->insertAction(win->actionSrcProperties, win->actionConfigIPS);

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

void Gamelist::updateDeleteCfgMenu(const QString &gameName)
{
	QAction *actionMenuItem;
	DiskInfo *diskInfo;
	GameInfo *gameInfo = mameGame->games[currentGame];
	QString path;
	
	win->menuDeleteCfg->clear();
	win->menuDeleteCfg->setEnabled(false);
	deleteCfgFiles.clear();

	//ini file
	if (mameOpts.contains("inipath"))
	{
		QStringList dirPaths = mameOpts["inipath"]->currvalue.split(";");

		foreach (path, dirPaths)
		{
			addDeleteCfgMenu(utils->getPath(path), currentGame + ".ini");

			if (!gameInfo->cloneof.isEmpty())
				addDeleteCfgMenu(utils->getPath(path), gameInfo->cloneof + ".ini");

			QString biosof = gameInfo->biosof();
			if (!biosof.isEmpty())
				addDeleteCfgMenu(utils->getPath(path), biosof + ".ini");
		}
	}

	//cfg file
	if (mameOpts.contains("cfg_directory"))
	{
		path = mameOpts["cfg_directory"]->currvalue;
		addDeleteCfgMenu(utils->getPath(path), currentGame + ".cfg");
	}
 
 	//nvram file
	if (mameOpts.contains("nvram_directory"))
	{
		path = mameOpts["nvram_directory"]->currvalue;
		addDeleteCfgMenu(utils->getPath(path), currentGame + ".nv");
	}
   
	//Diff file
	if (mameOpts.contains("diff_directory"))
	{
		path  = mameOpts["diff_directory"]->currvalue;

		foreach (QString sha1, mameGame->games[currentGame]->disks.keys())
		{
			diskInfo = mameGame->games[currentGame]->disks[sha1];
			addDeleteCfgMenu(utils->getPath(path), diskInfo->name + ".dif");
		}
	}

	//All folder
	win->menuDeleteCfg->addSeparator();

	actionMenuItem = new QAction(tr("Remove All"), win->menuDeleteCfg->menuAction());
	actionMenuItem->setObjectName(QString(STR_DELCFG "all"));

	win->menuDeleteCfg->addAction(actionMenuItem);
	connect(actionMenuItem, SIGNAL(triggered()), this, SLOT(deleteCfg()));
}

void Gamelist::addDeleteCfgMenu(const QString &path, const QString &fileName)
{
	QAction *actionMenuItem;
	const QString fullPath = path + fileName;
	QFile file(fullPath);
 
	if (file.exists())
	{
		actionMenuItem = new QAction(fullPath, win->menuDeleteCfg->menuAction());
		deleteCfgFiles << fullPath;
		actionMenuItem->setObjectName(QString(STR_DELCFG "%1").arg(fullPath));
		actionMenuItem->setEnabled(file.permissions() & QFile::WriteUser);
		win->menuDeleteCfg->addAction(actionMenuItem);
		connect(actionMenuItem, SIGNAL(triggered()), this, SLOT(deleteCfg()));

		win->menuDeleteCfg->setEnabled(true);
	}
}

void Gamelist::deleteCfg()
{
	const QString itemName = ((QAction*)sender())->objectName();

	for (int i = 0 ; i < deleteCfgFiles.size() ; i++)
	{
		const QString fileName = deleteCfgFiles.at(i);
		if (itemName == QString(STR_DELCFG "%1").arg(fileName) ||  itemName == STR_DELCFG "all")
		{
			QFile delFile(fileName);
			delFile.remove();
		}
	}
}

void Gamelist::toggleDelegate(bool isHilite)
{
	if (isHilite)
		win->tvGameList->setItemDelegate(&gamelistDelegate);
	else if (defaultGameListDelegate != NULL)
		win->tvGameList->setItemDelegate(defaultGameListDelegate);
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
	  * _current == NULL means this call is from a signal, only process this case
	  */

	/*
	if (_current == NULL &&
		(currentFolder == "/" + intFolderNames[FOLDER_ALLGAME] || 
		currentFolder == "/" + intFolderNames[FOLDER_ALLARC] || 
		currentFolder == "/" + intFolderNames[FOLDER_AVAILABLE] || 
		currentFolder == "/" + intFolderNames[FOLDER_UNAVAILABLE] || 
		currentFolder == "/" + intFolderNames[FOLDER_CONSOLE] ||
		visibleGames.count() > 10000))
	{
		win->log("hack to reinit list");
		init(true, GAMELIST_INIT_DIR);
		return;
	}
	//*/

	visibleGames.clear();

	// update Refresh menuContext text
	QString folder;
	if (isConsoleFolder())
		folder = currentFolder;
	else
		folder = intFolderNames[FOLDER_ALLARC];
	win->actionRefresh->setText(tr("Refresh").append(": ").append(folder));

	gameListPModel->filterList.clear();

	QString folderName;
	//root folder
	if (current->parent() == NULL)
	{
		folderName = current->text(0);
		filterText.clear();

		if (folderName == intFolderNames[FOLDER_ALLGAME])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAME);
		else if (folderName == intFolderNames[FOLDER_ALLARC])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLARC);
		else if (folderName == intFolderNames[FOLDER_AVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_AVAILABLE);
		else if (folderName == intFolderNames[FOLDER_UNAVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_UNAVAILABLE);
		else if (folderName == intFolderNames[FOLDER_CONSOLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONSOLE);
		else if (folderName == intFolderNames[FOLDER_BIOS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS);
		else if (folderName == intFolderNames[FOLDER_HARDDISK])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_HARDDISK);
		else if (folderName == intFolderNames[FOLDER_SAMPLES])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SAMPLES);
		else if (folderName == intFolderNames[FOLDER_WORKING])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_WORKING);
		else if (folderName == intFolderNames[FOLDER_NONWORKING])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_NONWORKING);
		else if (folderName == intFolderNames[FOLDER_ORIGINALS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ORIGINALS);
		else if (folderName == intFolderNames[FOLDER_CLONES])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CLONES);
		else if (folderName == intFolderNames[FOLDER_SAVESTATE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SAVESTATE);
		else if (extFolderNames.contains(folderName))
		{
			initExtFolders(folderName, EXTFOLDER_MAGIC ROOT_FOLDER);
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

		if (folderName == intFolderNames[FOLDER_CONSOLE])
		{
			//override filterText desc
			filterText = consoleMap[filterText];
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONSOLE + MAX_FOLDERS);	//hack for console subfolders
		}
		else if (folderName == intFolderNames[FOLDER_MANUFACTURER])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_MANUFACTURER);
		else if (folderName == intFolderNames[FOLDER_YEAR])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_YEAR);
		else if (folderName == intFolderNames[FOLDER_SOURCE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SOURCE);
		else if (folderName == intFolderNames[FOLDER_BIOS])
		{
			//override filterText desc
			filterText = biosMap[filterText];
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS + MAX_FOLDERS);	//hack for bios subfolders
		}
		else if (folderName == intFolderNames[FOLDER_HARDDISK])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_HARDDISK + MAX_FOLDERS);
		else if (folderName == intFolderNames[FOLDER_CPU])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CPU);
		else if (folderName == intFolderNames[FOLDER_SND])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SND);
		else if (folderName == intFolderNames[FOLDER_DUMPING])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_DUMPING);		
		else if (folderName == intFolderNames[FOLDER_RESOLUTION])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_RESOLUTION);
		else if (folderName == intFolderNames[FOLDER_PALETTESIZE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_PALETTESIZE);
		else if (folderName == intFolderNames[FOLDER_REFRESH])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_REFRESH);
		else if (folderName == intFolderNames[FOLDER_DISPLAY])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_DISPLAY);
		else if (folderName == intFolderNames[FOLDER_CONTROLS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CONTROLS);
		else if (folderName == intFolderNames[FOLDER_CHANNELS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_CHANNELS);
		else if (extFolderNames.contains(folderName)) 
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

//called only once
void Gamelist::initFolders()
{
	GameInfo *gameInfo;

	QStringList	mftrList, 
				yearList, 
				srcList, 
				statusList, 
				regionList,
				cpuList, 
				audioList, 
				controlList, 
				displayList, 
				refreshList;
	QList<quint8>	channelsList, 
					playersList;
	QList<quint32>	palettesizeList; 
	QMap<quint32, QString> resolutionMap;

	foreach (QString gameName, mameGame->games.keys())
	{
		gameInfo = mameGame->games[gameName];
		QString item;
		const QString gameDesc = utils->getDesc(gameName);

		//console
		if (!gameInfo->devices.isEmpty())
			consoleMap.insert(gameDesc, gameName);

		//bios
		if (gameInfo->isBios)
			biosMap.insert(gameDesc, gameName);

		//manufacturer
		if (!mftrList.contains(gameInfo->manufacturer))
			mftrList << gameInfo->manufacturer;

		//year
		item = gameInfo->year;
		if (item.isEmpty())
			item = "?";
		if (!yearList.contains(item))
			yearList << item;

		//the following does not apply to ExtRoms
		if (gameInfo->isExtRom)
			continue;

		//sourcefile
		if (!srcList.contains(gameInfo->sourcefile))
			srcList << gameInfo->sourcefile;

		//palettesize
		if (!palettesizeList.contains(gameInfo->palettesize))
			palettesizeList << gameInfo->palettesize;

		//channels
		if (!channelsList.contains(gameInfo->channels))
			 channelsList << gameInfo->channels;

		foreach (ChipInfo* chipInfo, gameInfo->chips)
		{
			//cpu chips
			if (!cpuList.contains(chipInfo->name) && chipInfo->type == "cpu")
				cpuList << chipInfo->name;

			//audio chips
			if (!audioList.contains(chipInfo->name) && chipInfo->type == "audio")
				audioList << chipInfo->name;
		}

		//players
		if (!playersList.contains(gameInfo->players))
			 playersList << gameInfo->players;

		//rom status
		foreach (RomInfo* romsInfo, gameInfo->roms)
			if (!statusList.contains(romsInfo->status) && !romsInfo->status.isEmpty())
				statusList << romsInfo->status;

		//disk status
		foreach (DiskInfo* disksInfo, gameInfo->disks)
		{	
			if (!statusList.contains(disksInfo->status) && !disksInfo->status.isEmpty())
				statusList << disksInfo->status;

			//disk region
			if (!regionList.contains(disksInfo->region) && !disksInfo->region.isEmpty())
				regionList << disksInfo->region;
		}

		for (int i = 0; i < gameInfo->displays.size(); i++)
		{	
			DisplayInfo* displaysInfo = gameInfo->displays[i];
			//pixelSize is used to sort the resolution
			quint32 pixelSize = displaysInfo->width * displaysInfo->height;
			//distinguish horz and vert resolution, both should be even numbers
			if (!gameInfo->isHorz)
				pixelSize++;

			//display type
			if (!displayList.contains(displaysInfo->type))
				displayList << displaysInfo->type;

			//refresh
			if (!refreshList.contains(displaysInfo->refresh))
				refreshList << displaysInfo->refresh;

			//resolution
			if (!resolutionMap.contains(pixelSize) && displaysInfo->type != "vector")
				resolutionMap.insert(pixelSize, getResolution(gameInfo, i));
		}

		//control type
		foreach (ControlInfo *controlsInfo, gameInfo->controls)
			if (!controlList.contains(controlsInfo->type))
				controlList << controlsInfo->type;
	}

	//sort subfolder items
	mftrList.sort();
	yearList.sort();
	srcList.sort();
	statusList.sort();
	regionList.sort();
	cpuList.sort();
	audioList.sort();
	controlList.sort();
	displayList.sort();
	refreshList.sort();
	qSort(channelsList);
	qSort(playersList);
	qSort(palettesizeList);

	static QIcon icoFolder(":/res/32x32/folder.png");

	hiddenFolders = pGuiSettings->value("hide_folders").toStringList();
	if (hiddenFolders.isEmpty())
		hiddenFolders << intFolderNames0[FOLDER_ALLGAME];
	hiddenFolders.removeDuplicates();
	//remove not recognized values
	foreach (QString folderName, hiddenFolders)
		if (!intFolderNames0.contains(folderName))
			hiddenFolders.removeOne(folderName);

	win->treeFolders->clear();
	for (int i = 0; i < intFolderNames.size(); i++)
	{
		intFolderItems.append(new QTreeWidgetItem(win->treeFolders, QStringList(intFolderNames[i])));

		win->treeFolders->addTopLevelItems(intFolderItems);
		intFolderItems[i]->setIcon(0, icoFolder);

		if (hiddenFolders.contains(intFolderNames0[i]))
			intFolderItems[i]->setHidden(true);

		if (i == FOLDER_CONSOLE)
		{
			foreach (QString name, consoleMap.keys())
			{
				QTreeWidgetItem *subItem = new QTreeWidgetItem(intFolderItems[i], QStringList(name));
				intFolderItems[i]->addChild(subItem);

				QString path = consoleMap[name] + "_extra_software";
				if (!pGuiSettings->contains(path) || pGuiSettings->value(path).toString().isEmpty())
					subItem->setHidden(true);
			}
		}

		else if (i == FOLDER_MANUFACTURER)
			foreach (QString name, mftrList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));

		else if (i == FOLDER_YEAR)
			foreach (QString name, yearList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));

		else if (i == FOLDER_SOURCE)
			foreach (QString name, srcList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));

		else if (i == FOLDER_BIOS)
			foreach (QString name, biosMap)
			{
/*				QPixmap pm;
				pm.loadFromData(gameInfo->icondata, "ico");
				QIcon icon(pm);
*/
				QTreeWidgetItem *item = new QTreeWidgetItem(intFolderItems[i], QStringList(mameGame->games[name]->description));
//				item->setIcon(0, icon);

				intFolderItems[i]->addChild(item);
			}
		else if (i == FOLDER_HARDDISK)
			foreach (QString name, regionList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_CPU)
			foreach (QString name, cpuList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_SND)
			foreach (QString name, audioList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_DUMPING)
			foreach (QString name, statusList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_DISPLAY)
		{
			foreach (QString name, displayList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
		
			intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(tr("Horizontal"))));
			intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(tr("Vertical"))));
		}

		else if (i == FOLDER_REFRESH)
			foreach (QString name, refreshList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name + " Hz")));

		else if (i == FOLDER_RESOLUTION)
			foreach (QString name, resolutionMap)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));
 
		else if (i == FOLDER_CONTROLS)
		{
			foreach (QString name, controlList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
			foreach (quint8 name, playersList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(QString("%1P").arg(name))));
		}
 
		else if (i == FOLDER_CHANNELS)
			foreach (quint8 name, channelsList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(QString::number(name))));

		else if (i == FOLDER_PALETTESIZE)
			foreach (quint32 name, palettesizeList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(QString::number(name))));
	}

	//init ext folders
	QString folderPath = utils->getPath(pGuiSettings->value("folder_directory", "folders").toString());
	QStringList dirPaths = folderPath.split(";");

	extFolderNames.clear();
	foreach (QString _dirPath, dirPaths)
	{
		QDir dir(_dirPath);

		QStringList folderFiles = dir.entryList((QStringList() << "*" INI_EXT), QDir::Files | QDir::Readable);
		
		foreach (QString folderFile, folderFiles)
		{
			QFile f(folderPath + folderFile);
			QFileInfo fi(f);
			extFolderNames.append(fi.completeBaseName());
		}
	}

	//init menu for toggling folders
	QAction *actionMenuItem;
	int i = 0;
	foreach (QString folderName, intFolderNames0)
	{
		actionMenuItem = new QAction(tr(qPrintable(folderName)), win->menuShowFolders->menuAction());
		actionMenuItem->setObjectName(QString(STR_TOGGLE_FOLDER "%1").arg(folderName));
		actionMenuItem->setCheckable(true);

		QTreeWidgetItem *item = intFolderItems[i++];
		actionMenuItem->setChecked(!item->isHidden());

		win->menuShowFolders->addAction(actionMenuItem);
		connect(actionMenuItem, SIGNAL(triggered()), this, SLOT(toggleFolder()));
	}
/*	
	win->menuShowFolders->addSeparator();
	
	foreach (QString folderName, extFolderNames)
	{
		actionMenuItem = new QAction(folderName, win->menuShowFolders->menuAction());
		actionMenuItem->setObjectName(QString(STR_TOGGLE_FOLDER "%1").arg(folderName));
		actionMenuItem->setCheckable(true);
		actionMenuItem->setChecked(true);
		win->menuShowFolders->addAction(actionMenuItem);
		connect(actionMenuItem, SIGNAL(triggered()), this, SLOT(toggleFolder()));
	}
*/
	foreach (QString extFolder, extFolderNames)
		initExtFolders(extFolder, NULL);

	disconnect(win->treeFolders, SIGNAL(itemSelectionChanged()), this, SLOT(filterFolderChanged()));
	connect(win->treeFolders, SIGNAL(itemSelectionChanged()), this, SLOT(filterFolderChanged()));

	disconnect(win->actionRemoveFromFolder, SIGNAL(triggered()), this, SLOT(removeFromExtFolder()));
	connect(win->actionRemoveFromFolder, SIGNAL(triggered()), this, SLOT(removeFromExtFolder()));
}

//fixme:tv
QString Gamelist::getResolution(GameInfo *gameInfo, int id)
{
	DisplayInfo* displaysInfo = gameInfo->displays[id];

	return QString("%1 x %2 %3")
		.arg(displaysInfo->width)
		.arg(displaysInfo->height)
		.arg(gameInfo->isHorz ? tr("(H)") : tr("(V)"));
}

void Gamelist::toggleFolder()
{
	const QString itemName = ((QAction*)sender())->objectName().remove(STR_TOGGLE_FOLDER);
	QTreeWidgetItem *item = intFolderItems[intFolderNames0.indexOf(itemName)];
	const bool visible = !item->isHidden();

	if (visible)
	{
		if (!hiddenFolders.contains(itemName))
			hiddenFolders.append(itemName);
	}
	else
	{
		if (hiddenFolders.contains(itemName))
			hiddenFolders.removeOne(itemName);
	}

	item->setHidden(visible);
}

int Gamelist::parseExtFolders(const QString &folderName)
{
	QString folderPath = utils->getSinglePath(pGuiSettings->value("folder_directory", "folders").toString(), folderName + INI_EXT);
	QFile inFile(folderPath);

	if (!inFile.open(QFile::ReadOnly | QFile::Text))
		return -1;

	const bool isWritable = inFile.permissions() & QFile::WriteUser;

	//start parsing folder .ini
	QString line, key;
	QTextStream in(&inFile);
	in.setCodec("UTF-8");

	extFolderMap.clear();
	//fill in extFolderMap
	do
	{
		line = in.readLine().trimmed();
		if (!line.isEmpty())
		{
			if (line.startsWith("[") && line.endsWith("]"))
			{
				key = line.mid(1, line.size() - 2);
				//prepend a magic string for special tags
				if (key == ROOT_FOLDER || key == "FOLDER_SETTINGS")
					key = EXTFOLDER_MAGIC + key;
			}
			else if (!key.isEmpty())
				extFolderMap.insert(key, line);
		}
	}
	while (!line.isNull());

	return isWritable ? 1 : 0;
}

void Gamelist::initExtFolders(const QString &folderName, const QString &subFolderName)
{
	const bool parseResult = parseExtFolders(folderName);

	if ( parseResult < 0)
		return;

	//build GUI tree
	if (subFolderName.isEmpty())
	{
		static QIcon icoFolder(":/res/32x32/folder.png");
		QTreeWidgetItem *treeitemExtFolder;
		QMenu *menuExtFolder;
		QAction *actionExtSubFolder;

		QList<QString> keys = extFolderMap.uniqueKeys();
		if (!keys.isEmpty())
		{
			treeitemExtFolder = new QTreeWidgetItem(win->treeFolders, QStringList(folderName));
			treeitemExtFolder->setIcon(0, icoFolder);

			if (parseResult > 0)
			{
				menuExtFolder = new QMenu(folderName, win->menuAddtoFolder);
				menuExtFolder->menuAction()->setObjectName(QString(STR_EXTFOLDER "%1").arg(folderName));
				win->menuAddtoFolder->addMenu(menuExtFolder);

				actionExtSubFolder = new QAction(tr("Root Folder [.]"), menuExtFolder->menuAction());
				actionExtSubFolder->setObjectName(QString(STR_EXTSFOLDER "%1").arg(EXTFOLDER_MAGIC ROOT_FOLDER));
				menuExtFolder->addAction(actionExtSubFolder);
				connect(actionExtSubFolder, SIGNAL(triggered()), this, SLOT(addToExtFolder()));

//				menuExtFolder->addAction(tr("New Folder..."));
				menuExtFolder->addSeparator();
			}
			
			foreach (QString key, keys)
			{
				if (key.startsWith(EXTFOLDER_MAGIC))
					continue;

				new QTreeWidgetItem(treeitemExtFolder, QStringList(key));

				if (parseResult > 0)
				{
					actionExtSubFolder = new QAction(key, menuExtFolder->menuAction());
					actionExtSubFolder->setObjectName(QString(STR_EXTSFOLDER "%1").arg(key));
					menuExtFolder->addAction(actionExtSubFolder);
					connect(actionExtSubFolder, SIGNAL(triggered()), this, SLOT(addToExtFolder()));
				}
			}
		}
	}
	//apply the filter
	//fixme: move to a stand alone method
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

void Gamelist::saveExtFolders(const QString &folderName)
{
	QString folderPath = utils->getSinglePath(pGuiSettings->value("folder_directory", "folders").toString(), folderName + INI_EXT);
	QFile outFile(folderPath);

	if (!outFile.open(QFile::WriteOnly | QFile::Text))
		return;

	QTextStream out(&outFile);
	out.setCodec("UTF-8");
//	out.setGenerateByteOrderMark(true);

	QStringList subFolderNames = extFolderMap.uniqueKeys();
	subFolderNames.sort();
	foreach (QString subFolderName, subFolderNames)
	{
		QString _subFolderName = subFolderName;
		if (_subFolderName.startsWith(EXTFOLDER_MAGIC))
			_subFolderName = _subFolderName.right(_subFolderName.size() - QString(EXTFOLDER_MAGIC).size());
			
		out << "[" << _subFolderName << "]" << endl;
		
		QStringList gameNames = extFolderMap.values(subFolderName);
		gameNames.sort();
		foreach (QString gameName, gameNames)
		{
			out << gameName << endl;
		}
		out << endl;
	}
}

void Gamelist::addToExtFolder()
{
	QString extFolderName = ((QAction*)sender()->parent())->objectName();
	extFolderName = extFolderName.right(extFolderName.size() - QString(STR_EXTFOLDER).size());

	QString extSubFolderName = ((QAction*)sender())->objectName();
	extSubFolderName = extSubFolderName.right(extSubFolderName.size() - QString(STR_EXTSFOLDER).size());

	if (parseExtFolders(extFolderName) < 0)
		return;

	extFolderMap.insert(extSubFolderName, currentGame);
	saveExtFolders(extFolderName);
}

void Gamelist::removeFromExtFolder()
{
	QString extFolderName, extSubFolderName;
	QStringList paths = utils->split2Str(currentFolder, "/");
	
	if (paths.first().isEmpty())
	{
		extFolderName = paths.last();
		extSubFolderName = EXTFOLDER_MAGIC ROOT_FOLDER;
	}
	else
	{
		extFolderName = paths.first();
		extSubFolderName = paths.last();
	}
	
	if (parseExtFolders(extFolderName) < 0)
		return;

	extFolderMap.remove(extSubFolderName, currentGame);
	saveExtFolders(extFolderName);

	filterFolderChanged(win->treeFolders->currentItem());
}

void Gamelist::restoreFolderSelection(bool isForce)
{
	//if currentFolder has been set, it's a folder switching call
	if(!currentFolder.isEmpty() && !isForce)
		return;
	
	currentFolder = pGuiSettings->value("default_folder", "/" + intFolderNames[0]).toString();
	int sep = currentFolder.indexOf("/");
	QString parentFolder = currentFolder.left(sep);
	QString subFolder = currentFolder.right(currentFolder.size() - sep - 1);

	if (parentFolder.isEmpty())
	{
		parentFolder = subFolder;
		subFolder.clear();
	}

	parentFolder = tr(qPrintable(parentFolder));

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
//						win->log(QString("treeb.gamecount %1").arg(mameGame->games.count()));
						return;
					}
				}
			}
		}
	}
	//fallback
	win->treeFolders->setCurrentItem(rootItem->child(FOLDER_ALLARC));
}

bool Gamelist::isAuditConsoleFolder(const QString &consoleName)
{
	QStringList paths = utils->split2Str(currentFolder, "/");
	if (!paths.last().isEmpty())
	{
		const QString rightFolder = paths.last();

		if (consoleMap.contains(rightFolder) && 
			consoleMap[rightFolder] == consoleName)
			return true;
	}

	return false;		
}

bool Gamelist::isConsoleFolder()
{
	QStringList paths = utils->split2Str(currentFolder, "/");
	if (!paths.last().isEmpty())
	{
		const QString rightFolder = paths.last();

		if (rightFolder == intFolderNames[FOLDER_CONSOLE])
			return true;

		else if (consoleMap.contains(rightFolder) &&
				mameGame->games.contains(consoleMap[rightFolder]))
		{
			GameInfo *gameInfo = mameGame->games[consoleMap[rightFolder]];
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

	//block process during M1 loading
	if (currentDir != QDir::currentPath())
	{
		win->poplog(tr("Loading M1, please wait..."));
		return;
	}

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
		// extract rom from the merged file
		if (!hasTempRom && gameName.contains(SZIP_EXT "/"))
		{
			QStringList paths = gameName.split(SZIP_EXT "/");
			QString archName = paths.first() + SZIP_EXT;
			QString romFileName = paths.last();
			QFileInfo fileInfo(archName);
			
			currentTempROM = QDir::tempPath() + "/" + romFileName;
			QHash<QString, MameFileInfo *> mameFileInfoList = 
				utils->iterateMameFile(fileInfo.path(), fileInfo.completeBaseName(), romFileName, MAMEFILE_EXTRACT);
			utils->clearMameFileInfoList(mameFileInfoList);
			runMame(true);
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


GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent) : 
QSortFilterProxyModel(parent)
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
	QString encfilterText = utils->getShortName(filterText);
	bool tmpresult = false;

	GameInfo *gameInfo = mameGame->games[gameName];
	if (!gameInfo->devices.isEmpty() && !gameNameExtRom.isEmpty())
		gameInfo = mameGame->games[gameNameExtRom];

	//fixme: how to filter MESS games
	bool isConsole = gameInfo->sourcefile == "cpschngr.c" || !gameInfo->devices.isEmpty();
	bool isBIOS = gameInfo->isBios;
	bool isExtRom = gameInfo->isExtRom;

	// apply search filter
	if (!searchText.isEmpty())
	{
		QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
		QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);

		result = gameName.contains(regExpSearch)|| 
				 gameDesc.contains(regExpSearch) || 
				 utils->getDesc(gameName, false).contains(regExpSearch);

		// also true if any of a parent's clone matches
		if (!isExtRom && !gameInfo->clones.isEmpty())
		{
			foreach (QString gameName2, gameInfo->clones)
			{
				if (gameName2.contains(regExpSearch) ||
					utils->getDesc(gameName2).contains(regExpSearch) ||
					utils->getDesc(gameName2, false).contains(regExpSearch))
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

	case Qt::UserRole + FOLDER_HARDDISK:
		result = result && !gameInfo->disks.isEmpty();
		break;

	case Qt::UserRole + FOLDER_SAMPLES:
		result = result && !gameInfo->samples.isEmpty();
		break;

	case Qt::UserRole + FOLDER_WORKING:
		result = result && !isExtRom && gameInfo->status;
		break;
		
	case Qt::UserRole + FOLDER_NONWORKING:
		result = result && !isExtRom && !gameInfo->status;
		break;
		
	case Qt::UserRole + FOLDER_ORIGINALS:
		result = result && !isBIOS && !isExtRom && gameInfo->cloneof.isEmpty();
		break;

	case Qt::UserRole + FOLDER_CLONES:
		result = result && !isBIOS && !isExtRom && !gameInfo->cloneof.isEmpty();
		break;

	case Qt::UserRole + FOLDER_CHANNELS:
		result = result && !isExtRom && (QString::number(gameInfo->channels) == filterText);
		break;

	case Qt::UserRole + FOLDER_SAVESTATE:
		result = result && !isExtRom && gameInfo->savestate;
		break;

	case Qt::UserRole + FOLDER_CPU:
		foreach (ChipInfo* chipInfo, gameInfo->chips)
		{
			if (chipInfo->name == filterText && chipInfo->type == "cpu")
			{
				tmpresult = true;
				break;
			}
		}

		result = result && tmpresult;
		break;

	case Qt::UserRole + FOLDER_SND:
		foreach (ChipInfo* chipInfo, gameInfo->chips)
		{
			if (chipInfo->name == filterText && chipInfo->type == "audio")
			{
				tmpresult = true;
				break;
			}
		}

		result = result && tmpresult;
		break;

	case Qt::UserRole + FOLDER_HARDDISK + MAX_FOLDERS:
		foreach (DiskInfo* disksInfo, gameInfo->disks)
		{
			if (disksInfo->region == encfilterText)
			{
				tmpresult = true;
				break;
			}
		}
		
		result = result && tmpresult;
		break;
		
	case Qt::UserRole + FOLDER_DUMPING:
		foreach (RomInfo* romsInfo, gameInfo->roms)
		{
			if (romsInfo->status == encfilterText)
			{
				tmpresult = true;
				break;
			}
		}

		foreach (DiskInfo* disksInfo, gameInfo->disks)
		{
			if (disksInfo->status == encfilterText)
			{
				tmpresult = true;
				break;
			}
		}
		
		result = result && tmpresult;
		break;

	case Qt::UserRole + FOLDER_DISPLAY:
	
		if (filterText == tr("Horizontal"))
		{
			result = result && !isExtRom && gameInfo->isHorz;
			break;
		}
		else if (filterText ==	tr("Vertical"))
		{
			result = result && !isExtRom && !gameInfo->isHorz;
			break;
		}
	
		foreach (DisplayInfo* displaysInfo, gameInfo->displays)
		{	
			if (displaysInfo->type == encfilterText)
			{
				tmpresult = true;
				break;
			}
		}
			
		result = result && !isExtRom && tmpresult;
		break;


	case Qt::UserRole + FOLDER_RESOLUTION:

		for (int i = 0; i < gameInfo->displays.size(); i++)
		{
			if (gameList->getResolution(gameInfo, i) == filterText)
			{
				tmpresult = true;
				break;
			}
		}

		result = result && !isExtRom && tmpresult;
		break;

	case Qt::UserRole + FOLDER_PALETTESIZE:
			result = result && !isExtRom && (QString::number(gameInfo->palettesize) == filterText);
			break;
		
	case Qt::UserRole + FOLDER_REFRESH:
		foreach (DisplayInfo* displaysInfo, gameInfo->displays)
		{
			if (displaysInfo->refresh + " Hz" == filterText )
			{
				tmpresult = true;
				break;
			}
		}

		result = result && !isExtRom && tmpresult;
		break;

	case Qt::UserRole + FOLDER_CONTROLS:
		if (QString("%1P").arg(gameInfo->players) == filterText)
		{
			result = result && !isExtRom;
			break;
		}

		foreach (ControlInfo* controlsInfo, gameInfo->controls)
		{		
			if (controlsInfo->type == encfilterText)
			{
				tmpresult = true;
				break;
			}
		}

		result = result && !isExtRom && tmpresult;
		break;

	case Qt::UserRole + FOLDER_BIOS + MAX_FOLDERS:	//hack for bios subfolders
		result = result && !isBIOS && gameInfo->biosof() == filterText;
		break;

	case Qt::UserRole + FOLDER_EXT:
		result = result && !isBIOS && filterList.contains((isExtRom) ? gameNameExtRom : gameName);
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
		visibleGames.insert((isExtRom) ? gameNameExtRom : gameName);

	return result;
}
