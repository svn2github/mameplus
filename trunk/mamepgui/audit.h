#ifndef _AUDIT_H_
#define _AUDIT_H_

#include <QtGui>

enum
{
	AUDIT_ONLY = 0,
	AUDIT_EXPORT_COMPLETE,
	AUDIT_EXPORT_ALL,
	AUDIT_EXPORT_INCOMPLETE,
	AUDIT_EXPORT_MISSING
};

enum
{
	VERIFY_CURRENT_ROMS = 0,
	VERIFY_ALL_ROMS,
	VERIFY_CURRENT_SAMPLES,
	VERIFY_ALL_SAMPLES
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

class MameExeRomAuditor : public QObject
{
Q_OBJECT
public:
	QProcess *loadProc;

	MameExeRomAuditor(QObject *parent = 0);
	void audit(int = VERIFY_CURRENT_ROMS);

public slots:
	void auditorReadyReadStandardOutput();
	void auditorClosed();

private:
	QDialog dlgAudit;
	QTextBrowser *tbAudit;
};

#endif
