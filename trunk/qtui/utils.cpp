#include "mamepguimain.h"

#undef _DEBUG_
Utils *utils;
ProcessManager *procMan = NULL;

Utils::Utils(QObject *parent)
: QObject(parent)
{
	QFile icoFile(":/res/win_roms.ico");
	icoFile.open(QIODevice::ReadOnly);
	deficondata = icoFile.readAll();

	QFile snapFile(":/res/mamegui/mame.png");
	snapFile.open(QIODevice::ReadOnly);
	defsnapdata = snapFile.readAll();
	
	spaceRegex = QRegExp("\\s+");
}

QString Utils::capitalizeStr(const QString & str)
{
	QStringList strlist = str.split("_");
	// capitalize first char
	strlist[0][0] = strlist[0][0].toUpper();

	return strlist.join(" ");
}

QString Utils::getPath(QString dirpath)
{
	dirpath.replace("$HOME", QDir::homePath());
	QDir dir(dirpath);
	return dir.path() + "/";	//clean it up
}

QByteArray Utils::getScreenshot(const QString &dirpath0, const QString &gameName)
{
	QStringList dirpaths = dirpath0.split(";");
	QByteArray snapdata = QByteArray();

	foreach (QString _dirpath, dirpaths)
	{
		QDir dir(_dirpath);
		QString dirpath = getPath(_dirpath);

		// try to load directly	
		QFile snapFile(dirpath + gameName + ".png");
		if (snapFile.open(QIODevice::ReadOnly))
			snapdata = snapFile.readAll();

		// try to add .zip to nearest folder name
		if (snapdata.isNull())
		{
			QuaZip zip(dirpath + dir.dirName() + ".zip");
			if (zip.open(QuaZip::mdUnzip))
			{
				QuaZipFile zipfile(&zip);
				if (zip.setCurrentFile(gameName + ".png"))
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
		GameInfo *gameinfo = mameGame->nameInfoMap[gameName];
 		if (!gameinfo->cloneof.isEmpty())
			snapdata = getScreenshot(dirpath0, gameinfo->cloneof);

		// fallback to default image, first getScreenshot() can't reach here
		if (snapdata.isNull())
			snapdata = defsnapdata;
	}
	
	return snapdata;
}

QString Utils::getHistory(const QString &fileName, const QString &gameName)
{
	QFile datFile(fileName);
	QString buf = "";

	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
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
						QStringList games = line.split(',');

						foreach (QString game, games)
						{
							//found the entry, start recording
							if (game == gameName)
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
						line.replace("<a href=", "<a style=\"color:#006d9f\" href=");
						buf += line;
						buf += "<br><br>";
					}
					else if (recData)
						buf += "<br>";

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

	if (buf.trimmed().isEmpty())
	{
		GameInfo *gameInfo = mameGame->nameInfoMap[gameName];
		if (!gameInfo->cloneof.isEmpty())
			buf = getHistory(fileName, gameInfo->cloneof);
	}
	
	return buf.trimmed();
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

void Utils::getMameVersionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	procMan->procMap.remove(proc);
	procMan->procCount--;
	loadProc = NULL;

	mameVersion.replace(QRegExp(".*(\\d+\\.[^ ]+\\s+\\([\\w\\s]+\\)).*"), "\\1");
//	0.124u4a (Apr 24 2008)
	win->log(QString("mamever: %1").arg(mameVersion));
}

bool Utils::isAuditFolder(QString consoleName)
{
	QStringList paths = currentFolder.split("/");
	if (paths.size() == 2)
	{
		if (paths[1] == tr("Consoles"))
			return true;

		else if(paths[1] == consoleName)
		{
			win->log("current audit: " + consoleName);
			return true;
		}
	}

	return false;		
}

bool Utils::isConsoleFolder()
{
	QStringList paths = currentFolder.split("/");
	if (paths.size() == 2)
	{
		if (paths[1] == tr("Consoles"))
			return true;

		else if (mameGame->nameInfoMap.contains(paths[1]))
		{
			GameInfo *gameInfo = mameGame->nameInfoMap[paths[1]];
			if (!gameInfo->nameDeviceInfoMap.isEmpty())
				return true;
		}
	}

	return false;
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

ProcessManager::~ProcessManager()
{
//  win->log("~ProcessManager()");
}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect)
{
#ifdef _DEBUG_
	QString logMsg = "DEBUG: ProcessManager::start(QString &command = \"" + command + "\", QStringList &arguments = \"";
	int argCount;
	for (argCount = 0; argCount < arguments.count(); argCount++)
		logMsg += QString(argCount > 0 ? " " + arguments[argCount] : arguments[argCount]);
	logMsg += "\", bool autoConnect = " + QString(autoConnect ? "TRUE" : "FALSE") + ")";
//	win->log(logMsg);
#endif

	QProcess *proc = new QProcess(this);
	if ( autoConnect ) {
		lastCommand = command;
    int i;
    for (i = 0; i < arguments.count(); i++)
      lastCommand += " " + arguments[i];
//    win->log(tr("starting emulator #%1, command = %2").arg(procCount).arg(lastCommand));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(proc, SIGNAL(started()), this, SLOT(started()));
    connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
  }
  proc->start(command, arguments);

  procMap[proc] = procCount++;
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
#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::terminate(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

//  win->log(tr("terminating emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::terminate(ushort index = " + QString::number(index) + ")");
#endif

  terminate(process(index));
}

void ProcessManager::kill(QProcess *proc)
{
#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::kill(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

//  win->log(tr("killing emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->kill();
}

void ProcessManager::kill(ushort index)
{
#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::kill(ushort index = " + QString::number(index) + ")");
#endif

  kill(process(index));
}

void ProcessManager::readyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::readyReadStandardOutput(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      win->log(tr("stdout[#%1]: ").arg(procMap[proc]) + s, LOG_MAME);
  }
}

void ProcessManager::readyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::readyReadStandardError(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardError();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      win->log(tr("stderr[#%1]: ").arg(procMap[proc]) + s, LOG_MAME);
  }
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::finished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = "+ QString::number(exitStatus) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

//  win->log(tr("emulator #%1 finished, exit code = %2, exit status = %3, remaining emulators = %4").arg(procMap[proc]).arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))).arg(procMap.count() - 1));
  procMap.remove(proc);
}

void ProcessManager::started()
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::started(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

//  win->log(tr("emulator #%1 started, PID = %2, running emulators = %3").arg(procMap[proc]).arg((quint64)proc->pid()).arg(procMap.count()));
}

void ProcessManager::error(QProcess::ProcessError processError)
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
//  win->log("DEBUG: ProcessManager::error(QProcess::ProcessError processError = " + QString::number(processError) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}

void ProcessManager::stateChanged(QProcess::ProcessState processState)
{
  QProcess *proc = (QProcess *)sender();

#ifdef _DEBUG_
  win->log("DEBUG: ProcessManager::stateChanged(QProcess::ProcessState processState = " + QString::number(processState) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}

