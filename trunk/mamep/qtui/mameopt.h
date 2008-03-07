#ifndef _MAMEOPT_H_
#define _MAMEOPT_H_

#include <QString>
#include <QTime>
#include <QProcess>
#include <QIcon>
#include <QFile>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QList>
#include <QTreeView>
#include <QVariant>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include <QFileIconProvider>
#include <QThread>
#include <QItemDelegate>
#include <QObject>
#include <QSize>
#include <QSpinBox>


class OptionDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	OptionDelegate(QObject *parent = 0);

	QSize OptionDelegate::sizeHint ( const QStyleOptionViewItem & option, 
		const QModelIndex & index ) const;

	void OptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index ) const;

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
		const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;

};



class MameOption : public QObject
{
public:
	QString guiname, defvalue, description, currvalue, section, max, min;
	int type;
	QList<QString> values;
	QList<QString> guivalues;

	MameOption(QObject *parent = 0);
};

class MameOptions : public QObject
{
	Q_OBJECT

public:
	QHash<QString, MameOption *> nameMameOptionMap;

	MameOptions(QObject *parent = 0);

public slots:
	void load();
//	void exportToIni(QString useFileName = QString());
//	void importFromIni(QString useFileName = QString());
};

enum
{
	MAMEOPT_TYPE_BOOL = 100,
	MAMEOPT_TYPE_INT,
	MAMEOPT_TYPE_FLOAT,
	MAMEOPT_TYPE_STRING,
	MAMEOPT_TYPE_UNKNOWN
	//MAMEOPT_TYPE_COLOR,
	//MAMEOPT_TYPE_CSV,
};

enum
{
	USERROLE_TYPE = 0,
	USERROLE_MIN,
	USERROLE_MAX,
	USERROLE_VALLIST,
	USERROLE_GUIVALLIST
};


#endif
