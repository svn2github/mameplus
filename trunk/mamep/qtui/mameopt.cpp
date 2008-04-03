#include "qmc2main.h"

OptionUtils *optUtils;

static QHash<QString, MameOption*> mameOpts;
static QList<OptionInfo *> optInfos;
static QMap<QString, QStringList> optCatMap;

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
	USERROLE_TITLE = 0,
	USERROLE_KEY,
	USERROLE_TYPE,
	USERROLE_MIN,
	USERROLE_MAX,
	USERROLE_DEFAULT,
	USERROLE_VALLIST,
	USERROLE_GUIVALLIST
};

OptionInfo::OptionInfo(QTreeView *optv, QObject *parent)
: QObject(parent)
{
	optView = optv;
	optModel = NULL;
}

ResetWidget::ResetWidget(/*QtProperty *property, */ QWidget *parent) :
QWidget(parent),
//m_property(property),
m_textLabel(new QLabel(this)),
m_iconLabel(new QLabel(this)),
m_button(new QToolButton(this)),
m_spacing(-1)
{
	m_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
	m_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	m_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_button->setIcon(QIcon(":/res/reset_property.png"));
	m_button->setIconSize(QSize(8,8));
	m_button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
	//	connect(m_button, SIGNAL(clicked()), this, SLOT(slotClicked()));
	QLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(m_spacing);
	layout->addWidget(m_iconLabel);
	layout->addWidget(m_textLabel);
	layout->addWidget(m_button);
	setFocusProxy(m_textLabel);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
	m_spacing = spacing;
	layout()->setSpacing(m_spacing);
}

void ResetWidget::setWidget(QWidget *widget)
{
	if (m_textLabel) {
		delete m_textLabel;
		m_textLabel = 0;
	}
	if (m_iconLabel) {
		delete m_iconLabel;
		m_iconLabel = 0;
	}
	delete layout();
	QLayout *layout = new QHBoxLayout(this);
	subWidget = widget;
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->addWidget(widget);
	layout->addWidget(m_button);
	setFocusProxy(widget);
}

void ResetWidget::setResetEnabled(bool enabled)
{
	m_button->setEnabled(enabled);
}

/*
void ResetWidget::setValueText(const QString &text)
{
	if (m_textLabel)
		m_textLabel->setText(text);
}

void ResetWidget::setValueIcon(const QIcon &icon)
{
	QPixmap pix = icon.pixmap(QSize(16, 16));
	if (m_iconLabel) {
		m_iconLabel->setVisible(!pix.isNull());
		m_iconLabel->setPixmap(pix);
	}
}

void ResetWidget::slotClicked()
{
	emit resetProperty(m_property);
}
*/

OptionDelegate::OptionDelegate(QObject *parent)
: QItemDelegate(parent)
{
}

QSize OptionDelegate::sizeHint ( const QStyleOptionViewItem & option, 
								const QModelIndex & index ) const
{
	return QSize(60,18);
}

void OptionDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
						   const QModelIndex &index ) const
{
	QStyleOptionViewItem option = opt;
	QBrush brush;

	if (!optUtils->isTitle(index))
	{
		brush = QBrush(optUtils->inheritColor(index));
		//render option name
		if (index.column() == 0)
		{
			if (optUtils->isChanged(index))
				option.font.setBold(true);
		}
		//render option value
		else
		{
			int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
			switch (optType)
			{
			case MAMEOPT_TYPE_BOOL:
				{
					bool value = index.model()->data(index, Qt::EditRole).toBool();
					QStyleOptionButton ctrl;
					ctrl.state = QStyle::State_Enabled | (value ? QStyle::State_On : QStyle::State_Off);
					ctrl.direction = QApplication::layoutDirection();
					ctrl.rect = option.rect;
					ctrl.fontMetrics = QApplication::fontMetrics();

					if (index.column() == 1)
						option.state &= ~QStyle::State_Selected;

					// fixme: selection background is unwanted
					if (option.state & QStyle::State_Selected)
						painter->fillRect(option.rect, option.palette.highlight());
					else
						painter->fillRect(option.rect, brush);

					QApplication::style()->drawControl(QStyle::CE_CheckBox, &ctrl, painter);
					return;
				}
			}
		}
	}
	else
	{
		option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
		option.font.setBold(true);
		brush = option.palette.dark();
	}
	
//	if (index.column() == 1)
//		option.state &= ~QStyle::State_Selected;

	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());
	else
		painter->fillRect(option.rect, brush);

	QItemDelegate::paint(painter, option, index);
	return;
}

QWidget *OptionDelegate::createEditor(QWidget *parent,
									  const QStyleOptionViewItem & option,
									  const QModelIndex & index) const
{
	if (index.column() == 0)
		return 0;

	ResetWidget *resetWidget = new ResetWidget(parent);

	int opttype = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (opttype)
	{
	case MAMEOPT_TYPE_BOOL:
		{
			QCheckBox *ctrl = new QCheckBox(parent);
			
			resetWidget->setWidget(ctrl);
			return resetWidget;
		}

	case MAMEOPT_TYPE_INT:
		{
			int min, max;
			min = optUtils->getField(index, USERROLE_MIN).toInt();
			max = optUtils->getField(index, USERROLE_MAX).toInt();

			QSpinBox *ctrl = new QSpinBox(parent);
			ctrl->setMinimum(min);
			ctrl->setMaximum(max);

			resetWidget->setWidget(ctrl);
			return resetWidget;
		}
	case MAMEOPT_TYPE_FLOAT:
		{
			float min, max;
			min = optUtils->getField(index, USERROLE_MIN).toDouble();
			max = optUtils->getField(index, USERROLE_MAX).toDouble();

			QDoubleSpinBox *ctrl = new QDoubleSpinBox(parent);
			ctrl->setMinimum(min);
			ctrl->setMaximum(max);
			ctrl->setSingleStep(0.01);

			resetWidget->setWidget(ctrl);
			return resetWidget;
		}
	case MAMEOPT_TYPE_STRING:
		{
			QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
			if (guivalues.size() > 0)
			{
				QComboBox *ctrl = new QComboBox(resetWidget);
				//	ctrl->installEventFilter(const_cast<OptionDelegate*>(this));
				foreach (QVariant guivalue, guivalues)
					ctrl->addItem(guivalue.toString());
				
				resetWidget->setWidget(ctrl);
				return resetWidget;
			}
			//fall to default
		}
	default:
		{
			QLineEdit *ctrl = new QLineEdit(resetWidget);
			resetWidget->setWidget(ctrl);
			return resetWidget;
		}
	}
}

void OptionDelegate::setEditorData(QWidget *editor,
								   const QModelIndex &index) const
{
	if (index.column() == 0)
		return;

	ResetWidget *resetWidget = static_cast<ResetWidget*>(editor);
	if (optUtils->isChanged(index))
		resetWidget->setResetEnabled(true);
	else
		resetWidget->setResetEnabled(false);

	int opttype = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (opttype)
	{
	case MAMEOPT_TYPE_BOOL:
		{
			bool value = index.model()->data(index, Qt::EditRole).toBool();
			QCheckBox *ctrl = static_cast<QCheckBox*>(resetWidget->subWidget);
			ctrl->setCheckState(value ? Qt::Checked : Qt::Unchecked);
			break;
		}
	case MAMEOPT_TYPE_INT:
		{
			int value = index.model()->data(index, Qt::EditRole).toInt();
			QSpinBox *ctrl = static_cast<QSpinBox*>(resetWidget->subWidget);
			ctrl->setValue(value);
			break;
		}
	case MAMEOPT_TYPE_FLOAT:
		{
			float value = index.model()->data(index, Qt::EditRole).toDouble();
			QDoubleSpinBox *ctrl = static_cast<QDoubleSpinBox*>(resetWidget->subWidget);
			ctrl->setValue(value);
			break;
		}
	case MAMEOPT_TYPE_STRING:
		{
			QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();

			QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
			if (guivalues.size() > 0)
			{
				QComboBox *ctrl = static_cast<QComboBox*>(resetWidget->subWidget);
				ctrl->setCurrentIndex(guivalues.indexOf(guivalue));
				break;
			}
		}
	default:
		{
			QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();
			
			QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);
			ctrl->setText(guivalue);
//			QItemDelegate::setEditorData(editor, index);
		}
	}
}

void OptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								  const QModelIndex &index) const
{
	if (index.column() == 0)
		return;

	// update control's display value
	QVariant dispValue;
	ResetWidget *resetWidget = static_cast<ResetWidget*>(editor);
	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	
	switch (optType)
	{
	case MAMEOPT_TYPE_BOOL:
		{
			QCheckBox *ctrl = static_cast<QCheckBox*>(resetWidget->subWidget);
			dispValue = (ctrl->checkState() == Qt::Checked) ? true : false;
			break;
		}
	case MAMEOPT_TYPE_INT:
		{
			QSpinBox *ctrl = static_cast<QSpinBox*>(resetWidget->subWidget);
			ctrl->interpretText();
			dispValue = ctrl->value();
			break;
		}
	case MAMEOPT_TYPE_FLOAT:
		{
			QDoubleSpinBox *ctrl = static_cast<QDoubleSpinBox*>(resetWidget->subWidget);
			ctrl->interpretText();
			// ensure .00 display
			dispValue = optUtils->getLongValue(optName, QString::number(ctrl->value()));
			break;
		}
	case MAMEOPT_TYPE_STRING:
		{
			QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
			if (guivalues.size() > 0)
			{
				QComboBox *ctrl = static_cast<QComboBox*>(resetWidget->subWidget);
				dispValue = ctrl->currentText();
				break;
			}
		}
	default:
		{
			QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);
			dispValue = ctrl->text();
//			QItemDelegate::setModelData(editor, model, index);
		}
	}
	
	model->setData(index, dispValue);


	// process inherited value overrides
	MameOption *pMameOpt = mameOpts[optName];
	int optInfoType = index.model()->objectName().remove("optModel").toInt();
	QString prevVal;
	QAbstractItemModel *overrideModel;
	QModelIndex overrideIndex;
	
	switch (optInfoType)
	{
	case OPTNFO_GLOBAL:
		{
			prevVal = pMameOpt->globalvalue;
			
			pMameOpt->globalvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = win->treeGlobalOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
			// fall to next case
		}
	case OPTNFO_SRC:
		{
			// prevent overwrite prevVal from prev case
			if (optInfoType == OPTNFO_SRC)
				prevVal = pMameOpt->srcvalue;

			if (pMameOpt->srcvalue == prevVal)
			{
				pMameOpt->srcvalue = optUtils->getShortValue(optName, dispValue.toString());

				overrideModel = win->treeSourceOpt->model();
				overrideIndex = overrideModel->index(index.row(), 1);
				overrideModel->setData(overrideIndex, dispValue);
				// fall to next case
			}
			else
				break;
		}
	case OPTNFO_BIOS:
		{
			if (optInfoType == OPTNFO_BIOS)
				prevVal = pMameOpt->biosvalue;

			if (pMameOpt->biosvalue == prevVal)
			{
				pMameOpt->biosvalue = optUtils->getShortValue(optName, dispValue.toString());

				overrideModel = win->treeBiosOpt->model();
				overrideIndex = overrideModel->index(index.row(), 1);
				overrideModel->setData(overrideIndex, dispValue);
			}
			else
				break;
		}
	case OPTNFO_CLONEOF:
		{
			if (optInfoType == OPTNFO_CLONEOF)
				prevVal = pMameOpt->cloneofvalue;

			if (pMameOpt->currvalue == prevVal)
			{
				pMameOpt->cloneofvalue = optUtils->getShortValue(optName, dispValue.toString());

				overrideModel = win->treeCloneofOpt->model();
				overrideIndex = overrideModel->index(index.row(), 1);
				overrideModel->setData(overrideIndex, dispValue);
			}
			else
				break;
		}
	case OPTNFO_CURR:
		{
			if (optInfoType == OPTNFO_CURR)
				prevVal = pMameOpt->currvalue;
		
			if (pMameOpt->currvalue == prevVal)
			{
				pMameOpt->currvalue = optUtils->getShortValue(optName, dispValue.toString());
			
				overrideModel = win->treeCurrOpt->model();
				overrideIndex = overrideModel->index(index.row(), 1);
				overrideModel->setData(overrideIndex, dispValue);
			}
		}
	}

	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[currentGame];

	optUtils->save(OPTNFO_GLOBAL, optName, "mame.ini");
	
	QString iniFileName = "ini/source/" + gameInfo->sourcefile.remove(".c") + ".ini";
	optUtils->save(OPTNFO_SRC, optName, iniFileName);
	
//	iniFileName = "ini/fixmebios.ini";	//fixme
//	optUtils->save(O, optName, iniFileName);

	iniFileName = "ini/" + gameInfo->cloneof + ".ini";
	optUtils->save(OPTNFO_CLONEOF, optName, iniFileName);
	
	iniFileName = "ini/" + currentGame + ".ini";
	optUtils->save(OPTNFO_CURR, optName, iniFileName);
}

void OptionDelegate::updateEditorGeometry(QWidget *editor,
										  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

// parse listxml and init default mame opts
class OptionXMLHandler : public QXmlDefaultHandler
{
private:
	MameOption *pMameOpt;
	QString currentText;
	bool metRootTag;

public:
	OptionXMLHandler(int d = 0)
	{
		pMameOpt = NULL;
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
		else if (qName == "option")
		{
			QString optName = attributes.value("name");

			if (mameOpts.contains(optName))
			{
				pMameOpt = mameOpts[optName];

				pMameOpt->guiname = attributes.value("guiname");
				pMameOpt->max = attributes.value("max");
				pMameOpt->min = attributes.value("min");

				QString type = attributes.value("type");
				if (type == "string")
					pMameOpt->type = MAMEOPT_TYPE_STRING;
				else if (type == "int")
					pMameOpt->type = MAMEOPT_TYPE_INT;
				else if (type == "float")
					pMameOpt->type = MAMEOPT_TYPE_FLOAT;
				else if (type == "bool")
					pMameOpt->type = MAMEOPT_TYPE_BOOL;
				else
					pMameOpt->type = MAMEOPT_TYPE_UNKNOWN;

				mameOpts[attributes.value("name")] = pMameOpt;				
			}
			else
				pMameOpt = NULL;
		}
		else if (qName == "value")
		{
			if (pMameOpt != NULL)
				pMameOpt->guivalues << attributes.value("guivalue");
		}

		currentText.clear();
		return true;
	}

	bool endElement(const QString & /* namespaceURI */,
		const QString & /* localName */,
		const QString &qName)
	{
		if (pMameOpt != NULL)
		{
			if (qName == "description")
				pMameOpt->description = currentText;
			else if (qName == "value")
				pMameOpt->values << currentText;
		}
		return true;
	}

	bool characters(const QString &str)
	{
		currentText += str;
		return true;
	}	
};


MameOption::MameOption(QObject *parent)
: QObject(parent)
{
	//	win->log(LOG_QMC2, "# MameOption()");
}

QVariant OptionUtils::getField(const QModelIndex &index, int field)
{
	QModelIndex i = index.sibling(index.row(), 0);
	QString optname = index.model()->data(i, Qt::UserRole + USERROLE_KEY).toString();

	if (mameOpts.contains(optname))
	{
		switch (field)
		{
		case USERROLE_KEY:
			{
				return optname;
			}
		case USERROLE_TYPE:
			{
				return mameOpts[optname]->type;
			}
		case USERROLE_MIN:
			{
				return mameOpts[optname]->min;
			}
		case USERROLE_MAX:
			{
				return mameOpts[optname]->max;
			}
		case USERROLE_DEFAULT:
			{
				return mameOpts[optname]->defvalue;
			}
		case USERROLE_VALLIST:
			{
				return QVariant(mameOpts[optname]->values);
			}
		case USERROLE_GUIVALLIST:
			{
				return QVariant(mameOpts[optname]->guivalues);
			}
		}
	}

	return QVariant();
}

const QString OptionUtils::getLongName(QString optname)
{
	const QString longname = mameOpts[optname]->guiname;
	if (longname.isEmpty())
		return utils->capitalizeStr(optname);

	return longname;
}

const QString OptionUtils::getLongValue(const QString &optname, const QString &optval)
{
	const MameOption *pMameOpt = mameOpts[optname];

	if (pMameOpt->type == MAMEOPT_TYPE_BOOL)
		return (optval=="0") ? "false" : "true";

	else if (pMameOpt->type == MAMEOPT_TYPE_FLOAT)
		return QString().sprintf("%.2f", optval.toFloat());

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING)
	{
		//fill value or GUI value desc
		int i = pMameOpt->values.indexOf(optval);
		if ( i > -1)
			return pMameOpt->guivalues[i];
	}

	return optval;
}

const QString OptionUtils::getShortValue(const QString &optname, const QString &optval)
{
	const MameOption *pMameOpt = mameOpts[optname];

	if (pMameOpt->type == MAMEOPT_TYPE_BOOL)
		return (optval=="true") ? "1" : "0";

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING)
	{
		//fill value or GUI value desc
		int i = pMameOpt->guivalues.indexOf(optval);
		if ( i > -1)
			return pMameOpt->values[i];
	}

	return optval;
}

QColor OptionUtils::inheritColor(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();

	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];

	if (dispVal == optUtils->getLongValue(optName, pMameOpt->defvalue))
		return Qt::transparent;
	else
		return QColor(0, 255, 0, 32);
}

bool OptionUtils::isChanged(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	int optInfoType = index.model()->objectName().remove("optModel").toInt();
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();
	
	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];
	switch (optInfoType)
	{
	case OPTNFO_GLOBAL:
		{
			compVal = pMameOpt->defvalue;
			break;
		}
	case OPTNFO_SRC:
		{
			compVal = pMameOpt->globalvalue;
			break;
		}
	case OPTNFO_BIOS:
		{
			compVal = pMameOpt->srcvalue;
			break;
		}
	case OPTNFO_CLONEOF:
		{
			compVal = pMameOpt->biosvalue;
			break;
		}
	case OPTNFO_CURR:
		{
			compVal = pMameOpt->cloneofvalue;
			break;
		}
	}

	compVal = optUtils->getLongValue(optName, compVal);
//	win->log(LOG_QMC2, QString("%1 / %2").arg(dispVal).arg(compVal));

	if (dispVal != compVal)
		return true;
	else
		return false;
}

bool OptionUtils::isTitle(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 0);
	return index.model()->data(i, Qt::UserRole + USERROLE_TITLE).toString() == "OPTIONTITLE";
}

OptionUtils::OptionUtils(QObject *parent)
: QObject(parent)
{
}

void OptionUtils::loadDefault(const QString &iniFileName)
{
	const QStringList optCatList = (QStringList()
			<< "00_Global Misc_00_CORE CONFIGURATION"
			<< "00_Global Misc_01_CORE PALETTE"
			<< "00_Global Misc_02_CORE LANGUAGE"

			<< "01_Global Path_00_CORE SEARCH PATH"
			<< "01_Global Path_01_CORE OUTPUT DIRECTORY"
			<< "01_Global Path_02_CORE FILENAME"

			<< "02_Video_00_CORE PERFORMANCE"
			<< "02_Video_01_CORE ROTATION"
			<< "02_Video_02_CORE SCREEN"
			<< "02_Video_03_WINDOWS VIDEO"
			<< "02_Video_04_DIRECTDRAW-SPECIFIC"
			<< "02_Video_05_DIRECT3D-SPECIFIC"
			<< "02_Video_06_PER-WINDOW VIDEO"
			<< "02_Video_07_FULL SCREEN"

			<< "03_Audio_00_CORE SOUND"
			<< "03_Audio_01_WINDOWS SOUND"

			<< "04_Control_00_CORE INPUT"
			<< "04_Control_01_CORE INPUT AUTOMATIC ENABLE"
			<< "04_Control_02_INPUT DEVICE"

			<< "05_Vector_00_CORE VECTOR"

			<< "06_Misc_00_CORE MISC"
			<< "06_Misc_01_CORE STATE/PLAYBACK"
			<< "06_Misc_02_CORE ARTWORK"
			<< "06_Misc_03_WINDOWS PERFORMANCE"
			<< "06_Misc_04_CORE DEBUGGING"
			<< "06_Misc_05_WINDOWS DEBUGGING"
			<< "06_Misc_06_MESS SPECIFIC"
			<< "06_Misc_07_WINDOWS MESS SPECIFIC"
			);

	QFile inFile(iniFileName);
	QString line, line0;

	if (inFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&inFile);
		in.setCodec("UTF-8");

		QString header = "Error";

		do
		{
			line0 = line = in.readLine().trimmed();
			
			//init option headers
			if (line.startsWith("#"))
			{
				if (line.size() > 2)
				{
					line.remove("#");
					line.remove("OPTIONS");
					line = line.trimmed();
					
					int c = optCatList.indexOf (QRegExp(".*" + line + ".*"));
					if (c > 0)
						header = optCatList[c];
					else
						header = "06_Misc_99_" + line;
					
				//	win->log(LOG_QMC2, "header: " + header);
				}
			}
			else
			{
				line.replace(QRegExp("(\\w+)\\s+.*"), "\\1");
				if (line.size() > 0)
				{
					optCatMap[header] << line;

					QStringList list = line0.split(QRegExp("\\s+"));
					MameOption *pMameOpt = new MameOption(0);	//fixme parent
					if (list.count() == 2)
					{
						pMameOpt->defvalue = list[1];
						mameOpts[list[0]] = pMameOpt;
					}
					else
					{
						pMameOpt->defvalue = "";
						mameOpts[line0.trimmed()] = pMameOpt;
					}
				}
			}
		}
		while (!line.isNull());
	}
}

//open option template file
void OptionUtils::load()
{
	QFile file(":/res/optiontemplate.xml");

	QXmlInputSource xmlInputSource(&file);
	OptionXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);
	reader.parse(xmlInputSource);
}

void OptionUtils::load(int optInfoType, const QString &iniFileName)
{
	QHash<QString, QString> inisettings = optUtils->readIniFile(iniFileName);
	
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];
		
		//take ini override
		if (inisettings.contains(optName))
		{
			pMameOpt->currvalue = inisettings[optName];

			switch (optInfoType)
			{
			case OPTNFO_GLOBAL:
				{
					pMameOpt->globalvalue = pMameOpt->currvalue;
					break;
				}
			case OPTNFO_SRC:
				{
					pMameOpt->srcvalue = pMameOpt->currvalue;
					break;
				}
			case OPTNFO_BIOS:
				{
					pMameOpt->biosvalue = pMameOpt->currvalue;
					break;
				}
			case OPTNFO_CLONEOF:
				{
					pMameOpt->cloneofvalue = pMameOpt->currvalue;
					break;
				}
			}
		}
		//inherit from higher level opts
		else
		{
			switch (optInfoType)
			{
			case OPTNFO_GLOBAL:
				{
					pMameOpt->currvalue = pMameOpt->globalvalue = pMameOpt->defvalue;
					break;
				}
			case OPTNFO_SRC:
				{
					pMameOpt->currvalue = pMameOpt->srcvalue = pMameOpt->globalvalue;
					break;
				}
			case OPTNFO_BIOS:
				{
					pMameOpt->currvalue = pMameOpt->biosvalue = pMameOpt->globalvalue;
					break;
				}
			case OPTNFO_CLONEOF:
				{
					pMameOpt->currvalue = pMameOpt->cloneofvalue= pMameOpt->srcvalue;
					break;
				}
			case OPTNFO_CURR:
				{
					pMameOpt->currvalue = pMameOpt->cloneofvalue;
					break;
				}
			}
		}
/*		win->log(LOG_QMC2, QString("%1: %2 / %3 / %4 / %5 / %6 / %7\n")
				.arg(optName)
				.arg(pMameOpt->defvalue)
				.arg(pMameOpt->globalvalue)
				.arg(pMameOpt->currvalue)
				.arg(pMameOpt->srcvalue)
				.arg(pMameOpt->cloneofvalue)
				.arg(inisettings[optName])
				);*/
	}
}

void OptionUtils::save(int optInfoType, const QString &optName, const QString &iniFileName)
{
//load ini text
	
//get option map
//compare with current value
//update/new/delete the spec entry
//save text	

	MameOption *pMameOpt = mameOpts[optName];
	QString currVal;
	switch (optInfoType)
	{
	case OPTNFO_GLOBAL:
		{
			currVal = pMameOpt->globalvalue;
			break;
		}
	case OPTNFO_SRC:
		{
			currVal = pMameOpt->srcvalue;
			break;
		}
	case OPTNFO_BIOS:
		{
			currVal = pMameOpt->biosvalue;
			break;
		}
	case OPTNFO_CLONEOF:
		{
			currVal = pMameOpt->cloneofvalue;
			break;
		}
	case OPTNFO_CURR:
		{
			currVal = pMameOpt->currvalue;
			break;
		}
	}


	QFile inFile(iniFileName);
	QTemporaryFile outFile;
	bool changed = false;
	QString line, line0;

	if (inFile.open(QFile::ReadOnly | QFile::Text) &&
		outFile.open())
	{
		outFile.setTextModeEnabled(true);
		QTextStream in(&inFile);
		QTextStream out(&outFile);
		in.setCodec("UTF-8");
		out.setCodec("UTF-8");

		do
		{
			line0 = line = in.readLine().trimmed();
			if (line.startsWith(optName) && line.remove(optName).trimmed() != currVal)
			{
				//replace value
				line = line0;
				line.replace(QRegExp("(\\w+\\s+).*"), QString("\\1%1").arg(currVal));
				changed = true;
			}
			if (!line.isNull())
				out << line << endl;
		}
		while (!line.isNull());
	}
	
	if (changed)
	{
		inFile.remove();
		outFile.copy(inFile.fileName());
	}
}

QHash<QString, QString> OptionUtils::readIniFile(const QString &iniFileName)
{
	QFile inFile(iniFileName);
	QTemporaryFile outFile;

	QString line, line0;

	if (inFile.open(QFile::ReadOnly | QFile::Text) &&
		outFile.open())
	{
		outFile.setTextModeEnabled(true);
		QTextStream in(&inFile);
		QTextStream out(&outFile);
		in.setCodec("UTF-8");
		out.setCodec("UTF-8");

		do
		{
			line0 = line = in.readLine().trimmed();
			if (!line.startsWith("#") && line.size() > 0)
			{
				//replace space with =
				//fixme, dirname with space
				line.replace(QRegExp("(\\w+)\\s+(.*)"), "\\1=\\2");
				//replace escape
				line.replace("\\", "\\\\");
				if (line0 != line)
			 		out << line << endl;
				else
					//for empty entry
					out << line << "=" << endl;
			}
		}
		while (!line.isNull());
	}
	
	QHash<QString, QString> settings;

	QSettings inisettings(outFile.fileName(), QSettings::IniFormat);
	QStringList keys = inisettings.allKeys();
	foreach (QString key, keys)
		settings[key] = inisettings.value(key).toString();
	
	return settings;
}

void OptionUtils::initOption()
{
	optInfos = (QList<OptionInfo *>() 
			<< new OptionInfo(0, win)
			<< new OptionInfo(win->treeGlobalOpt, win)
			<< new OptionInfo(win->treeSourceOpt, win)
			<< new OptionInfo(win->treeBiosOpt, win)
			<< new OptionInfo(win->treeCloneofOpt, win)
			<< new OptionInfo(win->treeCurrOpt, win));
}

void OptionUtils::setupModelData(int optType)
{
	QStandardItemModel *optModel = optInfos[optType]->optModel;
	QTreeView *optView = optInfos[optType]->optView;

	//init option listview
	//fixme: not needed every time
	if (optModel)
		delete optModel;
	optModel = new QStandardItemModel(mameOpts.size(), 2, win);
	optModel->setObjectName(QString("optModel%1").arg(optType));
	optView->setModel(optModel);

	optModel->setHeaderData(0, Qt::Horizontal, QObject::tr("Option"));
	optModel->setHeaderData(1, Qt::Horizontal, QObject::tr("Value"));

	int row = 0;
	
	foreach (QString optCatName, optCatMap.keys())
	{
		QStringList optNames = optCatMap[optCatName];

		QModelIndex index = optModel->index(row, 0, QModelIndex());
		optModel->setData(index, optCatName.split('_').last(), Qt::DisplayRole);
		optModel->setData(index, "OPTIONTITLE", Qt::UserRole + USERROLE_TITLE);
		row++;
		
		foreach (QString optName, optNames)
		{
//			win->log(LOG_QMC2, QString("%1//%2").arg(optCatName).arg(optName));
		
			MameOption *pMameOpt = mameOpts[optName];
			
			if (pMameOpt == NULL)
				continue;
	
			QString tooltip = "[" + optName + "] " + pMameOpt->description;

			// prepare GUI value desc
			for (int i=0; i<pMameOpt->guivalues.size(); i++)
			{
				if (pMameOpt->guivalues[i].isEmpty())
					pMameOpt->guivalues[i] = utils->capitalizeStr(pMameOpt->values[i]);
			}

			index = optModel->index(row, 0, QModelIndex());
			optModel->setData(index, optUtils->getLongName(optName), Qt::DisplayRole);
			optModel->setData(index, optName, Qt::UserRole + USERROLE_KEY);
			optModel->setData(index, tooltip, Qt::ToolTipRole);

			index = optModel->index(row, 1, QModelIndex());
			optModel->setData(index, optUtils->getLongValue(optName, pMameOpt->currvalue));
			optModel->setData(index, tooltip, Qt::ToolTipRole);

			row++;
		}
	}
/*
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		QString tooltip = "[" + optName + "] " + pMameOpt->description;

		// prepare GUI value desc
		for (int i=0; i<pMameOpt->guivalues.size(); i++)
		{
			if (pMameOpt->guivalues[i].isEmpty())
				pMameOpt->guivalues[i] = utils->capitalizeStr(pMameOpt->values[i]);
		}

		QModelIndex index = optModel->index(row, 0, QModelIndex());
		optModel->setData(index, optUtils->getLongName(optName), Qt::DisplayRole);
		optModel->setData(index, optName, Qt::UserRole + USERROLE_KEY);
		optModel->setData(index, tooltip, Qt::ToolTipRole);

		index = optModel->index(row, 1, QModelIndex());
		optModel->setData(index, optUtils->getLongValue(optName, pMameOpt->currvalue));
		optModel->setData(index, tooltip, Qt::ToolTipRole);

		row++;
	}
*/
	// share delegate among all opt treeviews
	static OptionDelegate optdelegate(win);
	optView->setItemDelegate(&optdelegate);
	
	optView->resizeColumnToContents(1);
}

