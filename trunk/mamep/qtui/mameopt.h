#ifndef _MAMEOPT_H_
#define _MAMEOPT_H_

#include <QtGui>

class ResetWidget : public QWidget
{
	Q_OBJECT
public:
	QWidget *subWidget;

	ResetWidget(/*QtProperty *property,*/ QWidget *parent = 0);

	void setWidget(QWidget *widget);
	void setResetEnabled(bool enabled);
/*	void setValueText(const QString &text);
	void setValueIcon(const QIcon &icon);*/
	void setSpacing(int spacing);

signals:
//	void resetProperty(QtProperty *property);
private slots:
	void slotClicked();

private:
//	QtProperty *m_property;
	QLabel *m_textLabel;
	QLabel *m_iconLabel;
	QToolButton *m_button;
	int m_spacing;
};


class OptionDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	OptionDelegate(QObject *parent = 0);

	QSize sizeHint ( const QStyleOptionViewItem & option, 
		const QModelIndex & index ) const;
	void paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index ) const;
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
		const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor,
		const QStyleOptionViewItem &option, const QModelIndex &index) const;

public slots:
	void sync();

private:
	bool isReset;
};

class MameOption : public QObject
{
public:
	QString guiname, defvalue, description, currvalue, max, min, 
		globalvalue, srcvalue, biosvalue, cloneofvalue;
	
	int type;
	bool guivisible, globalvisible, srcvisible, biosvisible, cloneofvisible, gamevisible;
	QList<QString> values;
	QList<QString> guivalues;

	MameOption(QObject *parent = 0);
};

class OptionUtils : public QObject
{
	Q_OBJECT

public:
	OptionUtils(QObject *parent = 0);

	int getType(const QModelIndex &);
	QVariant getField(const QModelIndex &, int);
	const QString getLongName(QString);
	const QString getLongValue(const QString &, const QString &);
	const QString getShortValue(const QString &, const QString &);
	QColor inheritColor(const QModelIndex &);
	bool isChanged(const QModelIndex &);
	bool isTitle(const QModelIndex &);

public slots:
	void loadDefault(QString);
	void loadTemplate();
	void loadIni(int, const QString &);
	void save(int , const QString &);
	QHash<QString, QString> readIniFile(const QString &);

	void initOption();
	void updateModel(QListWidgetItem *currItem = 0, int optType = -1);
	void setupModelData(QString, int);
	void updateHeaderSize(int, int, int);

private:
	void addModelItemTitle(QStandardItemModel*, QString);
	void addModelItem(QStandardItemModel*, QString);
//	void exportToIni(QString useFileName = QString());
//	void importFromIni(QString useFileName = QString());
};

enum
{
	OPTNFO_DEF = 0,
	OPTNFO_GLOBAL,
	OPTNFO_SRC,
	OPTNFO_BIOS,
	OPTNFO_CLONEOF,
	OPTNFO_CURR,
	OPTNFO_LAST
};

class OptionInfo : public QObject
{
public:
	QListWidget *catView;
	QTreeView *optView;
	QStandardItemModel *optModel;
	
	OptionInfo(QListWidget *, QTreeView *, QObject *parent = 0);
};

#endif
