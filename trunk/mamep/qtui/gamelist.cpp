#include "mamepguimain.h"

MameGame *mamegame;
QString currentGame;
Gamelist *gamelist = NULL;
GameListSortFilterProxyModel *gameListPModel;

static TreeModel *gameListModel;
static QTimer *searchTimer = NULL;
static GamelistDelegate gamelistDelegate(0);

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

enum
{
	FOLDER_ALLGAMES = 0,
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

		if (mamegame)
			delete mamegame;
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
			gameinfo->sourcefile = attributes.value("sourcefile");
			gameinfo->isbios = attributes.value("isbios") == "yes";
			gameinfo->cloneof = attributes.value("cloneof");
			gameinfo->romof = attributes.value("romof");
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
		else if (gameinfo->isbios && qName == "biosset")
		{
			BiosInfo *biosinfo = new BiosInfo(gameinfo);
			biosinfo->description = attributes.value("description");
			biosinfo->isdefault = attributes.value("default") == "yes";

			gameinfo->nameBiosInfoMap[attributes.value("name")] = biosinfo;
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
	bool metMameTag;
};

void AuditROMThread::audit()
{
	if (!isRunning())
		start(LowPriority);
}

void AuditROMThread::run()
{
	QStringList dirpaths = mameOpts["rompath"]->currvalue.split(";");

	GameInfo *gameinfo, *gameinfo2;
	RomInfo *rominfo;

	foreach (QString dirpath, dirpaths)
	{
//		QString dirpath = utils->getPath(dirpath);
		QDir dir(dirpath);

		QStringList nameFilter;
		nameFilter << "*.zip";
		
		QStringList romFiles = dir.entryList(nameFilter, QDir::Files | QDir::Readable);

		emit progressSwitched(romFiles.count(), "Auditing " + dirpath);
		win->log(QString("auditing %1, %2").arg(dirpath).arg(romFiles.count()));

		//iterate romfiles
		for (int i = 0; i < romFiles.count(); i++)
		{
			//update progressbar
			if ( i % 100 == 0 )
				emit progressUpdated(i);

			QString gamename = romFiles[i].toLower().remove(".zip");

			if (mamegame->gamenameGameInfoMap.contains(gamename))
			{
				QuaZip zip(utils->getPath(dirpath) + romFiles[i]);

				if(!zip.open(QuaZip::mdUnzip))
					continue;

				QuaZipFileInfo info;
				QuaZipFile zipFile(&zip);
				gameinfo = mamegame->gamenameGameInfoMap[gamename];

				//iterate all files in the zip
				for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
				{
					if(!zip.getCurrentFileInfo(&info))
						continue;

					quint32 crc = info.crc;
					// file crc recognized
					if (gameinfo->crcRomInfoMap.contains(crc))
						gameinfo->crcRomInfoMap[crc]->available = true; 
					//check rom for clones
					else
					{
						foreach (QString clonename, gameinfo->clones)
						{
							gameinfo2 = mamegame->gamenameGameInfoMap[clonename];
							if (gameinfo2->crcRomInfoMap.contains(crc))
								gameinfo2->crcRomInfoMap[crc]->available = true; 
						}
					}
				}
			}
		}
	}

	win->log("audit 1");
	//see if any rom of a game is not available
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		gameinfo->available = 1;
		foreach (quint32 crc, gameinfo->crcRomInfoMap.keys())
		{
			rominfo = gameinfo->crcRomInfoMap[crc];
			if (!rominfo->available)
			{
				if (rominfo->status == "nodump")
					continue;

				//check parent
				if (!gameinfo->romof.isEmpty())
				{
					gameinfo2 = mamegame->gamenameGameInfoMap[gameinfo->romof];
					if (gameinfo2->crcRomInfoMap.contains(crc) && gameinfo2->crcRomInfoMap[crc]->available)
						continue;
					else
						emit logUpdated(LOG_QMC2, gameinfo->romof + "/" + rominfo->name + " not found");

					//check bios
					if (!gameinfo2->romof.isEmpty())
					{
						gameinfo2 = mamegame->gamenameGameInfoMap[gameinfo2->romof];
						if (gameinfo2->crcRomInfoMap.contains(crc) && gameinfo2->crcRomInfoMap[crc]->available)
							continue;
						else
							emit logUpdated(LOG_QMC2, gameinfo2->romof + "/" + rominfo->name + " not found");
					}
				}
				//failed audit
				emit logUpdated(LOG_QMC2, gamename + "/" + rominfo->name + " failed");

				gameinfo->available = 0;
				break;
			}
		}
	}
	win->log("audit 2");
	emit progressSwitched(-1);
}

MameThread::MameThread(QObject *parent)
: QThread(parent)
{
	abort = false;
}

MameThread::~MameThread()
{
	mutex.lock();
	abort = true;
	mutex.lock();
	
	wait();
}

void MameThread::load()
{
	QMutexLocker locker(&mutex);

	if (!isRunning())
		start(LowPriority);
}

void MameThread::run()
{

}

LoadIconThread::LoadIconThread(QObject *parent)
: QThread(parent)
{
	abort = false;
}

LoadIconThread::~LoadIconThread()
{
	mutex.lock();
	abort = true;
	mutex.lock();
	
	wait();
}

void LoadIconThread::load()
{
	QMutexLocker locker(&mutex);

	if (!isRunning())
		start(LowPriority);
}

void LoadIconThread::run()
{
	while (!iconQueue.isEmpty() && !abort)
	{
		QString gameName = iconQueue.dequeue();
		QIcon icon = utils->loadIcon(gameName);

		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
		gameinfo->icon = icon;
	}
}

UpdateSelectionThread::UpdateSelectionThread(QObject *parent)
: QThread(parent)
{
	abort = false;
}

UpdateSelectionThread::~UpdateSelectionThread()
{
	mutex.lock();
	abort = true;
	mutex.lock();
	
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
			pmSnap = utils->getScreenshot(mameOpts["snapshot_directory"]->globalvalue, gameName, QSize());
		if (win->ssFlyer->isVisible())
			pmFlyer = utils->getScreenshot(flyer_directory, gameName, QSize());
		if (win->ssCabinet->isVisible())
			pmCabinet = utils->getScreenshot(cabinet_directory, gameName, QSize());
		if (win->ssMarquee->isVisible())			
			pmMarquee = utils->getScreenshot(marquee_directory, gameName, QSize());
		if (win->ssTitle->isVisible())
			pmTitle = utils->getScreenshot(title_directory, gameName, QSize());
		if (win->ssCPanel->isVisible())
			pmCPanel = utils->getScreenshot(cpanel_directory, gameName, QSize());
		if (win->ssPCB->isVisible())
			pmPCB = utils->getScreenshot(pcb_directory, gameName, QSize());
//		static QMovie movie( "xxx.mng" );
//		win->lblPCB->setMovie( &movie );

		if (win->tbHistory->isVisible())
			historyText = utils->getHistory(gameName, "history.dat");
		if (win->tbMameinfo->isVisible())
			mameinfoText = utils->getHistory(gameName, "mameinfo.dat");
		if (win->tbStory->isVisible())
			storyText = utils->getHistory(gameName, "story.dat");
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

TreeModel::TreeModel(QObject *parent)
: QAbstractItemModel(parent)
{
	QList<QVariant> rootData;

	foreach (QString header, columnList)
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
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextColorRole)
	{
		return qVariantFromValue(QColor(Qt::black));
	}

	TreeItem *item = getItem(index);
	QString gameName = item->data(1).toString();
	GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
	
	if (role == Qt::DecorationRole && index.column() == COL_DESC)
	{
		if (gameinfo->icon.isNull())
		{
			gamelist->iconThread.iconQueue.setSize(win->treeViewGameList->viewport()->height() / 17);
			gamelist->iconThread.iconQueue.enqueue(gameName);
			gamelist->iconThread.load();
			return utils->deficon;
		}
		return gameinfo->icon;
	}
	
	if (role == Qt::UserRole + FOLDER_BIOS)
		return gameinfo->biosof();
		
	if (role != Qt::DisplayRole)
		return QVariant();

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
			const QIcon icon(qvariant_cast<QIcon>(value));
			if(!icon.isNull())
			{
				QString gameName = item->data(1).toString();
				GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
				gameinfo->icon = icon;
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
		columnData << gameinfo->available;
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

BiosInfo::~BiosInfo()
{
	//    win->log("# ~BiosInfo()");
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
	//  win->log("# ~GameInfo()");
}

QString GameInfo::biosof()
{
	QString biosof;
	GameInfo *gameinfo;

	if (!romof.isEmpty())
	{
		biosof = romof;
		gameinfo = mamegame->gamenameGameInfoMap[romof];

		if (!gameinfo->romof.isEmpty())
		{
			biosof = gameinfo->romof;
			gameinfo = mamegame->gamenameGameInfoMap[gameinfo->romof];
		}
	}
	
	if (gameinfo && gameinfo->isbios)
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
		out << gameinfo->isbios;
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
	}
	gamelist->switchProgress(-1, "");
	
	guisettings.setValue("list_cache", qCompress(ba, 9));
}

int MameGame::des11n()
{
	QByteArray ba = qUncompress(guisettings.value("list_cache").toByteArray());
	QDataStream in(ba);

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
	if (mamegame)
		delete mamegame;
	mamegame = new MameGame(win);

	// verify MAME Version
	mamegame->mameVersion = utils->getMameVersion();
	QString mameVersion0;
	in >> mameVersion0;

	if (mamegame->mameVersion != mameVersion0)
	{
//		win->poplog(QString("new mame version: %1 / %2").arg(mameVersion0).arg(mamegame->mameVersion));
			return QDataStream::ReadCorruptData;
	}

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
		in >> gameinfo->isbios;
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
	QString str = utils->getViewString(index, COL_NAME);

	if (currentGame == str)
		return QSize(1,33);
	else
		return QSize(1,17);
}

void GamelistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
							 const QModelIndex &index ) const
{
	QModelIndex j = index.sibling(index.row(), COL_NAME);
	QString str = j.model()->data(j, Qt::DisplayRole).toString();

	if (currentGame == str && index.column() == COL_DESC)
	{
		j = index.sibling(index.row(), COL_DESC);

		//draw big icon
		QRect rc = option.rect;
		QPoint p = rc.topLeft();
		p.setX(p.x() + 2);
		rc = QRect(p, rc.bottomRight());

		QVariant v = j.model()->data(j, Qt::DecorationRole);
		v.convert(QVariant::Icon);
		QIcon icon = qvariant_cast<QIcon>(v);
		QApplication::style()->drawItemPixmap (painter, rc, Qt::AlignLeft | Qt::AlignVCenter, icon.pixmap(QSize(32,32)));

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
		str = j.model()->data(j, Qt::DisplayRole).toString();
		QApplication::style()->drawItemText (painter, rc, Qt::AlignLeft | Qt::AlignVCenter, option.palette, true, str, QPalette::HighlightedText);
		return;
	}

	QItemDelegate::paint(painter, option, index);
	return;
}


Gamelist::Gamelist(QObject *parent)
: QObject(parent)
{
//	win->log("DEBUG: Gamelist::Gamelist()");

	connect(&auditThread, SIGNAL(progressSwitched(int, QString)), this, SLOT(switchProgress(int, QString)));
	connect(&auditThread, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
	connect(&auditThread, SIGNAL(finished()), this, SLOT(setupAudit()));

	connect(&iconThread, SIGNAL(finished()), this, SLOT(setupIcon()));
	
	connect(&selectThread, SIGNAL(finished()), this, SLOT(setupHistory()));
	
	if (!searchTimer)
		searchTimer = new QTimer(this);
	connect(searchTimer, SIGNAL(timeout()), this, SLOT(filterRegExpChanged()));

	numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = -1;
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
	win->labelProgress->setText(title);

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

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		QString sel = utils->getViewString(current, COL_NAME);
		if (sel.isEmpty())
			return;
		currentGame = sel;
		
		//update statusbar
		win->labelProgress->setText(utils->getViewString(current, COL_DESC));

		selectThread.myqueue.enqueue(currentGame);
		selectThread.update();

		//update selected rows
		gameListModel->rowChanged(gameListPModel->mapToSource(current));
		gameListModel->rowChanged(gameListPModel->mapToSource(previous));
	}
}

void Gamelist::restoreSelection()
{
	//fixme: hack?
	if (!gameListModel || !gameListPModel)
		return;

	QString gameName = currentGame;
	QModelIndex i;
	
	// select current game
//	win->log("#: " + gameName);
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

	win->treeViewGameList->setCurrentIndex(pi);
	//fixme: PositionAtCenter doesnt work well (auto scroll right)
	win->treeViewGameList->scrollTo(pi, QAbstractItemView::PositionAtTop);
}

void Gamelist::setupIcon()
{
	GameInfo *gameinfo;
	QIcon icon;
	int i = 0;
	static int step = mamegame->gamenameGameInfoMap.count() / 8;
//	win->log("setupIcon()");

/*	
	int viewport_h = win->treeViewGameList->viewport()->height();

	for (int h = 0; h < viewport_h; h += 17)
	{
		gameListModel->rowChanged(gameListPModel->mapToSource(
			win->treeViewGameList->indexAt(QPoint(0, h))));	
	}
*/
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];

		gameListModel->setData(gameListModel->index(0, gameinfo->pModItem), 
			gameinfo->icon, Qt::DecorationRole);

		//reduce stall time when updating icons
		if (i++ % step == 0)
			qApp->processEvents();
	}
}

void Gamelist::setupAudit()
{
	GameInfo *gameinfo;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		gameListModel->setData(gameListModel->index(2, gameinfo->pModItem), 
			gameinfo->available, Qt::DisplayRole);
	}
	mamegame->s11n();
}

void Gamelist::setupHistory()
{
	win->ssSnap->setPixmap(selectThread.pmSnap);
	win->ssFlyer->setPixmap(selectThread.pmFlyer);
	win->ssCabinet->setPixmap(selectThread.pmCabinet);
	win->ssMarquee->setPixmap(selectThread.pmMarquee);
	win->ssTitle->setPixmap(selectThread.pmTitle);
	win->ssCPanel->setPixmap(selectThread.pmCPanel);
	win->ssPCB->setPixmap(selectThread.pmPCB);

	win->tbHistory->setText(selectThread.historyText);
	win->tbMameinfo->setText(selectThread.mameinfoText);
	win->tbStory->setText(selectThread.storyText);
}

void Gamelist::init()
{
	int des11n_status = QDataStream::Ok;
	if (!mamegame)
		des11n_status = mamegame->des11n();	//fixme: illogical call before mamegame init

	if (des11n_status == QDataStream::Ok)
	{
		gameListModel = new TreeModel(win);
		gameListPModel = new GameListSortFilterProxyModel(win);
		gameListPModel->setSourceModel(gameListModel);
		gameListPModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		win->treeViewGameList->setModel(gameListPModel);

		/* init everything else here after we have mamegame */
 
		// init folders
		gamelist->initFolders();

		// init options
		optUtils->loadDefault(mamegame->mameDefaultIni);
		optUtils->loadTemplate();
		
		optUtils->loadIni(OPTNFO_GLOBAL, "mame.ini");

		// connect gameListModel/gameListPModel signals after the view init completed
		// connect gameListModel/gameListPModel signals after mameOpts init
		connect(gameListPModel, SIGNAL(layoutChanged()), this, SLOT(restoreSelection()));
		connect(win->treeViewGameList->selectionModel(),
				SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
				this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));

		connect(win->treeViewGameList,
				SIGNAL(activated(const QModelIndex &)),
				this, SLOT(runMame()));

		win->treeViewGameList->setItemDelegate(&gamelistDelegate);

		// restore view column state
		win->treeViewGameList->header()->restoreState(guisettings.value("column_state").toByteArray());		
		win->treeViewGameList->setSortingEnabled(true);
		win->treeViewGameList->sortByColumn(guisettings.value("sort_column").toInt(), 
			(guisettings.value("sort_reverse").toInt() == 0) ? Qt::AscendingOrder : Qt::DescendingOrder);

		win->treeViewGameList->expandAll();
		win->treeViewGameList->setFocus();

		// restore view selection
		restoreSelection();

		win->setWindowTitle(win->windowTitle() + " - " + mamegame->mameVersion);
		win->actionDefaultOptions->setEnabled(true);

		// auditThread.audit();

		//fixme delete mamegame;
		// mamegame = 0;
	}
	else
	{
		mameOutputBuf = "";
		QString command = mame_binary;
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

	win->labelProgress->setText(QString(tr("Loading listxml: %1 games")).arg(numTotalGames));
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
	QString command = mame_binary;
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

	mamegame->s11n();
	
	//reload gamelist
	init();
	win->log("end of gamelist->loadDefFin()");
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

	GameInfo *gameinfo, *gameinfo2;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{

		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		if (!gameinfo->cloneof.isEmpty())
		{
			gameinfo2 = mamegame->gamenameGameInfoMap[gameinfo->cloneof];
			gameinfo2->clones.insert(gamename);
		}
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

	QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
	text.replace(utils->spaceRegex, "*");

	//fixme: doesnt use filterregexp
	QRegExp regExp(text, Qt::CaseInsensitive, syntax);
	gameListPModel->searchText = text;
	gameListPModel->setFilterRegExp(regExp);
	win->treeViewGameList->expandAll();
	restoreSelection();
}

void Gamelist::filterRegExpChanged2(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	QString filterText;

	if (!current->parent())
	{
		QString folderName = current->text(0);
		filterText = "";

		if (folderName == folderList[FOLDER_AVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_AVAILABLE);
		else if (folderName == folderList[FOLDER_UNAVAILABLE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_UNAVAILABLE);
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAMES);
	}
	else
	{
		QString folderName = current->parent()->text(0);
		filterText = current->text(0);

		if (folderName == folderList[FOLDER_MANUFACTURER])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_MANUFACTURER);
		else if (folderName == folderList[FOLDER_YEAR])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_YEAR);
		else if (folderName == folderList[FOLDER_SOURCE])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_SOURCE);
		else if (folderName == folderList[FOLDER_BIOS])
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_BIOS);
		//fixme
		else
			gameListPModel->setFilterRole(Qt::UserRole + FOLDER_ALLGAMES);
	}

	//fixme: doesnt use filterregexp
	QRegExp regExp(filterText);
	gameListPModel->filterText = filterText;
	gameListPModel->setFilterRegExp(regExp);
	win->treeViewGameList->expandAll();
	restoreSelection();
}

void Gamelist::initFolders()
{
	folderList
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

	GameInfo *gameinfo;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		if (gameinfo->isbios)
			biosList << gamename;
		if (!mftrList.contains(gameinfo->manufacturer))
			mftrList << gameinfo->manufacturer;
		if (!yearList.contains(gameinfo->year))
			yearList << gameinfo->year;
		if (!srcList.contains(gameinfo->sourcefile))
			srcList << gameinfo->sourcefile;
	}
	
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

		if (i == FOLDER_MANUFACTURER)
			foreach (QString name, mftrList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		if (i == FOLDER_YEAR)
			foreach (QString name, yearList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));

		if (i == FOLDER_SOURCE)
			foreach (QString name, srcList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));
		if (i == FOLDER_BIOS)
			foreach (QString name, biosList)
				items[i]->addChild(new QTreeWidgetItem(items[i], QStringList(name)));
	}
	connect(win->treeFolders, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		this, SLOT(filterRegExpChanged2(QTreeWidgetItem *, QTreeWidgetItem *)));
			
	win->treeFolders->header()->hide();
}

void Gamelist::runMame()
{
	// doesnt support multi mame session for now
//	if (procMan->procCount > 0)
//		return;

	QString command = mame_binary;
	QStringList args;
	args << currentGame;
	procMan->process(procMan->start(command, args));
}


GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent)
: QSortFilterProxyModel(parent)
{
}

bool GameListSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	bool result;
	QModelIndex index0, index1, index2;

	QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
	QRegExp regExpSearch(searchText, Qt::CaseInsensitive, syntax);
	
	QRegExp regExpFilter(filterText);
		
//	win->log(QString("filter: %1, %2").arg(searchText).arg(filterText));

	// apply search filter
	index0 = sourceModel()->index(sourceRow, COL_DESC, sourceParent);
	index1 = sourceModel()->index(sourceRow, COL_NAME, sourceParent);

	result = (sourceModel()->data(index0).toString().contains(regExpSearch)
			||sourceModel()->data(index1).toString().contains(regExpSearch));

	// apply folder filter
	if (filterRole() == Qt::UserRole + FOLDER_AVAILABLE)
	{
		index2 = sourceModel()->index(sourceRow, COL_ROM, sourceParent);
		result = result && sourceModel()->data(index2).toString() == "Yes";
	}
	else if (filterRole() == Qt::UserRole + FOLDER_UNAVAILABLE)
	{
		index2 = sourceModel()->index(sourceRow, COL_ROM, sourceParent);
		result = result && sourceModel()->data(index2).toString() != "Yes";
	}
	else if (filterRole() == Qt::UserRole + FOLDER_MANUFACTURER)
	{
		index2 = sourceModel()->index(sourceRow, COL_MFTR, sourceParent);
		result = result && sourceModel()->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_YEAR)
	{
		index2 = sourceModel()->index(sourceRow, COL_YEAR, sourceParent);
		result = result && sourceModel()->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_SOURCE)
	{
		index2 = sourceModel()->index(sourceRow, COL_SRC, sourceParent);
		result = result && sourceModel()->data(index2).toString() == filterText;
	}
	else if (filterRole() == Qt::UserRole + FOLDER_BIOS)
	{
		result = result && sourceModel()->data(index0, Qt::UserRole + FOLDER_BIOS).toString() == filterText;
	}

	return result;
}

