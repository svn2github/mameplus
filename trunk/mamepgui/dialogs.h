#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include <QtGui>

#include "ui_playoptions.h"
#include "ui_directories.h"
#include "ui_about.h"
#include "ui_cmd.h"

class PlayOptionsUI: public QDialog, public Ui::PlayOptionsUI
{
Q_OBJECT

public:
	PlayOptionsUI(QWidget *parent = 0);
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

class DirsUI: public QDialog, public Ui::DirsUI
{
Q_OBJECT

public:
	DirsUI(QWidget *parent = 0);
	void init(QString);
	QString getDirs();

public slots:
	// game menu
	void setDirectory(bool = false);
	void appendDirectory();
	void removeDirectory();
};


class AboutUI: public QDialog, public Ui::AboutUI
{
Q_OBJECT

public:
	AboutUI(QWidget *parent = 0);
};

class CmdUI: public QDialog, public Ui::CmdUI
{
Q_OBJECT

public:
	CmdUI(QWidget *parent = 0);
};

#endif
