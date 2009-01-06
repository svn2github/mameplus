#include "dialogs.h"

#include "mamepguimain.h"

/* global */
Options *optionsUI = NULL;
Dirs *dirsUI = NULL;
About *aboutUI = NULL;

QList<QListWidget *> optCtrls;
QByteArray option_geometry;

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

	buttonBoxDlg->disconnect(SIGNAL(accepted()));
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

