#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include <QtGui>

#include "ui_playoptions.h"
#include "ui_directories.h"
#include "ui_about.h"
#include "ui_cmd.h"

class PlayOptions: public QDialog, public Ui::PlayOptions
{
Q_OBJECT

public:
	PlayOptions(QWidget *parent = 0);
	void initSavestate();
	void initPlayback();
	void initRecord();
	void initMNG();
	void initAVI();
	void initWave();
	QStringList getOptions();

public slots:
	void setSavestateFile();
	void setPlaybackFile();
	void setRecordFile();
	void setMNGFile();
	void setAVIFile();
	void setWaveFile();

private:
	QString gameName;
	
	void init(QLineEdit *, QString, QString);
	void setFile(QLineEdit *, QString);
	void clear();

private slots:
	void runMame();
};

class Dirs: public QDialog, public Ui::Dirs
{
Q_OBJECT

public:
	Dirs(QWidget *parent = 0);
	void init(QString);
	QString getDirs();

public slots:
	// game menu
	void setDirectory(bool = false);
	void appendDirectory();
	void removeDirectory();
};


class About: public QDialog, public Ui::About
{
Q_OBJECT

public:
	About(QWidget *parent = 0);
};

class Cmd: public QDialog, public Ui::Cmd
{
Q_OBJECT

public:
	Cmd(QWidget *parent = 0);
};


extern PlayOptions *playOptionsUI;
extern Dirs *dirsUI;
extern About *aboutUI;
extern Cmd *cmdUI;
#endif
