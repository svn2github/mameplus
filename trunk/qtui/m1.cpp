#include "mamepguimain.h"
#include "m1ui.h"

M1 *m1 = NULL;

M1::M1(QObject *parent) : 
QObject(parent),
max_games(0)
{
	//set current dir to m1, so that m1.xml and list could be loaded
	m1_dir = utils->getPath(guiSettings.value("m1_directory", "bin/m1").toString());

	QString currentDir = QDir::currentPath();
	QDir::setCurrent(m1_dir);
	QLibrary m1Lib(0);
	m1Lib.setFileName(m1_dir + "m1");

	if (m1Lib.load())
	{
		m1snd_init = (fp_m1snd_init)m1Lib.resolve("m1snd_init");
		m1snd_run = (fp_m1snd_run)m1Lib.resolve("m1snd_run");
		m1snd_shutdown = (fp_m1snd_shutdown)m1Lib.resolve("m1snd_shutdown");
		m1snd_setoption =  (fp_m1snd_setoption)m1Lib.resolve("m1snd_setoption");
		m1snd_get_info_int = (fp_m1snd_get_info_int)m1Lib.resolve("m1snd_get_info_int");
		m1snd_get_info_str = (fp_m1snd_get_info_str)m1Lib.resolve("m1snd_get_info_str");
		m1snd_set_info_int = (fp_m1snd_set_info_int)m1Lib.resolve("m1snd_set_info_int");
		m1snd_set_info_str = (fp_m1snd_set_info_str)m1Lib.resolve("m1snd_set_info_str");

		if (m1snd_init &&
			m1snd_run &&
			m1snd_shutdown &&
			m1snd_setoption &&
			m1snd_get_info_int &&
			m1snd_get_info_str &&
			m1snd_set_info_int &&
			m1snd_set_info_str)
		{
			m1snd_setoption(M1_OPT_RETRIGGER, 0);
			m1snd_setoption(M1_OPT_WAVELOG, 0);
			m1snd_setoption(M1_OPT_NORMALIZE, 1);
			m1snd_setoption(M1_OPT_LANGUAGE, M1_LANG_EN);
//			m1snd_setoption(M1_OPT_USELIST, 1);
			m1snd_setoption(M1_OPT_RESETNORMALIZE, 0);

			m1snd_init(0, m1ui_message);

			//if m1 is not loaded successfully, max_games keeps 0
			max_games = m1snd_get_info_int(M1_IINF_TOTALGAMES, 0);
			version = QString(m1snd_get_info_str(M1_SINF_COREVERSION, 0));
		}
	}

	//restore current dir
	QDir::setCurrent(currentDir);
}

M1::~M1()
{
	m1snd_shutdown();
}

QString M1::getVersion()
{
	return version;
}

int M1::getMaxGames()
{
	return max_games;
}

void M1::updateList()
{
	QString gameName = currentGame;

	QString fileName = QString("%1lists/%2/%3%4")
		.arg(m1_dir)
		.arg(dlgM1->cmbLang->currentText())
		.arg(gameName)
		.arg(".lst");
	QFile datFile(fileName);

	QString buf = "";

	dlgM1->twSongList->clear();

	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");

//		bool isFound, recData = false;
		QString line;

		QList<QTreeWidgetItem *> items;
		//valid track line
		QRegExp rxTrack("[#$]([\\da-fA-F]+)\\s+(.*)");
		QRegExp rxTime("<time=\"([\\d.:]*)\">");
		int pos;

		do
		{
			line = in.readLine();
			if (line.startsWith("#") || line.startsWith("$"))
			{
				pos = rxTrack.indexIn(line);
				QStringList list = rxTrack.capturedTexts();

				if (pos > -1)
				{
					//remove the complete matching \0
					list.removeFirst();
					//look for track length
					pos = rxTime.indexIn(list[1]);
					if (pos > -1)
					{
						list[1] = list[1].left(pos);
						list.append(rxTime.cap(1).trimmed());
					}

					items.append(new QTreeWidgetItem(dlgM1->twSongList, list));
				}
			}
			else
			{
				QStringList list;
				list.append("");
				list.append(line.trimmed());

				QTreeWidgetItem *item = new QTreeWidgetItem(dlgM1->twSongList, list);
				item->setDisabled(true);
				items.append(item);
			}
		}
		while (!line.isNull());

		dlgM1->twSongList->header()->setResizeMode(2,QHeaderView::ResizeToContents);
		dlgM1->twSongList->header()->setResizeMode(0,QHeaderView::ResizeToContents);
		dlgM1->twSongList->header()->setResizeMode(1,QHeaderView::Stretch);
	}
}

int M1::m1ui_message(void *pthis, int message, char *txt, int iparm)
{
//	win->log(QString("M1 callback %1, %2").arg(message).arg(txt));
	switch (message)
	{
		case M1_MSG_HARDWAREDESC:
		{
			GameInfo *gameInfo = mameGame->nameInfoMap[currentGame];
			
			dlgM1->lblTrackInfo->setText(
				QString(
				"<b>Manufacturer: </b>%1 %2<br>"
				"<b>Hardware: </b>%3"
				)
				.arg(gameInfo->year)
				.arg(gameInfo->manufacturer)
				.arg(txt)
				);
			break;
		}
		case M1_MSG_ROMLOADERR:
			win->log("M1 ERR: ROMLOADERR");
			m1->m1Thread.stop();
			break;

		case M1_MSG_STARTINGSONG:
			win->log(QString("M1 INF: STARTINGSONG %1").arg(iparm));
			break;

		case M1_MSG_ERROR:
			win->log(QString("M1 ERR: %1").arg(txt));
			m1->m1Thread.stop();
			break;

		case M1_MSG_HARDWAREERROR:
			win->log("M1 ERR: HARDWAREERROR");
			m1->m1Thread.stop();
			break;

		case M1_MSG_MATCHPATH:
		{
//			win->log(QString("M1 INF: MATCHPATH: %1").arg(txt));

			//test if rom's available
			QStringList dirpaths = mameOpts["rompath"]->currvalue.split(";");
			foreach (QString _dirpath, dirpaths)
			{
				QDir dir(_dirpath);
				QString dirpath = utils->getPath(_dirpath);

				QString path = dirpath + currentGame + ".zip";
				QFile file(path);
				if (file.exists())
				{
					strcpy(txt, qPrintable(path));
					m1->m1snd_set_info_str(M1_SINF_SET_ROMPATH, txt, 0, 0, 0);
	//				win->log(QString("M1 INF: MATCHPATH2: %1").arg(txt));
					return 1;
				}
			}

			win->log("M1 ERR: MATCHPATH");
			return 0;
		}

		case M1_MSG_GETWAVPATH:
		{
			int song = m1->m1snd_get_info_int(M1_IINF_CURCMD, 0);
			int game = m1->m1snd_get_info_int(M1_IINF_CURGAME, 0); 
	
			sprintf(txt, "%s%s%04d.wav", "", m1->m1snd_get_info_str(M1_SINF_ROMNAME, game), song);
//			win->log(txt);
			break;
		}
	}
	return 0;
}

M1Player::M1Player(QObject *parent)
: QThread(parent),
gameNum(-1),
cmdNum(0)
{
}

M1Player::~M1Player()
{
	done = true;
	wait();
}

void M1Player::pause(){}
void M1Player::prev(){}
void M1Player::next(){}
void M1Player::record(){}

void M1Player::stop()
{
	//stop current thread
	if (isRunning())
	{
		cancel = true;
		wait();
	}
}

void M1Player::play(QTreeWidgetItem*, int)
{

	QList<QTreeWidgetItem *> selectedItems = dlgM1->twSongList->selectedItems();
	if (selectedItems.isEmpty())
		return;

	bool ok, isHex;
	QString cmdStr = selectedItems[0]->text(0).trimmed();
	QRegExp rxHex("[a-fA-F]");
	if (cmdStr.contains(rxHex))
	{
		cmdNum = cmdStr.toInt(&ok, 16);
		isHex = true;
	}
	else
	{
		cmdNum = cmdStr.toInt(&ok);
		isHex = false;
	}

	if (!ok)
		return;

	if (isHex)
		dlgM1->lcdNumber->setHexMode();
	else
		dlgM1->lcdNumber->setDecMode();

	dlgM1->lcdNumber->display(cmdStr);
	dlgM1->lblTrackName->setText(selectedItems[0]->text(1));

	QMutexLocker locker(&mutex);

	for (int curgame = 0; curgame < m1->getMaxGames(); curgame++)
	{
		if (currentGame == m1->m1snd_get_info_str(M1_SINF_ROMNAME, curgame))
		{
			gameNum = curgame;
			break;
		}
	}

	stop();

	cancel = false;
	done = false;
	start(NormalPriority);
}

void M1Player::run()
{
	/*
	int maxtracks = m1snd_get_info_int(M1_IINF_TRACKS, curgame);

	for (int track = 0; track < maxtracks; track++)
	{
		int cmd = m1snd_get_info_int(M1_IINF_TRACKCMD, (track << 16) | curgame);
		win->log(QString::fromUtf8(m1snd_get_info_str(M1_SINF_TRKNAME, (cmd << 16) | curgame)));
	}
	*/
	
	m1->m1snd_setoption(M1_OPT_DEFCMD, cmdNum);
	m1->m1snd_run(M1_CMD_GAMEJMP, gameNum);
//	win->log(QString("play1: %1").arg(m1->m1snd_get_info_int(M1_IINF_CURSONG, 0)));

	while(!cancel)
		m1->m1snd_run(M1_CMD_IDLE, 0);

	//have to call this to stop sound
	m1->m1snd_run(M1_CMD_GAMEJMP, gameNum);
	m1->m1snd_run(M1_CMD_STOP, 0);
}

