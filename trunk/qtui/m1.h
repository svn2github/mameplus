#ifndef _M1_H_
#define _M1_H_

#include <QtGui>

class M1Player : public QThread
{
	Q_OBJECT

public:
	M1Player(QObject *parent = 0);
	~M1Player();

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
	QString version;

public:
	M1(QObject *parent = 0);
	~M1();

	int max_games;
	QString m1_dir;
	M1Player m1Thread;

	static int m1ui_message(void *, int, char *, int);
	QString getVersion();
	int getMaxGames();
	void updateList();

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
};

#endif
