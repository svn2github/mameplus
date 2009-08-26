#include "audit.h"

#include "mamepguimain.h"
#include "mameopt.h"

RomAuditor::RomAuditor(QObject *parent) : 
QThread(parent),
hasAudited(false),
method(AUDIT_ONLY)
{
}

RomAuditor::~RomAuditor()
{
	wait();
}

void RomAuditor::exportDat()
{
	if (method == AUDIT_ONLY || fixDatFileName.isEmpty())
		return;

	QFile outFile(fixDatFileName);

	if (outFile.open(QFile::WriteOnly | QFile::Text))
	{
		QXmlStreamWriter out(&outFile);
		out.setAutoFormatting(true);
		out.setAutoFormattingIndent(-1);

		GameInfo *gameInfo;

		QStringList gameNames;
		//append all parent names to the list and sort it
		foreach (QString gameName, mameGame->games.keys())
		{
			gameInfo = mameGame->games[gameName];
			if (gameInfo->cloneof.isEmpty())
				gameNames.append(gameName);
		}
		gameNames.sort();

		QStringList gameNames_copy(gameNames);

		//insert all sorted clone names to the list
		int i = 0;
		foreach (QString gameName, gameNames_copy)
		{
			gameInfo = mameGame->games[gameName];

			QList<QString> cloneList = gameInfo->clones.toList();
			qSort(cloneList);

			foreach (QString cloneName, cloneList)
				gameNames.insert(++i, cloneName);

			i ++;
		}

		//start writing xml
		out.writeStartDocument();
		out.writeDTD("<!DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\">");
		out.writeStartElement("datafile");

		foreach (QString gameName, gameNames)
		{
			gameInfo = mameGame->games[gameName];

			if (gameInfo->available == 0)
			{
				GameInfo *gameInfo2 = NULL, *gameBiosInfo = NULL;
				RomInfo *romInfo;

				//we'd like to sort the roms, QMap is used
				QMap<QString, quint32> missingRoms;

				//if all roms of a game are missing, excluding missing bios
				bool completelyMissing = false,
				//if all roms of a clone are missing
					 completelyMissingClone = true;

				//number of missing roms and nodumps
				int missingCount = 0,
					nodumpCount = 0,
					//dont count bioses #3
					biosRomsCount = 0;

				//the game doesnt have a parent, default completelyMissingClone to false
				if (gameInfo->romof.isEmpty())
					completelyMissingClone = false;
				else
				//find parent (gameInfo2) and bios (gameBiosInfo) of the game
				{
					gameInfo2 = mameGame->games[gameInfo->romof];

					if (!gameInfo2->romof.isEmpty())
						gameBiosInfo = mameGame->games[gameInfo2->romof];
					else if (gameInfo2->isBios)
						gameBiosInfo = gameInfo2;
				}

				if (gameBiosInfo != NULL)
					biosRomsCount = gameBiosInfo->roms.size();

				//start iterating all roms
				foreach (quint32 crc, gameInfo->roms.keys())
				{
					romInfo = gameInfo->roms[crc];

					if (romInfo->status == "nodump")
						nodumpCount++;

					if (!romInfo->available)
					{
						//to reduce redundant data, continue loop if parent is also missing this rom
						if (gameInfo2 != NULL && 
							gameInfo2->roms.contains(crc) && !gameInfo2->roms[crc]->available)
						{
							//dont count bioses #1
							if (/*!gameInfo2->isBios && */ (gameBiosInfo == NULL || !gameBiosInfo->roms.contains(crc)))
								missingCount++;
							continue;
						}

						missingRoms.insert(romInfo->name, crc);
						//dont count bioses #2
						if (gameBiosInfo == NULL || !gameBiosInfo->roms.contains(crc))
							missingCount++;
					}
					else if (completelyMissingClone && gameInfo2 != NULL && !gameInfo2->roms.contains(crc))
					{
						//clone-specific rom is available
						completelyMissingClone = false;
					}
				}

				if (missingRoms.size() == 0)
					continue;

				//it is possible that bios roms are nodump
				if (missingCount >= gameInfo->roms.size() - biosRomsCount - nodumpCount)
					completelyMissing = true;

				if (gameInfo2 != NULL)
					completelyMissing = completelyMissing || completelyMissingClone;

				//toggle export methods
				if (method == AUDIT_EXPORT_INCOMPLETE && completelyMissing || 
					method == AUDIT_EXPORT_MISSING && !completelyMissing)
					continue;

				//write xml
				out.writeStartElement("game");
				out.writeAttribute("name", gameName);
				out.writeAttribute("sourcefile", gameInfo->sourcefile);
				if (!gameInfo->romof.isEmpty())
					out.writeAttribute("romof", gameInfo->romof);

				out.writeTextElement("description", gameInfo->description);
				if (!gameInfo->year.isEmpty())
					out.writeTextElement("year", gameInfo->year);
				out.writeTextElement("manufacturer", gameInfo->manufacturer);

				foreach (quint32 crc, missingRoms)
				{
					romInfo = gameInfo->roms[crc];

					out.writeStartElement("rom");
					out.writeAttribute("name", romInfo->name);
					out.writeAttribute("size", QString("%1").arg(romInfo->size));
					out.writeAttribute("crc", QString("%1").arg(crc, 8, 16, QLatin1Char('0')));
					out.writeEndElement();
				}

				out.writeEndElement();
			}
		}
		out.writeEndDocument();
	}
	
	outFile.close();

	win->poplog("Finished.");
}

void RomAuditor::audit(bool autoAudit, int _method, QString fileName)
{
	if (!isRunning())
	{
		method = _method;
		fixDatFileName = fileName;

		//skip auditing and go export directly
		if (method != AUDIT_ONLY && hasAudited)
		{
			exportDat();
			return;
		}
	
		gameList->disableCtrls();

		//must clear mameGame in the main thread
		// fixme: currently only console games are cleared
		foreach (QString gameName, mameGame->games.keys())
		{
			GameInfo *gameInfo = mameGame->games[gameName];
			if (gameInfo->isExtRom && gameList->isAuditConsoleFolder(gameInfo->romof))
			{
				mameGame->games.remove(gameName);
				delete gameInfo;
			}
		}

		isConsoleFolder = gameList->isConsoleFolder();
		if (autoAudit)
		{
			gameList->autoAudit = false;
			isConsoleFolder = false;
		}

		hasAudited = true;
		start(LowPriority);
	}
}

void RomAuditor::run()
{
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

		QStringList dirPaths = mameOpts["rompath"]->currvalue.split(";");
		//iterate rompaths
		foreach (QString dirPath, dirPaths)
		{
			QDir dir(dirPath);
			QStringList nameFilter = QStringList() << "*" ZIP_EXT;
			QStringList romFiles = dir.entryList(nameFilter, QDir::Files | QDir::Readable | QDir::Hidden);
			QStringList romDirs = dir.entryList(QStringList(), QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable | QDir::Hidden);

			emit progressSwitched(romFiles.count(), QString(tr("Auditing %1 ...")).arg(dir.dirName() + "/"));

			//iterate gameName/*.chd files
			foreach (QString romDir, romDirs)
			{
				QDir dir2(utils->getPath(dirPath) + romDir);
				QString gameName = dir2.dirName().toLower();

				if (!mameGame->games.contains(gameName))
					continue;

				gameInfo = mameGame->games[gameName];

				QStringList nameFilter2 = QStringList() << "*.chd";
				QStringList chdFiles = dir2.entryList(nameFilter2, QDir::Files | QDir::Readable | QDir::Hidden);

				foreach (QString chdFile, chdFiles)
				{
					QFileInfo fileInfo = QFileInfo(chdFile);

					foreach (QString sha1, gameInfo->disks.keys())
					{
						DiskInfo *diskInfo = gameInfo->disks[sha1];

						if (diskInfo->name == fileInfo.baseName().toLower())
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
				//update progressbar every 10 files
				if (i % 10 == 0)
					emit progressUpdated(i);

				QString gameName = romFiles[i].toLower().remove(ZIP_EXT);

				if (!mameGame->games.contains(gameName))
					continue;

				gameInfo = mameGame->games[gameName];
				auditedGames.insert(gameName);

				//open zip file
				QuaZip zip(utils->getPath(dirPath) + romFiles[i]);
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
					//clone romInfo->available is already partially supplied by parent
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

				gameInfo->available = (!allNoDump && !allinParent && gameInfo->disks.isEmpty()) ? 0 : 1;
			}

			//iterate disks
			foreach (DiskInfo *diskInfo, gameInfo->disks)
			{
				if (!diskInfo->available)
					gameInfo->available = 0;
			}

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
					{
						romInfo->available = true;
						continue;
					}

					//check bios
					if (!gameInfo2->romof.isEmpty())
					{
						gameInfo2 = mameGame->games[gameInfo2->romof];

						//bios rom passed
						if (gameInfo2->roms.contains(crc) && gameInfo2->roms[crc]->available)
						{
							romInfo->available = true;
							continue;
						}
					}
				}

				//failed audit
				gameInfo->available = 0;
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
			if (!gameList->isAuditConsoleFolder(gameName))
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
	QString _dirpath = mameOpts[consoleName + "_extra_software"]->globalvalue;
	QDir dir(_dirpath);
	if (_dirpath.isEmpty() || !dir.exists())
		return;
	QString dirPath = utils->getPath(_dirpath);

	GameInfo *gameInfo = mameGame->games[consoleName];
	QString sourcefile = gameInfo->sourcefile;

	QStringList nameFilters;
	foreach (DeviceInfo *deviceInfo, gameInfo->devices)
		foreach (QString ext, deviceInfo->extensionNames)
			nameFilters << "*." + ext;

	const QString allExtensionNames = nameFilters.join(";");
		
	nameFilters << "*" ZIP_EXT;
	nameFilters << "*" SZIP_EXT;

	// iterate all files in the path
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable);
	emit progressSwitched(files.count(), QString(tr("Auditing %1 ...")).arg(consoleName));
	for (int i = 0; i < files.count(); i++)
	{
		QString fileName = files[i];
		QFileInfo fi(files[i]);
		const QString suffixName = "." + fi.suffix();

		if (suffixName == ZIP_EXT || suffixName == SZIP_EXT)	
		{
			QHash<QString, MameFileInfo *> mameFileInfoList = 
				utils->iterateMameFile(dirPath, fi.completeBaseName(), allExtensionNames, MAMEFILE_GETINFO);

			foreach (QString zipFileName, mameFileInfoList.keys())
			{
				mameFileInfoList[zipFileName]->size;

				QFileInfo zipFileInfo(zipFileName);

				gameInfo = new GameInfo(mameGame);
				gameInfo->description = zipFileInfo.completeBaseName();
				gameInfo->isExtRom = true;
				gameInfo->romof = consoleName;
				gameInfo->sourcefile = sourcefile;
				gameInfo->available = 1;
				
				QString key = dirPath + fileName + "/" + zipFileName;
				mameGame->games[key] = gameInfo;
			}
			
			utils->clearMameFileInfoList(mameFileInfoList);
		}
		else
		{
			gameInfo = new GameInfo(mameGame);
			gameInfo->description = fi.completeBaseName();
			gameInfo->isExtRom = true;
			gameInfo->romof = consoleName;
			gameInfo->sourcefile = sourcefile;
			gameInfo->available = 1;

			mameGame->games[dirPath + fileName] = gameInfo;
		}

		if (i % 10 == 0)
			emit progressUpdated(i);
	}
	emit progressSwitched(-1);
}


MameExeRomAuditor::MameExeRomAuditor(QObject *parent) : 
QObject(parent)
{
	dlgAudit.setModal(true);
	dlgAudit.setWindowTitle(tr("Checking..."));
	dlgAudit.resize(400, 300);

	QGridLayout *lay = new QGridLayout(&dlgAudit);
	tbAudit = new QTextBrowser(&dlgAudit);
	lay->addWidget(tbAudit, 0, 0, 1, 1);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(&dlgAudit);
	buttonBox->setOrientation(Qt::Horizontal);
	buttonBox->setStandardButtons(QDialogButtonBox::Ok);

	lay->addWidget(buttonBox, 1, 0, 1, 1);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(auditorClosed()));
}

void MameExeRomAuditor::auditorReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	tbAudit->append(QString::fromLocal8Bit(proc->readAllStandardOutput().data()));
}

void MameExeRomAuditor::auditorClosed()
{
	loadProc->kill();
	dlgAudit.accept();
}

void MameExeRomAuditor::audit(bool verifyAll)
{
	QStringList args;

	args << "-verifyroms";
	if (!verifyAll)
		args << currentGame;

	if (hasLanguage)
	{
		QString langpath = utils->getPath(mameOpts["langpath"]->globalvalue);
		args << "-langpath" << langpath;
		args << "-language" << language;
	}

	tbAudit->clear();

	loadProc = procMan->process(procMan->start(mame_binary, args, FALSE));

	connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(auditorReadyReadStandardOutput()));

	dlgAudit.show();
}

