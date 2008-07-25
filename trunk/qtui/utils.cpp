#include "mamepguimain.h"

Utils *utils;

Utils::Utils(QObject *parent)
: QObject(parent)
{
	QFile icoFile(":/res/win_roms.ico");
	icoFile.open(QIODevice::ReadOnly);
	deficondata = icoFile.readAll();

	QFile snapFile(":/res/mamegui/about.png");
	snapFile.open(QIODevice::ReadOnly);
	defsnapdata = snapFile.readAll();
	
	spaceRegex = QRegExp("\\s+");
}

QString Utils::capitalizeStr(const QString & str)
{
	QStringList strlist = str.split("_");
	// capitalize first char
	for (int i=0; i < strlist.size(); i++)
		strlist[i][0] = strlist[i][0].toUpper();

	return strlist.join(" ");
}

QString Utils::getPath(QString dirpath)
{
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
		GameInfo *gameinfo = mamegame->gamenameGameInfoMap[gameName];
 		if (!gameinfo->cloneof.isEmpty())
			snapdata = getScreenshot(dirpath0, gameinfo->cloneof);

		// fallback to default image, first getScreenshot() can't reach here
		if (snapdata.isNull())
			snapdata = defsnapdata;
	}
	
	return snapdata;
}

QString Utils::getHistory(const QString &gameName, const QString &fileName)
{
	QFile datFile(fileName);
	QString buf = "";

	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");

		bool recData = false;
		QString line;

		do
		{
			line = in.readLine().trimmed();
			if (!line.startsWith("#"))
			{
				if (line.startsWith("$"))
				{
					if (line.startsWith("$info="))
					{
						if (line.startsWith("$info=" + gameName))
							recData = true;
						else
						{
							if (recData)
								break;
							
							recData = false;
						}
					}
				}
				else if (recData)
				{
					buf += line;
					buf += "\n";
				}
			}
		}
		while (!line.isNull());
	}

	if (buf.trimmed().isEmpty())
	{
		GameInfo *gameInfo = mamegame->gamenameGameInfoMap[gameName];
		if (!gameInfo->cloneof.isEmpty())
			buf = getHistory(gameInfo->cloneof, fileName);
	}
	
	return buf.trimmed();
}

void Utils::tranaparentBg(QWidget * w)
{
	QPalette palette;
	QBrush brush(QColor(255, 255, 255, 140));
	brush.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Base, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
	palette.setBrush(QPalette::Disabled, QPalette::Base, brush);
/*	QBrush brushText(QColor(255, 255, 255, 255));
	brushText.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Text, brushText);
	palette.setBrush(QPalette::Inactive, QPalette::Text, brushText);
	palette.setBrush(QPalette::Disabled, QPalette::Text, brushText);
*/
	w->setPalette(palette);
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
		if (paths[1] == "Consoles")
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
		if (paths[1] == "Consoles")
			return true;

		else if (mamegame->gamenameGameInfoMap.contains(paths[1]))
		{
			GameInfo *gameInfo = mamegame->gamenameGameInfoMap[paths[1]];
			if (!gameInfo->nameDeviceInfoMap.isEmpty())
				return true;
		}
	}

	return false;
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

