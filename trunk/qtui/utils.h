#ifndef _UTILS_H_
#define _UTILS_H_

#include <QtGui>

class Utils : public QObject
{
Q_OBJECT
public:
	QByteArray deficondata;
	QByteArray defsnapdata;

	QRegExp spaceRegex;
	QProcess *loadProc;
	
	Utils(QObject *parent = 0);

	QByteArray getScreenshot(const QString &, const QString &);
	QString getHistory(const QString &, const QString &, int = 0);

	QString capitalizeStr(const QString & str);
	QString getPath(QString);
	QString getMameVersion();

	bool isAuditFolder(QString);
	bool isConsoleFolder();
	quint8 getStatus(QString);
	QString getStatusString(quint8, bool = false);

signals:
	void icoUpdated(QString);

public slots:
	void getMameVersionReadyReadStandardOutput();
	void getMameVersionFinished(int, QProcess::ExitStatus);

private:
	QString mameVersion;
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
	int count() const;

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
#endif

