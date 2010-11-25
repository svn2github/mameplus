#include "processmanager.h"
#include "mainwindow.h"

/* global */
ProcessManager *procMan = NULL;


ProcessManager::ProcessManager(QWidget *parent) : 
	QObject(parent),
	procCount(0)
{
}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect)
{
	stdOut.clear();
	stdErr.clear();

	QProcess *proc = new QProcess(this);
	if (autoConnect)
	{
		lastCommand = command;

		for (int i = 0; i < arguments.size(); i++)
			lastCommand += " " + arguments[i];

		connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
		connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
		connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
		connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
		connect(proc, SIGNAL(started()), this, SLOT(started()));
	}
	
#ifdef Q_WS_WIN
	//explicitly assign WorkingDirectory during M1 loading
	proc->setWorkingDirectory(currentAppDir);
#endif
	procMap[proc] = procCount++;
	proc->start(command, arguments);
	return procCount - 1;
}

QProcess *ProcessManager::process(ushort index)
{
	QList<QProcess *> vl = procMap.keys(index);
	if ( vl.size() > 0 )
		return vl.at(0);
	else
		return NULL;
}

void ProcessManager::terminate(QProcess *proc)
{
	proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
	terminate(process(index));
}

void ProcessManager::kill(QProcess *proc)
{
	proc->kill();
}

void ProcessManager::kill(ushort index)
{
	kill(process(index));
}

void ProcessManager::readyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString s = QString::fromLocal8Bit(proc->readAllStandardOutput().data());
	QStringList sl = s.split("\n");
	int i;

	for (i = 0; i < sl.size(); i++)
	{
		s = sl[i].simplified();
		if ( !s.isEmpty() )
		{
			stdOut.append(s + "\n");
			win->log(QString("stdout[#%1]: ").arg(procMap[proc]) + s);\
		}
	}
}

void ProcessManager::readyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();
	QString s = QString::fromLocal8Bit(proc->readAllStandardError().data());
	QStringList sl = s.split("\n");
	int i;

	for (i = 0; i < sl.size(); i++)
	{
		s = sl[i].simplified();
		if ( !s.isEmpty() )
		{
			stdErr.append(s + "\n");
			win->log(QString("stderr[#%1]: ").arg(procMap[proc]) + s);
		}
	}
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();

	win->log(QString("proc #%1 finished, exit: %2, remaining: %3").arg(procMap[proc]).arg(exitCode).arg(procMap.size() - 1));
	procMap.remove(proc);
}

void ProcessManager::started()
{
	QProcess *proc = (QProcess *)sender();
	win->log(QString("proc #%1 started, active: %3").arg(procMap[proc]).arg(procMap.size()));
}

void ProcessManager::error(QProcess::ProcessError processError)
{
}
