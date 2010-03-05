#ifndef _UTILS_H_
#define _UTILS_H_

#include <QtGui>

enum
{
	MAMEFILE_GETINFO = 0,
	MAMEFILE_GETDATINFO,
	MAMEFILE_READ,
	MAMEFILE_EXTRACT
};

class MameFileInfo : public QObject
{
public:
	quint32 crc;
	quint64 size;
	QByteArray data;
};

class Utils : public QObject
{
Q_OBJECT
public:
	QProcess *loadProc;

	Utils(QObject *parent = 0);

	QString getDesc(const QString &, bool = true);
	QSize getScaledSize(QSize, QSize, bool);
	QString capitalizeStr(const QString &);
	void lowerTrimmed(QStringList &);
	QStringList split2Str(const QString &, const QString &, bool = false);
	QString getPath(QString);
	QString getSinglePath(QString, QString);
	QString getMameVersion();

	quint8 getStatus(QString);
	QString getStatusString(quint8, bool = false);
	QString getLongName(const QString &);

	QHash<QString, MameFileInfo *> iterateMameFile(const QString &, const QString &, const QString &, int);
	void clearMameFileInfoList(QHash<QString, MameFileInfo *>);

signals:
	void icoUpdated(QString);

public slots:
	void getMameVersionReadyReadStandardOutput();
	void getMameVersionFinished(int, QProcess::ExitStatus);

private:
	QString mameVersion;
	QMap<QString, QString> descMap;
	void initDescMap();
	bool matchMameFile(const QString &, const QStringList &);
	bool extractMameFile(const QString &, MameFileInfo *);
};

class MyQueue : public QObject
{
	Q_OBJECT

public:
	MyQueue(QObject *parent = 0);
	void setSize(int);
	QString dequeue();
	void enqueue(const QString &);
	bool isEmpty() const;
	bool contains(const QString &) const;
	QString value(int);
	int size() const;

signals:
	void logStatusUpdated(QString);
	
private:
	int capacity;
	QQueue<QString> queue;
	mutable QMutex mutex;
};

class ProcessManager : public QObject
{
Q_OBJECT

public:
	QMap<QProcess *, ushort> procMap;
	ushort procCount;
	QString lastCommand;
	QString stdOut;
	QString stdErr;

	ProcessManager(QWidget *parent = 0);

	int start(QString &, QStringList &, bool autoConnect = TRUE);
	QProcess *process(ushort);
	QString readStandardOutput(QProcess *);
	QString readStandardOutput(ushort);
	QString readStandardError(QProcess *);
	QString readStandardError(ushort);
	void terminate(QProcess *);
	void terminate(ushort);
	void kill(QProcess *);
	void kill(ushort);

public slots:
	void started();
	void finished(int, QProcess::ExitStatus);
	void readyReadStandardOutput();
	void readyReadStandardError();
	void error(QProcess::ProcessError);
};

extern Utils *utils;
extern ProcessManager *procMan;
extern QRegExp rxSpace;

#endif
