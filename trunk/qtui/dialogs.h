#ifndef _DIALOGS_H_
#define _DIALOGS_H_

#include "ui_options.h"
#include "ui_about.h"

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

class About: public QDialog, public Ui::About
{
  Q_OBJECT

  public:
    About(QWidget *parent = 0);
};

#endif
