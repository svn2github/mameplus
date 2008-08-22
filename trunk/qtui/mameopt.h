#ifndef _MAMEOPT_H_
#define _MAMEOPT_H_

#include <QtGui>

class ResetWidget : public QWidget
{
	Q_OBJECT
public:
	QWidget *subWidget;
	QWidget *subWidget2;

	ResetWidget(/*QtProperty *property,*/ QWidget *parent = 0);

	void setWidget(QWidget *, QWidget * = NULL);
	void setResetEnabled(bool enabled);
/*	void setValueText(const QString &text);
	void setValueIcon(const QIcon &icon);*/
	void setSpacing(int spacing);

signals:
//	void resetProperty(QtProperty *property);
private slots:
	void slotClicked();

public slots:
	void updateSliderLabel(int);


private:
//	QtProperty *m_property;
	QLabel *m_textLabel;
	QLabel *m_iconLabel;
	QSlider *m_slider;
	QLabel *m_sliderLabel;
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
	void initOption();
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
	void saveIniFile(int , const QString &);
	QHash<QString, QString> readIniFile(const QString &);
	void updateModel(QListWidgetItem *currItem = 0, int optType = -1);
	void updateHeaderSize(int, int, int);

private:
	void addModelItemTitle(QStandardItemModel*, QString);
	void addModelItem(QStandardItemModel*, QString);
	void updateModelData(QString, int);
};

enum
{
	OPTLEVEL_DEF = 0,
	OPTLEVEL_GLOBAL,
	OPTLEVEL_SRC,
	OPTLEVEL_BIOS,
	OPTLEVEL_CLONEOF,
	OPTLEVEL_CURR,
	OPTLEVEL_LAST
};

class OptInfo : public QObject
{
public:
	QListWidget *lstCatView;
	QTreeView *optView;
	QStandardItemModel *optModel;

	OptInfo(QListWidget *, QTreeView *, QObject *parent = 0);
};

#endif
