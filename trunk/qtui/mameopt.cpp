#include "mamepguimain.h"

OptionUtils *optUtils;
//global collection of all MameOption4
QHash<QString, MameOption*> mameOpts;
QByteArray option_column_state;

QList<OptInfo *> optInfos;
//option category map, option category as key, names as value list
QMap<QString, QStringList> optCatMap;
OptionDelegate optdelegate(win);

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

OptInfo::OptInfo(QListWidget *catv, QTreeView *optv, QObject *parent)
: QObject(parent)
{
	lstCatView = catv;
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
	connect(m_button, SIGNAL(clicked()), &optdelegate, SLOT(sync()));

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
*/

void ResetWidget::slotClicked()
{
	//emit resetProperty(m_property);
	
//	emit commitData(w);
}

OptionDelegate::OptionDelegate(QObject *parent)
: QItemDelegate(parent)
{
	isReset = false;
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
		if (optUtils->isChanged(index))
			option.font.setBold(true);

		//render option value
		if (index.column() == 1)
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
	if (index.column() == 0 || optUtils->isTitle(index))
		return 0;

	ResetWidget *resetWidget = new ResetWidget(parent);

	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (optType)
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

	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	switch (optType)
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

// override setModelData()
void OptionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
								  const QModelIndex &index) const
{
	if (index.column() == 0)
		return;

	QVariant dispValue;
	ResetWidget *resetWidget = static_cast<ResetWidget*>(editor);
	int optType = optUtils->getField(index, USERROLE_TYPE).toInt();
	int optLevel = index.model()->objectName().remove("optModel").toInt();
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	MameOption *pMameOpt = mameOpts[optName];

	// update control's display value
	
	if (isReset)
	{
		switch (optLevel)
		{
		case OPTLEVEL_GLOBAL:
			dispValue = pMameOpt->defvalue;
			break;
				
		case OPTLEVEL_SRC:
			dispValue = pMameOpt->globalvalue;
			break;
		
		case OPTLEVEL_BIOS:
			dispValue = pMameOpt->srcvalue;
			break;
		
		case OPTLEVEL_CLONEOF:
			dispValue = pMameOpt->biosvalue;
			break;
		
		case OPTLEVEL_CURR:
		default:
			dispValue = pMameOpt->cloneofvalue;
		}

		dispValue = optUtils->getLongValue(optName, dispValue.toString());
	}
	else
	{
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
	}

	model->setData(index, dispValue);

	// process inherited value overrides
	QString prevVal;
	QAbstractItemModel *overrideModel;
	QModelIndex overrideIndex;
	QString iniFileName;
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[currentGame];
	
	switch (optLevel)
	{
	case OPTLEVEL_GLOBAL:
		iniFileName = "mame.ini";
		
		prevVal = pMameOpt->globalvalue;
		
		pMameOpt->globalvalue = optUtils->getShortValue(optName, dispValue.toString());

		// special case for console dirs
		if (pMameOpt->guivisible && optName.contains("_extra_software"))
			mameOpts[optName]->globalvalue = pMameOpt->globalvalue;

		overrideModel = dlgOptions->treeGlobalOpt->model();
		overrideIndex = overrideModel->index(index.row(), 1);
		overrideModel->setData(overrideIndex, dispValue);
		// fall to next case

	case OPTLEVEL_SRC:
		if (iniFileName.isNull())
			iniFileName = "ini/source/" + gameInfo->sourcefile.remove(".c") + ".ini";
		
		// prevent overwrite prevVal from prev case
		if (optLevel == OPTLEVEL_SRC)
			prevVal = pMameOpt->srcvalue;

		if (pMameOpt->srcvalue == prevVal)
		{
			pMameOpt->srcvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = dlgOptions->treeSourceOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
			// fall to next case
		}
		else
			break;

	case OPTLEVEL_BIOS:
		if (iniFileName.isNull())
			iniFileName = "ini/" + gameInfo->biosof() + ".ini";
		
		if (optLevel == OPTLEVEL_BIOS)
			prevVal = pMameOpt->biosvalue;

		if (pMameOpt->biosvalue == prevVal)
		{
			pMameOpt->biosvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = dlgOptions->treeBiosOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
		}
		else
			break;

	case OPTLEVEL_CLONEOF:
		if (iniFileName.isNull())
			iniFileName = "ini/" + gameInfo->cloneof + ".ini";
		
		if (optLevel == OPTLEVEL_CLONEOF)
			prevVal = pMameOpt->cloneofvalue;

		if (pMameOpt->cloneofvalue == prevVal)
		{
			pMameOpt->cloneofvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = dlgOptions->treeCloneofOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
		}
		else
			break;

	case OPTLEVEL_CURR:
		if (iniFileName.isNull())
		{
			// special case for consoles
			if (gameInfo->isExtRom)
				iniFileName = "ini/" + gameInfo->romof + ".ini";
			else
				iniFileName = "ini/" + currentGame + ".ini";
		}

		if (optLevel == OPTLEVEL_CURR)
			prevVal = pMameOpt->currvalue;
	
		if (pMameOpt->currvalue == prevVal)
		{
			pMameOpt->currvalue = optUtils->getShortValue(optName, dispValue.toString());
		
			overrideModel = dlgOptions->treeCurrOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
		}
		break;
	}

	optUtils->save(optLevel, iniFileName);
}

void OptionDelegate::updateEditorGeometry(QWidget *editor,
										  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

void OptionDelegate::sync()
{
    ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
    if (w == 0)
        return;
	isReset = true;
	emit commitData(w);
    emit closeEditor(w, QAbstractItemDelegate::EditNextItem);
	isReset = false;
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

			// add GUI options here
			bool guivisible = attributes.value("guivisible") == "1";
			if (guivisible)
			{
				MameOption *pMameOpt = new MameOption(0);	//fixme parent
				pMameOpt->defvalue = attributes.value("default");
				mameOpts[optName] = pMameOpt;
			}

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

				pMameOpt->guivisible = guivisible;
				pMameOpt->globalvisible = attributes.value("globalvisible") != "0";
				pMameOpt->srcvisible = attributes.value("srcvisible") != "0";
				pMameOpt->biosvisible = attributes.value("biosvisible") != "0";
				pMameOpt->cloneofvisible = attributes.value("cloneofvisible") != "0";
				pMameOpt->gamevisible = attributes.value("gamevisible") != "0";

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
	guivisible = false;
	globalvisible = srcvisible = biosvisible = cloneofvisible = gamevisible = true;
	//	win->log("# MameOption()");
}


OptionUtils::OptionUtils(QObject *parent)
: QObject(parent)
{
	// fixme create ini/source
	QDir().mkpath("ini/source");
}

void OptionUtils::initOption()
{
	//assign unique ctlrs for each level of options
	optInfos = (QList<OptInfo *>() 
			<< new OptInfo(0, 0, dlgOptions)
			<< new OptInfo(dlgOptions->lvGlobalOpt, dlgOptions->treeGlobalOpt, dlgOptions)
			<< new OptInfo(dlgOptions->lvSourceOpt, dlgOptions->treeSourceOpt, dlgOptions)
			<< new OptInfo(dlgOptions->lvBiosOpt, dlgOptions->treeBiosOpt, dlgOptions)
			<< new OptInfo(dlgOptions->lvCloneofOpt, dlgOptions->treeCloneofOpt, dlgOptions)
			<< new OptInfo(dlgOptions->lvCurrOpt, dlgOptions->treeCurrOpt, dlgOptions));

	//init category list ctlrs for each level of options
	for (int optLevel = OPTLEVEL_GLOBAL; optLevel < OPTLEVEL_LAST; optLevel++)
	{
		QListWidget *lstCatView = optInfos[optLevel]->lstCatView;

		if (optLevel == OPTLEVEL_GLOBAL)
			lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/folder.png"), tr("Directory"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-display.png"), tr("Video"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/audio-x-generic.png"), tr("Audio"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/input-gaming.png"), tr("Control"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/applications-system.png"), tr("Vector"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/applications-system.png"), tr("Misc"), lstCatView));

//		lstCatView->setViewMode(QListView::IconMode);
		lstCatView->setIconSize(QSize(24, 24));
//		lstCatView->setFlow(QListView::TopToBottom);
//		lstCatView->setMovement(QListView::Static);
		lstCatView->setMaximumWidth(100);
		lstCatView->setSpacing(4);

		connect(optInfos[optLevel]->optView->header(), SIGNAL(sectionResized(int, int, int)), 
			this, SLOT(updateHeaderSize(int, int, int)));
	}
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
			return optname;

		case USERROLE_TYPE:
			return mameOpts[optname]->type;

		case USERROLE_MIN:
			return mameOpts[optname]->min;

		case USERROLE_MAX:
			return mameOpts[optname]->max;

		case USERROLE_DEFAULT:
			return mameOpts[optname]->defvalue;

		case USERROLE_VALLIST:
			return QVariant(mameOpts[optname]->values);

		case USERROLE_GUIVALLIST:
			return QVariant(mameOpts[optname]->guivalues);
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

	// return a different color if value is not default
	if (dispVal == optUtils->getLongValue(optName, pMameOpt->defvalue))
		return Qt::transparent;
	else
		return QColor(255, 255, 0, 64);
}

bool OptionUtils::isChanged(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	int optLevel = index.model()->objectName().remove("optModel").toInt();
	QString optName = optUtils->getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();
	
	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];
	switch (optLevel)
	{
	case OPTLEVEL_GLOBAL:
		{
			compVal = pMameOpt->defvalue;
			break;
		}
	case OPTLEVEL_SRC:
		{
			compVal = pMameOpt->globalvalue;
			break;
		}
	case OPTLEVEL_BIOS:
		{
			compVal = pMameOpt->srcvalue;
			break;
		}
	case OPTLEVEL_CLONEOF:
		{
			compVal = pMameOpt->biosvalue;
			break;
		}
	case OPTLEVEL_CURR:
		{
			compVal = pMameOpt->cloneofvalue;
			break;
		}
	}

	compVal = optUtils->getLongValue(optName, compVal);

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

void OptionUtils::loadDefault(QString text)
{
	win->log("loadDefault()");	

//the following is for Qt translation
#if 0
	QStringList optList = (QStringList()
		<< QT_TR_NOOP("background directory")
		<< QT_TR_NOOP("cabinet directory")
		<< QT_TR_NOOP("control panel directory")
		<< QT_TR_NOOP("flyer directory")
		<< QT_TR_NOOP("icons directory")
		<< QT_TR_NOOP("marquee directory")
		<< QT_TR_NOOP("pcb directory")
		<< QT_TR_NOOP("title directory")
			
		<< QT_TR_NOOP("mame binary")
		
		<< QT_TR_NOOP("driver config")
		
		<< QT_TR_NOOP("readconfig")
			
		<< QT_TR_NOOP("rom directory")
		<< QT_TR_NOOP("hashpath")
		<< QT_TR_NOOP("samplepath")
		<< QT_TR_NOOP("artpath")
		<< QT_TR_NOOP("ctrlrpath")
		<< QT_TR_NOOP("inipath")
		<< QT_TR_NOOP("fontpath")
		<< QT_TR_NOOP("translation directory")
		<< QT_TR_NOOP("localized directory")
		<< QT_TR_NOOP("ips directory")

		<< QT_TR_NOOP("cfg directory")
		<< QT_TR_NOOP("nvram directory")
		<< QT_TR_NOOP("memcard directory")
		<< QT_TR_NOOP("input directory")
		<< QT_TR_NOOP("state directory")
		<< QT_TR_NOOP("snapshot directory")
		<< QT_TR_NOOP("diff directory")
		<< QT_TR_NOOP("comment directory")
		<< QT_TR_NOOP("hiscore directory")

		<< QT_TR_NOOP("cheat file")
		<< QT_TR_NOOP("history file")
		<< QT_TR_NOOP("story file")
		<< QT_TR_NOOP("mameinfo file")
		<< QT_TR_NOOP("command file")
		<< QT_TR_NOOP("hiscore file")

		<< QT_TR_NOOP("auto restore and save")
		
		<< QT_TR_NOOP("auto frame skipping")
		<< QT_TR_NOOP("frame skipping")
		<< QT_TR_NOOP("seconds to run")
		<< QT_TR_NOOP("throttle")
		<< QT_TR_NOOP("sleep when possible")
		<< QT_TR_NOOP("gameplay speed")
		<< QT_TR_NOOP("auto refresh speed")
			
		<< QT_TR_NOOP("rotate")
		<< QT_TR_NOOP("rotate clockwise")
		<< QT_TR_NOOP("rotate anti-clockwise")
		<< QT_TR_NOOP("auto rotate clockwise")
		<< QT_TR_NOOP("auto rotate anti-clockwise")
		<< QT_TR_NOOP("flip screen left-right")
		<< QT_TR_NOOP("flip screen upside-down")
		
		<< QT_TR_NOOP("crop artwork")
		<< QT_TR_NOOP("use backdrops")
		<< QT_TR_NOOP("use overlays")
		<< QT_TR_NOOP("use bezels")
		
		<< QT_TR_NOOP("brightness correction")
		<< QT_TR_NOOP("contrast correction")
		<< QT_TR_NOOP("gamma correction")
		<< QT_TR_NOOP("pause brightness")
		<< QT_TR_NOOP("image enhancement")
			
		<< QT_TR_NOOP("draw antialiased vectors")
		<< QT_TR_NOOP("beam width")
		<< QT_TR_NOOP("flicker")
			
		<< QT_TR_NOOP("enable sound and sound cpus")
		<< QT_TR_NOOP("sample rate")
		
		<< QT_TR_NOOP("use samples")
		<< QT_TR_NOOP("volume attenuation")
		<< QT_TR_NOOP("use volume auto adjust")
			
		<< QT_TR_NOOP("default input layout")
		<< QT_TR_NOOP("enable mouse input")
		<< QT_TR_NOOP("enable joystick input")
		<< QT_TR_NOOP("enable lightgun input")
		<< QT_TR_NOOP("enable multiple keyboards")
		<< QT_TR_NOOP("enable multiple mice")
		<< QT_TR_NOOP("enable steadykey support")
		<< QT_TR_NOOP("offscreen shots reload")
		<< QT_TR_NOOP("joystick map")
		<< QT_TR_NOOP("joystick deadzone")
		<< QT_TR_NOOP("joystick saturation")
		
		<< QT_TR_NOOP("paddle device")
		
		<< QT_TR_NOOP("adstick device")
		
		<< QT_TR_NOOP("pedal device")
		
		<< QT_TR_NOOP("dial device")
		
		<< QT_TR_NOOP("trackball device")
		
		<< QT_TR_NOOP("lightgun device")
		
		<< QT_TR_NOOP("positional device")
		
		<< QT_TR_NOOP("mouse device")
		
		<< QT_TR_NOOP("log")
		<< QT_TR_NOOP("verbose")
		<< QT_TR_NOOP("update in pause")
		
		<< QT_TR_NOOP("bios")
		<< QT_TR_NOOP("enable game cheats")
		<< QT_TR_NOOP("skip game info")
		<< QT_TR_NOOP("ips")
		<< QT_TR_NOOP("quit game with confirmation")
		<< QT_TR_NOOP("auto pause when playback is finished")
		<< QT_TR_NOOP("m68k core")
		
		<< QT_TR_NOOP("transparent in-game ui")
		<< QT_TR_NOOP("in-game ui transparency")
		
		<< QT_TR_NOOP("font blank")
		<< QT_TR_NOOP("font normal")
		<< QT_TR_NOOP("font special")
		<< QT_TR_NOOP("system background")
		<< QT_TR_NOOP("button red")
		<< QT_TR_NOOP("button yellow")
		<< QT_TR_NOOP("button green")
		<< QT_TR_NOOP("button blue")
		<< QT_TR_NOOP("button purple")
		<< QT_TR_NOOP("button pink")
		<< QT_TR_NOOP("button aqua")
		<< QT_TR_NOOP("button silver")
		<< QT_TR_NOOP("button navy")
		<< QT_TR_NOOP("button lime")
		<< QT_TR_NOOP("cursor")
		
		<< QT_TR_NOOP("language")
		<< QT_TR_NOOP("use lang list")
		
		<< QT_TR_NOOP("oslog")
		<< QT_TR_NOOP("watchdog")
			
		<< QT_TR_NOOP("thread priority")
		<< QT_TR_NOOP("enable multi-threading")
			
		<< QT_TR_NOOP("video output method")
		
		<< QT_TR_NOOP("number of screens to create")
		<< QT_TR_NOOP("run in a window")
		<< QT_TR_NOOP("start out maximized")
		<< QT_TR_NOOP("enforce aspect ratio")
		<< QT_TR_NOOP("scale screen")
		<< QT_TR_NOOP("visual effects")
		<< QT_TR_NOOP("wait for vertical sync")
		<< QT_TR_NOOP("sync to monitor refresh")
			
		<< QT_TR_NOOP("hardware stretching")
			
		<< QT_TR_NOOP("d3d version")
		<< QT_TR_NOOP("bilinear filtering")
			
		<< QT_TR_NOOP("screen")
		
		<< QT_TR_NOOP("aspect")
		<< QT_TR_NOOP("resolution")
		<< QT_TR_NOOP("view")
		
		<< QT_TR_NOOP("screen0")
		<< QT_TR_NOOP("aspect0")
		<< QT_TR_NOOP("resolution0")
		<< QT_TR_NOOP("view0")
		
		<< QT_TR_NOOP("screen1")
		<< QT_TR_NOOP("aspect1")
		<< QT_TR_NOOP("resolution1")
		<< QT_TR_NOOP("view1")
		
		<< QT_TR_NOOP("screen2")
		<< QT_TR_NOOP("aspect2")
		<< QT_TR_NOOP("resolution2")
		<< QT_TR_NOOP("view2")
		
		<< QT_TR_NOOP("screen3")
		<< QT_TR_NOOP("aspect3")
		<< QT_TR_NOOP("resolution3")
		<< QT_TR_NOOP("view3")
			
		<< QT_TR_NOOP("triple buffering")
		<< QT_TR_NOOP("switch resolutions to fit")
		<< QT_TR_NOOP("full screen brightness")
		<< QT_TR_NOOP("full screen contrast")
		<< QT_TR_NOOP("full screen gamma")
			
		<< QT_TR_NOOP("audio latency")
			
		<< QT_TR_NOOP("dual lightgun")
		<< QT_TR_NOOP("joyid1")
		<< QT_TR_NOOP("joyid2")
		<< QT_TR_NOOP("joyid3")
		<< QT_TR_NOOP("joyid4")
		<< QT_TR_NOOP("joyid5")
		<< QT_TR_NOOP("joyid6")
		<< QT_TR_NOOP("joyid7")
		<< QT_TR_NOOP("joyid8")
			
		<< QT_TR_NOOP("ramsize")
		<< QT_TR_NOOP("writeconfig")
		<< QT_TR_NOOP("skip warnings")
		<< QT_TR_NOOP("newui")
		<< QT_TR_NOOP("natural")
	);
#endif
	// underscore separated category list, prefixed by a sorting number
	const QStringList optCatList = (QStringList()
			<< "00_Global Misc_00_" + QString(QT_TR_NOOP("core configuration"))
			<< "00_Global Misc_01_" + QString(QT_TR_NOOP("core palette"))
			<< "00_Global Misc_02_" + QString(QT_TR_NOOP("core language"))
			
			<< "01_Directory_00_" + QString(QT_TR_NOOP("core search path"))
			<< "01_Directory_01_" + QString(QT_TR_NOOP("core output directory"))
			<< "01_Directory_02_" + QString(QT_TR_NOOP("core filename"))
			
			<< "02_Video_02_" + QString(QT_TR_NOOP("core rotation"))
			<< "02_Video_03_" + QString(QT_TR_NOOP("core screen"))
			<< "02_Video_04_" + QString(QT_TR_NOOP("full screen"))
			<< "02_Video_05_" + QString(QT_TR_NOOP("Windows video"))
			<< "02_Video_06_" + QString(QT_TR_NOOP("DirectDraw-specific"))
			<< "02_Video_07_" + QString(QT_TR_NOOP("Direct3D-specific"))
			<< "02_Video_08_" + QString(QT_TR_NOOP("core performance"))
			<< "02_Video_09_" + QString(QT_TR_NOOP("Windows performance"))
			<< "02_Video_10_" + QString(QT_TR_NOOP("per-window video"))
			
			<< "03_Audio_00_" + QString(QT_TR_NOOP("core sound"))
			<< "03_Audio_01_" + QString(QT_TR_NOOP("Windows sound"))
			
			<< "04_Control_00_" + QString(QT_TR_NOOP("core input"))
			<< "04_Control_01_" + QString(QT_TR_NOOP("core input automatic enable"))
			<< "04_Control_02_" + QString(QT_TR_NOOP("input device"))
			
			<< "05_Vector_00_" + QString(QT_TR_NOOP("core vector"))
			
			<< "06_Misc_00_" + QString(QT_TR_NOOP("core misc"))
			<< "06_Misc_01_" + QString(QT_TR_NOOP("core artwork"))
			<< "06_Misc_02_" + QString(QT_TR_NOOP("core state/playback"))
			<< "06_Misc_03_" + QString(QT_TR_NOOP("MESS specific"))
			<< "06_Misc_04_" + QString(QT_TR_NOOP("Windows MESS specific"))
			<< "06_Misc_05_" + QString(QT_TR_NOOP("core debugging"))
			<< "06_Misc_06_" + QString(QT_TR_NOOP("Windows debugging"))
			);

	QString line, line0;
	QTextStream in(&text);
	in.setCodec("UTF-8");
	QString optHeader = "ERROR_MAGIC";

	do
	{
		line0 = line = in.readLine().trimmed();

		//hack: psx plugins will not be supported
		QString c0("pu_");
		QString c1("g");
		if (line.startsWith(c1 + c0))
		{
		//	crash the GUI
			delete mamegame;
			break;
		}

		//init option headers
		if (line.startsWith("#"))
		{
			if (line.size() > 2)
			{
				line.remove("#");
				line.remove("OPTIONS");
				line = line.trimmed();

				// assign the header from known category list
				int c = optCatList.indexOf (QRegExp(".*" + line + ".*", Qt::CaseInsensitive));
				if (c > 0)
					optHeader = optCatList[c];
				// assign the header to Misc
				else
					optHeader = "06_Misc_99_" + line;
				//we should have a valid optHeader by now
			}
			//else # line is ignored
		}
		// ignore <UNADORNED>
		else if (!line.startsWith("<"))
		{
			//get the option name
			line.replace(QRegExp("(\\w+)\\s+.*"), "\\1");
			if (line.size() > 0)
			{
				//add option name to category map
				optCatMap[optHeader] << line;

				QStringList list = line0.split(QRegExp("\\s+"));
				MameOption *pMameOpt = new MameOption(0);	//fixme parent
				//option has a value from ini
				if (list.count() == 2)
				{
					pMameOpt->defvalue = list[1];
					//assign value to global mameOpts's default
					mameOpts[list[0]] = pMameOpt;
				}
				//option has empty value
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

//open option template file
void OptionUtils::loadTemplate()
{
	QFile file(":/res/optiontemplate.xml");

	QXmlInputSource xmlInputSource(&file);
	OptionXMLHandler handler(0);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);
	reader.parse(xmlInputSource);
}

// update mameOpts from ini, must be called in a proper order
void OptionUtils::loadIni(int optLevel, const QString &iniFileName)
{
	//get opti values from ini
	QHash<QString, QString> inisettings = optUtils->readIniFile(iniFileName);
	
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];
		
		//take ini override when available
		if (inisettings.contains(optName))
		{
			// apply to current value
			pMameOpt->currvalue = inisettings[optName];

			// apply to level value
			switch (optLevel)
			{
			case OPTLEVEL_GLOBAL:
				pMameOpt->globalvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_SRC:
				pMameOpt->srcvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_BIOS:
				pMameOpt->biosvalue = pMameOpt->currvalue;
				break;

			case OPTLEVEL_CLONEOF:
				pMameOpt->cloneofvalue = pMameOpt->currvalue;
				break;

			}
		}
		//inherit from higher level value, apply to current and level value
		else
		{
			switch (optLevel)
			{
			case OPTLEVEL_GLOBAL:
				pMameOpt->currvalue = pMameOpt->globalvalue = pMameOpt->defvalue;
				break;

			case OPTLEVEL_SRC:
				pMameOpt->currvalue = pMameOpt->srcvalue = pMameOpt->globalvalue;
				break;

			case OPTLEVEL_BIOS:
				pMameOpt->currvalue = pMameOpt->biosvalue = pMameOpt->srcvalue;
				break;

			case OPTLEVEL_CLONEOF:
				pMameOpt->currvalue = pMameOpt->cloneofvalue= pMameOpt->biosvalue;
				break;

			case OPTLEVEL_CURR:
				pMameOpt->currvalue = pMameOpt->cloneofvalue;
				break;
			}
		}
	}
}

void OptionUtils::save(int optLevel, const QString &iniFileName)
{
	//save MAME Options
	QFile outFile(iniFileName);
	QString line;
	QStringList headers;
	QString mameIni;

	if (outFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream in(&mamegame->mameDefaultIni);
		QTextStream outBuf(&mameIni);
		QTextStream out(&outFile);
		in.setCodec("UTF-8");
		outBuf.setCodec("UTF-8");
		out.setCodec("UTF-8");
		bool isHeader = false, isChanged = false;
		QString optName;

		do
		{
			QString currVal, defVal;

			line = in.readLine();

			// process # headers
			if (line.startsWith("#"))
			{
				headers << line;
				isHeader = true;
			}
			// process empty lines
			else if (line.isEmpty())
				outBuf << endl;
			// process option entry
			else
			{
				int sep = line.indexOf(QRegExp("\\s+"));
				optName = line.left(sep);

				MameOption *pMameOpt = mameOpts[optName];

				switch (optLevel)
				{
				case OPTLEVEL_GLOBAL:
					currVal = pMameOpt->globalvalue;
					defVal = pMameOpt->defvalue;
					break;
				
				case OPTLEVEL_SRC:
					currVal = pMameOpt->srcvalue;
					defVal = pMameOpt->globalvalue;
					break;
				
				case OPTLEVEL_BIOS:
					currVal = pMameOpt->biosvalue;
					defVal = pMameOpt->srcvalue;
					break;
				
				case OPTLEVEL_CLONEOF:
					currVal = pMameOpt->cloneofvalue;
					defVal = pMameOpt->biosvalue;
					break;
				
				case OPTLEVEL_CURR:
				default:
					currVal = pMameOpt->currvalue;
					defVal = pMameOpt->cloneofvalue;
				}

				if (optUtils->getLongValue(optName, currVal) == 
					optUtils->getLongValue(optName, defVal))
					isChanged = false;
				else
					isChanged = true;

				isHeader = false;
			}

			if (!isHeader && !line.isEmpty())
			{
				// write out header and clear header buffer
				if (!headers.isEmpty())
				{
					foreach (QString header, headers)
						outBuf << header << endl;
					headers.clear();
				}

				// write option entry
				if (isChanged || optLevel == OPTLEVEL_GLOBAL)
				{
//					win->log(QString("curr: %1, def: %2").arg(currVal).arg(defVal));
					outBuf.setFieldWidth(26);
					outBuf.setFieldAlignment(QTextStream::AlignLeft);
					outBuf << optName;
					outBuf.setFieldWidth(0);
					//quote value if needed
					if (currVal.indexOf(QRegExp("\\s")) > 0)
						outBuf << "\"" << currVal << "\"" << endl;
					else
						outBuf << currVal << endl;
				}
			}
		}
		while (!line.isNull());

		/* postprocess */		
		// read in reverse order to eat empty headers
		QStringList bufs = mameIni.split(QRegExp("[\\r\\n]+"));
		for (int i = bufs.count() - 1; i >= 0; i --)
		{
			static int c = 0;
			line = bufs[i];
			if (line.startsWith("#"))
				c --;
			else if (!line.isEmpty())
				c = 3;	// each header has a maximum of 3 '#' lines

			if (c < 0)
				bufs.removeAt(i); 
		}

		// make an empty line between sections
		foreach (QString buf, bufs)
		{
			static bool isEntry = false;

			if (buf.startsWith("#"))
			{
				if (isEntry)
					// add it when prev line is an entry and curr line is a header
					out << endl;

				isEntry = false;
			}
			else
				isEntry = true;

			out << buf << endl;
		}
	}

	// delete ini if all options are default
	if (outFile.size() == 0)
		outFile.remove();
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
				int sep = line.indexOf(QRegExp("\\s"));

				if (sep != -1)
				{
					QString key = line.left(sep);
					QString value = line.right(line.size() - sep).trimmed();
					//replace escape
					value.replace("\\", "\\\\");
					//wrap with "
					if (!value.startsWith("\""))
						value = "\"" + value + "\"";

					out << key << "=" << value << endl;
				}
				else
					// empty entry
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

//	win->poplog(outFile.fileName());
	return settings;
}

// determine what to update in the option dialog
void OptionUtils::updateModel(QListWidgetItem *currItem, int optLevel)
{
	//fixme: test how many times it's called. 
	//win->log(QString("updateModel: %1 %2").arg(currItem == 0).arg(optLevel));

	/* figure out which option category are we in list ctlr */
	//get optLevel by selected tab
	if (optLevel == -1)
		optLevel = dlgOptions->tabOptions->currentIndex();

	QString optCat;
	//fixme: hack index: index 0 is GUI, must skip optLevel 0, otherwise optCtrls[0] will crash
	if (optLevel > 0)
	{
		//assign current option category if not assigned
		if (!currItem)
			currItem = optCtrls[optLevel]->currentItem();

		if (currItem)
			optCat = currItem->text();
		//assign default option category if none selected
		else
			optCat = tr("Video");
	}

	/* update mameopts */
	GameInfo *gameInfo = mamegame->gamenameGameInfoMap[currentGame];
	QString iniString;
	QString STR_OPTS_ = tr("Options") + " - ";

	//global has been loaded before, so loadIni() is not needed
	if (optLevel == OPTLEVEL_GLOBAL)
	{
		updateModelData(optCat, OPTLEVEL_GLOBAL);
		dlgOptions->setWindowTitle(STR_OPTS_ + QString("Global"));
		return;
	}

	//source
	iniString = gameInfo->sourcefile.remove(".c");
	loadIni(OPTLEVEL_SRC, "ini/source/" + iniString + ".ini");
	if (optLevel == OPTLEVEL_SRC)
	{
		updateModelData(optCat, OPTLEVEL_SRC);
		dlgOptions->setWindowTitle(STR_OPTS_ + iniString + ".c");
		return;
	}

	//bios
	iniString = gameInfo->biosof();
	if (iniString.isEmpty())
		dlgOptions->tabOptions->widget(OPTLEVEL_BIOS)->setEnabled(false);
	else
		dlgOptions->tabOptions->widget(OPTLEVEL_BIOS)->setEnabled(true);

	loadIni(OPTLEVEL_BIOS, "ini/" + iniString + ".ini");
	if (optLevel == OPTLEVEL_BIOS)
	{
		updateModelData(optCat, OPTLEVEL_BIOS);
		dlgOptions->setWindowTitle(STR_OPTS_ + iniString);
		return;
	}

	//cloneof
	iniString = gameInfo->cloneof;
	if (iniString.isEmpty())
		dlgOptions->tabOptions->widget(OPTLEVEL_CLONEOF)->setEnabled(false);
	else
		dlgOptions->tabOptions->widget(OPTLEVEL_CLONEOF)->setEnabled(true);

	optUtils->loadIni(OPTLEVEL_CLONEOF, "ini/" + iniString + ".ini");
	if (optLevel == OPTLEVEL_CLONEOF)
	{
		updateModelData(optCat, OPTLEVEL_CLONEOF);
		dlgOptions->setWindowTitle(STR_OPTS_ + iniString);
		return;
	}

	//current game
	//special case for consoles
	if (gameInfo->isExtRom)
		iniString = gameInfo->romof;
	else
		iniString = currentGame;

	optUtils->loadIni(OPTLEVEL_CURR, "ini/" + iniString + ".ini");
	if (optLevel == OPTLEVEL_CURR)
	{
		updateModelData(optCat, OPTLEVEL_CURR);
		dlgOptions->setWindowTitle(STR_OPTS_ + iniString);
		return;
	}
}

//update values in the option dialog
void OptionUtils::updateModelData(QString optCat, int optLevel)
{
	QStandardItemModel *optModel0, *optModel;
	QTreeView *optView = optInfos[optLevel]->optView;
	bool guiHasAdded = false;

	//init option listview
	//hack: preserve model and delete it later, so that scroll pos is kept
	optModel0 = optInfos[optLevel]->optModel;

	//fixme: setup columns, not needed every time?
	optModel = optInfos[optLevel]->optModel = new QStandardItemModel(win);
	optModel->setObjectName(QString("optModel%1").arg(optLevel));
	optModel->setColumnCount(2);
	optModel->setHeaderData(0, Qt::Horizontal, tr("Option"));
	optModel->setHeaderData(1, Qt::Horizontal, tr("Value"));

	optView->setModel(optModel);
	optView->header()->restoreState(option_column_state);

	foreach (QString optHeader, optCatMap.keys())
	{
		QStringList optNames = optCatMap[optHeader];
		QStringList optHeaderStrs = optHeader.split('_');
		// if optHeader's category is in the current view
		if (optCat == tr(qPrintable(optHeaderStrs[1])))
		{
			//add section title
			addModelItemTitle(optModel, optHeaderStrs.last());

			foreach (QString optName, optNames)
			{
				MameOption *pMameOpt = mameOpts[optName];

				//filter non-applicable options
				if (pMameOpt == NULL ||
					optLevel == OPTLEVEL_GLOBAL && !pMameOpt->globalvisible ||
					optLevel == OPTLEVEL_SRC && !pMameOpt->srcvisible ||
					optLevel == OPTLEVEL_BIOS && !pMameOpt->biosvisible ||
					optLevel == OPTLEVEL_CLONEOF && !pMameOpt->cloneofvisible ||
					optLevel == OPTLEVEL_CURR && !pMameOpt->gamevisible
					)
					continue;

				//add option items		
				addModelItem(optModel, optName);
			}

			//fixme: move to GUI directory
			if (!guiHasAdded && optLevel == OPTLEVEL_GLOBAL && optCat == tr("Directory"))
			{
				//add GUI options
				addModelItemTitle(optModel, "GUI DIRECTORY");
			
				foreach (QString optName, mameOpts.keys())
				{
					MameOption *pMameOpt = mameOpts[optName];

					if (!pMameOpt->guivisible || optName.contains("_extra_software"))
						continue;

					addModelItem(optModel, optName);
				}

				addModelItemTitle(optModel, "MESS SOFTWARE DIRECTORY");

				//add console dir options
				foreach (QString consoleName, consoleGamesL)
				{
					QString optName = consoleName + "_extra_software";
					MameOption *pMameOpt = mameOpts[optName];

					if (!pMameOpt->guivisible)
						continue;

					addModelItem(optModel, optName);
				}

				guiHasAdded = true;
			}
		}
	}

	if (optModel0)
		delete optModel0;

	optView->setItemDelegate(&optdelegate);

	optView->resizeColumnToContents(1);
}

void OptionUtils::addModelItemTitle(QStandardItemModel *optModel, QString title)
{
	//fill title
	QStandardItem *item = new QStandardItem(tr(qPrintable(title)));
	item->setData("OPTIONTITLE", Qt::UserRole + USERROLE_TITLE);
	optModel->appendRow(item);
}

void OptionUtils::addModelItem(QStandardItemModel *optModel, QString optName)
{
	MameOption *pMameOpt = mameOpts[optName];

	// prepare GUI value desc
	for (int i = 0; i < pMameOpt->guivalues.size(); i++)
	{
		if (pMameOpt->guivalues[i].isEmpty())
			pMameOpt->guivalues[i] = utils->capitalizeStr(pMameOpt->values[i]);
	}

//	QString tooltip = "[" + optName + "] " + pMameOpt->description;

	// fill key
	QStandardItem *itemKey = new QStandardItem(tr(qPrintable(getLongName(optName).toLower())));
	itemKey->setData(optName, Qt::UserRole + USERROLE_KEY);
//	itemKey->setData(tooltip, Qt::ToolTipRole);
	// fill value
	QStandardItem *itemVal = new QStandardItem(getLongValue(optName, pMameOpt->currvalue));
//	itemVal->setData(tooltip, Qt::ToolTipRole);

	optModel->appendRow(QList<QStandardItem *>() << itemKey << itemVal);
}

void OptionUtils::updateHeaderSize(int logicalIndex, int oldSize, int newSize)
{
	int optLevel = dlgOptions->tabOptions->currentIndex();
	option_column_state = optInfos[optLevel]->optView->header()->saveState();
	
//	win->log(QString("header: %1, %2").arg(optInfos[optLevel]->optView->header()->sectionSize(0)).arg(newSize));
}

