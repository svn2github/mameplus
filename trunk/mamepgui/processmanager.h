#ifndef _PROCESSMANAGER_H_
#define _PROCESSMANAGER_H_

#include <QtGui>

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

extern ProcessManager *procMan;

#endif /* _PROCESSMANAGER_H_ */
