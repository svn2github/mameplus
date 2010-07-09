#ifndef _MAMEOPT_H_
#define _MAMEOPT_H_

#include <QtGui>

#include "ui_options.h"
#include "ui_csvcfg.h"

enum
{
	MAMEOPT_TYPE_BOOL = 100,
	MAMEOPT_TYPE_INT,
	MAMEOPT_TYPE_FLOAT,
	MAMEOPT_TYPE_STRING,
	MAMEOPT_TYPE_STRING_EDITABLE,
	MAMEOPT_TYPE_FILE,
	MAMEOPT_TYPE_DATFILE,
	MAMEOPT_TYPE_CFGFILE,
	MAMEOPT_TYPE_EXEFILE,
	MAMEOPT_TYPE_DIR,
	MAMEOPT_TYPE_DIRS,
	MAMEOPT_TYPE_CSV,
	MAMEOPT_TYPE_UNKNOWN
	//MAMEOPT_TYPE_COLOR,
};

enum
{
	OPTLEVEL_GUI = 0,
	OPTLEVEL_GLOBAL,
	OPTLEVEL_SRC,
	OPTLEVEL_BIOS,
	OPTLEVEL_CLONEOF,
	OPTLEVEL_CURR,
	OPTLEVEL_LAST
};

class OptionsUI : public QDialog, public Ui::OptionsUI
{
Q_OBJECT

public:
	OptionsUI(QWidget *parent = 0);
	QList<QListWidget *> optCtrls;
	void init(int, int = -1);

protected:
	void showEvent(QShowEvent *);
	void closeEvent(QCloseEvent *);
};

//a custom widget shown in the right side of each option item to restore default value
class ResetWidget : public QWidget
{
	Q_OBJECT
 
public:
	QWidget *subWidget;
	QWidget *subWidget2;

	ResetWidget(/*QtProperty *property,*/ QWidget *parent = 0);

	void setWidget(QWidget *, QWidget * = NULL, int = 0, int = 0);
	void setResetEnabled(bool enabled);
/*	void setValueText(const QString &text);
	void setValueIcon(const QIcon &icon);*/
	void setSpacing(int spacing);

signals:
//	void resetProperty(QtProperty *property);

public slots:
	void updateSliderLabel(int);

private:
//	QtProperty *m_property;
	QLabel *_textLabel;
	QLabel *_iconLabel;
	QSlider *_slider;
	QLabel *_sliderLabel;
	QToolButton *_btnSetDlg;
	QToolButton *_btnReset;
	int ctrlSpacing;
	int optType;
	int sliderOffset;

private slots:
	void slotClicked();
};

//for csv options
class CsvCfgUI : public QDialog, public Ui::CsvCfgUI
{
Q_OBJECT

public:
	CsvCfgUI(QWidget *parent = 0);
	void init(QString, QMap<QString, bool>);
	QString getCSV();
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
	void setChangesAccepted();
	void setCSV();
	void setCSVAccepted();
	void setDirectories();
	void setDirectoriesAccepted();
	void setDirectory();
	void setFile(QString = "", ResetWidget* = NULL);
	void setDatFile();
	void setExeFile();
	void setCfgFile();

private:
	ResetWidget *rWidget;
	bool isReset;
	QString csvBuf;
};

class MameOption : public QObject
{
public:
	QString guiname, defvalue, description, currvalue, max, min, 
		globalvalue, srcvalue, biosvalue, cloneofvalue;
	
	int type;
	bool guivisible, globalvisible, srcvisible, biosvisible, cloneofvisible, gamevisible;
	QStringList values;
	QStringList guivalues;

	MameOption(QObject *parent = 0);
};

class OptInfo : public QObject
{
public:
	QListWidget *lstCatView;
	QTreeView *optView;
	QStandardItemModel *optModel;

	OptInfo(QListWidget *, QTreeView *, QObject *parent = 0);
};

class OptionUtils : public QObject
{
Q_OBJECT

public:
	OptionUtils(QObject *parent = 0);
	void init();
	QVariant getField(const QModelIndex &, int);
	const QString getLongName(QString);
	const QString getLongValue(const QString &, const QString &);
	const QString getShortValue(const QString &, const QString &);
	QColor inheritColor(const QModelIndex &);
	bool isChanged(const QModelIndex &);
	bool isTitle(const QModelIndex &);
	void updateSelectableItems(QString);
	void loadDefault(QString);
	void saveIniFile(int , const QString &);

public slots:
	void preUpdateModel(QListWidgetItem *currItem = 0, int optType = -1, const QString &gameName = "", int method = 0);
	void updateHeaderSize(int, int, int);

private:
	//option category map, option category as key, names as value list
	QMap<QString, QStringList> optCatMap;
	QList<OptInfo *> optInfos;

	void loadIni(int, const QString &);
	void loadTemplate();
	QHash<QString, QString> parseIniFile(const QString &);
	void addModelItemTitle(QStandardItemModel*, QString);
	void addModelItem(QStandardItemModel*, QString);
	void updateModel(QString, int);
};

extern OptionUtils *optUtils;

extern QHash<QString, MameOption*> mameOpts;
extern QByteArray option_column_state;
extern QByteArray option_geometry;
extern QString mameIniPath;

extern bool isSDLPort;
extern bool hasLanguage;
extern bool hasIPS;
extern bool hasDevices;

#endif
