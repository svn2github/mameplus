#include "mamepguimain.h"

Options *dlgOptions = NULL;
QByteArray dlgOptionsGeo;

Options::Options(QWidget *parent)
  : QDialog(parent)
{
	setupUi(this);
}

Options::~Options()
{

}

void Options::showEvent(QShowEvent *e)
{
	restoreGeometry(dlgOptionsGeo);
	e->accept();
}

void Options::closeEvent(QCloseEvent *event)
{
	dlgOptionsGeo = saveGeometry();
	event->accept();
}

