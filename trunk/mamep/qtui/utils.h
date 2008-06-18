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
	QString getHistory(const QString &, const QString &);

	QString capitalizeStr(const QString & str);
	QString getPath(QString);
	void tranaparentBg(QWidget *);
	QString getMameVersion();

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

#endif

