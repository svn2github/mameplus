#include "mamepguimain.h"

Options *dlgOptions = NULL;
QByteArray dlgOptionsGeo;
QList<QListWidget *> optCtrlList;

Options::Options(QWidget *parent)
  : QDialog(parent)
{
	setupUi(this);

	if (optCtrlList.isEmpty())
		optCtrlList << 0	//place holder for GUI
					<< lvGlobalOpt
					<< lvSourceOpt
					<< lvBiosOpt
					<< lvCloneofOpt
					<< lvCurrOpt;
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

