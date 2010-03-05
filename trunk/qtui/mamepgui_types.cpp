#include "mamepgui_types.h"

#include "mamepgui_main.h"

/* global */
MameDat *pMameDat = NULL;
MameDat *pFixDat = NULL;
MameDat *pTempDat = NULL;

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
isHorz(true),
available(GAME_MISSING)
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
		gameInfo = pMameDat->games[romof];

		if (!gameInfo->romof.isEmpty())
		{
			biosof = gameInfo->romof;
//			if (gameInfo->romof.trimmed() == "")
//				win->log("ERR5");
			gameInfo = pMameDat->games[gameInfo->romof];
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


class XmlDatHandler : public QXmlDefaultHandler
{
private:
	GameInfo *gameInfo;
	DeviceInfo *deviceInfo;
	QString currentText;
	bool isDefaultRamOption;
	MameDat *_pMameDat;
	bool metMameTag;
	QString mameTag;
	int method;

public:
	XmlDatHandler(MameDat *__pMameDat, int _method)
	{
		gameInfo = 0;
		_pMameDat = __pMameDat;
		metMameTag = false;
		method = _method;

		if (method == 0)
			mameTag = (isMESS ? "mess" : "mame");
		else
			mameTag = "datafile";
	}

	bool startElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName,
		const QXmlAttributes &attributes)
	{
		if (!metMameTag && qName != mameTag)
			return false;

		if (qName == mameTag)
		{
			metMameTag = true;
			_pMameDat->version = attributes.value("build");
		}
		else if (qName == "game" || qName == "machine")
		{
			//update progress
			static int i;
			gameList->updateProgress(i++);
			qApp->processEvents();
			
			gameInfo = new GameInfo(_pMameDat);
			gameInfo->sourcefile = attributes.value("sourcefile");
			gameInfo->isBios = attributes.value("isbios") == "yes";
			gameInfo->cloneof = attributes.value("cloneof");
			gameInfo->romof = attributes.value("romof");
			gameInfo->sampleof = attributes.value("sampleof");

			_pMameDat->games[attributes.value("name")] = gameInfo;
		}
		//MESS doesnt use "isbios" attrib, so all MESS entries with "biosset" is a bios
		else if ((isMESS || gameInfo->isBios) && qName == "biosset")
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

			//it's a multihash, use insert instead of []
			bool ok;
			gameInfo->roms.insert(attributes.value("crc").toUInt(&ok, 16), romInfo);
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


MameDat::MameDat(QObject *parent, int method) : 
QObject(parent),
loadProc(NULL),
numTotalGames(-1)
{
	if (method == 0)
		return;

	QStringList args;
	args << "-listxml";

	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));

	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadListXmlReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadListXmlFinished(int, QProcess::ExitStatus)));
}

MameDat::MameDat(const QString &fileName)
{
	QString line;

	QFile datFile(fileName);
	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");

		do
		{
			line = in.readLine();
			mameOutputBuf += line;
			mameOutputBuf += "\n";
		}
		while (!line.isNull());
	}

	parseListXml(1);
}

void MameDat::save()
{
//	win->log("start save()");

	QDir().mkpath(CFG_PREFIX + "cache");
	QFile file(CFG_PREFIX + "cache/gamelist.cache");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	out << (quint32)MAMEPLUS_SIG; //mameplus signature
	out << (qint16)S11N_VER; //s11n version
	out.setVersion(QDataStream::Qt_4_4);
	out << version;
	out << defaultIni;	//default.ini
	out << games.size();

	win->log(QString("s11n %1 games").arg(games.size()));

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
			out << gameInfo->biosSets.size();
			foreach (QString name, gameInfo->biosSets.keys())
			{
				BiosSet *biosSet = gameInfo->biosSets[name];
				out << name;
				out << biosSet->description;
				out << biosSet->isDefault;
			}
		}

		/* rom */
		out << gameInfo->roms.size();
		foreach (quint32 crc, gameInfo->roms.keys())
		{
			RomInfo *romInfo = gameInfo->roms.value(crc);
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
			out << gameInfo->disks.size();
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
			out << gameInfo->chips.size();
			foreach (ChipInfo* chipInfo, gameInfo->chips)
			{
				out << chipInfo->name;
				out << chipInfo->tag;
				out << chipInfo->type;
				out << chipInfo->clock;
			}

			/* display */
			out << gameInfo->displays.size();
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

			out << gameInfo->controls.size();
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

int MameDat::load()
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
	qint16 streamVersion;
	in >> streamVersion;
	if (streamVersion != S11N_VER)
	{
		win->log(tr("Cache streamVersion has been updated. A full refresh is required."));
		return QDataStream::ReadCorruptData;
	}

	if (streamVersion < 1)
		in.setVersion(QDataStream::Qt_4_2);
	else
		in.setVersion(QDataStream::Qt_4_4);

	// MAME Version
	version = utils->getMameVersion();
	QString mameVersion0;
	in >> mameVersion0;

	// default mame.ini text
	in >> defaultIni;
	
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
			gameInfo->roms.insert(crc, romInfo);
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

	win->log(QString("loaded %1 games from cache.").arg(gamecount));

	// verify MAME Version
	if (version != mameVersion0)
	{
		win-> log(QString("new MAME version: %1 vs %2").arg(mameVersion0).arg(version));
		return QDataStream::ReadCorruptData;
	}

	return completeData() | in.status();
}

void MameDat::parseListXml(int method)
{
	XmlDatHandler handler(this, method);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QXmlInputSource *pXmlInputSource = new QXmlInputSource();
	pXmlInputSource->setData(mameOutputBuf);

//	win->log("DEBUG: Gamelist::start parseListXml()");

	gameList->switchProgress(numTotalGames, tr("Parsing listxml"));
	reader.parse(*pXmlInputSource);
	gameList->switchProgress(-1, "");

	if (method == 0)
	{
		GameInfo *_gameInfo, *gameInfo0;
		bool noAutoAudit = false;

		// restore previous audit results and other info
		foreach (QString gameName, games.keys())
		{
			_gameInfo = games[gameName];

			if (pTempDat != NULL && pTempDat->games.contains(gameName))
			{
				gameInfo0 = pTempDat->games[gameName];

				//if successfully loaded results, dont auto refresh game list
				_gameInfo->available = gameInfo0->available;
				if (_gameInfo->available == GAME_COMPLETE)
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
		if (pTempDat != NULL)
		{
			foreach (QString gameName, pTempDat->games.keys())
			{
				gameInfo0 = pTempDat->games[gameName];

				if (gameInfo0->isExtRom && 
					//the console is supported by current mame version
					games.contains(gameInfo0->romof))
				{
					_gameInfo = new GameInfo(this);
					_gameInfo->description = gameInfo0->description;
					_gameInfo->isExtRom = true;
					_gameInfo->romof = gameInfo0->romof;
					_gameInfo->sourcefile = gameInfo0->sourcefile;
					_gameInfo->available = GAME_COMPLETE;
					games[gameName] = _gameInfo;
				}
			}
			
			delete pTempDat;
			pTempDat = NULL;
		}
	}
	
	delete pXmlInputSource;
	mameOutputBuf.clear();
}

int MameDat::completeData()
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

void MameDat::loadListXmlReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString buf = proc->readAllStandardOutput();
	
	//mamep: remove windows endl
	buf.replace(QString("\r"), QString(""));
	
	numTotalGames += buf.count("<game name=");
	mameOutputBuf += buf;

	win->logStatus(QString(tr("Loading listxml: %1 games")).arg(numTotalGames));
}

void MameDat::loadListXmlFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	parseListXml();

	QStringList args;
	args << "-showconfig" << "-noreadconfig";

	defaultIni.clear();
	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));
	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadDefaultIniReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadDefaultIniFinished(int, QProcess::ExitStatus)));
	//reload gameList. this is a chained call from loadListXmlFinished()
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), gameList, SLOT(update()));
}

void MameDat::loadDefaultIniReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	//hack for Mac OS X, readAllStandardOutput() may not readAll
	defaultIni.append(proc->readAllStandardOutput());
}

void MameDat::loadDefaultIniFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);
	//fixme move to a better place
	win->setVersion();
}

