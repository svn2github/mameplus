#ifndef _UTILS_H_
#define _UTILS_H_

#include <QtGui>

class Utils : public QObject
{
public:
	QIcon deficon;
	QRegExp spaceRegex;
	
	Utils(QObject *parent = 0);

	QPixmap updateScreenshot(const QString & gameName);
	QIcon loadIcon(const QString & gameName);
	QIcon loadWinIco(QIODevice *device);
	QString getViewString(const QModelIndex &index, int column) const;
	QString getHistory(const QString &, const QString &);

	QString capitalizeStr(const QString & str);
};

class MyQueue : public QObject
{
	Q_OBJECT

public:
	MyQueue(int c, QObject *parent = 0);
	QString dequeue();
	void enqueue(const QString & str);
	bool isEmpty() const;
private:
	int capacity;
	QQueue<QString> queue;
	mutable QMutex mutex;
};

#endif

