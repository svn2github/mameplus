#include "gamelist.h"

#include "mamepgui_types.h"
#include "mamepgui_main.h"
#include "mameopt.h"
#include "ips.h"
#include "m1.h"
#include "dialogs.h"

#ifdef USE_SDL
#undef main
#include "SDL.h"
#endif /* USE_SDL */

/* global */
Gamelist *gameList = NULL;
QString currentGame, currentFolder;
QStringList hiddenFolders;
QMap<QString, QString> consoleMap;
QActionGroup *colSortActionGroup;
QList<QAction *> colToggleActions;

//fixme: used in audit
TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;

/* internal */
GameListDelegate gamelistDelegate(0);
QSet<QString> visibleGames;
QMultiMap<QString, QString> extFolderMap;
QStringList deleteCfgFiles;
QStringList columnList;
QMap<QString, QString> biosMap;

QByteArray defIconDataGreen;
QByteArray defIconDataYellow;
QByteArray defIconDataRed;

QByteArray defMameSnapData;
QByteArray defMessSnapData;

QList<SDL_Joystick *> joysticks;

static QRegExp emptyRegex("");

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

//from treeview.h
enum
{
	F_CLONES		= 0x0001,
	F_NONWORKING	= 0x0002,
	F_UNAVAILABLE	= 0x0004,
	F_VECTOR		= 0x0008,
	F_RASTER		= 0x0010,
	F_ORIGINALS 	= 0x0020,
	F_WORKING		= 0x0040,
	F_AVAILABLE 	= 0x0080,
	F_HORIZONTAL	= 0x1000,
	F_VERTICAL		= 0x2000,
	F_COMPUTER		= 0x0200,
	F_CONSOLE		= 0x0400,
	F_MODIFIED		= 0x0800,
	F_MASK			= 0xFFFF
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

UpdateSelectionThread::UpdateSelectionThread(QObject *parent) : 
QThread(parent),
gameName(""),
abort(false)
{
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

	if (currentGame.isEmpty()/* || currentGame == gameName */)
		return;

	gameName = currentGame;

	if (!isRunning())
	{
		abort = false;
		start(LowPriority);
	}
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
		{ NULL,				NULL,			0,				NULL,			NULL }
	};

	//save a local copy
	QString _gameName = gameName;

	while (!abort)
	{
		//update snaps
		for (int snapType = DOCK_SNAP; snapType <= DOCK_PCB; snapType ++)
		{
			if (_gameName != gameName)
			{
				abort = true;
				break;
			}

			
			if (!abort && win->dockCtrls[snapType]->isVisible() && win->isDockTabVisible(win->dockCtrlNames[snapType]))
			{
				pmSnapData[snapType] = getScreenshot(mameOpts[validGuiSettings[snapType]]->globalvalue, _gameName, snapType);
				emit snapUpdated(snapType);
			}
		}

		QString path, localPath;
		const DockInfo *dockInfoList = _dockInfoList;

		//update documents
		for ( ; dockInfoList->optName != NULL; dockInfoList++)
		{
			if (_gameName != gameName)
			{
				abort = true;
				break;
			}

			if (!abort && win->tbHistory->isVisible() && win->isDockTabVisible(dockInfoList->title))
			{
				if (hasLanguage)
					localPath = utils->getPath(mameOpts["langpath"]->globalvalue);

				dockInfoList->buffer->clear();
			
				path = dockInfoList->fileName;

				if (!localPath.isEmpty())
				{
					localPath = localPath + language + "/" + path;

					*dockInfoList->buffer = getHistory(localPath, _gameName, dockInfoList->type + DOCK_LAST /*hack for local*/);
					if (!dockInfoList->buffer->isEmpty())
						dockInfoList->buffer->append("<hr>");
				}

				if (mameOpts.contains(dockInfoList->optName))
					path = mameOpts[dockInfoList->optName]->globalvalue;

				//don't display the same dat twice
				if (localPath != path)
					dockInfoList->buffer->append(getHistory(path, _gameName, dockInfoList->type));

				//special handling
				switch (dockInfoList->type)
				{
				case DOCK_MAMEINFO:
					convertMameInfo(mameinfoText, _gameName);
					break;

				case DOCK_COMMAND:
					convertCommand(commandText);
					break;
					
				default:
					break;
				}


				emit snapUpdated(dockInfoList->type);
			}
		}

		if (abort)
		{
			_gameName = gameName;
			abort = false;
		}
		else
			break;
	}
}

QString UpdateSelectionThread::getHistory(const QString &fileName, const QString &gameName, int method)
{
	QString buf = "";

	QString searchTag = gameName;
	GameInfo *gameInfo = pMameDat->games[searchTag];
	if (gameInfo->isExtRom)
	{
		searchTag = gameInfo->romof;
//		gameInfo = pMameDat->games[searchTag];
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
		while (!line.isNull() && !abort);
	}

	utils->clearMameFileInfoList(mameFileInfoList);

	buf = buf.trimmed();

	if (buf.isEmpty() && pMameDat->games.contains(searchTag))
	{
		gameInfo = pMameDat->games[searchTag];
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

void UpdateSelectionThread::convertHistory(QString &text, const QString &gameName)
{
}

void UpdateSelectionThread::convertMameInfo(QString &text, const QString &gameName)
{
	GameInfo *gameInfo = pMameDat->games[gameName];
	RomInfo *romInfo;
	DiskInfo *diskInfo;
	QString buf = "";

	if (gameInfo->roms.isEmpty() && gameInfo->disks.isEmpty())
		return;

	buf.append("Rom Region:");
	buf.append("<table>");

	QMap<QString, quint32> romInfos;
	foreach (quint32 crc, gameInfo->roms.keys())
	{
		romInfo = gameInfo->roms.value(crc);
		romInfos.insert(romInfo->region + romInfo->name, crc);
	}

	foreach (quint32 crc, romInfos)
	{
		buf.append("<tr>");

		romInfo = gameInfo->roms.value(crc);
		buf.append("<td>" + romInfo->region + "</td>");
		buf.append("<td> </td>");
		buf.append("<td>" + romInfo->name + "</td>");

		buf.append("</tr>");
	}

	QMap<QString, QString> diskInfos;
	foreach (QString sha1, gameInfo->disks.keys())
	{
		diskInfo = gameInfo->disks.value(sha1);
		diskInfos.insert(diskInfo->region + diskInfo->name, sha1);
	}

	foreach (QString sha1, diskInfos)
	{
		diskInfo = gameInfo->disks.value(sha1);
		buf.append("<tr>");

		buf.append("<td>" + diskInfo->region + "</td>");
		buf.append("<td> </td>");
		buf.append("<td>" + diskInfo->name + ".chd</td>");
		
		buf.append("</tr>");
	}

	buf.append("</table><hr>");

	text.prepend(buf);	
}

void UpdateSelectionThread::convertCommand(QString &text)
{
	struct CmdTable
	{
		QRegExp regex;
		QString repl;
	};

	static const CmdTable _cmdTable[] =
	{
		{ QRegExp("<br>\\s+"),	"<br>" },
		// directions, generate duplicated symbols
		{ QRegExp("_2_1_4_1_2_3_6"), "_2_1_4_4_1_2_3_6" },
		{ QRegExp("_2_3_6_3_2_1_4"), "_2_3_6_6_3_2_1_4" },
		{ QRegExp("_4_1_2_3_6"), "<img src=\":/res/16x16/dir-hcf.png\" />" },
		{ QRegExp("_6_3_2_1_4"), "<img src=\":/res/16x16/dir-hcb.png\" />" },
		{ QRegExp("_2_3_6"), "<img src=\":/res/16x16/dir-qdf.png\" />" },
		{ QRegExp("_2_1_4"), "<img src=\":/res/16x16/dir-qdb.png\" />" },
		{ QRegExp("_(\\d)"), "<img src=\":/res/16x16/dir-\\1.png\" />" },
		// buttons
		{ QRegExp("_([A-DGKNPS\\+])"), "<img src=\":/res/16x16/btn-\\1.png\" />" },
		{ QRegExp("_([a-f])"), "<img src=\":/res/16x16/btn-n\\1.png\" />" },
		//------
		{ QRegExp("<br>[\\x2500-]{8,}<br>"), "<hr>" },
		//special moves, starts with <br> || <hr>
		{ QRegExp(">\\x2605"), "><img src=\":/res/16x16/star_gold.png\" />" },
		{ QRegExp(">\\x2606"), "><img src=\":/res/16x16/star_silver.png\" />" },
		{ QRegExp(">\\x25B2"), "><img src=\":/res/16x16/tri-r.png\" />" },
		{ QRegExp(">\\x25CB"), "><img src=\":/res/16x16/cir-y.png\" />" },
		{ QRegExp(">\\x25CE"), "><img src=\":/res/16x16/cir-r.png\" />" }, 		
		{ QRegExp(">\\x25CF"), "><img src=\":/res/16x16/cir-g.png\" />" },
		{ QRegExp("\\x2192"), "<img src=\":/res/16x16/blank.png\" /><img src=\":/res/16x16/arrow-r.png\" />" },
	//	{ QRegExp("\\0x3000"), "<img src=\":/res/16x16/blank.png\" />" },

	/* colors
	Y: +45
	G: +120 0 -28
	B: -150 0 -20
	C: -32
	P: -90
	*/

		{ QRegExp(), NULL }
	};

	const CmdTable *cmdTable = _cmdTable;
	
	/* loop over entries until we hit a NULL name */
	for ( ; !abort && cmdTable->repl != NULL; cmdTable++)
	{
		text.replace(cmdTable->regex, cmdTable->repl);
	}
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
 		utils->iterateMameFile(dirPaths, zipName, fileNameFilters, MAMEFILE_READ);

	if (mameFileInfoList.size() > 0)
		snapdata = mameFileInfoList[mameFileInfoList.keys().first()]->data;

	utils->clearMameFileInfoList(mameFileInfoList);

	if (!snapdata.isNull())
		return snapdata;

	// recursively load parent image
	GameInfo *gameInfo = pMameDat->games[gameName];
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
	return childItems.size();
}

int TreeItem::columnCount() const
{
	return itemData.size();
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

TreeModel::TreeModel(QObject *parent)
: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;

	columnList = (QStringList() 
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

	foreach (QString gameName, pMameDat->games.keys())
	{
		setupModelData(rootItem, gameName);
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

QVariant TreeModel::displayData(GameInfo *gameInfo, int col) const
{
	TreeItem *item = gameInfo->pModItem;

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
			return tr("No");
	
		case 1:
			return tr("Yes");
		}
	}
	
	return item->data(col);
}

//mandatory
QVariant TreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	TreeItem *item = getItem(index);
	const QString gameName = item->data(COL_NAME).toString();
	GameInfo *gameInfo = pMameDat->games[gameName];
	int col = index.column();

	switch (role)
	{
	case Qt::ForegroundRole:
		if (gameInfo->emulation == 0 && !gameInfo->isExtRom)
			return qVariantFromValue(QColor(isDarkBg ? QColor(255, 96, 96) : Qt::darkRed));
		else
			return qVariantFromValue(QColor((isDarkBg) ? Qt::white : Qt::black));

 	case Qt::UserRole + FOLDER_BIOS:
		return gameInfo->biosof();
		
	case Qt::UserRole + FOLDER_CONSOLE:
		return gameInfo->isExtRom ? true : false;

	case Qt::UserRole + SORT_STR:
	{
//		[parentsortkey]_[parentname]_[0/9][sortkey]

		//fixme: move to static
		const bool sortOrder = win->tvGameList->header()->sortIndicatorOrder() == Qt::AscendingOrder;
		
		QString sortKey = displayData(gameInfo, col).toString();
		QString parentSortKey = sortKey;
		QString sortMagic;
		QString parent = gameName;

		if (gameList->listMode != "Grouped")
			return sortKey;

		if (!gameInfo->cloneof.isEmpty())
		{
			parent = gameInfo->cloneof;
			GameInfo *gameInfo2 = pMameDat->games[parent];
			parentSortKey = displayData(gameInfo2, col).toString();
			//always keep parent on top
			sortMagic = sortOrder ? "_9" : "_0";
		}
		else
			sortMagic = sortOrder ? "_0" : "_9";

		return parentSortKey + "_" + parent + sortMagic + sortKey;
	}

	//convert 'Name' column for ext roms
	case Qt::UserRole:
		if (col == COL_NAME && gameInfo->isExtRom)
			return item->data(col);
		break;

	case Qt::DisplayRole:
		return displayData(gameInfo, col);

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

TreeItem * TreeModel::setupModelData(TreeItem *parent, QString gameName)
{
	GameInfo *gameInfo = pMameDat->games[gameName];

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


GameListTreeView::GameListTreeView(QWidget *parent) : 
QTreeView(parent)
{
}

void GameListTreeView::paintEvent(QPaintEvent *e)
{
	QTreeView::paintEvent(e);

	if (visibleGames.size() < 1 || 
		!win->actionRowDelegate->isChecked() || 
		gameList->listMode == "LargeIcons")
		return;

	QPainter painter(viewport());

	QRect _decoRect = gameList->rectDeco;
	if (gameList->rectDeco.top() >= 8)
		_decoRect.setTop(gameList->rectDeco.top() - 8);
//			if (_decoRect.top() < 0)
//				_decoRect.setTop(0);
	_decoRect.setWidth(38);
	_decoRect.setHeight(38);

	/*
	win->log(QString("p: %1 %2 %3 %4")
		.arg(gameList->rectDeco.top())
		.arg(gameList->rectDeco.left())
		.arg(gameList->rectDeco.width())
		.arg(gameList->rectDeco.height())
		);
	//*/

	if (gameList->rectDeco.top() + 8 >= 0)
		painter.drawPixmap(_decoRect, gameList->pmDeco);
}

QModelIndex GameListTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
	QModelIndex index = QTreeView::moveCursor(cursorAction, modifiers);
	gameList->centerGameSelection(index);
	return index;
}

//Mac: start game with Return/Enter
void GameListTreeView::keyPressEvent(QKeyEvent *event)
{

	switch (event->key())
	{
#ifdef Q_WS_MAC
	case Qt::Key_Enter:
	case Qt::Key_Return:
		// ### we can't open the editor on enter, becuse
		// some widgets will forward the enter event back
		// to the viewport, starting an endless loop
		if (state() != EditingState || hasFocus())
		{
			if (currentIndex().isValid())
				emit activated(currentIndex());
			event->ignore();
		}
		break;
#endif
	default:
		QTreeView::keyPressEvent(event);
		break;
	}
}


GameListDelegate::GameListDelegate(QObject *parent) : 
QItemDelegate(parent)
{
}

QSize GameListDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
	return QSize(1,18);
}

void GameListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							 const QModelIndex &index ) const
{
	if (index.column() != COL_DESC)
	{
		QItemDelegate::paint(painter, option, index);
		return;
	}

	QString gameName = gameList->getViewString(index, COL_NAME);
	GameInfo *gameInfo = gameList->getGameInfo(index, gameName);

	int cloneOffset = 0;
	//clone
	if (gameList->listMode == "Grouped" && !gameInfo->cloneof.isEmpty())
	{
		if (visibleGames.contains(gameInfo->cloneof))
			cloneOffset = 16;
	}

	QModelIndex i, pi;
	i = gameListModel->index(COL_DESC, pMameDat->games[currentGame]->pModItem);

	if (i.isValid())
		pi = gameListPModel->mapFromSource(i);
	
	if (pi.isValid())
		gameList->rectDeco = win->tvGameList->visualRect(pi);
	else
		gameList->rectDeco = QRect();

	QRect rectDeco, rectText;
	rectDeco = rectText = option.rect;
	
	//load original icon
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

	QPixmap pmFinal, pmIcon;

	pmIcon.loadFromData(icondata);
	const bool isLargeIcon = pmIcon.width() > 16;
	const bool isZooming = win->actionRowDelegate->isChecked();

	if (isLargeIcon)
	{
		if (currentGame == gameName && isZooming)
			pmFinal.load(isDarkBg ? ":/res/mamegui/deco-darkbg.png" : ":/res/mamegui/deco-brightbg.png");
		else
			pmFinal = pmIcon = pmIcon.scaled(QSize(16, 16), Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else
		pmFinal = pmIcon;

	//paint unavailable icon
	QPainter p;
	p.begin(&pmFinal);

	if (currentGame == gameName && isLargeIcon && isZooming)
		p.drawPixmap(3, 3, 32, 32, pmIcon);

	// paint the unavailable icon on top of original icon
	if(gameInfo->available != GAME_COMPLETE)
	{
		int offset = 8;
		if (currentGame == gameName && isLargeIcon && isZooming)
			offset = 27;
		
		p.drawPixmap(offset, offset, 8, 8, QPixmap(":/res/status-na.png"));
	}

	p.end();

	//save it for zooming
	int icoSize, decoSize;

	if (pmFinal.width() > 16)
	{
		icoSize = 32;
		decoSize = 38;

		gameList->pmDeco = pmFinal;
	}
	else
	{
		icoSize = decoSize = 16;

		if (currentGame == gameName)
			gameList->pmDeco = QPixmap();
	}

	rectDeco.setLeft(rectDeco.left() + cloneOffset);
	rectDeco.setWidth(icoSize + 6);

	if (currentGame == gameName && isLargeIcon && isZooming)
		rectText.setLeft(rectText.left() + decoSize);
	else
		rectText.setLeft(rectText.left() + decoSize + cloneOffset + 6);

	//paint item text
/*	QColor foreColor;
	if (gameInfo->emulation == 0 && !gameInfo->isExtRom)
		foreColor = QColor(isDarkBg ? QColor(255, 96, 96) : Qt::darkRed);
	else
		foreColor = QColor((isDarkBg) ? Qt::white : Qt::black);
	painter->setPen(foreColor);
//*/
	drawDisplay(painter, option, rectText, gameList->getViewString(index, COL_DESC));

	//paint item icon
	if (currentGame != gameName || !isZooming || 
		(currentGame == gameName && !isLargeIcon))
		drawDecoration(painter, option, rectDeco, pmFinal);	

	return;
}

Gamelist::Gamelist(QObject *parent) : 
QObject(parent),
loadProc(NULL),
menuContext(NULL),
headerMenu(NULL),
autoAudit(false),
hasInitd(false),
defaultGameListDelegate(NULL)
{
	//init joystick
	connect(&timerJoy, SIGNAL(timeout()), this, SLOT(processJoyEvents()));
	openJoysticks();

	connect(&selectionThread, SIGNAL(snapUpdated(int)), this, SLOT(setupSnap(int)));
}

Gamelist::~Gamelist()
{
//	win->log("DEBUG: Gamelist::~Gamelist()");
	if (loadProc)
		loadProc->terminate();

	closeJoysticks();
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

// override gameName and gameInfo for console roms
GameInfo* Gamelist::getGameInfo (const QModelIndex &index, QString& gameName)
{
	GameInfo *gameInfo = pMameDat->games[gameName];
	
	if (!gameInfo->devices.isEmpty())
	{
		QString gameName2 = getViewString(index, COL_NAME + COL_LAST);
		if (!gameName2.isEmpty())
		{
			gameInfo = pMameDat->games[gameName2];
			if (gameInfo && gameInfo->isExtRom)
				gameName = gameName2;
		}
	}

	return gameInfo;
}

void Gamelist::updateSelection()
{
#ifdef Q_OS_WIN
	static bool m1Loaded = false;
	if (!m1Loaded && m1UI->isVisible() && win->isDockTabVisible("M1"))
	{
		m1Loaded = true;
		m1->init();
	}
#endif /* Q_OS_WIN */

	if (hasInitd && pMameDat->games.contains(currentGame))
		selectionThread.update();
}

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		//fixme: merge with filter accept
		QString gameName = getViewString(current, COL_NAME);
		if (gameName.isEmpty())
			return;

		for (int snapType = DOCK_SNAP; snapType <= DOCK_PCB; snapType ++)
		{
			if (win->dockCtrls[snapType]->isVisible() && win->isDockTabVisible(win->dockCtrlNames[snapType]))
			{
				((Screenshot*)win->dockCtrls[snapType])->updateScreenshotLabel(true);
			}
		}
	
		QString gameDesc = getViewString(current, COL_DESC);
		GameInfo *gameInfo = getGameInfo(current, gameName);

		currentGame = gameName;
		
		//update statusbar
		win->logStatus(gameDesc);
		win->logStatus(gameInfo);

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
		currentGame = pMameDat->games.keys().first();

//	win->log("currentGame: " + currentGame);
}

void Gamelist::restoreGameSelection()
{
	if (gameListModel == NULL || gameListPModel == NULL || !hasInitd)
		return;

	if (!pMameDat->games.contains(currentGame))
		return;

	//fixme: should consider other columns
	GameInfo *gameInfo = pMameDat->games[currentGame];
	QModelIndex currIndex, proxyIndex;

	// select current game
	currIndex = gameListModel->index(COL_DESC, gameInfo->pModItem);

	if (currIndex.isValid())
		proxyIndex = gameListPModel->mapFromSource(currIndex);

	// select first row otherwise
	if (!proxyIndex.isValid())
		proxyIndex = gameListPModel->index(0, 0, QModelIndex());

	if (!proxyIndex.isValid())
		return;

/*
	//offset
	if (method != 0)
	{
		proxyIndex = proxyIndex.sibling(proxyIndex.row() + method, proxyIndex.column());

		if (!proxyIndex.isValid())
			return;
	}
//*/

	centerGameSelection(proxyIndex);

	win->labelGameCount->setText(tr("%1 games").arg(visibleGames.size()));

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

void Gamelist::centerGameSelection(QModelIndex index)
{
	bool isLView = false;
//	if (win->actionLargeIcons->isChecked())
//		isLView = true;

	//fixme: time consuming?
	if (isLView)
	{
		win->lvGameList->setCurrentIndex(index);
		win->lvGameList->scrollTo(index, QAbstractItemView::PositionAtCenter);
		win->lvGameList->setFocus();
	}
	else
	{
		win->tvGameList->setCurrentIndex(index);
		win->tvGameList->scrollTo(index, QAbstractItemView::PositionAtCenter);
		win->tvGameList->setFocus();
	}
}

// must update GUI in main thread
void Gamelist::setupSnap(int snapType)
{
//	if (!selectionThread.done)
//		return;

	switch (snapType)
	{
	case DOCK_SNAP:
	case DOCK_TITLE:
		((Screenshot*)win->dockCtrls[snapType])->setPixmap(selectionThread.pmSnapData[snapType], win->actionEnforceAspect->isChecked());
		break;
	case DOCK_FLYER:
	case DOCK_CABINET:
	case DOCK_MARQUEE:
	case DOCK_CPANEL:
	case DOCK_PCB:
		((Screenshot*)win->dockCtrls[snapType])->setPixmap(selectionThread.pmSnapData[snapType], false);
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
	{
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
	}

	foreach (QString folderName, intFolderNames0)
		intFolderNames << tr(qPrintable(folderName));

	bool isLView = false;

	// get current game list mode
	if (win->actionDetails->isChecked())
	{
		listMode = "Details";
	}
//	else if (win->actionLargeIcons->isChecked())
//	{
//		listMode = "LargeIcons";
//		isLView = true;
//	}
	else
		listMode = "Grouped";

	//validate currentGame
	if (!pMameDat->games.contains(currentGame))
		currentGame = pMameDat->games.keys().first();

	disableCtrls();

	//init the model
	gameListModel = new TreeModel(win);
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

		win->tvGameList->setItemDelegate(&gamelistDelegate);
	}
	win->show();

	if (initMethod == GAMELIST_INIT_FULL)
	{
		/* init everything else here after we have pMameDat */

		//fixme: something here should be moved to opt
		// init options from default mame.ini
		optUtils->loadDefault(pMameDat->defaultIni);

		// load mame.ini overrides
		optUtils->preUpdateModel(NULL, OPTLEVEL_GLOBAL, currentGame, 1);

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
//		if (!hasInitd)
//			win->setVersion();
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

	//restore filters
	filterFlags = pGuiSettings->value("folder_flag").toInt();
	win->actionFilterClones->setChecked((F_CLONES & filterFlags) != 0);
	win->actionFilterNonWorking->setChecked((F_NONWORKING & filterFlags) != 0);
	win->actionFilterUnavailable->setChecked((F_UNAVAILABLE & filterFlags) != 0);

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

	//fixme: hack to update snapshot_directory for non-Windows build
	optUtils->preUpdateModel(NULL, OPTLEVEL_GLOBAL, currentGame, 1);

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
//	win->log(QString("init'd %1 games").arg(pMameDat->games.size()));

	//for re-init list from folders
	restoreGameSelection();
	updateSelection();
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
		utils->iterateMameFile(mameOpts["icons_directory"]->globalvalue, "icons;.", "*" ICO_EXT, MAMEFILE_READ);

	foreach (QString key, mameFileInfoList.keys())
	{
		QString gameName = key;
		gameName.chop(4 /* sizeof ICO_EXT */);
		if (pMameDat->games.contains(gameName))
		{
			gameInfo = pMameDat->games[gameName];
			gameInfo->icondata = mameFileInfoList[key]->data;
		}
	}

	utils->clearMameFileInfoList(mameFileInfoList);

	//complete data
	foreach (QString gameName, pMameDat->games.keys())
	{
		gameInfo = pMameDat->games[gameName];

		// get clone icons from parent
		if (!gameInfo->isExtRom && gameInfo->icondata.isNull() && !gameInfo->cloneof.isEmpty())
		{
			gameInfo2 = pMameDat->games[gameInfo->cloneof];
			if (!gameInfo2->icondata.isNull())
			{
				gameInfo->icondata = gameInfo2->icondata;
//				emit icoUpdated(gameName);
			}
		}

		// get ext rom icons from system
		if (gameInfo->isExtRom && gameInfo->icondata.isNull())
		{
			gameInfo2 = pMameDat->games[gameInfo->romof];
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

	foreach (QString gameName, pMameDat->games.keys())
	{
		GameInfo *gameInfo = pMameDat->games[gameName];
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
//	if (win->actionLargeIcons->isChecked())
//		isLView = true;

	colSortActionGroup = new QActionGroup(win->menuArrangeIcons);

	win->menuArrangeIcons->addSeparator();

	for (int c = COL_DESC; c < COL_LAST; c++)
	{
		QAction *actionMenuItem, *actionMenuItem2;
		QString columnName = TreeModel::tr(qPrintable(columnList[c]));

		actionMenuItem = new QAction(QString(tr("by %1")).arg(columnName), win->menuArrangeIcons->menuAction());
		actionMenuItem2 = new QAction(columnName, win->menuCustomizeFields->menuAction());

		actionMenuItem->setCheckable(true);
		actionMenuItem2->setCheckable(true);

		if (c == COL_DESC)
			actionMenuItem2->setEnabled(false);

		colSortActionGroup->addAction(actionMenuItem);
		win->menuArrangeIcons->addAction(actionMenuItem);
		
		colToggleActions.append(actionMenuItem2);
		win->menuCustomizeFields->addAction(actionMenuItem2);

		connect(actionMenuItem, SIGNAL(triggered()), win, SLOT(on_actionColSortAscending_activated()));
		connect(actionMenuItem2, SIGNAL(triggered()), win, SLOT(toggleGameListColumn()));
	}

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
		foreach (QAction *action, colToggleActions)
		{
			headerMenu->addAction(action);
		}

		// add sorting action to an exclusive group
		QActionGroup *sortingOrderActions = new QActionGroup(headerMenu);
		sortingOrderActions->addAction(win->actionColSortAscending);
		sortingOrderActions->addAction(win->actionColSortDescending);
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

	disconnect(win->menuArrangeIcons, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));
	connect(win->menuArrangeIcons, SIGNAL(aboutToShow()), this, SLOT(updateHeaderContextMenu()));
}

void Gamelist::showContextMenu(const QPoint &p)
{
    menuContext->popup(win->tvGameList->mapToGlobal(p));
}

void Gamelist::updateContextMenu()
{
	if (!pMameDat->games.contains(currentGame))
		return;

	//play menu
	QString gameName = currentGame;
	GameInfo *gameInfo = pMameDat->games[gameName];

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
	GameInfo *gameInfo = pMameDat->games[gameName];

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

		const QMap<QString, DeviceInfo *> systemDevices = pMameDat->games[gameInfo->romof]->devices;
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
	GameInfo *gameInfo = pMameDat->games[gameName];
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
	GameInfo *gameInfo = pMameDat->games[gameName];
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
	headerMenu->popup(win->tvGameList->mapToGlobal(p));
}

void Gamelist::updateHeaderContextMenu()
{
	QHeaderView *header = win->tvGameList->header();

	QList<QAction *>actions;

	actions = colToggleActions;
	for (int i = 0; i < actions.size(); i++)
	{
		actions[i]->setChecked(!header->isSectionHidden(i));
	}
	
	actions = colSortActionGroup->actions();
	for (int i = 0; i < actions.size(); i++)
	{
		if (win->tvGameList->header()->sortIndicatorSection() == i)
			actions[i]->setChecked(true);
	}

	if (header->sortIndicatorOrder() == Qt::AscendingOrder)
		win->actionColSortAscending->setChecked(true);
	else
		win->actionColSortDescending->setChecked(true);
}

void Gamelist::updateDeleteCfgMenu(const QString &/*gameName*/)
{
	QAction *actionMenuItem;
	DiskInfo *diskInfo;
	GameInfo *gameInfo = pMameDat->games[currentGame];
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

		foreach (QString sha1, pMameDat->games[currentGame]->disks.keys())
		{
			diskInfo = pMameDat->games[currentGame]->disks[sha1];
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

// delete the extracted rom
void Gamelist::runMergedFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	QFile tmpMergedFile(currentTempROM);
	tmpMergedFile.remove();
	currentTempROM.clear();
}

void Gamelist::filterFlagsChanged(bool isChecked)
{
	QAction* actionFilter = (QAction*)sender();
	quint16 currentFlag = 0x0;
	if (actionFilter->objectName() == "actionFilterClones")
		currentFlag = F_CLONES;
	else if (actionFilter->objectName() == "actionFilterNonWorking")
		currentFlag = F_NONWORKING;
	else if (actionFilter->objectName() == "actionFilterUnavailable")
		currentFlag = F_UNAVAILABLE;

	if (isChecked)
		filterFlags |= currentFlag;
	else
		filterFlags ^= currentFlag;

	// set it for a callback to refresh the list
	gameListPModel->setFilterRegExp(emptyRegex);

	//fixme: must have this, otherwise the list cannot be expanded properly
	qApp->processEvents();
	win->tvGameList->expandAll();

	restoreGameSelection();
//	win->log(QString("filterFlags: %1").arg(filterFlags));
}

void Gamelist::filterSearchCleared()
{
	if (win->lineEditSearch->text().size() < 1)
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
	if (text.size() == 1 && text.at(0).unicode() < 0x3000 /* CJK symbols start */)
		return;

	visibleGames.clear();
	text.replace(rxSpace, "*");

	//fixme: doesnt use filterregexp
	gameListPModel->searchText = text;
	// set it for a callback to refresh the list
	gameListPModel->setFilterRegExp(emptyRegex);
	qApp->processEvents();
	win->tvGameList->expandAll();
	restoreGameSelection();
}

//apply folder switching
void Gamelist::filterFolderChanged(QTreeWidgetItem *_current, QTreeWidgetItem */*previous*/)
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

	// set it for a callback to refresh the list
	gameListPModel->filterText = filterText;	// must set before regExp
	gameListPModel->setFilterRegExp(emptyRegex);

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

	foreach (QString gameName, pMameDat->games.keys())
	{
		gameInfo = pMameDat->games[gameName];
		QString itemStr;
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
		itemStr = gameInfo->year;
		if (itemStr.isEmpty())
			itemStr = "?";
		if (!yearList.contains(itemStr))
			yearList << itemStr;

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
		{
			itemStr = utils->getLongName(romsInfo->status);
			if (!statusList.contains(itemStr) && !itemStr.isEmpty())
				statusList << itemStr;
		}

		//disk status
		foreach (DiskInfo* disksInfo, gameInfo->disks)
		{	
			itemStr = utils->getLongName(disksInfo->status);
			if (!statusList.contains(itemStr) && !itemStr.isEmpty())
				statusList << itemStr;

			//disk region
			itemStr = utils->getLongName(disksInfo->region);
			if (!regionList.contains(itemStr) && !itemStr.isEmpty())
				regionList << itemStr;
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
			itemStr = utils->getLongName(displaysInfo->type);
			if (!displayList.contains(itemStr))
				displayList << itemStr;

			//refresh
			if (!refreshList.contains(displaysInfo->refresh))
				refreshList << displaysInfo->refresh;

			//resolution
			if (!resolutionMap.contains(pixelSize) && displaysInfo->type != "vector")
				resolutionMap.insert(pixelSize, getResolution(gameInfo, i));
		}

		//control type
		foreach (ControlInfo *controlsInfo, gameInfo->controls)
		{
			itemStr = utils->getLongName(controlsInfo->type);
			if (!controlList.contains(itemStr))
				controlList << itemStr;
		}
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

	//prepare hidden folders
	hiddenFolders = pGuiSettings->value("hide_folders").toStringList();
	if (hiddenFolders.isEmpty())
		hiddenFolders << intFolderNames0[FOLDER_ALLGAME];
	hiddenFolders.removeDuplicates();
	
	//remove unrecognized values
	foreach (QString folderName, hiddenFolders)
		if (!intFolderNames0.contains(folderName))
			hiddenFolders.removeOne(folderName);

	win->treeFolders->clear();
	for (int i = 0; i < intFolderNames.size(); i++)
	{
		intFolderItems.append(new QTreeWidgetItem(win->treeFolders, QStringList(intFolderNames[i])));

		win->treeFolders->addTopLevelItems(intFolderItems);
		intFolderItems[i]->setIcon(0, icoFolder);

		//hide folders
		if (hiddenFolders.contains(intFolderNames0[i]) || 
			(!hasDevices && (i == FOLDER_ALLGAME || i == FOLDER_CONSOLE)))
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
				QTreeWidgetItem *item = new QTreeWidgetItem(intFolderItems[i], QStringList(pMameDat->games[name]->description));
//				item->setIcon(0, icon);

				intFolderItems[i]->addChild(item);
			}
		else if (i == FOLDER_HARDDISK)
			foreach (QString name, regionList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));
 
		else if (i == FOLDER_CPU)
			foreach (QString name, cpuList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_SND)
			foreach (QString name, audioList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(utils->getLongName(name))));
 
		else if (i == FOLDER_DUMPING)
			foreach (QString name, statusList)
				intFolderItems[i]->addChild(new QTreeWidgetItem(intFolderItems[i], QStringList(name)));
 
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
		QMenu *menuExtFolder = NULL;
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

			if (!pMameDat->games.contains(gameName))
				continue;
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
//						win->log(QString("treeb.gamecount %1").arg(pMameDat->games.size()));
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
				pMameDat->games.contains(consoleMap[rightFolder]))
		{
			GameInfo *gameInfo = pMameDat->games[consoleMap[rightFolder]];
			if (!gameInfo->devices.isEmpty())
				return true;
		}
	}

	return false;
}

//enable GUI joystick listener
void Gamelist::openJoysticks()
{
#ifdef USE_SDL
	//only enable when MAME is not active
	if (procMan->procMap.size() == 0)
	{
		for (int i = 0; i < SDL_NumJoysticks(); ++i)
			joysticks.append(SDL_JoystickOpen(i));

		timerJoy.start(50);
	}
#endif /* USE_SDL */
}

//disable GUI joystick listener
void Gamelist::closeJoysticks()
{
#ifdef USE_SDL
	timerJoy.stop();

	foreach (SDL_Joystick *joystick, joysticks)
		SDL_JoystickClose(joystick);

	joysticks.clear();
#endif /* USE_SDL */
}

void Gamelist::processJoyEvents()
{
#ifdef USE_SDL
	SDL_Event event;
	static int key = 0;	//invalid key code
	bool isRepeatEvent = true;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_JOYAXISMOTION:
		{
			/*
			win->log(QString("Joystick %1 axis %2 value: %3")
				   .arg(event.jaxis.which)
				   .arg(event.jaxis.axis)
				   .arg(event.jaxis.value));
			//*/

			//joy up / down
			if (event.jaxis.axis == 0)
			{
				if (event.jaxis.value < -3200)
					key = Qt::Key_Up;
				else if(event.jaxis.value > 3200)
					key = Qt::Key_Down;
				//centered, so we clear the repeat state
				else
					key = 0;
			}
			//joy left / right
			else if (event.jaxis.axis == 1)
			{
				if (event.jaxis.value < -3200)
					key = Qt::Key_PageUp;
				else if (event.jaxis.value > 3200)
					key = Qt::Key_PageDown;
				else
					key = 0;
			}

			timeJoyRepeatDelay.restart();
			isRepeatEvent = false;

			break;
		}
		case SDL_JOYBUTTONDOWN:
		{
			//close poplog message box
			if (win->pop != NULL && win->pop->isVisible())
				win->pop->close();
			else
			{
				QKeyEvent keyEvent (QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
				QApplication::sendEvent(win->tvGameList, &keyEvent);
			}

			/*
			win->log(QString("Joystick %1 button %2 down")
				   .arg(event.jbutton.which)
				   .arg(event.jbutton.button));
			//*/
			break;
		}
		default:
			break;
		}
	}

	//if button has been pressed for more than 500 ms, send the KeyPress event
	if (key != 0 && (!isRepeatEvent || timeJoyRepeatDelay.elapsed() >= 500))
	{
		QKeyEvent keyEvent(QEvent::KeyPress, key, Qt::NoModifier);
		QApplication::sendEvent(win->tvGameList, &keyEvent);
	}
#endif /* USE_SDL */
}

void Gamelist::runMame(int method, QStringList playArgs)
{
	//block multi mame session for now
	//if (procMan->procCount > 0)
	//	return;

	closeJoysticks();

	//block process during M1 loading
	if (currentAppDir != QDir::currentPath())
	{
		win->poplog(tr("Loading M1, please wait..."));
		return;
	}

	QStringList args;
	args << playArgs;

	//force update ext rom fields
	updateDynamicMenu(win->menuFile);

	const QString gameName = currentGame;
	GameInfo *gameInfo = pMameDat->games[gameName];

	if (gameInfo->devices.isEmpty())
	{
		//MAME game
		args << gameName;
	}
	else
	// run MESS roms, add necessary params
	{
		// extract rom from the merged file
		if (method != RUNMAME_EXTROM && gameName.contains(SZIP_EXT "/"))
		{
			QStringList paths = gameName.split(SZIP_EXT "/");
			QString archName = paths.first() + SZIP_EXT;
			QString romFileName = paths.last();
			QFileInfo fileInfo(archName);
			
			currentTempROM = QDir::tempPath() + "/" + romFileName;
			
			QHash<QString, MameFileInfo *> mameFileInfoList = 
				utils->iterateMameFile(fileInfo.path(), fileInfo.completeBaseName(), romFileName, MAMEFILE_EXTRACT);
			if (mameFileInfoList.size() > 0)
				runMame(RUNMAME_EXTROM);
			else
				win->poplog(
				tr("Could not load:\n\n") + 
				fileInfo.path() + "/\n" + fileInfo.completeBaseName() + "/\n" + romFileName + "\n\n" +
				tr("Please refresh the game list."));
				
			utils->clearMameFileInfoList(mameFileInfoList);
			
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
				if (method == RUNMAME_EXTROM && !tempRomLoaded)
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

	if (hasLanguage && language != "ru_RU" /* no core support yet */)
	{
		QString langpath = utils->getPath(mameOpts["langpath"]->globalvalue);
		args << "-langpath" << langpath;
		args << "-language" << language;
	}

	if (method == RUNMAME_CMD)
	{
		QStringList strOptions;

		//update mameOpts
		optUtils->preUpdateModel(NULL, OPTLEVEL_CURR, currentGame, 1);

		QStringList optNames = mameOpts.keys();
		optNames.sort();

		foreach (QString optName, optNames)
		{
			MameOption *pMameOpt = mameOpts[optName];
			QString currVal = pMameOpt->currvalue;
			if (pMameOpt->defvalue != currVal && 
				!optName.endsWith("_extra_software") && 
				!pGuiSettings->contains(optName) &&
				optName != "langpath" &&
				optName != "language"
				)
			{
				if (pMameOpt->type == MAMEOPT_TYPE_BOOL)
				{
					QString opt = optName;
					if (currVal == "0")
						opt.prepend("no");
					
					strOptions << "-" + opt;
				}
				else
					strOptions << "-" + optName + " " + currVal;
			}
		}
		
		cmdUI->textCommand->setText((args + strOptions).join(" "));
		if (cmdUI->exec() == QDialog::Accepted)
		{
			QString cmdArgs = cmdUI->textCommand->toPlainText();
			args.clear();
			args << "-noreadconfig";
			args << cmdArgs.split(" ", QString::SkipEmptyParts);
		}
		else
			return;
	}

	loadProc = procMan->process(procMan->start(mame_binary, args, method != RUNMAME_EXTROM));

	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMameFinished(int, QProcess::ExitStatus)));
	// delete the extracted rom
	if (method == RUNMAME_EXTROM)
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(runMergedFinished(int, QProcess::ExitStatus)));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), win, SLOT(toggleTrayIcon(int, QProcess::ExitStatus)));

	win->toggleTrayIcon(0, QProcess::NormalExit, true);
}

void Gamelist::runMameFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	openJoysticks();

	QFile tmpNvFile(utils->getPath(QDir::tempPath()) + currentGame + ".nv");
	tmpNvFile.remove();

	if (exitCode != 0 || exitStatus != QProcess::NormalExit)
		win->poplog(procMan->stdErr);
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
	bool tmpresult = false;

	GameInfo *gameInfo = pMameDat->games[gameName];
	if (!gameInfo->devices.isEmpty() && !gameNameExtRom.isEmpty())
		gameInfo = pMameDat->games[gameNameExtRom];

	//fixme: how to filter MESS games
	const bool isConsole = gameInfo->sourcefile == "cpschngr.c" || !gameInfo->devices.isEmpty();
	const bool isBIOS = gameInfo->isBios;
	const bool isExtRom = gameInfo->isExtRom;
	const bool isClone = !gameInfo->cloneof.isEmpty();

	// apply filter flags
	if (F_CLONES & gameList->filterFlags && !isExtRom)
		result = result && !isClone;
	if (F_NONWORKING & gameList->filterFlags&& !isExtRom)
		result = result && gameInfo->status;
	if (F_UNAVAILABLE & gameList->filterFlags&& !isExtRom)
		result = result && gameInfo->available == GAME_COMPLETE;

	// apply search filter
	if (!searchText.isEmpty())
	{
		QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);
		QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);

		result = gameName.contains(regExpSearch)|| 
				 gameDesc.contains(regExpSearch) || 
				 utils->getDesc(gameName, false).contains(regExpSearch);
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
			&& (gameInfo->available == GAME_COMPLETE);
		if (!isMESS)
			result = result && !isConsole;		
		break;

	case Qt::UserRole + FOLDER_UNAVAILABLE:
		result = result && !isBIOS && !isExtRom
			&& (gameInfo->available != GAME_COMPLETE);
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
		result = result && !isBIOS && !isExtRom && !isClone;
		break;

	case Qt::UserRole + FOLDER_CLONES:
		result = result && !isBIOS && !isExtRom && isClone;
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
			if (utils->getLongName(disksInfo->region) == filterText)
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
			if (utils->getLongName(romsInfo->status) == filterText)
			{
				tmpresult = true;
				break;
			}
		}

		foreach (DiskInfo* disksInfo, gameInfo->disks)
		{
			if (utils->getLongName(disksInfo->status) == filterText)
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
			if (utils->getLongName(displaysInfo->type) == filterText)
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
			if (utils->getLongName(controlsInfo->type) == filterText)
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

bool GameListSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	const QAbstractItemModel *srcModel = sourceModel();

	return QString::localeAwareCompare(
		srcModel->data(left, Qt::UserRole + SORT_STR).toString(), 
		srcModel->data(right, Qt::UserRole + SORT_STR).toString()) < 0;
}

