#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include "ui_options.h"
#include "ui_directories.h"
#include "ui_about.h"
#include "ui_m1.h"

class Options: public QDialog, public Ui::Options
{
Q_OBJECT

public:
	Options(QWidget *parent = 0);
	~Options();

public slots:

protected:
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent *);
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

class M1UI: public QDockWidget, public Ui::M1UI
{
Q_OBJECT

public:
	M1UI(QWidget *parent = 0);
};

#endif
