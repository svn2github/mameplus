#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <QtGui>

class RomAuditor : public QThread
{
	Q_OBJECT

public:
	~RomAuditor();
	void audit();

signals:
	void progressSwitched(int max, QString title = "");
	void progressUpdated(int progress);
	void logUpdated(char, QString);

protected:
	void run();

private:
	void auditConsole(QString);

	bool isConsoleFolder;
	QMutex mutex;
};

class MergedRomAuditor : public QObject
{
	Q_OBJECT
public:
	QProcess *loadProc;

	MergedRomAuditor(QObject *parent = 0);

public slots:
	void init();
	void auditorReadyReadStandardOutput();
	void auditorFinished(int, QProcess::ExitStatus);

private:
	void audit();

	QString outBuf, consoleName, consolePath, mergedName;
	QList<QStringList> consoleInfoList;
};

#endif
