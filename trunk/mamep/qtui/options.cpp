#include "qmc2main.h"

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
	//update mameopts
/*	GameInfo *gameinfo = mamegame->gamenameGameInfoMap[currentGame];
	QString iniFileName = "ini/source/" + gameinfo->sourcefile.remove(".c") + ".ini";
	
	optUtils->load(OPTNFO_SRC, iniFileName);
	optUtils->setupModelData(OPTNFO_SRC);
	
	iniFileName = "ini/fixmebios.ini";	//fixme
	optUtils->load(OPTNFO_BIOS, iniFileName);
	optUtils->setupModelData(OPTNFO_BIOS);
	
	iniFileName = "ini/" + gameinfo->cloneof + ".ini";
	optUtils->load(OPTNFO_CLONEOF, iniFileName);
	optUtils->setupModelData(OPTNFO_CLONEOF);
	
	iniFileName = "ini/" + currentGame + ".ini";
	optUtils->load(OPTNFO_CURR, iniFileName);
	optUtils->setupModelData(OPTNFO_CURR);
*/
	restoreGeometry(dlgOptionsGeo);
	e->accept();
}

void Options::closeEvent(QCloseEvent *event)
{
	dlgOptionsGeo = saveGeometry();
	event->accept();
}

