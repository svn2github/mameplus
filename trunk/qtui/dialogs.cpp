#include "dialogs.h"

#include "mamepguimain.h"
#include "mameopt.h"

/* global */
PlayOptions *playOptionsUI = NULL;
Dirs *dirsUI = NULL;
About *aboutUI = NULL;
Cmd *cmdUI = NULL;

PlayOptions::PlayOptions(QWidget *parent) : 
QDialog(parent)
{
	setupUi(this);

	connect(btnBrowseSavestate, SIGNAL(clicked()), this, SLOT(setSavestateFile()));
	connect(btnBrowsePlayback, SIGNAL(clicked()), this, SLOT(setPlaybackFile()));
	connect(btnBrowseRecord, SIGNAL(clicked()), this, SLOT(setRecordFile()));
	connect(btnBrowseMNG, SIGNAL(clicked()), this, SLOT(setMNGFile()));
	connect(btnBrowseAVI, SIGNAL(clicked()), this, SLOT(setAVIFile()));
	connect(btnBrowseWave, SIGNAL(clicked()), this, SLOT(setWaveFile()));
	connect(this, SIGNAL(accepted()), this, SLOT(runMame()));
}

void PlayOptions::init(QLineEdit *lineEdit, QString optName, QString extension)
{
	QString dirPath, fileName, filePath;
	QFileInfo fileInfo;
	bool isGenerated = false;

	//clear play options dialog
	if (gameName != currentGame)
	{
		gameName = currentGame;
		clear();
	}

	//get default path from mameOpts
	if (mameOpts.contains(optName))
	{
		QStringList dirPaths = mameOpts[optName]->globalvalue.split(";");
		dirPath = utils->getPath(dirPaths.first());
	}

	//get an existing filename for input
	if (lineEdit == lineEditSavestate || lineEdit == lineEditPlayback)
	{
		if (lineEdit == lineEditSavestate)
			dirPath.append(gameName + "/");

		QDir dir(dirPath);
		QStringList nameFilter = QStringList() << "*" + extension;
		QStringList entryFiles = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		if (!entryFiles.isEmpty())
			filePath = dirPath + entryFiles.first();
	}
	
	//generate a filename for output or input
	if (filePath.isEmpty())
	{
		for (int i = 0; i < 999; i++)
		{
			fileName.clear();
			fileName.append(gameName);
			fileName.append(QString("_%1").arg(i, 3, 10, QLatin1Char('0')));
			fileName.append(extension);

			filePath = dirPath + fileName;
			isGenerated = true;

			fileInfo.setFile(filePath);
			if (!fileInfo.exists())
				break;
		}
	}

	lineEdit->setText(filePath);
	if (isGenerated)
		lineEdit->setSelection(dirPath.size(), fileName.size() - extension.size());
	lineEdit->setFocus();
}

void PlayOptions::initSavestate()
{
	init(lineEditSavestate, "state_directory", STA_EXT);
	setSavestateFile();
}

void PlayOptions::initPlayback()
{
	lineEditRecord->clear();
	init(lineEditPlayback, "input_directory", INP_EXT);
	setPlaybackFile();
}

void PlayOptions::initRecord()
{
	lineEditPlayback->clear();
	init(lineEditRecord, "input_directory", INP_EXT);
}

void PlayOptions::initMNG()
{
	init(lineEditMNG, "snapshot_directory", MNG_EXT);
}

void PlayOptions::initAVI()
{
	init(lineEditAVI, "snapshot_directory", AVI_EXT);
}

void PlayOptions::initWave()
{
	init(lineEditWave, "input_directory", WAV_EXT);
}

void PlayOptions::runMame()
{
	QString arg1, arg2;
	QStringList args;
	QFileInfo fileInfo;
	
	arg2 = lineEditSavestate->text().trimmed();
	if (!arg2.isEmpty())
	{
		arg1 = "-state";
		args << arg1 << arg2;
	}

	arg2 = lineEditPlayback->text().trimmed();
	if (!arg2.isEmpty())
	{
		fileInfo.setFile(arg2);
		if (fileInfo.exists())
		{
			args << "-input_directory" << fileInfo.absolutePath();
			arg1 = "-playback";
			args << arg1 << fileInfo.fileName();
			args << "-nvram_directory" << QDir::tempPath();
		}
	}

	arg2 = lineEditRecord->text().trimmed();
	if (!arg2.isEmpty())
	{
		//if -state exist, rename and copy the .sta to inp
		fileInfo.setFile(arg2);

		QString staFileName = lineEditSavestate->text().trimmed();
		if (!staFileName.isEmpty())
		{
			QFile staFile(staFileName);
			QString newStaFileName = utils->getPath(fileInfo.absolutePath()) + fileInfo.baseName() + STA_EXT;
			QFile newFile(newStaFileName);

			newFile.remove();
			staFile.copy(newStaFileName);
		}

		args << "-input_directory" << fileInfo.absolutePath();
		arg1 = "-record";
		args << arg1 << fileInfo.fileName();
	}

	arg2 = lineEditMNG->text().trimmed();
	if (!arg2.isEmpty())
	{
		fileInfo.setFile(arg2);
		args << "-snapshot_directory" << fileInfo.absolutePath();
		arg1 = "-mngwrite";
		args << arg1 << fileInfo.fileName();
	}

	arg2 = lineEditAVI->text().trimmed();
	if (!arg2.isEmpty())
	{
		fileInfo.setFile(arg2);
		args << "-snapshot_directory" << fileInfo.absolutePath();
		arg1 = "-aviwrite";
		args << arg1 << fileInfo.fileName();
	}

	arg2 = lineEditWave->text().trimmed();
	if (!arg2.isEmpty())
	{
		arg1 = "-wavwrite";
		args << arg1 << arg2;
	}

	gameList->runMame(RUNMAME_NORMAL, args);
}

void PlayOptions::clear()
{
	lineEditSavestate->clear();
	lineEditPlayback->clear();
	lineEditRecord->clear();
	lineEditMNG->clear();
	lineEditAVI->clear();
	lineEditWave->clear();
}

void PlayOptions::setFile(QLineEdit *lineEdit, QString filter)
{
	QFileInfo fi;
	QDir dir;

	//take existing dir from lineEdit ctrl
	QString initPath = lineEdit->text();
	fi.setFile(initPath);
	dir.setPath(fi.absolutePath());
	
	if (!dir.exists())
	{
		//use mame dir as fallback
		fi.setFile(mame_binary);
		dir.setPath(fi.absolutePath());
	}

	QString fileName;
	if (lineEdit == lineEditSavestate || lineEdit == lineEditPlayback)
		fileName = QFileDialog::getOpenFileName(this, tr("File name:"), dir.path(), filter);
	else
		fileName = QFileDialog::getSaveFileName(this, tr("File name:"), dir.path(), filter);

	if (!fileName.isEmpty())
		lineEdit->setText(fileName);
}

void PlayOptions::setSavestateFile()
{
	//fixme: filefield could be empty
	setFile(lineEditSavestate, tr("Savestate files") + " (*" STA_EXT ")");
}

void PlayOptions::setPlaybackFile()
{
	//fixme: filefield could be empty
	setFile(lineEditPlayback, tr("Input files") + " (*" INP_EXT " *" ZIP_EXT ")");

	//test if .sta exists, auto fill it
	QString pbFileName = lineEditPlayback->text().trimmed();
	if (!pbFileName.isEmpty())
	{
		QFileInfo pbFileInfo, staFileInfo;
		pbFileInfo.setFile(pbFileName);
		staFileInfo.setFile(utils->getPath(pbFileInfo.absolutePath()) + pbFileInfo.baseName() + STA_EXT);
		if (staFileInfo.exists())
			lineEditSavestate->setText(staFileInfo.absoluteFilePath());
	}
}

void PlayOptions::setRecordFile()
{
	if (lineEditRecord->text().isEmpty())
		initRecord();

	setFile(lineEditRecord, tr("Input files") + " (*" INP_EXT ")");
}

void PlayOptions::setMNGFile()
{
	if (lineEditMNG->text().isEmpty())
		initMNG();

	setFile(lineEditMNG, tr("Videos") + " (*" MNG_EXT ")");
}

void PlayOptions::setAVIFile()
{
	if (lineEditAVI->text().isEmpty())
		initAVI();

	setFile(lineEditAVI, tr("Videos") + " (*" AVI_EXT ")");
}

void PlayOptions::setWaveFile()
{
	if (lineEditWave->text().isEmpty())
		initWave();

	setFile(lineEditWave, tr("Sounds") + " (*" WAV_EXT ")");
}

Dirs::Dirs(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);

	connect(btnBrowse, SIGNAL(clicked()), this, SLOT(setDirectory()));
	connect(btnDelete, SIGNAL(clicked()), this, SLOT(removeDirectory()));
	connect(btnInsert, SIGNAL(clicked()), this, SLOT(appendDirectory()));
}

void Dirs::init(QString initPath)
{
	lstwDirs->clear();
	lstwDirs->addItems(initPath.split(";"));
	lstwDirs->setCurrentItem(lstwDirs->item(0));
}

QString Dirs::getDirs()
{
	QString paths = "";
	
	for (int i = 0; i < lstwDirs->count(); i ++)
	{
		QString path = lstwDirs->item(i)->text();
		if (!path.isEmpty())
		{
			paths.append(path);
			paths.append(";");
		}
	}

	if (paths.endsWith(';'))
		paths.chop(1);

	return paths;
}

void Dirs::setDirectory(bool isAppend)
{
	//take current dir
	QString initPath;
	if (lstwDirs->count() > 0)
	{
		initPath = lstwDirs->currentItem()->text();
		QFileInfo fi(initPath);
		if (!fi.exists())
		{
			//take mame_binary dir
			fi.setFile(mame_binary);
			initPath = fi.absolutePath();
		}
	}

	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QString directory = QFileDialog::getExistingDirectory(lstwDirs,
								tr("Directory name:"),
								initPath,
								options);

	if (!directory.isEmpty())
	{
		if (isAppend)
			lstwDirs->addItem(directory);
		else
			lstwDirs->currentItem()->setText(directory);
	}
}

void Dirs::appendDirectory()
{
	setDirectory(true);
}

void Dirs::removeDirectory()
{
	delete lstwDirs->takeItem(lstwDirs->currentRow());
}

About::About(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);
}

Cmd::Cmd(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);
}

