#include "dialogs.h"

#include "mamepguimain.h"
#include "mameopt.h"

/* global */
PlayOptions *playOptionsUI = NULL;
Options *optionsUI = NULL;
Dirs *dirsUI = NULL;
About *aboutUI = NULL;

QList<QListWidget *> optCtrls;
QByteArray option_geometry;

PlayOptions::PlayOptions(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);

	connect(btnBrowseSavestate, SIGNAL(clicked()), this, SLOT(setSavestateFile()));
	connect(btnBrowsePlayback, SIGNAL(clicked()), this, SLOT(setPlaybackFile()));
	connect(btnBrowseRecord, SIGNAL(clicked()), this, SLOT(setRecordFile()));
	connect(btnBrowseMNG, SIGNAL(clicked()), this, SLOT(setMNGFile()));
	connect(btnBrowseAVI, SIGNAL(clicked()), this, SLOT(setAVIFile()));
	connect(btnBrowseWave, SIGNAL(clicked()), this, SLOT(setWaveFile()));
}

void PlayOptions::init(QLineEdit *lineEdit, QString optName, QString extension)
{
	QString dir, fileName, filePath;
	QFileInfo fileInfo;

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
		dir = utils->getPath(dirPaths.first());
	}

	//generate a filename
	for (int i = 0; i < 999; i++)
	{
		fileName.clear();
		fileName.append(gameName);
		fileName.append(QString("_%1").arg(i, 3, 10, QLatin1Char('0')));
		fileName.append(extension);

		filePath = dir + fileName;

		fileInfo.setFile(filePath);
		if (!fileInfo.exists())
			break;
	}

	lineEdit->setText(filePath);
	lineEdit->setSelection(dir.size(), fileName.size() - extension.size());
	lineEdit->setFocus();
}

void PlayOptions::initSavestate()
{
	init(lineEditSavestate, "state_directory", STA_EXT);
}

void PlayOptions::initPlayback()
{
	lineEditRecord->clear();
	init(lineEditPlayback, "input_directory", INP_EXT);
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
	init(lineEditWave, "snapshot_directory", WAV_EXT);
}

QStringList PlayOptions::getOptions()
{
	return QStringList();
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

	QString fileName = QFileDialog::getSaveFileName(this, tr("File name:"), dir.path(), filter);

	if (!fileName.isEmpty())
		lineEdit->setText(fileName);
}

void PlayOptions::setSavestateFile()
{
	if (lineEditSavestate->text().isEmpty())
		initSavestate();

	setFile(lineEditSavestate, tr("Savestate files") + "(*" + STA_EXT + ")");
}

void PlayOptions::setPlaybackFile()
{
	if (lineEditPlayback->text().isEmpty())
		initPlayback();

	setFile(lineEditPlayback, tr("Input files") + "(*" + INP_EXT + " *" + ZIP_EXT + ")");
}

void PlayOptions::setRecordFile()
{
	if (lineEditRecord->text().isEmpty())
		initRecord();

	setFile(lineEditRecord, tr("Input files") + "(*" + INP_EXT + ")");
}

void PlayOptions::setMNGFile()
{
	if (lineEditMNG->text().isEmpty())
		initMNG();

	setFile(lineEditMNG, tr("Videos") + "(*" + MNG_EXT + ")");
}

void PlayOptions::setAVIFile()
{
	if (lineEditAVI->text().isEmpty())
		initAVI();

	setFile(lineEditAVI, tr("Videos") + "(*" + AVI_EXT + ")");
}

void PlayOptions::setWaveFile()
{
	if (lineEditWave->text().isEmpty())
		initWave();

	setFile(lineEditWave, tr("Sounds") + "(*" + WAV_EXT + ")");
}

Options::Options(QWidget *parent)
:QDialog(parent)
{
	setupUi(this);

	if (optCtrls.isEmpty())
		optCtrls << 0	//place holder for GUI
					<< lvGlobalOpt
					<< lvSourceOpt
					<< lvBiosOpt
					<< lvCloneofOpt
					<< lvCurrOpt;

	disconnect(buttonBoxDlg, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBoxDlg, SIGNAL(accepted()), this, SLOT(accept()));
}

Options::~Options()
{
}

void Options::showEvent(QShowEvent *e)
{
	restoreGeometry(option_geometry);
	e->accept();
}

void Options::closeEvent(QCloseEvent *event)
{
	option_geometry = saveGeometry();
	event->accept();
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
	QString initPath = lstwDirs->currentItem()->text();
	QFileInfo fi(initPath);
	if (!fi.exists())
	{
		//take mame_binary dir
		fi.setFile(mame_binary);
		initPath = fi.absolutePath();
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

