#include "qmc2main.h"

ProcessManager *procMan = NULL;

ProcessManager::ProcessManager(QWidget *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::ProcessManager(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  procCount = 0;
}

ProcessManager::~ProcessManager()
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::~ProcessManager()");
#endif

}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect)
{
#ifdef QMC2_DEBUG
  QString logMsg = "DEBUG: ProcessManager::start(QString &command = \"" + command + "\", QStringList &arguments = \"";
  int argCount;
  for (argCount = 0; argCount < arguments.count(); argCount++)
    logMsg += QString(argCount > 0 ? " " + arguments[argCount] : arguments[argCount]);
  logMsg += "\", bool autoConnect = " + QString(autoConnect ? "TRUE" : "FALSE") + ")";
  win->log(LOG_QMC2, logMsg);
#endif

  QProcess *proc = new QProcess(this);
  if ( autoConnect ) {
    lastCommand = command;
    int i;
    for (i = 0; i < arguments.count(); i++)
      lastCommand += " " + arguments[i];
    win->log(LOG_QMC2, tr("starting emulator #%1, command = %2").arg(procCount).arg(lastCommand));
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(proc, SIGNAL(started()), this, SLOT(started()));
    connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
  }
  proc->start(command, arguments);

  procMap[proc] = procCount++;
  return procCount - 1;
}

QProcess *ProcessManager::process(ushort index)
{
  QList<QProcess *> vl = procMap.keys(index);
  if ( vl.count() > 0 )
    return vl.at(0);
  else
    return NULL;
}

void ProcessManager::terminate(QProcess *proc)
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::terminate(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

  win->log(LOG_QMC2, tr("terminating emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::terminate(ushort index = " + QString::number(index) + ")");
#endif

  terminate(process(index));
}

void ProcessManager::kill(QProcess *proc)
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::kill(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

  win->log(LOG_QMC2, tr("killing emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->kill();
}

void ProcessManager::kill(ushort index)
{
#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::kill(ushort index = " + QString::number(index) + ")");
#endif

  kill(process(index));
}

void ProcessManager::readyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::readyReadStandardOutput(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      win->log(LOG_MAME, tr("stdout[#%1]: ").arg(procMap[proc]) + s);
  }
}

void ProcessManager::readyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::readyReadStandardError(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardError();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      win->log(LOG_MAME, tr("stderr[#%1]: ").arg(procMap[proc]) + s);
  }
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::finished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = "+ QString::number(exitStatus) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  win->log(LOG_QMC2, tr("emulator #%1 finished, exit code = %2, exit status = %3, remaining emulators = %4").arg(procMap[proc]).arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))).arg(procMap.count() - 1));
  procMap.remove(proc);
}

void ProcessManager::started()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::started(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  win->log(LOG_QMC2, tr("emulator #%1 started, PID = %2, running emulators = %3").arg(procMap[proc]).arg((quint64)proc->pid()).arg(procMap.count()));
}

void ProcessManager::error(QProcess::ProcessError processError)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::error(QProcess::ProcessError processError = " + QString::number(processError) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}

void ProcessManager::stateChanged(QProcess::ProcessState processState)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  win->log(LOG_QMC2, "DEBUG: ProcessManager::stateChanged(QProcess::ProcessState processState = " + QString::number(processState) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}
