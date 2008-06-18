#include "mamepguimain.h"

Options *dlgOptions = NULL;
QByteArray option_geometry;
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
	restoreGeometry(option_geometry);
	e->accept();
}

void Options::closeEvent(QCloseEvent *event)
{
	option_geometry = saveGeometry();
	event->accept();
}

