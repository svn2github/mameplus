#include "audit.h"

#include "mamepguimain.h"
#include "mameopt.h"

void RomAuditor::audit(bool autoAudit)
{
	if (!isRunning())
	{
		gameList->disableCtrls();

		//must clear mameGame in the main thread
		// fixme: currently only console games are cleared
		foreach (QString gameName, mameGame->games.keys())
		{
			GameInfo *gameInfo = mameGame->games[gameName];
			if (gameInfo->isExtRom && utils->isAuditFolder(gameInfo->romof))
			{
				mameGame->games.remove(gameName);
				delete gameInfo;
			}
		}

		isConsoleFolder = utils->isConsoleFolder();
		if (autoAudit)
		{
			gameList->autoAudit = false;
			isConsoleFolder = false;
		}

		start(LowPriority);
	}
}

void RomAuditor::run()
{
	QStringList dirpaths = mameOpts["rompath"]->currvalue.split(";");

	GameInfo *gameInfo, *gameInfo2;
	RomInfo *romInfo;

	//audit MAME
	if(!isConsoleFolder)
	{
		QSet<QString> auditedGames;

		//clear current state
		foreach (QString gameName, mameGame->games.keys())
		{
			gameInfo = mameGame->games[gameName];

			foreach (RomInfo *romInfo, gameInfo->roms)
				if (romInfo->status == "nodump")
					romInfo->available = true;
				else
					romInfo->available = false;

			foreach (DiskInfo *diskInfo, gameInfo->disks)
			{
				if (diskInfo->status == "nodump")
					diskInfo->available = true;
				else
					diskInfo->available = false;
			}
		}

		//iterate rompaths
		foreach (QString dirpath, dirpaths)
		{
			QDir dir(dirpath);
			QStringList nameFilter = QStringList() << "*" + ZIP_EXT;
			QStringList romFiles = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
			QStringList romDirs = dir.entryList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable);

			emit progressSwitched(romFiles.count(), tr("Auditing") + " " + dirpath + " ...");

			//iterate gameName/*.chd files
			foreach (QString romDir, romDirs)
			{
				QDir dir2(utils->getPath(dirpath) + romDir);
				QString gameName = dir2.dirName();

				if (!mameGame->games.contains(gameName))
					continue;

				gameInfo = mameGame->games[gameName];

				QStringList nameFilter2 = QStringList() << "*.chd";
				QStringList chdFiles = dir2.entryList(nameFilter2, QDir::Files | QDir::Readable);

				foreach (QString chdFile, chdFiles)
				{
					QFileInfo fileInfo = QFileInfo(chdFile);

					foreach (QString sha1, gameInfo->disks.keys())
					{
						DiskInfo *diskInfo = gameInfo->disks[sha1];

						if (diskInfo->name == fileInfo.baseName())
						{
							diskInfo->available = true;

							//also fill clones
							foreach (QString cloneName, gameInfo->clones)
							{
								gameInfo2 = mameGame->games[cloneName];
								if (gameInfo2->disks.contains(sha1))
									gameInfo2->disks[sha1]->available = true;
							}
						}
					}
				}
			}
				
			//iterate rom files
			for (int i = 0; i < romFiles.count(); i++)
			{
				//update progressbar per 100 files
				if (i % 100 == 0)
					emit progressUpdated(i);

				QString gameName = romFiles[i].toLower().remove(ZIP_EXT);

				if (!mameGame->games.contains(gameName))
					continue;
				gameInfo = mameGame->games[gameName];

				auditedGames.insert(gameName);

				//open zip file
				QuaZip zip(utils->getPath(dirpath) + romFiles[i]);
				if(!zip.open(QuaZip::mdUnzip))
					continue;

				QuaZipFileInfo zipFileInfo;
				QuaZipFile zipFile(&zip);

				//iterate all files in the zip
				for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
				{
					if(!zip.getCurrentFileInfo(&zipFileInfo))
						continue;

					//fill rom available status if crc recognized
					if (gameInfo->roms.contains(zipFileInfo.crc))
						gameInfo->roms[zipFileInfo.crc]->available = true;

					//check if rom belongs to a clone
					foreach (QString cloneName, gameInfo->clones)
					{
						auditedGames.insert(cloneName);
						gameInfo2 = mameGame->games[cloneName];
						if (gameInfo2->roms.contains(zipFileInfo.crc))
							gameInfo2->roms[zipFileInfo.crc]->available = true;
					}
				}
			}
		}

//		win->log(QString("audit 1.gamecount %1").arg(mameGame->games.count()));

		/* see if any rom of a game is not available */
		//iterate games
		foreach (QString gameName, mameGame->games.keys())
		{
			gameInfo = mameGame->games[gameName];
			//fixme: skip auditing for consoles
			if (gameInfo->isExtRom)
				continue;

			//reset isCloneAvailable, will call completeData() later
			gameInfo->isCloneAvailable = false;

			//if game rom file exists, default to passed, if not, skip and fail it
			if (auditedGames.contains(gameName))
				gameInfo->available = 1;
			else
			{
				//fail unless: 1. all roms are nodump; 2. all roms are available in parent
				bool allNoDump = true;
				bool allinParent = true;

				foreach (RomInfo *romInfo, gameInfo->roms)
				{
					if (!romInfo->available)
					{
						allinParent = false;
						break;
					}
				}

				foreach (RomInfo *romInfo, gameInfo->roms)
				{
					if (romInfo->status != "nodump")
					{
						allNoDump = false;
						break;
					}
				}

				gameInfo->available = (allNoDump || allinParent) ? 1 : 0;
			}
			//shortcircuit
			if (gameInfo->available == 0)
				continue;

			//iterate disks
			foreach (DiskInfo *diskInfo, gameInfo->disks)
			{
				if (!diskInfo->available)
				{
					//win->log("NA :" + gameName + "/" + diskInfo->name);
					gameInfo->available = 0;
					break;
				}
			}
			//shortcircuit
			if (gameInfo->available == 0)
				continue;

			//iterate roms
			foreach (quint32 crc, gameInfo->roms.keys())
			{
				romInfo = gameInfo->roms[crc];

				//game rom passed
				if (romInfo->available)
					continue;

				if (!gameInfo->romof.isEmpty())
				{
					//check parent
					gameInfo2 = mameGame->games[gameInfo->romof];

					//parent rom passed
					if (gameInfo2->roms.contains(crc) && gameInfo2->roms[crc]->available)
						continue;

					//check bios
					if (!gameInfo2->romof.isEmpty())
					{
						gameInfo2 = mameGame->games[gameInfo2->romof];

						//bios rom passed
						if (gameInfo2->roms.contains(crc) && gameInfo2->roms[crc]->available)
							continue;
					}
				}

				//failed audit
				gameInfo->available = 0;
				break;
			}
		}
	}
//	win->log("finished auditing MAME games.");

	mameGame->completeData();

	//audit each MESS system
	foreach (QString gameName, mameGame->games.keys())
	{
		GameInfo *gameInfo = mameGame->games[gameName];
		if (!gameInfo->devices.isEmpty())
		{
			if (!utils->isAuditFolder(gameName))
				continue;
		
			auditConsole(gameName);
		}
	}
//	win->log("finished auditing MESS systems.");

	emit progressSwitched(-1);
}

// must be static func in a thread
void RomAuditor::auditConsole(QString consoleName)
{
	GameInfo *gameInfo;

	QString _dirpath = mameOpts[consoleName + "_extra_software"]->globalvalue;
	QDir dir(_dirpath);
	if (_dirpath.isEmpty() || !dir.exists())
		return;
	QString dirpath = utils->getPath(_dirpath);

	gameInfo = mameGame->games[consoleName];

	QString sourcefile = gameInfo->sourcefile;

	QStringList nameFilter;
	foreach (DeviceInfo *deviceInfo, gameInfo->devices)
		foreach (QString ext, deviceInfo->extensionNames)
			nameFilter << "*." + ext;
	nameFilter << "*" + ZIP_EXT + "";

	// iterate all files in the path
	QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
	for (int i = 0; i < files.count(); i++)
	{
		QString gameName = files[i];
		QFileInfo fi(files[i]);
		gameInfo = new GameInfo(mameGame);
		gameInfo->description = fi.completeBaseName();
		gameInfo->isExtRom = true;
		gameInfo->romof = consoleName;
		gameInfo->sourcefile = sourcefile;
		gameInfo->available = 1;
		mameGame->games[dirpath + gameName] = gameInfo;
	}

//	win->poplog(QString("%1, %2").arg(consoleName).arg(_dirpath));
}

RomAuditor::~RomAuditor()
{
	wait();
}


MergedRomAuditor::MergedRomAuditor(QObject *parent)
: QObject(parent)
{
}

void MergedRomAuditor::init()
{
	//init loader, put sw path of all console systems to a list, for non-param pop operation needed by process mgt
	consoleInfoList.clear();
	GameInfo *gameInfo;
	foreach (QString gameName, mameGame->games.keys())
	{
		gameInfo = mameGame->games[gameName];
		if (!gameInfo->devices.isEmpty())
		{
			if (!utils->isAuditFolder(gameName))
				continue;
		
			QString _dirpath = mameOpts[gameName + "_extra_software"]->globalvalue;
			QDir dir(_dirpath);
			if (_dirpath.isEmpty() || !dir.exists())
				continue;
			QString dirpath = utils->getPath(_dirpath);

			QStringList nameFilter;
			nameFilter << "*.7z";
			QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);

			foreach (QString fileName, files)
			{
				QStringList consoleInfo;
			    consoleInfo << gameName << dirpath << fileName;
				consoleInfoList.append(consoleInfo);
			}
		}
	}

	audit();
}

void MergedRomAuditor::audit()
{
	QString command = "bin/7z";

	if (!consoleInfoList.isEmpty())
	{
		QStringList consoleInfo = consoleInfoList.takeFirst();

		consoleName = consoleInfo[0];
		consolePath = consoleInfo[1];
		mergedName = consoleInfo[2];

		QStringList args;
		args << "l" << consolePath + mergedName  << "-slt";

		win->logStatus(consolePath + mergedName);

		outBuf.clear();

		loadProc = procMan->process(procMan->start(command, args, FALSE));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(auditorReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(auditorFinished(int, QProcess::ExitStatus)));
	}
	else
	{
		// must load it in the main thread, or the SLOTs cant get connected
		gameList->init(true, GAMELIST_INIT_AUDIT);
	}
}

void MergedRomAuditor::auditorReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	outBuf.append(proc->readAllStandardOutput());
}

void MergedRomAuditor::auditorFinished(int, QProcess::ExitStatus)
{
	QProcess *proc = (QProcess *)sender();
	procMan->procMap.remove(proc);

	GameInfo *gameInfo = mameGame->games[consoleName];
	QString sourcefile = gameInfo->sourcefile;

// filter rom ext in the merged file
//	foreach (QString ext, deviceinfo->extensionNames)
//		nameFilter << "*." + ext;

	QStringList bufs = outBuf.split(QRegExp("[\\r\\n]+"));
	for (int i = 0; i < bufs.count(); i++)
	{
		QString line = bufs[i];
		if (line.startsWith("Path = "))
		{
			line.remove(0, 7);

			QString gameName = line;
			QFileInfo fi(line);

			gameInfo = new GameInfo(mameGame);
			gameInfo->description = fi.completeBaseName();
			gameInfo->isExtRom = true;
			gameInfo->romof = consoleName;
			gameInfo->sourcefile = sourcefile;
			gameInfo->available = 1;
			mameGame->games[consolePath + mergedName + "/" + gameName] = gameInfo;
		}
	}

	audit();
}


MameExeRomAuditor::MameExeRomAuditor(QObject *parent) : 
QObject(parent)
{
}

void MameExeRomAuditor::auditorReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	outBuf.append(QString::fromLocal8Bit(proc->readAllStandardOutput().data()));
}

void MameExeRomAuditor::auditorFinished(int, QProcess::ExitStatus)
{
	QMessageBox msgBox;
	msgBox.setText(outBuf);
	msgBox.exec();
}

void MameExeRomAuditor::audit()
{
	QStringList args;
	args << "-verifyroms" << currentGame;
	if (isMAMEPlus)
	{
		QString langpath = utils->getPath(mameOpts["langpath"]->globalvalue);
		args << "-langpath" << langpath;
		args << "-language" << language;
	}

	outBuf.clear();

	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));

	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(auditorReadyReadStandardOutput()));
	connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(auditorFinished(int, QProcess::ExitStatus)));
}

