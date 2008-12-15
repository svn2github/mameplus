#ifndef _IPS_H_
#define _IPS_H_

#include <QtGui>
#include "ui_ips.h"

class IPS: public QDialog, public Ui::IPS
{
Q_OBJECT

public:
	IPS(QWidget *parent = 0);
	void init();
	bool checkAvailable(const QString & = NULL);
	void updateList();
	void parseRelations();

public slots:
	void parse(QTreeWidgetItem *, QTreeWidgetItem *, const QString & = NULL);
	void save();
	void clear();
	void applyRelations(QTreeWidgetItem * = NULL, int = 0);

private:
	QString ipspath;
	QString ipsValues;
	QStringList datFiles;
	bool stopListenRelations;

	QList <QStringList> confTable;
	QMultiHash <QString, QStringList> depTable;
	QHash <QString, int> itemStateTable;

	enum
	{
		ITR_SAVE = 0,
		ITR_CLEAR,
		ITR_RELATIONS
	};

	void validateConf(const QString &);
	void validateDep(const QString &);
	void iterateItems(QTreeWidgetItem *, int);
};

#endif
