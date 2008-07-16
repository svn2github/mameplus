#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "ui_options.h"

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
#endif
