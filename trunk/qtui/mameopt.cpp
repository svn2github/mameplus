#include "mameopt.h"

#include "mamepguimain.h"
#include "dialogs.h"

#ifdef Q_OS_WIN
#include "SDL.h"
#undef main
#endif

/* global */
OptionUtils *optUtils;
//collection of all MameOptions
QHash<QString, MameOption*> mameOpts;
QByteArray option_column_state;
QString mameIniPath = "";

/* internal */
OptionDelegate optdelegate(win);

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
_textLabel(new QLabel(this)),
_iconLabel(new QLabel(this)),
_btnReset(new QToolButton(this)),
ctrlSpacing(-1),
optType(0),
sliderOffset(0)
{
	_textLabel->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed));
	_iconLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	_btnReset->setToolButtonStyle(Qt::ToolButtonIconOnly);
	_btnReset->setIcon(QIcon(":/res/reset_property.png"));
	_btnReset->setMaximumWidth(24);
	_btnReset->setMinimumWidth(24);
	_btnReset->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));
	_btnReset->setToolTip("Reset to default"); 
	connect(_btnReset, SIGNAL(clicked()), &optdelegate, SLOT(sync()));

	QLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(ctrlSpacing);
	layout->addWidget(_iconLabel);
	layout->addWidget(_textLabel);
	layout->addWidget(_btnReset);
	setFocusProxy(_textLabel);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
}

void ResetWidget::setSpacing(int spacing)
{
	ctrlSpacing = spacing;
	layout()->setSpacing(ctrlSpacing);
}

//insert some widgets before reset button
void ResetWidget::setWidget(QWidget *widget, QWidget *widget2, int optType, int sliderOffset)
{
	this->optType = optType;
	this->sliderOffset = sliderOffset;

	//delete the ctrls
	if (_textLabel) {
		delete _textLabel;
		_textLabel = 0;
	}
	if (_iconLabel) {
		delete _iconLabel;
		_iconLabel = 0;
	}
	delete layout();

	QLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);

	layout->addWidget(widget);
	subWidget = widget;
	
	if (widget2)
	{
		layout->addWidget(widget2);
		subWidget2 = widget2;

		switch (optType)
		{
		case MAMEOPT_TYPE_INT:
		case MAMEOPT_TYPE_FLOAT:
			_slider = static_cast<QSlider*>(subWidget);
			_sliderLabel = static_cast<QLabel*>(subWidget2);

			disconnect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderLabel(int)));
			connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(updateSliderLabel(int)));
			break;
		
		case MAMEOPT_TYPE_FILE:
		case MAMEOPT_TYPE_DATFILE:
		case MAMEOPT_TYPE_CFGFILE:
		case MAMEOPT_TYPE_EXEFILE:
		case MAMEOPT_TYPE_DIR:
		case MAMEOPT_TYPE_DIRS:
			_btnFileDlg = static_cast<QToolButton*>(subWidget2);
			_btnFileDlg->disconnect(SIGNAL(clicked()));
			if (optType == MAMEOPT_TYPE_DIRS)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setDirectories()));
			else if (optType == MAMEOPT_TYPE_DIR)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setDirectory()));
			else if (optType == MAMEOPT_TYPE_FILE)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setFile()));
			else if (optType == MAMEOPT_TYPE_DATFILE)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setDatFile()));
			else if (optType == MAMEOPT_TYPE_CFGFILE)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setCfgFile()));
			else if (optType == MAMEOPT_TYPE_EXEFILE)
				connect(_btnFileDlg, SIGNAL(clicked()), &optdelegate, SLOT(setExeFile()));
			break;
		}
	}
	layout->addWidget(_btnReset);

	setFocusProxy(widget);
}

void ResetWidget::setResetEnabled(bool enabled)
{
	_btnReset->setEnabled(enabled);
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

void ResetWidget::updateSliderLabel(int value)
{
	QLabel *ctrl2 = static_cast<QLabel*>(subWidget2);

	double multiplier = 1.0;
	QString format = "%.0f";
	if (optType == MAMEOPT_TYPE_FLOAT)
	{
		multiplier = 100.0;
		format = "%.2f";
	}

	ctrl2->setMaximumWidth(36);
	ctrl2->setMinimumWidth(36);
	ctrl2->setAlignment(Qt::AlignRight);
	ctrl2->setText(QString().sprintf(qPrintable(format), (value - sliderOffset) / multiplier));
}

OptionDelegate::OptionDelegate(QObject *parent)
: QItemDelegate(parent)
{
	isReset = false;
	pathBuf = "";
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

			case MAMEOPT_TYPE_INT:
			case MAMEOPT_TYPE_FLOAT:
			{
				double min, max, value;
				min = optUtils->getField(index, USERROLE_MIN).toDouble();
				max = optUtils->getField(index, USERROLE_MAX).toDouble();
				value = index.model()->data(index, Qt::EditRole).toDouble();

				QStyleOptionProgressBar ctrl;

				ctrl.state = QStyle::State_Enabled;
				ctrl.direction = QApplication::layoutDirection();

				QRect rc = option.rect;

				int sliderWidth = rc.width() - 36 - 24; /*reset button+label*/
				rc.setWidth(sliderWidth);
				rc.setY(rc.y() + (int)(rc.height() * 0.1));
				rc.setHeight((int)(rc.height() * 0.8));	//don't occupy full height

				int multiplier = 1;
				int offset = 0;
				QString format = "%.0f";
				
				if (optType == MAMEOPT_TYPE_FLOAT)
				{
					multiplier = 100;
					format = "%.2f";
				}
				else if (min < 0)
				{
					offset = (int)(0 - min);
					max += offset;
					value += offset;
					min = 0;
				}

				ctrl.rect = rc;
				ctrl.fontMetrics = QApplication::fontMetrics();
				ctrl.minimum = (int)(multiplier * min);
				ctrl.maximum = (int)(multiplier * max);
				ctrl.textAlignment = Qt::AlignRight;
				ctrl.textVisible = false;
				
				ctrl.progress = (int)(multiplier * value);
				ctrl.text = QString().sprintf(qPrintable(format), value - offset);

				// Draw the progress bar
				QApplication::style()->drawControl(QStyle::CE_ProgressBar, &ctrl, painter);

				rc = option.rect;
				rc.setWidth(rc.width() - 24);
				rc.setY(rc.y() + (int)(rc.height() * 0.1));
				rc.setHeight((int)(rc.height() * 0.8));	//don't occupy full height
				QApplication::style()->drawItemText(painter, rc, Qt::AlignRight | Qt::AlignVCenter, option.palette, true,
					ctrl.text, QPalette::Text);

				return;
			}
			}
		}
	}
	else
	{
//		option.palette.setColor(QPalette::Text, option.palette.color(QPalette::BrightText));
		option.palette.setColor(QPalette::Text, QColor(0, 21, 110, 255));
		option.font.setBold(true);
//		brush = option.palette.dark();
		brush = QBrush(QColor(221, 231, 238, 255));
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
	case MAMEOPT_TYPE_FLOAT:
	{
		double min, max;
		min = optUtils->getField(index, USERROLE_MIN).toDouble();
		max = optUtils->getField(index, USERROLE_MAX).toDouble();

		int multiplier = 1;
		int offset = 0;
		if (optType == MAMEOPT_TYPE_FLOAT)
			multiplier = 100;
		else if (min < 0)
		{
			offset = (int)(0 - min);
			max += offset;
			min = 0;
		}

		QSlider *ctrl = new QSlider(Qt::Horizontal, parent);
		ctrl->setStyleSheet("background-color: white;");//fixme: hack
		ctrl->setMinimum((int)(min * multiplier));
		ctrl->setMaximum((int)(max * multiplier));

		QLabel *ctrl2 = new QLabel(parent);
		ctrl2->setAlignment(Qt::AlignHCenter);
		ctrl2->setStyleSheet("background-color: white;");//fixme: hack

		resetWidget->setWidget(ctrl, ctrl2, optType, offset);
		return resetWidget;
	}

	case MAMEOPT_TYPE_FILE:
	case MAMEOPT_TYPE_DATFILE:
	case MAMEOPT_TYPE_CFGFILE:
	case MAMEOPT_TYPE_EXEFILE:
	case MAMEOPT_TYPE_DIR:
	case MAMEOPT_TYPE_DIRS:
	{
		QLineEdit *ctrl = new QLineEdit(resetWidget);

		QToolButton *ctrl2 = new QToolButton(resetWidget);
		ctrl2->setText("...");

		resetWidget->setWidget(ctrl, ctrl2, optType);
		return resetWidget;
	}

	case MAMEOPT_TYPE_STRING:
	case MAMEOPT_TYPE_STRING_EDITABLE:
	{
		QList<QVariant> guivalues = optUtils->getField(index, USERROLE_GUIVALLIST).toList();
		if (guivalues.size() > 0)
		{
			QComboBox *ctrl = new QComboBox(resetWidget);
			if (optType == MAMEOPT_TYPE_STRING_EDITABLE)
				ctrl->setEditable(true);
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
///*
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
	case MAMEOPT_TYPE_FLOAT:
	{
		double min, value;
		min = optUtils->getField(index, USERROLE_MIN).toDouble();
		value = index.model()->data(index, Qt::EditRole).toDouble();

		int multiplier = 1;
		int offset = 0;
		if (optType == MAMEOPT_TYPE_FLOAT)
			multiplier = 100;
		else if (min < 0)
		{
			offset = (int)(0 - min);
			value += offset;
		}

		QSlider *ctrl = static_cast<QSlider*>(resetWidget->subWidget);
		ctrl->setPageStep(2);
		int intVal = ((int)((value + 0.001) * multiplier));	//hack: precision fix
		ctrl->setValue(intVal);
//		win->log(QString("val: %1").arg(intVal));
		resetWidget->updateSliderLabel(intVal);
		break;
	}
	
	case MAMEOPT_TYPE_STRING:
	case MAMEOPT_TYPE_STRING_EDITABLE:
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

	//file, dir, dirs case also here
	default:
	{
		QString guivalue = index.model()->data(index, Qt::DisplayRole).toString();
		
		QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);
		ctrl->setText(guivalue);
//			QItemDelegate::setEditorData(editor, index);
	}
	}
//*/
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

	/* update control's display value */
	//reset to default value
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
	//set new value
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
		case MAMEOPT_TYPE_FLOAT:
		{
			double min, value;
			min = optUtils->getField(index, USERROLE_MIN).toDouble();
			value = index.model()->data(index, Qt::EditRole).toDouble();
			
			double multiplier = 1.0;
			int offset = 0;
			if (optType == MAMEOPT_TYPE_FLOAT)
				multiplier = 100.0;
			else if (min < 0)
			{
				offset = (int)(0 - min);
				value += offset;
			}
			
			QSlider *ctrl = static_cast<QSlider*>(resetWidget->subWidget);
			// ensure .00 display for MAMEOPT_TYPE_FLOAT

			dispValue = optUtils->getLongValue(optName, QString::number((ctrl->value() - offset) / multiplier));
			break;
		}
		case MAMEOPT_TYPE_STRING:
		case MAMEOPT_TYPE_STRING_EDITABLE:
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

			// set file dialog result
			if (!pathBuf.isEmpty())
				dispValue = pathBuf;
			else
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
	GameInfo *gameInfo = mameGame->games[currentGame];
	
	switch (optLevel)
	{
	case OPTLEVEL_GLOBAL:
		iniFileName = mameIniPath + "mame.ini";
		
		prevVal = pMameOpt->globalvalue;
		
		pMameOpt->globalvalue = optUtils->getShortValue(optName, dispValue.toString());

		// special case for console dirs
		if (pMameOpt->guivisible && optName.contains("_extra_software"))
			mameOpts[optName]->globalvalue = pMameOpt->globalvalue;

		overrideModel = optionsUI->treeGlobalOpt->model();
		overrideIndex = overrideModel->index(index.row(), 1);
		overrideModel->setData(overrideIndex, dispValue);
		// fall to next case

	case OPTLEVEL_SRC:
		if (iniFileName.isNull())
		{
			iniFileName = gameInfo->sourcefile;
			iniFileName.replace(".c", INI_EXT);
			iniFileName = mameIniPath + "ini/source/" + iniFileName;
		}
		
		// prevent overwrite prevVal from prev case
		if (optLevel == OPTLEVEL_SRC)
			prevVal = pMameOpt->srcvalue;

		if (pMameOpt->srcvalue == prevVal)
		{
			pMameOpt->srcvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = optionsUI->treeSourceOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
			// fall to next case
		}
		else
			break;

	case OPTLEVEL_BIOS:
		if (iniFileName.isNull())
			iniFileName = mameIniPath + "ini/" + gameInfo->biosof() + INI_EXT;
		
		if (optLevel == OPTLEVEL_BIOS)
			prevVal = pMameOpt->biosvalue;

		if (pMameOpt->biosvalue == prevVal)
		{
			pMameOpt->biosvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = optionsUI->treeBiosOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
		}
		else
			break;

	case OPTLEVEL_CLONEOF:
		if (iniFileName.isNull())
			iniFileName = mameIniPath + "ini/" + gameInfo->cloneof + INI_EXT;
		
		if (optLevel == OPTLEVEL_CLONEOF)
			prevVal = pMameOpt->cloneofvalue;

		if (pMameOpt->cloneofvalue == prevVal)
		{
			pMameOpt->cloneofvalue = optUtils->getShortValue(optName, dispValue.toString());

			overrideModel = optionsUI->treeCloneofOpt->model();
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
				iniFileName = mameIniPath + "ini/" + gameInfo->romof + INI_EXT;
			else
				iniFileName = mameIniPath + "ini/" + currentGame + INI_EXT;
		}

		if (optLevel == OPTLEVEL_CURR)
			prevVal = pMameOpt->currvalue;
	
		if (pMameOpt->currvalue == prevVal)
		{
			pMameOpt->currvalue = optUtils->getShortValue(optName, dispValue.toString());
		
			overrideModel = optionsUI->treeCurrOpt->model();
			overrideIndex = overrideModel->index(index.row(), 1);
			overrideModel->setData(overrideIndex, dispValue);
		}
		break;
	}

	//special case for screen, also updates resolution
	if (optName.startsWith("screen"))
	{
		QString optName2 = optName;
		optName2.replace("screen", "resolution");
		optUtils->updateSelectableItems(optName2);
	}

	optUtils->saveIniFile(optLevel, iniFileName);
	win->saveSettings();
}

void OptionDelegate::updateEditorGeometry(QWidget *editor,
										  const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

void OptionDelegate::sync()
{
	ResetWidget *w = qobject_cast<ResetWidget*>(sender()->parent());
	if (w == NULL)
		return;
	isReset = true;
	emit commitData(w);
	emit closeEditor(w, QAbstractItemDelegate::EditNextItem);
	isReset = false;
}

void OptionDelegate::setChangesAccepted()
{
	emit commitData(rWidget);
}

void OptionDelegate::setDirectories()
{
	rWidget = qobject_cast<ResetWidget*>(sender()->parent());
	if (rWidget == NULL)
		return;

	QLineEdit *ctrl = static_cast<QLineEdit*>(rWidget->subWidget);

	disconnect(dirsUI, SIGNAL(accepted()), this, SLOT(setDirectoriesAccepted()));
	connect(dirsUI, SIGNAL(accepted()), this, SLOT(setDirectoriesAccepted()));

	//take existing dir
	dirsUI->init(ctrl->text());
	dirsUI->exec();
}

void OptionDelegate::setDirectoriesAccepted()
{
	pathBuf = dirsUI->getDirs();
	emit commitData(rWidget);
	pathBuf.clear();
}

//todo: merge with setFile
void OptionDelegate::setDirectory()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	if (resetWidget == NULL)
		return;
	
	QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);

	//take existing dir
	QString initPath = ctrl->text();
	QFileInfo fi(initPath);
	if (!fi.exists())
	{
		//take mame_binary dir
		fi.setFile(mame_binary);
		initPath = fi.absolutePath();
	}

	QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
	QString directory = QFileDialog::getExistingDirectory(resetWidget,
								tr("Directory name:"),
								initPath,
								options);
	if (!directory.isEmpty())
	{
		pathBuf = directory;
		emit commitData(resetWidget);
		pathBuf.clear();
	}
}

void OptionDelegate::setFile(QString filter, ResetWidget *resetWidget)
{
	win->log(filter);

	if (resetWidget == NULL)
		resetWidget = qobject_cast<ResetWidget*>(sender()->parent());

	if (resetWidget == NULL)
		return;

	QLineEdit *ctrl = static_cast<QLineEdit*>(resetWidget->subWidget);

	//take existing dir
	QString initPath = ctrl->text();
	QFileInfo fi(initPath);
	if (!fi.exists())
	{
		//take mame_binary dir
		fi.setFile(mame_binary);
	}
	initPath = fi.absolutePath();

	if (!filter.isEmpty())
		filter.append(";;");
	filter.append(tr("All Files (*)"));
	win->log(filter);

	QString fileName = QFileDialog::getOpenFileName
		(resetWidget, tr("File name:"), initPath, filter);

	if (!fileName.isEmpty())
	{
		pathBuf = fileName;
		emit commitData(resetWidget);
		pathBuf.clear();
	}
}

void OptionDelegate::setDatFile()
{
	win->log("dat!");

	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Dat files (*.dat)"), resetWidget);
}

void OptionDelegate::setExeFile()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Executable files (*" EXEC_EXT ")"), resetWidget);
}

void OptionDelegate::setCfgFile()
{
	ResetWidget *resetWidget = qobject_cast<ResetWidget*>(sender()->parent());
	setFile(tr("Config files (*.cfg)"), resetWidget);
}


// parse listxml and init default mame opts
class OptionXMLHandler : public QXmlDefaultHandler
{
private:
	MameOption *pMameOpt;
	QString currentText;
	QString OSType;
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
				else if (type == "stringeditable")
					pMameOpt->type = MAMEOPT_TYPE_STRING_EDITABLE;
				else if (type == "file")
					pMameOpt->type = MAMEOPT_TYPE_FILE;
				else if (type == "datfile")
					pMameOpt->type = MAMEOPT_TYPE_DATFILE;
				else if (type == "exefile")
					pMameOpt->type = MAMEOPT_TYPE_EXEFILE;
				else if (type == "cfgfile")
					pMameOpt->type = MAMEOPT_TYPE_CFGFILE;
				else if (type == "dir")
					pMameOpt->type = MAMEOPT_TYPE_DIR;
				else if (type == "dirs")
					pMameOpt->type = MAMEOPT_TYPE_DIRS;
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
			OSType = attributes.value("os");
			if (pMameOpt != NULL && (OSType.isEmpty() ||
#ifdef Q_WS_WIN
				OSType == "win"
#else
				OSType == "sdl"
#endif
			))
			{
				pMameOpt->guivalues << attributes.value("guivalue");
			}
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
			{
				if (OSType.isEmpty() ||
#ifdef Q_WS_WIN
					OSType == "win"
#else
					OSType == "sdl"
#endif
				)
				pMameOpt->values << currentText;
			}
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
}

void OptionUtils::initOption()
{
	//assign unique ctlrs for each level of options
	optInfos = (QList<OptInfo *>() 
			<< new OptInfo(0, 0, optionsUI)
			<< new OptInfo(optionsUI->lvGlobalOpt, optionsUI->treeGlobalOpt, optionsUI)
			<< new OptInfo(optionsUI->lvSourceOpt, optionsUI->treeSourceOpt, optionsUI)
			<< new OptInfo(optionsUI->lvBiosOpt, optionsUI->treeBiosOpt, optionsUI)
			<< new OptInfo(optionsUI->lvCloneofOpt, optionsUI->treeCloneofOpt, optionsUI)
			<< new OptInfo(optionsUI->lvCurrOpt, optionsUI->treeCurrOpt, optionsUI));

	//init category list ctlrs for each level of options
	for (int optLevel = OPTLEVEL_GLOBAL; optLevel < OPTLEVEL_LAST; optLevel++)
	{
		QListWidget *lstCatView = optInfos[optLevel]->lstCatView;

		if (optLevel == OPTLEVEL_GLOBAL)
			lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/folder.png"), tr("Directory"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-display.png"), tr("Core Video"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-osd.png"), tr("OSD Video"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-display-blue.png"), tr("Screen"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/audio-x-generic.png"), tr("Audio"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/input-gaming.png"), tr("Control"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/video-vector.png"), tr("Vector"), lstCatView));
		lstCatView->addItem(new QListWidgetItem(QIcon(":/res/32x32/applications-system.png"), tr("Misc"), lstCatView));

		const QSize itemSz(1, 32);
		for (int i = 0; i < lstCatView->count(); i ++)
			lstCatView->item(i)->setSizeHint(itemSz);

		lstCatView->setIconSize(QSize(24, 24));
		lstCatView->setMaximumWidth(130);

		QHeaderView *header = optInfos[optLevel]->optView->header();
		disconnect(header, SIGNAL(sectionResized(int, int, int)), this, SLOT(updateHeaderSize(int, int, int)));
		connect(header, SIGNAL(sectionResized(int, int, int)), this, SLOT(updateHeaderSize(int, int, int)));

		disconnect(optionsUI, SIGNAL(accepted()), this, SLOT(setChangesAccepted()));
		connect(optionsUI, SIGNAL(accepted()), this, SLOT(setChangesAccepted()));
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

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING 
		  || pMameOpt->type == MAMEOPT_TYPE_STRING_EDITABLE)
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

	else if (pMameOpt->type == MAMEOPT_TYPE_STRING 
		  || pMameOpt->type == MAMEOPT_TYPE_STRING_EDITABLE)
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
	QString optName = getField(index, USERROLE_KEY).toString();
	QString dispVal = index.model()->data(i).toString();

	QString compVal;
	MameOption *pMameOpt = mameOpts[optName];

	// return a different color if value is not default
	if (dispVal == getLongValue(optName, pMameOpt->defvalue))
		return Qt::transparent;
	else
		return QColor(255, 255, 0, 64);
}

bool OptionUtils::isChanged(const QModelIndex &index)
{
	QModelIndex i = index.sibling(index.row(), 1);
	int optLevel = index.model()->objectName().remove("optModel").toInt();
	QString optName = getField(index, USERROLE_KEY).toString();
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

	compVal = getLongValue(optName, compVal);

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
//the following is for Qt translation
#if 0
	QStringList optList = (QStringList()
		<< QT_TR_NOOP("cabinet directory")
		<< QT_TR_NOOP("control panel directory")
		<< QT_TR_NOOP("flyer directory")
		<< QT_TR_NOOP("marquee directory")
		<< QT_TR_NOOP("pcb directory")
		<< QT_TR_NOOP("title directory")
		<< QT_TR_NOOP("icons directory")
		<< QT_TR_NOOP("background directory")
		<< QT_TR_NOOP("external folder list")

		<< QT_TR_NOOP("m1 directory")
		<< QT_TR_NOOP("history file")
		<< QT_TR_NOOP("story file")
		<< QT_TR_NOOP("mameinfo file")
		<< QT_TR_NOOP("mame binary")
		
		<< QT_TR_NOOP("driver config")
		
		<< QT_TR_NOOP("readconfig")
			
		<< QT_TR_NOOP("romsets directory")
		<< QT_TR_NOOP("hash files directory")
		<< QT_TR_NOOP("samplesets directory")
		<< QT_TR_NOOP("artwork files directory")
		<< QT_TR_NOOP("controller definitions directory")
		<< QT_TR_NOOP("ini files directory")
		<< QT_TR_NOOP("font files directory")
		<< QT_TR_NOOP("cheat files directory")
		<< QT_TR_NOOP("language files directory")
		<< QT_TR_NOOP("localized directory")
		<< QT_TR_NOOP("ips files directory")

		<< QT_TR_NOOP("cfg directory")
		<< QT_TR_NOOP("nvram directory")
		<< QT_TR_NOOP("memcard directory")
		<< QT_TR_NOOP("input directory")
		<< QT_TR_NOOP("state directory")
		<< QT_TR_NOOP("snapshot directory")
		<< QT_TR_NOOP("diff directory")
		<< QT_TR_NOOP("comment directory")
		<< QT_TR_NOOP("hiscore directory")

		<< QT_TR_NOOP("command file")
		<< QT_TR_NOOP("hiscore file")

		<< QT_TR_NOOP("auto restore and save")
		<< QT_TR_NOOP("snapshot/movie view")
		
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
			
		<< QT_TR_NOOP("enable sound output")
		<< QT_TR_NOOP("sample rate")
		
		<< QT_TR_NOOP("use samples")
		<< QT_TR_NOOP("volume attenuation")
		<< QT_TR_NOOP("use volume auto adjust")

		<< QT_TR_NOOP("coin lockout")
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
			
		<< QT_TR_NOOP("all screens: physical monitor")
		<< QT_TR_NOOP("all screens: aspect ratio")
		<< QT_TR_NOOP("all screens: resolution")
		<< QT_TR_NOOP("all screens: view")
		
		<< QT_TR_NOOP("screen 1: physical monitor")
		<< QT_TR_NOOP("screen 1: aspect ratio")
		<< QT_TR_NOOP("screen 1: resolution")
		<< QT_TR_NOOP("screen 1: view")
		
		<< QT_TR_NOOP("screen 2: physical monitor")
		<< QT_TR_NOOP("screen 2: aspect ratio")
		<< QT_TR_NOOP("screen 2: resolution")
		<< QT_TR_NOOP("screen 2: view")
		
		<< QT_TR_NOOP("screen 3: physical monitor")
		<< QT_TR_NOOP("screen 3: aspect ratio")
		<< QT_TR_NOOP("screen 3: resolution")
		<< QT_TR_NOOP("screen 3: view")
		
		<< QT_TR_NOOP("screen 4: physical monitor")
		<< QT_TR_NOOP("screen 4: aspect ratio")
		<< QT_TR_NOOP("screen 4: resolution")
		<< QT_TR_NOOP("screen 4: view")
			
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
		
		<< "02_Core Video_02_" + QString(QT_TR_NOOP("core rotation"))
		<< "02_Core Video_03_" + QString(QT_TR_NOOP("core screen"))
		<< "02_Core Video_04_" + QString(QT_TR_NOOP("full screen"))
		<< "02_Core Video_08_" + QString(QT_TR_NOOP("core performance"))

		<< "03_OSD Video_05_" + QString(QT_TR_NOOP("Windows video"))
		<< "03_OSD Video_06_" + QString(QT_TR_NOOP("DirectDraw-specific"))
		<< "03_OSD Video_07_" + QString(QT_TR_NOOP("Direct3D-specific"))
		<< "03_OSD Video_09_" + QString(QT_TR_NOOP("Windows performance"))
		<< "03_OSD Video_10_" + QString(QT_TR_NOOP("OpenGL-specific"))

		<< "04_Screen_10_" + QString(QT_TR_NOOP("per-window video"))
		
		<< "05_Audio_00_" + QString(QT_TR_NOOP("core sound"))
		<< "05_Audio_01_" + QString(QT_TR_NOOP("Windows sound"))
		
		<< "06_Control_00_" + QString(QT_TR_NOOP("core input"))
		<< "06_Control_01_" + QString(QT_TR_NOOP("core input automatic enable"))
		<< "06_Control_02_" + QString(QT_TR_NOOP("input device"))
		<< "06_Control_03_" + QString(QT_TR_NOOP("SDL keyboard mapping"))
		<< "06_Control_04_" + QString(QT_TR_NOOP("SDL joystick mapping"))
		
		<< "07_Vector_00_" + QString(QT_TR_NOOP("core vector"))
		
		<< "08_Misc_00_" + QString(QT_TR_NOOP("core misc"))
		<< "08_Misc_01_" + QString(QT_TR_NOOP("core artwork"))
		<< "08_Misc_02_" + QString(QT_TR_NOOP("core state/playback"))
		<< "08_Misc_03_" + QString(QT_TR_NOOP("SDL lowlevel driver"))
		<< "08_Misc_04_" + QString(QT_TR_NOOP("MESS specific"))
		<< "08_Misc_05_" + QString(QT_TR_NOOP("Windows MESS specific"))
		<< "08_Misc_06_" + QString(QT_TR_NOOP("core debugging"))
		<< "08_Misc_07_" + QString(QT_TR_NOOP("Windows debugging"))
		);

	QString line, line0;
	QTextStream in(&text);
	in.setCodec("UTF-8");
	QString optHeader = "ERROR_MAGIC";
	optCatMap.clear();

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

				QStringList list = utils->split2Str(line0, " ");
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

	/* take the first value of mame's default inipath csv as our inipath */
	QStringList inipaths = mameOpts["inipath"]->defvalue.split(";");
	mameIniPath = utils->getPath(inipaths[0]);

	/* test ini readable/writable */
	QString warnings = "";
	QFile iniFile(mameIniPath + "mame.ini");
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append(QFileInfo(iniFile).absoluteFilePath());
	iniFile.close();
	if (iniFile.size() == 0)
		iniFile.remove();

	// mkdir for individual game settings
	QDir().mkpath(mameIniPath + "ini/source");

	iniFile.setFileName(mameIniPath + "ini/puckman.ini");
	if (!iniFile.open(QIODevice::ReadWrite | QFile::Text))
		warnings.append("\n" + QFileInfo(mameIniPath + "ini").absoluteFilePath());
	iniFile.close();
	if (iniFile.size() == 0)
		iniFile.remove();

	if (warnings.size() > 0)
		win->poplog("Current user has no sufficient privilege to read/write:\n" + warnings + "\n\nCouldn't save MAME settings.");

	/* patch inipath for unofficial mame */
	QString inipath = mameOpts["inipath"]->defvalue;
	if (inipath != ".;ini")
	{
//		win->log("unofficial mame inipath");
		inipath.append(";");
		inipath.append(mameIniPath + "ini");
		inipath.remove("./");
		mameOpts["inipath"]->defvalue = inipath;
	}
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
	QHash<QString, QString> iniSettings = readIniFile(iniFileName);

	//iterate every option
	foreach (QString optName, mameOpts.keys())
	{
		MameOption *pMameOpt = mameOpts[optName];

		//ignore all guiSettings, they are not handled by mame's .ini
		if (guiSettings.contains(optName))
		{
			pMameOpt->globalvalue = pMameOpt->currvalue = guiSettings.value(optName).toString();
			continue;
		}

		//ips settings doesnt inherit
		if (optName == "ips")
		{
			//reset all to default
			pMameOpt->currvalue = pMameOpt->cloneofvalue= pMameOpt->biosvalue = pMameOpt->srcvalue = pMameOpt->globalvalue = pMameOpt->defvalue;

			if (iniSettings.contains(optName))
			{
				pMameOpt->currvalue = iniSettings[optName];
			}

			continue;
		}

		//take ini override when available in loaded value
		if (iniSettings.contains(optName))
		{
			// assign ini value to current value
			pMameOpt->currvalue = iniSettings[optName];

			// assign ini value to level value
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
		//inherit from higher level value, assign it to current and level value
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

void OptionUtils::saveIniFile(int optLevel, const QString &iniFileName)
{
	//saveIniFile MAME Options
	QFile outFile(iniFileName);
	QString line;
	QStringList headers;
	QString mameIni;

	if (outFile.open(QFile::WriteOnly | QFile::Text))
	{
		QTextStream in(&mameGame->mameDefaultIni);
		QTextStream outBuf(&mameIni);
		QTextStream out(&outFile);
		in.setCodec("UTF-8");
		outBuf.setCodec("UTF-8");
		out.setCodec("UTF-8");
		//mame.ini always uses a BOM, doesnt work otherwise
		out.setGenerateByteOrderMark(true);
		bool isHeader = false, isChanged = false;
		QString optName;

		do
		{
			QString currVal, defVal;

			line = in.readLine();

			// ignore <UNADORNED>
			if (line.startsWith("<"))
				continue;

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
				int sep = line.indexOf(spaceRegex);
				optName = line.left(sep);

				MameOption *pMameOpt = mameOpts[optName];

				switch (optLevel)
				{
				case OPTLEVEL_GLOBAL:
					currVal = pMameOpt->globalvalue;
					defVal = pMameOpt->defvalue;
					//ignore global bios setting
					if (optName == "bios")
						currVal = pMameOpt->defvalue;
					break;
				
				case OPTLEVEL_SRC:
					currVal = pMameOpt->srcvalue;
					defVal = pMameOpt->globalvalue;
					//ignore src bios setting
					if (optName == "bios")
						currVal = pMameOpt->defvalue;
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
					if (optName == "ips")
						defVal = pMameOpt->defvalue;
					else
						defVal = pMameOpt->cloneofvalue;
				}

				if (getLongValue(optName, currVal) == 
					getLongValue(optName, defVal))
					isChanged = false;
				else
					isChanged = true;

				isHeader = false;
			}

			if (!isHeader && !line.isEmpty())
			{
				// write header and clear header buffer
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
					if (currVal.indexOf(spaceRegex) > 0 && !(currVal.startsWith('"') && currVal.endsWith('"')))
						outBuf << "\"" << currVal << "\"" << endl;
					else
						outBuf << currVal << endl;
				}
			}
		}
		while (!line.isNull());

		/* postprocess */
		QStringList bufs = mameIni.split(QRegExp("[\\r\\n]+"));

		// remove language setting, it's set at runtime
		for (int i = 0; i < bufs.count(); i ++)
		{
			if (bufs[i].startsWith("language"))
				bufs.removeAt(i);
		}

		// read in reverse order to eat empty headers		
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

		// remove trailing blank lines
		for (int i = bufs.count() - 1; i >= 0; i --)
		{
			if (bufs[i].isEmpty())
				bufs.removeAt(i);
			else
				break;
		}

		for (int i = 0; i < bufs.count(); i ++)
		{
			static bool isEntry = false;

			// make an empty line between sections
			if (bufs[i].startsWith("#"))
			{
				// add it when prev line is an entry and curr line is a header
				if (isEntry && i > 3)	//skip first entry
					out << endl;

				isEntry = false;
			}
			else
				isEntry = true;

			// don't output readconfig
			if (!bufs[i].contains("readconfig"))
				out << bufs[i] << endl;
		}
	}

	// delete ini if all options are default
	if (outFile.size() == 0)
		outFile.remove();
}

QHash<QString, QString> OptionUtils::readIniFile(const QString &iniFileName)
{
	QFile inFile(iniFileName);

	QString line, line0;
	QHash<QString, QString> settings;

	if (inFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&inFile);
		in.setCodec("UTF-8");

		QString key, value;

		do
		{
			line0 = line = in.readLine().trimmed();
			//parse only !# lines
			if (!line.startsWith("#") && line.size() > 0)
			{
				//locate the first space
				int sep = line.indexOf(spaceRegex);

				//valid entry
				if (sep != -1)
				{
					key = line.left(sep);
					value = line.right(line.size() - sep).trimmed();

					//remove quoted value if needed	
					if (value.startsWith('"') && value.endsWith('"'))
					{
						value.remove(0, 1);
						value.chop(1);
					}
				}
				// empty entry
				else
				{
					key = line;
					value = "";
				}
			}
			settings[key] = value;
		}
		while (!line.isNull());
	}

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
		optLevel = optionsUI->tabOptions->currentIndex();

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
			optCat = tr("Core Video");
	}

	/* update mameopts */
	GameInfo *gameInfo = mameGame->games[currentGame];
	QString iniString;
	QString STR_OPTS_ = tr("Options") + " - ";

	//global
	loadIni(OPTLEVEL_GLOBAL, mameIniPath + "mame.ini");
	if (optLevel == OPTLEVEL_GLOBAL)
	{
		updateModelData(optCat, OPTLEVEL_GLOBAL);
		optionsUI->setWindowTitle(STR_OPTS_ + tr("Global"));
		return;
	}

	//source
	iniString = gameInfo->sourcefile;
	iniString.replace(".c", INI_EXT);
	loadIni(OPTLEVEL_SRC, mameIniPath + "ini/source/" + iniString);
	if (optLevel == OPTLEVEL_SRC)
	{
		updateModelData(optCat, OPTLEVEL_SRC);
		optionsUI->setWindowTitle(STR_OPTS_ + gameInfo->sourcefile);
		return;
	}

	//bios
	iniString = gameInfo->biosof();
	if (iniString.isEmpty())
		optionsUI->tabOptions->widget(OPTLEVEL_BIOS)->setEnabled(false);
	else
		optionsUI->tabOptions->widget(OPTLEVEL_BIOS)->setEnabled(true);

	loadIni(OPTLEVEL_BIOS, mameIniPath + "ini/" + iniString + INI_EXT);
	if (optLevel == OPTLEVEL_BIOS)
	{
		updateModelData(optCat, OPTLEVEL_BIOS);
		optionsUI->setWindowTitle(STR_OPTS_ + iniString);
		return;
	}

	//cloneof
	iniString = gameInfo->cloneof;
	if (iniString.isEmpty())
		optionsUI->tabOptions->widget(OPTLEVEL_CLONEOF)->setEnabled(false);
	else
		optionsUI->tabOptions->widget(OPTLEVEL_CLONEOF)->setEnabled(true);

	loadIni(OPTLEVEL_CLONEOF, mameIniPath + "ini/" + iniString + INI_EXT);
	if (optLevel == OPTLEVEL_CLONEOF)
	{
		updateModelData(optCat, OPTLEVEL_CLONEOF);
		optionsUI->setWindowTitle(STR_OPTS_ + iniString);
		return;
	}

	//current game
	//special case for consoles
	if (gameInfo->isExtRom)
		iniString = gameInfo->romof;
	else
		iniString = currentGame;
	
	loadIni(OPTLEVEL_CURR, mameIniPath + "ini/" + iniString + INI_EXT);
	if (optLevel == OPTLEVEL_CURR)
	{
		updateModelData(optCat, OPTLEVEL_CURR);
		optionsUI->setWindowTitle(STR_OPTS_ + iniString);
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
				addModelItemTitle(optModel, QT_TR_NOOP("GUI DIRECTORY"));
			
				foreach (QString optName, mameOpts.keys())
				{
					MameOption *pMameOpt = mameOpts[optName];

					if (!pMameOpt->guivisible || optName.contains("_extra_software"))
						continue;

					addModelItem(optModel, optName);
				}

				addModelItemTitle(optModel, QT_TR_NOOP("MESS SOFTWARE DIRECTORY"));

				//add console dir options
				foreach (QString consoleName, consoleGamesL)
				{
					QString optName = consoleName + "_extra_software";
					MameOption *pMameOpt = mameOpts[optName];
					pMameOpt->type = MAMEOPT_TYPE_DIR;

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
	QStandardItem *item = new QStandardItem(tr(qPrintable(title)).toUpper());
	item->setData("OPTIONTITLE", Qt::UserRole + USERROLE_TITLE);
	optModel->appendRow(item);
}

void OptionUtils::addModelItem(QStandardItemModel *optModel, QString optName)
{
	updateSelectableItems(optName);

	MameOption *pMameOpt = mameOpts[optName];
//	QString tooltip = "[" + optName + "] " + pMameOpt->description;

	/* fill key */
	QString key = utils->capitalizeStr(tr(qPrintable(getLongName(optName).toLower())));
	//fix incorrectly lower cased
	if (language == "en_US" && !mameOpts[optName]->guiname.isEmpty())
		key = utils->capitalizeStr(mameOpts[optName]->guiname);

	QStandardItem *itemKey = new QStandardItem(key);
	itemKey->setData(optName, Qt::UserRole + USERROLE_KEY);

	//hack: a blank icon for padding
	static const QIcon icon(":/res/16x16/blank.png");
	itemKey->setIcon(icon);
//	itemKey->setData(tooltip, Qt::ToolTipRole);

	/* fill value */
	QStandardItem *itemVal = new QStandardItem(getLongValue(optName, pMameOpt->currvalue));
//	itemVal->setData(tooltip, Qt::ToolTipRole);

	optModel->appendRow(QList<QStandardItem *>() << itemKey << itemVal);
}

void OptionUtils::updateSelectableItems(QString optName)
{
	MameOption *pMameOpt = mameOpts[optName];

	// init BIOS values
	if (optName == "bios")
	{
		GameInfo *gameInfo = mameGame->games[currentGame];
		QString biosof = gameInfo->biosof();
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		if ((gameInfo->isBios || !biosof.isEmpty()))
		{
			if (!gameInfo->isBios)
				gameInfo = mameGame->games[biosof];

			foreach (QString name, gameInfo->biosSets.keys())
			{
				BiosSet *biosSet = gameInfo->biosSets[name];

				pMameOpt->values.append(name);
				pMameOpt->guivalues.append(biosSet->description);
			}
		}
	}

	// init ctrlr values
	else if (optName == "ctrlr")
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		// iterate files
		QDir dir(mameOpts["ctrlrpath"]->currvalue);

		QStringList nameFilter;
		nameFilter << "*.cfg";
		
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		for (int i = 0; i < files.count(); i++)
		{
			QFileInfo fi(files[i]);
			QString ctrlr = fi.fileName();
			ctrlr.remove(".cfg");
			pMameOpt->values.append(ctrlr);
			pMameOpt->guivalues.append(ctrlr);
		}
	}
	
	//init effect values
	else if (optName == "effect")
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("none");
		pMameOpt->guivalues.append(tr("None"));

		// iterate files
		QDir dir(mameOpts["artpath"]->currvalue);

		QStringList nameFilter;
		nameFilter << "*.png";
		
		QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable);
		for (int i = 0; i < files.count(); i++)
		{
			QFileInfo fi(files[i]);
			QString ctrlr = fi.fileName();
			ctrlr.remove(".png");
			pMameOpt->values.append(ctrlr);
			pMameOpt->guivalues.append(ctrlr);
		}
	}

#ifdef Q_OS_WIN
	else if (optName.startsWith("screen"))
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("auto");
		pMameOpt->guivalues.append(tr("Auto"));

		if (sdlInited)
		{
			for (int d = 0; d < SDL_GetNumVideoDisplays(); ++d)
			{
				QString displayName = QString("\\\\.\\DISPLAY%1").arg(d + 1);
				pMameOpt->values.append(displayName);
				pMameOpt->guivalues.append(displayName);
			}
		}
	}

	else if (optName.startsWith("resolution"))
	{
		pMameOpt->values.clear();
		pMameOpt->guivalues.clear();

		pMameOpt->values.append("auto");
		pMameOpt->guivalues.append(tr("Auto"));

		if (sdlInited)
		{
			//select respective screen
			QString optName2 = optName;
			optName2.replace("resolution", "screen");
			QString displayName = mameOpts[optName2]->currvalue;
			bool ok;
			int d = displayName.remove("\\\\.\\DISPLAY").toInt(&ok);
			SDL_SelectVideoDisplay(ok ? d - 1 : 0);

			// emun available fullscreen video modes
			int nmodes = SDL_GetNumDisplayModes();
			if (nmodes == 0)
				win->log("SDL: No available fullscreen video modes");
			else
			{
				SDL_DisplayMode mode;
				int bpp;
				Uint32 Rmask, Gmask, Bmask, Amask;
			
				for (int m = 0; m < nmodes; ++ m)
				{
					SDL_GetDisplayMode(m, &mode);
					SDL_PixelFormatEnumToMasks(mode.format, &bpp, 
						&Rmask, &Gmask, &Bmask, &Amask);

					QString modeName = QString("%1x%2@%3")
						.arg(mode.w)
						.arg(mode.h)
						.arg(mode.refresh_rate);

					pMameOpt->values.append(modeName);
					pMameOpt->guivalues.append(modeName + "Hz");
				}
			}
		}
	}
#endif

	// prepare GUI value desc
	for (int i = 0; i < pMameOpt->guivalues.size(); i++)
	{
		if (pMameOpt->guivalues[i].isEmpty())
			pMameOpt->guivalues[i] = utils->capitalizeStr(pMameOpt->values[i]);
	}
}

void OptionUtils::updateHeaderSize(int logicalIndex, int oldSize, int newSize)
{
	if (logicalIndex > 0)
		return;

	int optLevel = optionsUI->tabOptions->currentIndex();
	option_column_state = optInfos[optLevel]->optView->header()->saveState();
	
	win->log(QString("header%3: %1 to %2").arg(optInfos[optLevel]->optView->header()->sectionSize(0)).arg(newSize).arg(logicalIndex));
}

