#include <QTextStream>
#include <QStringList>
#include <QFile>
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
extern QString currentGame;
extern MameGame *mamegame;

TreeModel *gameListModel;
GameListSortFilterProxyModel *gameListPModel;
GamelistDelegate *gamelistDelegate;
QTimer *searchTimer = NULL;
QIcon defIcon;

#define MAMEPLUS_SIG 0x704c7553
#define S11N_VER 4

class ListXMLHandler : public QXmlDefaultHandler
{
public:
	ListXMLHandler(int d = 0)
	{
		gameinfo = 0;
		metMameTag = false;
	
		if (mamegame)
			delete mamegame;
		mamegame = new MameGame(qmc2MainWindow);
	}

	bool startElement(const QString & /* namespaceURI */,
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
			gameinfo->romof = attributes.value("romof");
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

private:
	GameInfo *gameinfo;
    QString currentText;
    bool metMameTag;
};

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

	emit progressSwitched(iconFiles.count(), "Loading icons");

	//scan icon dir, assign icons to gameinfo
	GameInfo *gameinfo;
	for (int iconCount = 0; iconCount < iconFiles.count(); iconCount++)
	{
		if ( iconCount % 100 == 0 )
			emit progressUpdated(iconCount);

		QIcon icon = loadWinIco(icoDir + iconFiles[iconCount]);

		QString icofname = iconFiles[iconCount].toLower().remove(".ico");
		if (mamegame->gamenameGameInfoMap.contains(icofname))
		{
			gameinfo = mamegame->gamenameGameInfoMap[icofname];
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
	emit progressSwitched(romFiles.count(), "Auditing games");

	GameInfo *gameinfo, *gameinfo2;
	RomInfo *rominfo;
	//iterate romfiles
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
				continue;

			QuaZipFileInfo info;
			QuaZipFile zipFile(&zip);
			gameinfo = mamegame->gamenameGameInfoMap[gamename];

			for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
			{
				if(!zip.getCurrentFileInfo(&info))
					continue;

				quint32 crc = info.crc;
				//romfile == gamename
				if (gameinfo->crcRomInfoMap.contains(crc))
					gameinfo->crcRomInfoMap[crc]->available = true; 
				//check romfile in clones
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

	qmc2MainWindow->log(LOG_QMC2, "audit 1");
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
	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextColorRole)
	{
		return qVariantFromValue(QColor(Qt::black));
	}

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

	if (index.column() == 2)
	{
		int s = item->data(2).toInt();
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
	//	qmc2MainWindow->log(LOG_QMC2, "# RomInfo()");
}

RomInfo::~RomInfo()
{
	//    qmc2MainWindow->log(LOG_QMC2, "# ~RomInfo()");
}

GameInfo::GameInfo(QObject *parent)
: QObject(parent)
{
	//	qmc2MainWindow->log(LOG_QMC2, "# GameInfo()");
}

GameInfo::~GameInfo()
{
	available = -1;
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

void MameGame::s11n()
{
	QFile file("gui.cache");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_3);

	out << mamegame->gamenameGameInfoMap.count();

	GameInfo *gameinfo;
	RomInfo *rominfo;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];
		out << gamename;
		out << gameinfo->description;
		out << gameinfo->year;
		out << gameinfo->manufacturer;
		out << gameinfo->sourcefile;
		out << gameinfo->cloneof;
		out << gameinfo->romof;
		out << gameinfo->available;
		
		out << gameinfo->clones.count();
		foreach (QString clonename, gameinfo->clones)
			out << clonename;

		out << gameinfo->crcRomInfoMap.count();
		foreach (quint32 crc, gameinfo->crcRomInfoMap.keys())
		{
			rominfo = gameinfo->crcRomInfoMap[crc];
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
		mamegame = new MameGame(qmc2MainWindow);
	}

	GameInfo *gameinfo;
	QString gamename;
	RomInfo *rominfo;
	for (int i = 0; i < gamecount; i++)
	{
		gameinfo = new GameInfo(mamegame);
		in >> gamename;
		in >> gameinfo->description;
		in >> gameinfo->year;
		in >> gameinfo->manufacturer;
		in >> gameinfo->sourcefile;
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
			rominfo = new RomInfo(gameinfo);
			in >> crc;
			in >> rominfo->name;
			in >> rominfo->status;
			in >> rominfo->size;
			gameinfo->crcRomInfoMap[crc] = rominfo;
		}
	}

	return in.status();
}

GamelistDelegate::GamelistDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize GamelistDelegate::sizeHint ( const QStyleOptionViewItem & option, 
								const QModelIndex & index ) const
{
	QModelIndex j = index.sibling(index.row(), 1);
	QString str = j.model()->data(j, Qt::DisplayRole).toString();

	if (currentGame == str && index.column() == 0)
		return QSize(1,33);
	else
		return QSize(1,17);
}

void GamelistDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
				  			const QModelIndex &index ) const
{
	QModelIndex j = index.sibling(index.row(), 1);
	QString str = j.model()->data(j, Qt::DisplayRole).toString();

	if (currentGame == str && index.column() == 0)
	{
		j = index.sibling(index.row(), 0);
		
		//draw big icon
		QRect rc = option.rect;
		
		QVariant v = j.model()->data(j, Qt::DecorationRole);
		v.convert(QVariant::Icon);
		QIcon icon = qvariant_cast<QIcon>(v);
		QApplication::style()->drawItemPixmap (painter, rc, Qt::AlignLeft | Qt::AlignVCenter, icon.pixmap(QSize(32,32)));

		// calc text rect
		QPoint p = rc.topLeft();
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::Gamelist()");
#endif
	connect(&iconthread, SIGNAL(progressSwitched(int, QString)), this, SLOT(switchProgress(int, QString)));
	connect(&iconthread, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
	connect(&iconthread, SIGNAL(finished()), this, SLOT(setupIcon()));

	connect(&auditthread, SIGNAL(progressSwitched(int, QString)), this, SLOT(switchProgress(int, QString)));
	connect(&auditthread, SIGNAL(progressUpdated(int)), this, SLOT(updateProgress(int)));
	connect(&auditthread, SIGNAL(finished()), this, SLOT(setupAudit()));
//	connect(&auditthread, SIGNAL(logUpdated(char, QString)), this, SLOT(log(char, QString)));

	if (!searchTimer)
		searchTimer = new QTimer(this);
	connect(searchTimer, SIGNAL(timeout()), this, SLOT(filterRegExpChanged()));

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

void Gamelist::switchProgress(int max, QString title)
{
	qmc2MainWindow->labelProgress->setText(title);

	if (max != -1)
	{
		qmc2MainWindow->statusbar->addWidget(qmc2MainWindow->progressBarGamelist);
		qmc2MainWindow->progressBarGamelist->setRange(0, max);
		qmc2MainWindow->progressBarGamelist->reset();
		qmc2MainWindow->progressBarGamelist->show();
	}
	else
	{
		qmc2MainWindow->statusbar->removeWidget(qmc2MainWindow->progressBarGamelist);
	}
}

void Gamelist::updateSelection(const QModelIndex & current, const QModelIndex & previous)
{
	if (current.isValid())
	{
		QModelIndex i = current.sibling(current.row(), 1);
		QString str = i.model()->data(i, Qt::DisplayRole).toString();
		currentGame = str;
		updateScreenshot(str);
		
		i = current.sibling(current.row(), 0);
		str = i.model()->data(i, Qt::DisplayRole).toString();
		qmc2MainWindow->labelProgress->setText(str);

		//update for selected rows
		gameListModel->rowChanged(gameListPModel->mapToSource(current));
		gameListModel->rowChanged(gameListPModel->mapToSource(previous));
	}
}

void Gamelist::updateScreenshot(QString gameName)
{
	QPixmap pm = QPixmap("D:/mame/snap/snap/" + gameName + ".png");
	if (pm.isNull())
		pm = QPixmap(":/res/mamegui/about.png");
	qmc2MainWindow->labelSnapshot->setPixmap(pm);
}

void Gamelist::log(char c, QString s)
{
	qmc2MainWindow->log(c, s);
}

void Gamelist::setupIcon()
{
	GameInfo *gameinfo;
	QIcon icon;
	int i = 0;
	static int step = mamegame->gamenameGameInfoMap.count() / 8;
	foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
	{
		gameinfo = mamegame->gamenameGameInfoMap[gamename];

		//complete clone gameinfo with parent icons
		icon = gameinfo->icon; 
		if (icon.isNull() && !gameinfo->cloneof.isEmpty())
			icon = mamegame->gamenameGameInfoMap[gameinfo->cloneof]->icon;
		gameListModel->setData(gameListModel->index(0, gameinfo->pModItem), icon, Qt::DecorationRole);

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

void Gamelist::load()
{
	//connect searchtimer after mainwindow init
	connect(qmc2MainWindow->lineEditSearch, SIGNAL(textChanged(const QString &)), this, SLOT(filterTimer()));

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(LOG_QMC2, "DEBUG: Gamelist::load()");
#endif

	qmc2ReloadActive = qmc2EarlyReloadActive = TRUE;
	qmc2StopParser = FALSE;


	if (!mamegame->des11n())
	{
		QStringList headers;
		headers << "Description" << "Name" << "ROMs" << "Manufacturer" << "Driver" << "Year" << "Clone of";

		gameListModel = new TreeModel(headers, qmc2MainWindow);
		gameListPModel = new GameListSortFilterProxyModel(qmc2MainWindow);
		gameListPModel->setSourceModel(gameListModel);
		gameListPModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		qmc2MainWindow->treeViewGameList->setModel(gameListPModel);
		qmc2MainWindow->treeViewGameList->setSortingEnabled(true);
		qmc2MainWindow->treeViewGameList->sortByColumn(1, Qt::AscendingOrder);
		qmc2MainWindow->treeViewGameList->setColumnWidth(0, 180);
//		qmc2MainWindow->treeViewGameList->resizeColumnToContents(6);

		connect(qmc2MainWindow->treeViewGameList->selectionModel(),
				SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
				this, SLOT(updateSelection(const QModelIndex &, const QModelIndex &)));

		if (gamelistDelegate)
			delete gamelistDelegate;
		gamelistDelegate = new GamelistDelegate(qmc2MainWindow);
		qmc2MainWindow->treeViewGameList->setItemDelegate(gamelistDelegate);



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
	gamelistBuffer.clear();
	mamegame->s11n();
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

void Gamelist::filterRegExpChanged()
{
	// multiple space-separated keywords
	QString text = qmc2MainWindow->lineEditSearch->text();
	// do not search less than 2 chars
	if (text.count() == 1 && text.at(0).unicode() < 0x3000 /* CJK symbols start */)
		return;
	
    QRegExp::PatternSyntax syntax = QRegExp::PatternSyntax(QRegExp::Wildcard);	
	static QRegExp spaceRegex = QRegExp("\\s+");
	text.replace(spaceRegex, "*");
	
    QRegExp regExp(text, Qt::CaseInsensitive, syntax);
    gameListPModel->setFilterRegExp(regExp);
	qApp->processEvents();
}

void Gamelist::filterTimer()
{
	searchTimer->start(200);
	searchTimer->setSingleShot(true);
}

GameListSortFilterProxyModel::GameListSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

bool GameListSortFilterProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex &sourceParent) const
{
	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
	QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);

	return sourceModel()->data(index0).toString().contains(filterRegExp())
		|| sourceModel()->data(index1).toString().contains(filterRegExp());
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

