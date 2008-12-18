#include "mamepguimain.h"
#include "m1ui.h"

M1 *m1 = NULL;
M1UI *m1UI = NULL;

M1UI::M1UI(QWidget *parent)
:QDockWidget(parent)
{
	setupUi(this);

	const QStringList m1Headers = (QStringList()
		<< "#" << tr("Name") << tr("Len"));
	twSongList->setHeaderLabels(m1Headers);
	twSongList->header()->moveSection(2, 1);

	const QStringList m1Langs = (QStringList()
		<< "en" << tr("jp"));
	cmbLang->addItems(m1Langs);
	QString m1_language = guiSettings.value("m1_language").toString();
	cmbLang->setCurrentIndex((m1_language != "en") ? 1 : 0);

	if (m1_language == "jp")
	{
		QFont font;
		font.setFamily("MS Gothic");
		font.setFixedPitch(true);
		twSongList->setFont(font);
		lblTrackName->setFont(font);
	}

	setEnabled(false);
}

void M1UI::init()
{
	connect(btnPlay, SIGNAL(pressed()), &m1->m1Thread, SLOT(play()));
	connect(twSongList, SIGNAL(itemActivated(QTreeWidgetItem*, int)), &m1->m1Thread, SLOT(play(QTreeWidgetItem*, int)));
	connect(btnStop, SIGNAL(pressed()), &m1->m1Thread, SLOT(stop()));
	connect(cmbLang, SIGNAL(currentIndexChanged(const QString &)), m1, SLOT(updateList(const QString &)));

//	win->addDockWidget(static_cast<Qt::DockWidgetArea>(Qt::RightDockWidgetArea), this);
}

M1::M1(QObject *parent) : 
QObject(parent),
max_games(0),
isHex(false),
available(false)
{
}

M1::~M1()
{
	m1snd_shutdown();
}

void M1::init()
{
	connect(&m1Watcher, SIGNAL(finished()), this, SLOT(postInit()));
	QFuture<void> future = QtConcurrent::run(this, &M1::loadLib);
	m1Watcher.setFuture(future);
}

void M1::loadLib()
{
	//set current dir to m1, so that m1.xml and list could be loaded
	m1_dir = utils->getPath(guiSettings.value("m1_directory", "bin/m1").toString());

	QString currentDir = QDir::currentPath();
	QDir::setCurrent(m1_dir);
	QLibrary m1Lib(0);
	m1Lib.setFileName(m1_dir + "m1");

	//resolve symbols from m1 lib
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

void M1::postInit()
{
	if (max_games > 0)
	{
		available = true;

		m1UI->setEnabled(true);
		m1UI->init();
		updateList();
		win->setVersion();

		QAction *actionM1UI = m1UI->toggleViewAction();
		win->menuView->insertAction(win->actionVerticalTabs, actionM1UI);
		
		win->log("m1 loaded");
	}
	else
		win->removeDockWidget(m1UI);
}

void M1::updateList(const QString &)
{
	QString gameName = currentGame;

	QString fileName = QString("%1lists/%2/%3%4")
		.arg(m1_dir)
		.arg(m1UI->cmbLang->currentText())
		.arg(gameName)
		.arg(".lst");
	QFile datFile(fileName);

	QString buf = "";

	m1UI->twSongList->clear();

	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");

//		bool isFound, recData = false;
		QString line;

		QList<QTreeWidgetItem *> items;
		//valid track line
		QRegExp rxTrack("([#$][\\da-fA-F]+)\\s+(.*)");
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

					items.append(new QTreeWidgetItem(m1UI->twSongList, list));
				}
			}
			else
			{
				QStringList list;
				list.append("");
				list.append(line.trimmed());

				QTreeWidgetItem *item = new QTreeWidgetItem(m1UI->twSongList, list);
				item->setDisabled(true);
				items.append(item);
			}
		}
		while (!line.isNull());

		m1UI->twSongList->header()->setResizeMode(2,QHeaderView::ResizeToContents);
		m1UI->twSongList->header()->setResizeMode(0,QHeaderView::ResizeToContents);
		m1UI->twSongList->header()->setResizeMode(1,QHeaderView::Stretch);
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
			
			m1UI->lblTrackInfo->setText(
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

M1Thread::M1Thread(QObject *parent)
: QThread(parent),
gameNum(-1),
cmdNum(0)
{
}

M1Thread::~M1Thread()
{
	done = true;
	wait();
}

void M1Thread::pause(){}
void M1Thread::prev(){}
void M1Thread::next(){}
void M1Thread::record(){}

void M1Thread::stop()
{
	//stop current thread
	if (isRunning())
	{
		cancel = true;
		wait();
	}
}

void M1Thread::play(QTreeWidgetItem*, int)
{
	QList<QTreeWidgetItem *> selectedItems = m1UI->twSongList->selectedItems();
	if (selectedItems.isEmpty())
		return;

	bool ok;
	QString cmdStr = selectedItems[0]->text(0).trimmed();

	if (cmdStr.startsWith("$"))
	{
		cmdStr = cmdStr.remove(0, 1);
		cmdNum = cmdStr.toInt(&ok, 16);
		m1UI->lcdNumber->setHexMode();
	}
	else
	{
		cmdStr = cmdStr.remove(0, 1);
		cmdNum = cmdStr.toInt(&ok);
		m1UI->lcdNumber->setDecMode();
	}

	if (!ok)
		return;

	m1UI->lcdNumber->display(cmdStr);
	m1UI->lblTrackName->setText(selectedItems[0]->text(1));

	QMutexLocker locker(&mutex);

	for (int curgame = 0; curgame < m1->max_games; curgame++)
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

void M1Thread::run()
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

