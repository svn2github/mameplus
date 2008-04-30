#ifndef _UTILS_H_
#define _UTILS_H_

#include <QtGui>

class Utils : public QObject
{
Q_OBJECT
public:
	QIcon deficon;
	QRegExp spaceRegex;
	QProcess *loadProc;
	
	Utils(QObject *parent = 0);

	QIcon loadIcon(const QString & gameName);
	QIcon loadWinIco(QIODevice *device);
	QString getViewString(const QModelIndex &index, int column) const;
	QPixmap getScreenshot(const QString &, const QString &, const QSize &);
	QString getHistory(const QString &, const QString &);

	QString capitalizeStr(const QString & str);
	QString getPath(QString);
	void tranaparentBg(QWidget *);
	QString getMameVersion();

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
	void enqueue(const QString & str);
	bool isEmpty() const;

signals:
	void logStatusUpdated(QString);
	
private:
	int capacity;
	QQueue<QString> queue;
	mutable QMutex mutex;
};

#endif

