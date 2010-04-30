#ifndef _M1_H_
#define _M1_H_

#include <QtGui>

#include "ui_m1.h"

class M1UI: public QDockWidget, public Ui::M1UI
{
Q_OBJECT

public:
	M1UI(QWidget *parent = 0);
	void init();
};

class M1Thread : public QThread
{
	Q_OBJECT

public:
	M1Thread(QObject *parent = 0);
	~M1Thread();

public slots:
	void stop();
	void play(QTreeWidgetItem* = NULL, int = 0);
	void pause();
	void prev();
	void next();
	void record();

protected:
	void run();

private:
	int gameNum;
	int cmdNum;
	bool done;
	bool cancel;
	QMutex mutex;
};

class M1 : public QObject
{
Q_OBJECT

private:
	QString m1_dir;
	QFutureWatcher<void> m1Watcher;

public:
	M1(QObject *parent = 0);
	~M1();

	int max_games;
	bool isHex;
	M1Thread m1Thread;
	bool available;
	QString version;

	void init();
	void loadLib();
	static int m1ui_message(void *, int, char *, int);

	typedef void (*fp_m1snd_init)(void *, int (*m1ui_message)(void *,int, char *, int));
	fp_m1snd_init m1snd_init;
	typedef int (*fp_m1snd_run)(int, int);
	fp_m1snd_run m1snd_run;
	typedef void (*fp_m1snd_shutdown)();
	fp_m1snd_shutdown m1snd_shutdown;
	typedef void (*fp_m1snd_setoption)(int, int);
	fp_m1snd_setoption m1snd_setoption;
	typedef int (*fp_m1snd_get_info_int)(int, int);
	fp_m1snd_get_info_int m1snd_get_info_int;
	typedef char* (*fp_m1snd_get_info_str)(int, int);
	fp_m1snd_get_info_str m1snd_get_info_str;
	typedef void (*fp_m1snd_set_info_int)(int, int, int, int);
	fp_m1snd_set_info_int m1snd_set_info_int;
	typedef void (*fp_m1snd_set_info_str)(int, char*, int, int, int);
	fp_m1snd_set_info_str m1snd_set_info_str;

public slots:
	void postInit();
	void updateList(const QString & = NULL);
};

extern M1UI *m1UI;
extern M1 *m1;
#endif
