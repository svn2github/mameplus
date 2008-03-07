#include <QtGui>


#include "mameopt.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern MameOptions *mameopts;

static QStandardItemModel *optmodel;
OptionDelegate *optdelegate;

OptionDelegate::OptionDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize OptionDelegate::sizeHint ( const QStyleOptionViewItem & option, 
								const QModelIndex & index ) const
{
	return QSize(60,18);
}

void OptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
				  			const QModelIndex &index ) const
{
	int opttype = index.model()->data(index, Qt::UserRole + USERROLE_TYPE).toInt();
	switch (opttype)
	{
		case MAMEOPT_TYPE_BOOL:
		{
			bool value = index.model()->data(index, Qt::EditRole).toBool();
			QStyleOptionButton ctrl;
			ctrl.state = QStyle::State_Enabled | (value ? QStyle::State_On : QStyle::State_Off);
			ctrl.direction = QApplication::layoutDirection();
			ctrl.rect = option.rect;
			ctrl.fontMetrics = QApplication::fontMetrics();

			if (option.state & QStyle::State_Selected)
				painter->fillRect(option.rect, option.palette.highlight());

			QApplication::style()->drawControl(QStyle::CE_CheckBox, &ctrl, painter);

			return;
		}
		default:
		{
			QItemDelegate::paint(painter, option, index);
			return;
		}
	}
}


QWidget *OptionDelegate::createEditor(QWidget *parent,
									   const QStyleOptionViewItem & option,
									   const QModelIndex & index) const
{
	int opttype = index.model()->data(index, Qt::UserRole + USERROLE_TYPE).toInt();
	switch (opttype)
	{
		case MAMEOPT_TYPE_BOOL:
		{
			QCheckBox *ctrl = new QCheckBox(parent);
			return ctrl;
		}

		case MAMEOPT_TYPE_INT:
		{
			int min, max;
			min = index.model()->data(index, Qt::UserRole + USERROLE_MIN).toInt();
			max = index.model()->data(index, Qt::UserRole + USERROLE_MAX).toInt();

			QSpinBox *ctrl = new QSpinBox(parent);
			ctrl->setMinimum(min);
			ctrl->setMaximum(max);
				
			return ctrl;
		}
		case MAMEOPT_TYPE_FLOAT:
		{
			float min, max;
			min = index.model()->data(index, Qt::UserRole + USERROLE_MIN).toDouble();
			max = index.model()->data(index, Qt::UserRole + USERROLE_MAX).toDouble();

			QDoubleSpinBox *ctrl = new QDoubleSpinBox(parent);
			ctrl->setMinimum(min);
			ctrl->setMaximum(max);
			ctrl->setSingleStep(0.1);
				
			return ctrl;
		}
		case MAMEOPT_TYPE_STRING:
		{
			QComboBox *ctrl = new QComboBox(parent);
//			ctrl->installEventFilter(const_cast<OptionDelegate*>(this));

			QList<QVariant> guivalues = index.model()->data(index, Qt::UserRole + USERROLE_GUIVALLIST).toList();
			foreach (QVariant guivalue, guivalues)
				ctrl->addItem(guivalue.toString());

			return ctrl;
		}
		default:
		{
			return 0;
		}
	}
}

void OptionDelegate::setEditorData(QWidget *editor,
									const QModelIndex &index) const
{
	int opttype = index.model()->data(index, Qt::UserRole + USERROLE_TYPE).toInt();
	
	switch (opttype)
	{
		case MAMEOPT_TYPE_BOOL:
		{
			bool value = index.model()->data(index, Qt::EditRole).toBool();
			QCheckBox *ctrl = static_cast<QCheckBox*>(editor);
			ctrl->setCheckState(value ? Qt::Checked : Qt::Unchecked);
			break;
		}
		case MAMEOPT_TYPE_INT:
		{
			int value = index.model()->data(index, Qt::EditRole).toInt();
			QSpinBox *ctrl = static_cast<QSpinBox*>(editor);
			ctrl->setValue(value);
			break;
		}
		case MAMEOPT_TYPE_FLOAT:
		{
			float value = index.model()->data(index, Qt::EditRole).toDouble();
			QDoubleSpinBox *ctrl = static_cast<QDoubleSpinBox*>(editor);
			ctrl->setValue(value);
			break;
		}
		case MAMEOPT_TYPE_STRING:
		{
			QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();

			QList<QVariant> guivalues = index.model()->data(index, Qt::UserRole + USERROLE_GUIVALLIST).toList();
			QComboBox *ctrl = static_cast<QComboBox*>(editor);
			ctrl->setCurrentIndex(guivalues.indexOf(guivalue));
			break;
		}
		default:
		{
			QItemDelegate::setEditorData(editor, index);
		}
	}
}

void OptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								   const QModelIndex &index) const
{
	QVariant value;

	int opttype = index.model()->data(index, Qt::UserRole + USERROLE_TYPE).toInt();
	switch (opttype)
	{
		case MAMEOPT_TYPE_BOOL:
		{
			QCheckBox *ctrl = static_cast<QCheckBox*>(editor);
			value = (ctrl->checkState() == Qt::Checked) ? true : false;
			break;
		}
		case MAMEOPT_TYPE_INT:
		{
			QSpinBox *ctrl = static_cast<QSpinBox*>(editor);
			ctrl->interpretText();
			value = ctrl->value();
			break;
		}
		case MAMEOPT_TYPE_FLOAT:
		{
			QDoubleSpinBox *ctrl = static_cast<QDoubleSpinBox*>(editor);
			ctrl->interpretText();
			value = ctrl->value();
			break;
		}
		case MAMEOPT_TYPE_STRING:
		{
			QComboBox *ctrl = static_cast<QComboBox*>(editor);
			value = ctrl->currentText();

			// convert GUI description to value
			QList<QVariant> values = index.model()->data(index, Qt::UserRole + USERROLE_VALLIST).toList();
			model->setData(index, values[ctrl->currentIndex()], Qt::DisplayRole);
			break;
		}
		default:
		{
			QItemDelegate::setModelData(editor, model, index);
		}
	}

	model->setData(index, value);
}

void OptionDelegate::updateEditorGeometry(QWidget *editor,
										   const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}


class OptionXMLHandler : public QXmlDefaultHandler
{
public:
	OptionXMLHandler(int d = 0)
	{
		mameoption = 0;
		metRootTag = false;
	}

	bool startElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName,
		const QXmlAttributes &attributes)
	{
		if (!metRootTag && qName != "optiontemplate")
			return false;

		if (qName == "optiontemplate")
			metRootTag = true;
		else if (qName == "section")
		{
			section = attributes.value("name");
		}
		else if (qName == "option")
		{
			mameoption = new MameOption(mameopts);
			mameoption->section = section;
			mameoption->guiname = attributes.value("guiname");
			mameoption->defvalue = attributes.value("default");
			mameoption->max = attributes.value("max");
			mameoption->min = attributes.value("min");

			QString type = attributes.value("type");
			if (type == "string")
				mameoption->type = MAMEOPT_TYPE_STRING;
			else if (type == "int")
				mameoption->type = MAMEOPT_TYPE_INT;
			else if (type == "float")
				mameoption->type = MAMEOPT_TYPE_FLOAT;
			else if (type == "bool")
				mameoption->type = MAMEOPT_TYPE_BOOL;
			else
				mameoption->type = MAMEOPT_TYPE_UNKNOWN;

			mameopts->nameMameOptionMap[attributes.value("name")] = mameoption;
		}
		else if (qName == "value")
		{
			mameoption->guivalues << attributes.value("guivalue");
		}

		currentText.clear();
		return true;
	}

	bool OptionXMLHandler::endElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName)
	{
		if (qName == "description")
			mameoption->description = currentText;
		else if (qName == "value")
			mameoption->values << currentText;

		return true;
	}

	bool OptionXMLHandler::characters(const QString &str)
	{
		currentText += str;
		return true;
	}	

private:
	MameOption *mameoption;
	QString section;
	QString currentText;
	bool metRootTag;
};


MameOption::MameOption(QObject *parent)
: QObject(parent)
{
	//	qmc2MainWindow->log(LOG_QMC2, "# MameOption()");
}

MameOptions::MameOptions(QObject *parent)
: QObject(parent)
{
}

void MameOptions::load()
{
	QString fileName = ":/res/optiontemplate.xml";

	//open option template file
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(qmc2MainWindow->treeViewOption, tr("MAME GUI error"),
							  tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return;
	}

	QXmlInputSource xmlInputSource(&file);
	OptionXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	if (reader.parse(xmlInputSource))
		;//statusBar()->showMessage(tr("File loaded"), 2000);


	//init option listview
	if (optmodel)
		delete optmodel;
	optmodel = new QStandardItemModel(mameopts->nameMameOptionMap.size(), 2, qmc2MainWindow);
	qmc2MainWindow->treeViewOption->setModel(optmodel);

    optmodel->setHeaderData(0, Qt::Horizontal, QObject::tr("Option"));
    optmodel->setHeaderData(1, Qt::Horizontal, QObject::tr("Value "));
	
	if (optdelegate)
		delete optdelegate;
	optdelegate = new OptionDelegate(qmc2MainWindow);
	qmc2MainWindow->treeViewOption->setItemDelegate(optdelegate);

	MameOption *pmameopt;
	QModelIndex index;
	int row = 0;
	foreach (QString optname, mameopts->nameMameOptionMap.keys())
	{
		pmameopt = mameopts->nameMameOptionMap[optname];
		QString tooltip = "[" + optname + "] " + pmameopt->description;

		index = optmodel->index(row, 0, QModelIndex());
		optmodel->setData(index, pmameopt->guiname, Qt::DisplayRole);
		optmodel->setData(index, tooltip, Qt::ToolTipRole);
		index = optmodel->index(row, 1, QModelIndex());
		optmodel->setData(index, pmameopt->defvalue);
		optmodel->setData(index, tooltip, Qt::ToolTipRole);

		//fill GUI description instead
		int i = pmameopt->values.indexOf(pmameopt->defvalue);
		if ( i > -1)
			optmodel->setData(index, pmameopt->guivalues[i], Qt::DisplayRole);

		optmodel->setData(index, QVariant(pmameopt->type), Qt::UserRole + USERROLE_TYPE);
		optmodel->setData(index, QVariant(pmameopt->min), Qt::UserRole + USERROLE_MIN);
		optmodel->setData(index, QVariant(pmameopt->max), Qt::UserRole + USERROLE_MAX);
		optmodel->setData(index, QVariant(pmameopt->values), Qt::UserRole + USERROLE_VALLIST);
		optmodel->setData(index, QVariant(pmameopt->guivalues), Qt::UserRole + USERROLE_GUIVALLIST);

		row++;
	}

	qmc2MainWindow->treeViewOption->resizeColumnToContents(1);

//QMessageBox::warning(qmc2MainWindow->treeViewOption, tr("MAME GUI error"),
//						  QString::number(mameopts->nameMameOptionMap.size()));
}

