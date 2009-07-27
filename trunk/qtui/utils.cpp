#include "7zCrc.h"
#include "7zFile.h"
#include "7zVersion.h"

#include "7zAlloc.h"
#include "7zExtract.h"
#include "7zIn.h"

#include "utils.h"

#include "mamepguimain.h"
#include "gamelist.h"

#undef _DEBUG_
#define LOG_MAME	2

/* global */
Utils *utils;
ProcessManager *procMan = NULL;
QRegExp spaceRegex = QRegExp("\\s+");

Utils::Utils(QObject *parent)
: QObject(parent)
{
	initDescMap();
}

QSize Utils::getScaledSize(QSize orig, QSize bounding, bool forceAspect)
{
	if (!mameGame->games.contains(currentGame))
		return orig;

	GameInfo *gameInfo = mameGame->games[currentGame];

	const float FORCE_ASPECT = 0.75f;
	QSize scaledSize = orig;

	if (forceAspect)
	{
		if (gameInfo->isHorz)
		{
			 //horz
			if (scaledSize.height() < scaledSize.width() * FORCE_ASPECT)
				// need expand height
				scaledSize.setHeight((int)(scaledSize.width() * FORCE_ASPECT));
			else
				// need expand width
				scaledSize.setWidth((int)(scaledSize.height() / FORCE_ASPECT));
		}
		else
		{
			//vert
			if (scaledSize.height() < scaledSize.width() / FORCE_ASPECT)
				// need expand height
				scaledSize.setHeight((int)(scaledSize.width() / FORCE_ASPECT));
			else
				// need expand width
				scaledSize.setWidth((int)(scaledSize.height() * FORCE_ASPECT));
		}
	}

	QSize origSize = scaledSize;

	scaledSize.scale(bounding, Qt::KeepAspectRatio);

	// do not enlarge
	if (!win->actionStretchSshot->isChecked() && (
		scaledSize.width() > origSize.width() || 
		scaledSize.height() > origSize.height()))
		scaledSize = origSize;

	return scaledSize;
}

QString Utils::capitalizeStr(const QString &str)
{
	QStringList strlist = str.split("_");
	// capitalize first char
	strlist[0][0] = strlist[0][0].toUpper();

	return strlist.join(" ");
}

void Utils::lowerTrimmed(QStringList &list)
{
	for (int i = 0; i < list.size(); i++)
		list[i] = list[i].toLower().trimmed();
}

QStringList Utils::split2Str(const QString &str, const QString &separator, bool reverse)
{
	int pos;
	if (reverse)
		pos = str.lastIndexOf(separator);
	else
		pos = str.indexOf(separator);

	if (pos > -1)
		return (QStringList() << str.left(pos).trimmed()
							  << str.right(str.size() - pos - 1).trimmed());

	return QStringList() << str;
}

QString Utils::getPath(QString dirpath)
{
	dirpath.replace("$HOME", QDir::homePath());
	return QDir::cleanPath(dirpath) + "/";	//clean it up
}

QString Utils::getSinglePath(QString dirPaths0, QString fileName)
{
	QStringList dirPaths = dirPaths0.split(";");

	foreach (QString _dirPath, dirPaths)
	{
		QString dirPath = getPath(_dirPath);
		QFile file(dirPath + fileName);

		//start parsing folder .ini
		if (file.open(QFile::ReadOnly | QFile::Text))
			return dirPath + fileName;
	}

	return QString();
}

QString Utils::getDesc(const QString &gameName, bool useLocal)
{
	GameInfo *gameInfo = mameGame->games[gameName];
	if (local_game_list && !gameInfo->lcDesc.isEmpty() && useLocal)
		return gameInfo->lcDesc;
	else
		return gameInfo->description;
}

void Utils::initDescMap()
{
	descMap.insert("joy2way", tr("Joy 2-Way"));
	descMap.insert("joy4way", tr("Joy 4-Way"));
	descMap.insert("joy8way", tr("Joy 8-Way"));
	descMap.insert("paddle", tr("Paddle"));
	descMap.insert("doublejoy2way", tr("Double Joy 2-Way"));
	descMap.insert("doublejoy4way", tr("Double Joy 4-Way"));
	descMap.insert("doublejoy8way", tr("Double Joy 8-Way"));
	descMap.insert("dial", tr("Dial"));
	descMap.insert("lightgun", tr("Lightgun"));
	descMap.insert("pedal", tr("Pedal"));
	descMap.insert("stick", tr("Stick"));
	descMap.insert("trackball", tr("Trackball"));
	descMap.insert("vjoy2way", tr("Joy 2-Way (V)"));
	descMap.insert("vdoublejoy2way", tr("Double Joy 2-Way (V)"));
		
	descMap.insert("baddump", tr("Bad Dump"));
	descMap.insert("nodump", tr("No Dump"));
		
	descMap.insert("raster", tr("Raster"));
	descMap.insert("vector", tr("Vector"));
	descMap.insert("lcd", tr("LCD"));

	descMap.insert("card", tr("PC Card"));
	descMap.insert("cdrom", tr("CD-ROM"));
	descMap.insert("cdrom0", tr("CD-ROM") + " 0");
	descMap.insert("cdrom1", tr("CD-ROM") + " 1");
	descMap.insert("cfcard", tr("CompactFlash Card"));
	descMap.insert("disk", tr("Disk"));
	descMap.insert("disks", tr("Disks"));
	descMap.insert("gdrom", tr("GD-ROM"));
	descMap.insert("ide", tr("IDE"));
	descMap.insert("laserdisc", tr("Laserdisc"));
	descMap.insert("laserdisc2", tr("Laserdisc") + " 2");
	descMap.insert("scsi0", tr("SCSI") + " 0");
	descMap.insert("scsi1", tr("SCSI") + " 1");
	descMap.insert("vhs", tr("VHS"));
}

QString Utils::getShortName(const QString &str)
{	
	if (!descMap.key(str).isEmpty())
		return descMap.key(str);
	
	return str;
}

QString Utils::getLongName(const QString &str)
{
	if (!descMap.value(str).isEmpty())
		return descMap.value(str);

	return str;
}
QString Utils::getMameVersion()
{
	QStringList args;
	args << "-help";
	
	mameVersion = "";

	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));
	//block calling thread
	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(getMameVersionReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(getMameVersionFinished(int, QProcess::ExitStatus)));
	loadProc->waitForFinished();
	return mameVersion;
}

void Utils::getMameVersionReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	mameVersion += proc->readAllStandardOutput();
}

void Utils::getMameVersionFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	mameVersion.replace(QRegExp(".*(\\d+\\.[^ ]+\\s+\\([\\w\\s]+\\)).*"), "\\1");
//	0.124u4a (Apr 24 2008)
	win->log(QString("mamever: %1").arg(mameVersion));
}

quint8 Utils::getStatus(QString status)
{
	if(status == "good")
		return 1;
	else if(status == "imperfect")
		return 2;
	else if(status == "preliminary")
		return 0;
	else if(status == "supported")
		return 1;
	else if(status == "unsupported")
		return 0;
	else
		return 64;	//error
}

QString Utils::getStatusString(quint8 status, bool isSaveState)
{
	if(!isSaveState)
	{
		if(status == 1)
			return QT_TR_NOOP("good");
		else if(status == 2)
			return QT_TR_NOOP("imperfect");
		else if(status == 0)
			return QT_TR_NOOP("preliminary");
	}
	else
	{
		if(status == 1)
			return QT_TR_NOOP("supported");
		else if(status == 0)
			return QT_TR_NOOP("unsupported");
	}

	return "unknown";	//error
}

void Utils::clearMameFileInfoList(QHash<QString, MameFileInfo *> mameFileInfoList)
{
	foreach (QString key, mameFileInfoList.keys())
		delete mameFileInfoList[key];
}

bool Utils::matchMameFile(const QString &fileName, const QStringList &fileNameFilters)
{
	foreach (QString fileNameFilter, fileNameFilters)
	{
		//no filter
		if (fileNameFilter == "*")
			return true;

		//wildcard filter
		if (fileNameFilter.startsWith("*."))
		{
			//remove *
			fileNameFilter.remove(0, 1);
			if (fileName.endsWith(fileNameFilter, Qt::CaseInsensitive))
				return true;
		}
		//filename filter
		else
		{
			if (QString::compare(fileName, fileNameFilter, Qt::CaseInsensitive) == 0)
				return true;
		}
	}

	return false;
}

bool Utils::extractMameFile(const QString &fileName, MameFileInfo *mameFileInfo)
{
	bool result = false;
	
	QFileInfo fileInfo(fileName);
	QFile outFile(utils->getPath(QDir::tempPath()) + fileInfo.fileName());

	if (outFile.open(QIODevice::WriteOnly))
	{
		qint64 bytes = outFile.write(mameFileInfo->data);
		if (bytes == mameFileInfo->size)
			result = true;
	}

	outFile.close();
	
	return result;
}

//fixme: filter dir from zip and path, handle paths in the zip/7z, cases
QHash<QString, MameFileInfo *> Utils::iterateMameFile(const QString &_dirPaths, const QString &_archNames, const QString &_fileNameFilters, int method)
{
//fixme: clear mameFileInfoList
	QHash<QString, MameFileInfo *> mameFileInfoList;

	bool isSingleFile = true;
	QStringList dirPaths = _dirPaths.split(";");

	QStringList archNames = _archNames.split(";");
	QStringList fileNameFilters = _fileNameFilters.split(";");
	if (fileNameFilters.first().startsWith("*"))
		isSingleFile = false;

	MameFileInfo *mameFileInfo;

	// iterate split dirpath
	foreach (QString _dirPath, dirPaths)
	{
		_dirPath = utils->getPath(_dirPath);

		foreach (QString archName, archNames)
		{
			if (isSingleFile && mameFileInfoList.size() > 0)
				break;
		
			// iterate all files in the path
			QString dirPath = utils->getPath(_dirPath + archName);
			QDir dir(dirPath);
			QStringList fileNames = dir.entryList(fileNameFilters, QDir::Files | QDir::Readable);
//			win->log(QString("testing: %1, %2").arg(dirPath).arg(fileNames.count()));

			for (int i = 0; i < fileNames.count(); i++)
			{
//				win->log("testing: " + dirPath + fileNames[i]);
				QFile file(dirPath + fileNames[i]);
				if (!file.open(QIODevice::ReadOnly))
					continue;

				QFileInfo fileInfo(file);

				//already loaded
				QString fileName = fileInfo.fileName();
				if (mameFileInfoList.contains(fileName))
					continue;

				bool isCHD = fileName.endsWith(".chd");

				mameFileInfo = new MameFileInfo();
				mameFileInfo->size = file.size();
				if (method != MAMEFILE_EXTRACT && !isCHD)
					mameFileInfo->data = file.readAll();
				if (method == MAMEFILE_GETINFO && !isCHD)
				{
					mameFileInfo->crc = crc32(0L, Z_NULL, 0);
					mameFileInfo->crc = crc32(mameFileInfo->crc, (const Bytef*)mameFileInfo->data.data(), mameFileInfo->size);
				}
				else
					mameFileInfo->crc = 0L;
				mameFileInfoList.insert(fileName, mameFileInfo);

				if (isSingleFile)
					break;
			}
		}

		// generate a default archName
		QString archName = archNames.first();
		if (archName.isEmpty())
		{
			QString fileNameFilter = fileNameFilters.first();
			if (fileNameFilter.startsWith("*."))
			{
				// use nearest folder name
				QDir dir(_dirPath);
				archName = dir.dirName();
			}
			else
			{
				// use file base name
				QFileInfo fileInfo(_dirPath + fileNameFilter);
				archName = fileInfo.baseName();
			}
		}

		// iterate all fileNames in the .zip
		QuaZip zip(_dirPath + archName + ZIP_EXT);
//		win->log("testing: " + _dirPath + archName + ZIP_EXT);
		if(zip.open(QuaZip::mdUnzip))
		{
			for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
			{
				if (isSingleFile && mameFileInfoList.size() > 0)
					break;
			
				QuaZipFile inFile(&zip);
				if (!inFile.open(QIODevice::ReadOnly))
					continue;

				QuaZipFileInfo zipFileInfo;
				if(!zip.getCurrentFileInfo(&zipFileInfo))
					continue;

				//no directories
				if (zipFileInfo.name.endsWith("/"))
					continue;

				//already loaded
				if (mameFileInfoList.contains(zipFileInfo.name))
					continue;

				if (!matchMameFile(zipFileInfo.name, fileNameFilters))
					continue;

				mameFileInfo = new MameFileInfo();
				mameFileInfo->size = zipFileInfo.uncompressedSize;
				if (method != MAMEFILE_GETINFO)
					mameFileInfo->data = inFile.readAll();
				mameFileInfo->crc = zipFileInfo.crc;
				mameFileInfoList.insert(zipFileInfo.name, mameFileInfo);

				if (method == MAMEFILE_EXTRACT)
				{
					bool re = extractMameFile(zipFileInfo.name, mameFileInfo);
//					win->log(QString("ext re: %1").arg(re));
				}

				inFile.close();
			}
		}
		zip.close();

		// iterate all fileNames in the .7z
		// implementation from LZMA SDK's 7zMain.c
		CFileInStream archiveStream;
		CLookToRead lookStream;
		CSzArEx db;
		SRes res;
		ISzAlloc allocImp;
		ISzAlloc allocTempImp;

		if (InFile_Open(&archiveStream.file,  qPrintable(_dirPath + archName + SZIP_EXT)))
			continue;

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
				if (isSingleFile && mameFileInfoList.size() > 0)
					break;

				size_t offset;
				size_t outSizeProcessed;
				CSzFileItem *f = db.db.Files + i;

				//no directories
				if (f->IsDir)
				{
					continue;
				}

				//already loaded
				if (mameFileInfoList.contains(f->Name))
					continue;

				if (!matchMameFile(f->Name, fileNameFilters))
					continue;

				res = SzAr_Extract(&db, &lookStream.s, i,
					&blockIndex, &outBuffer, &outBufferSize,
					&offset, &outSizeProcessed,
					&allocImp, &allocTempImp);

				if (res != SZ_OK)
					break;

				mameFileInfo = new MameFileInfo();
				mameFileInfo->size = outSizeProcessed;
				if (method != MAMEFILE_GETINFO)
					mameFileInfo->data = QByteArray((const char *)outBuffer + offset, outSizeProcessed);
				mameFileInfo->crc = f->FileCRC;
				mameFileInfoList.insert(f->Name, mameFileInfo);

				if (method == MAMEFILE_EXTRACT)
				{
					bool re = extractMameFile(f->Name, mameFileInfo);
					win->log(QString("ext re: %1").arg(re));
				}
			}
			IAlloc_Free(&allocImp, outBuffer);
		}

		SzArEx_Free(&db, &allocImp);
		File_Close(&archiveStream.file);
	}
	return mameFileInfoList;
}

MyQueue::MyQueue(QObject *parent)
: QObject(parent)
{
	capacity = 1;
}

void MyQueue::setSize(int c)
{
	capacity = c;
}

QString MyQueue::dequeue()
{
//	emit logStatusUpdated(QString("deQueue: %1 %2").arg(queue.count()).arg(capacity));

	return queue.dequeue();
}

void MyQueue::enqueue(const QString & str)
{
//	emit logStatusUpdated(QString("enQueue: %1 %2").arg(queue.count()).arg(capacity));

	QMutexLocker locker(&mutex);
	// unique values only
	if (!queue.contains(str))
	{
		queue.enqueue(str);

		// pop if overflow
		if (queue.count() > capacity)
			queue.dequeue();
	}
}

bool MyQueue::isEmpty() const
{
	return queue.isEmpty();
}

bool MyQueue::contains (const QString &value) const
{
	return queue.contains(value);
}

QString MyQueue::value(int i)
{
	return queue.value(i);
}

int MyQueue::count() const
{
	return queue.count();
}


ProcessManager::ProcessManager(QWidget *parent) : 
QObject(parent),
procCount(0)
{
}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect)
{
#ifdef _DEBUG_
	QString logMsg = "DEBUG: ProcessManager::start(QString &command = \"" + command + "\", QStringList &arguments = \"";
	int argCount;
	for (argCount = 0; argCount < arguments.count(); argCount++)
		logMsg += QString(argCount > 0 ? " " + arguments[argCount] : arguments[argCount]);
	logMsg += "\", bool autoConnect = " + QString(autoConnect ? "TRUE" : "FALSE") + ")";
#endif

	QProcess *proc = new QProcess(this);
	if (autoConnect)
	{
		lastCommand = command;

		for (int i = 0; i < arguments.count(); i++)
			lastCommand += " " + arguments[i];

		connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
		connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
		connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
		connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
		connect(proc, SIGNAL(started()), this, SLOT(started()));
	}
	
#ifdef Q_WS_WIN
	//explicitly assign WorkingDirectory during M1 loading
	proc->setWorkingDirectory(currentDir);
#endif
	procMap[proc] = procCount++;
	proc->start(command, arguments);
	return procCount - 1;
}

QProcess *ProcessManager::process(ushort index)
{
	QList<QProcess *> vl = procMap.keys(index);
	if ( vl.count() > 0 )
		return vl.at(0);
	else
		return NULL;
}

void ProcessManager::terminate(QProcess *proc)
{
	proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
	terminate(process(index));
}

void ProcessManager::kill(QProcess *proc)
{
	proc->kill();
}

void ProcessManager::kill(ushort index)
{
	kill(process(index));
}

void ProcessManager::readyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

	QString s = proc->readAllStandardOutput();
	QStringList sl = s.split("\n");
	int i;
	for (i = 0; i < sl.count(); i++)
	{
		s = sl[i].simplified();
		if ( !s.isEmpty() )
			win->log(QString("stdout[#%1]: ").arg(procMap[proc]) + s, LOG_MAME);
	}
}

void ProcessManager::readyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();

	QString s = proc->readAllStandardError();
	QStringList sl = s.split("\n");
	int i;
	for (i = 0; i < sl.count(); i++)
	{
		s = sl[i].simplified();
		if ( !s.isEmpty() )
			win->log(QString("stderr[#%1]: ").arg(procMap[proc]) + s, LOG_MAME);
	}
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	win->log(QString("proc #%1 finished, exit: %2, remaining: %3").arg(procMap[proc]).arg(exitCode).arg(procMap.count() - 1));
	procMap.remove(proc);
}

void ProcessManager::started()
{
	QProcess *proc = (QProcess *)sender();

	win->log(QString("proc #%1 started, active: %3").arg(procMap[proc]).arg(procMap.count()));
}

void ProcessManager::error(QProcess::ProcessError processError)
{
}

