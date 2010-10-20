#ifndef _IPS_H_
#define _IPS_H_

#include <QtGui>

#include "ui_ips.h"

class IpsUI: public QDialog, public Ui::IpsUI
{
Q_OBJECT

public:
	IpsUI(QWidget *parent = 0);
	void init();
	bool checkAvailable(const QString & = "");
	void parseRelations();

public slots:
	void parse(QTreeWidgetItem *, QTreeWidgetItem *, const QString & = "", const QString & = "");
	void updateList();
	void save();
	void clear();
	void applyRelations(QTreeWidgetItem * = NULL, int = 0);

private:
	QStringList ipsLangs;
	QString ipspath;
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
