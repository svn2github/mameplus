#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <QtGui>

enum
{
	AUDIT_ONLY = 0,
	AUDIT_EXPORT_ALL,
	AUDIT_EXPORT_INCOMPLETE,
	AUDIT_EXPORT_MISSING
};

class RomAuditor : public QThread
{
	Q_OBJECT

public:
	RomAuditor(QObject *parent = 0);
	~RomAuditor();
	void audit(bool = false, int = AUDIT_ONLY, QString = "");

public slots:
	void exportDat();

signals:
	void progressSwitched(int max, QString title = "");
	void progressUpdated(int progress);
	void logUpdated(char, QString);

protected:
	void run();

private:
	void auditConsole(QString);

	bool isConsoleFolder;
	bool hasAudited;
	int method;
	QString fixDatFileName;
	QMutex mutex;
};

class MergedRomAuditor : public QThread
{
	Q_OBJECT

public:
	~MergedRomAuditor();

public slots:
	void audit();

signals:
	void progressSwitched(int max, QString title = "");
	void progressUpdated(int progress);
	void logUpdated(char, QString);

protected:
	void run();

private:
	QString outBuf, consoleName, consolePath, mergedName;
	QList<QStringList> consoleInfoList;
};

class MameExeRomAuditor : public QObject
{
	Q_OBJECT
public:
	QProcess *loadProc;

	MameExeRomAuditor(QObject *parent = 0);
	void audit();

public slots:
	void auditorReadyReadStandardOutput();
	void auditorFinished(int, QProcess::ExitStatus);

private:
	QString outBuf;
};


#endif
