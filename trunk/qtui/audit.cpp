#include "mamepguimain.h"

void RomAuditor::audit()
{
	if (!isRunning())
	{
		// disable ctrl updating before deleting its model
		win->lvGameList->hide();
		win->layMainView->removeWidget(win->lvGameList);
		win->tvGameList->hide();
		win->layMainView->removeWidget(win->tvGameList);
		// these are reenabled in gamelist->init()
		win->treeFolders->setEnabled(false);
//		win->actionLargeIcons->setEnabled(false);
		win->actionDetails->setEnabled(false);
		win->actionGrouped->setEnabled(false);


		extern TreeModel *gameListModel;
		if (gameListModel)
		{
			delete gameListModel;
			gameListModel = NULL;
		}

		extern GameListSortFilterProxyModel *gameListPModel;
		if (gameListPModel)
		{
			delete gameListPModel;
			gameListPModel = NULL;
		}

		//must clear mamegame in the main thread
		// fixme: currently only console games are cleared
		foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
		{
			GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
			if (gameInfo->isExtRom && utils->isAuditFolder(gameInfo->romof))
			{
				win->log(QString("delete: %1").arg(gameName));
				mamegame->gamenameGameInfoMap.remove(gameName);
				delete gameInfo;
			}
		}
		
		isConsoleFolder = utils->isConsoleFolder();

		start(LowPriority);
	}
}

void RomAuditor::run()
{
	QStringList dirpaths = mameOpts["rompath"]->currvalue.split(";");

	GameInfo *gameInfo, *gameInfo2;
	RomInfo *romInfo;

	if(!isConsoleFolder)
	{
		foreach (QString dirpath, dirpaths)
		{
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
					gameInfo = mamegame->gamenameGameInfoMap[gamename];

					//iterate all files in the zip
					for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
					{
						if(!zip.getCurrentFileInfo(&info))
							continue;

						quint32 crc = info.crc;
						// file crc recognized
						if (!gameInfo)
							win->log(QString("err: %1").arg(gamename));
						if (gameInfo->crcRomInfoMap.contains(crc))
							gameInfo->crcRomInfoMap[crc]->available = true; 
						//check rom for clones
						else
						{
							foreach (QString clonename, gameInfo->clones)
							{
								gameInfo2 = mamegame->gamenameGameInfoMap[clonename];
								if (gameInfo2->crcRomInfoMap.contains(crc))
									gameInfo2->crcRomInfoMap[crc]->available = true; 
							}
						}
					}
				}
			}
		}

		win->log(QString("audit 1.gamecount %1").arg(mamegame->gamenameGameInfoMap.count()));

		/* see if any rom of a game is not available */
		//iterate games
		foreach (QString gamename, mamegame->gamenameGameInfoMap.keys())
		{
			gameInfo = mamegame->gamenameGameInfoMap[gamename];
			//fixme: skip auditing for consoles
			if (gameInfo->isExtRom)
				continue;

			gameInfo->available = 1;
			bool passed = false;	//at least one rom has passed

			//iterate roms
			foreach (quint32 crc, gameInfo->crcRomInfoMap.keys())
			{
				romInfo = gameInfo->crcRomInfoMap[crc];
				if (!romInfo->available)
				{
					if (romInfo->status == "nodump")
						continue;	//passed but ignored

					//check parent
					if (!gameInfo->romof.isEmpty())
					{
						gameInfo2 = mamegame->gamenameGameInfoMap[gameInfo->romof];
						if (gameInfo2->crcRomInfoMap.contains(crc) && gameInfo2->crcRomInfoMap[crc]->available)
						{
							passed = true;
//							win->log(gameInfo->romof + "/" + romInfo->name + " passed");
							continue;	//clone passed
						}

						//check bios
						if (!gameInfo2->romof.isEmpty())
						{
							gameInfo2 = mamegame->gamenameGameInfoMap[gameInfo2->romof];
							if (gameInfo2->crcRomInfoMap.contains(crc) && gameInfo2->crcRomInfoMap[crc]->available)
							{
								passed = true;
//								win->log(gameInfo2->romof + "/" + romInfo->name + " passed");
								continue;	//clone passed
							}
						}
					}

					//failed audit
					gameInfo->available = 0;
					break;
				}
				//parent passed
				else
					passed = true;
			}
			//here

			// special case for all roms are "nodump"
			if (!passed)
			{
				gameInfo->available = 0;
			}
		}
	}
	win->log("audit Arcade finished");


	//audit each console system
	foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
	{
		GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
		{
			if (!utils->isAuditFolder(gameName))
				continue;
		
			auditConsole(gameName);
		}
	}
	win->log("audit Console finished");

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

	gameInfo = mamegame->gamenameGameInfoMap[consoleName];
	DeviceInfo *deviceinfo = gameInfo->nameDeviceInfoMap["cartridge"];
	QString sourcefile = gameInfo->sourcefile;

	QStringList nameFilter;
	foreach (QString ext, deviceinfo->extension)
		nameFilter << "*." + ext;
	nameFilter << "*.zip";

	// iterate all files in the path
	QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
	for (int i = 0; i < files.count(); i++)
	{
		QString gameName = files[i];
		QFileInfo fi(files[i]);
		gameInfo = new GameInfo(mamegame);
		gameInfo->description = fi.completeBaseName();
		gameInfo->isBios = false;
		gameInfo->isExtRom = true;
		gameInfo->romof = consoleName;
		gameInfo->sourcefile = sourcefile;
		gameInfo->available = 1;
		mamegame->gamenameGameInfoMap[dirpath + gameName] = gameInfo;
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
	foreach (QString gameName, mamegame->gamenameGameInfoMap.keys())
	{
		gameInfo = mamegame->gamenameGameInfoMap[gameName];
		if (!gameInfo->nameDeviceInfoMap.isEmpty())
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

		outBuf = "";

		loadProc = procMan->process(procMan->start(command, args, FALSE));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(auditorReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(auditorFinished(int, QProcess::ExitStatus)));
	}
	else
	{
		// must load it in the main thread, or the SLOTs cant get connected
		gamelist->init(true, GAMELIST_INIT_AUDIT);
	}
}

void MergedRomAuditor::auditorReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	outBuf += proc->readAllStandardOutput();
}

void MergedRomAuditor::auditorFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[consoleName];
	QString sourcefile = gameInfo->sourcefile;

// filter rom ext in the merged file
//	foreach (QString ext, deviceinfo->extension)
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

			gameInfo = new GameInfo(mamegame);
			gameInfo->description = fi.completeBaseName();
			gameInfo->isBios = false;
			gameInfo->isExtRom = true;
			gameInfo->romof = consoleName;
			gameInfo->sourcefile = sourcefile;
			gameInfo->available = 1;
			mamegame->gamenameGameInfoMap[consolePath + mergedName + "/" + gameName] = gameInfo;
		}
	}

	audit();
}

