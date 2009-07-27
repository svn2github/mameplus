#include "ips.h"

#include "mamepguimain.h"
#include "mameopt.h"

/* global */
IPS *ipsUI = NULL;

/* internal */
const QStringList ipsLangs = (QStringList()
	<< "zh_CN" << "zh_TW" << "en_US" );

IPS::IPS(QWidget *parent) : 
QDialog(parent),
ipspath(NULL)
{
	setupUi(this);
}

void IPS::init()
{
	const QStringList headers = (QStringList()
		<< tr("Description") << tr("Name"));
	twList->setHeaderLabels(headers);

	cmbLang->addItems(ipsLangs);

	QString ipsLanguage = pGuiSettings->value("ips_language").toString();
	int sel = ipsLangs.indexOf(ipsLanguage);
	if (sel < 0)
	{
		sel = ipsLangs.indexOf(language);
		if (sel < 0)
			sel = 0;
	}
	cmbLang->setCurrentIndex(sel);

	chkRelation->setChecked(pGuiSettings->value("ips_relationship", "1").toInt() == 1);

	connect(twList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), 
			this, SLOT(parse(QTreeWidgetItem *, QTreeWidgetItem *)));
	connect(this, SIGNAL(accepted()), this, SLOT(save()));
	connect(btnClear, SIGNAL(clicked(bool)), this, SLOT(clear()));
	//for relations management
	connect(twList, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(applyRelations(QTreeWidgetItem *, int)));
	connect(cmbLang, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(updateList()));
}

void IPS::applyRelations(QTreeWidgetItem *item, int col)
{
	if (col == 1 || stopListenRelations)
		return;

	QString datName = item->text(1).toLower().trimmed();
	itemStateTable[datName] = (item->checkState(0) == Qt::Checked) ? 1 : 0;

	if (!chkRelation->isChecked())
		return;

	validateConf(datName);
	validateDep(datName);

	stopListenRelations = true;
	iterateItems(twList->invisibleRootItem(), ITR_RELATIONS);
	stopListenRelations = false;
}

void IPS::validateConf(const QString &datName)
{
//	win->log("validateConf: " + datName);

	if (itemStateTable[datName] == 0)
		return;

	//iterate confTable
	foreach (QStringList confList, confTable)
	{
		//locate the confList we're interested in
		if (confList.contains(datName))
		{
			//uncheck items in the confList
			foreach (QString conf, confList)
			{
				if (datName == conf)
					continue;

				if (itemStateTable[conf] == 1)
				{
					itemStateTable.insert(conf, 0);
//					win->log("auto unchk: " + conf);
					validateDep(conf);
				}
			}
		}
	}
}

void IPS::validateDep(const QString &datName)
{
	//depend case A: if parent ($datName) is unchecked, uncheck all its dependant children
	if (itemStateTable[datName] == 0)
	{
		//depTable might contain more than 1 entries for a single parent
		QList<QStringList> depChildrenList = depTable.values(datName/*parent*/);
		for (int i = 0; i < depChildrenList.size(); i++)
		{
			//if dep parent is found in the depTable
			QStringList depChildren = depChildrenList.at(i);
			//uncheck all dependant children
			foreach (QString depChild, depChildren)
			{
				//only when a child is checked
				if (itemStateTable[depChild] == 0)
					continue;

				bool isUncheck = true;

				// if any other parent (that is not $datName) is checked, ignore the unchecking
				QHashIterator<QString, QStringList> it(depTable);
				while (it.hasNext())
				{
					it.next();
					QString parent = it.key();

					//if any other parent (that is not $datName)
					if (parent == datName)
						continue;
					//is checked
					if (itemStateTable[parent] == 0)
						continue;

					QStringList children = it.value();
					if (children.contains(depChild))
					{
						//ignore the unchecking
						isUncheck = false;
						break;
					}
				}

				if (isUncheck)
				{
					itemStateTable.insert(depChild, 0);
//					win->log("auto unchk: " + depChild);
					validateDep(depChild);
				}
			}
		}
	}
	//depend case B: if any child ($datName) is checked, check 1 of its parents
	else if (itemStateTable[datName] == 1)
	{
		QHashIterator<QString, QStringList> it(depTable);
		while (it.hasNext())
		{
			it.next();

			QStringList children = it.value();
			if (children.contains(datName))
			{
				QString parent = it.key();

				//only when another parent is unchecked
				if (itemStateTable[parent] == 1)
					continue;

				bool isCheck = true;

				// if any other parent (that is not $parent) of child ($datName) is checked, ignore the checking
				QHashIterator<QString, QStringList> _it(depTable);
				while (_it.hasNext())
				{
					_it.next();
					QString _parent = _it.key();

					//if any other parent (that is not $parent)
					if (_parent == parent)
						continue;

					QStringList _children = _it.value();
					//of child ($datName) is checked
					if (_children.contains(datName) 
						&& itemStateTable[_parent] == 1)
					{
						//ignore the checking
						isCheck = false;
						break;
					}
				}

				if (isCheck)
				{
					itemStateTable.insert(parent, 1);
//					win->log("auto chk: " + parent);
					validateConf(parent);
					validateDep(parent);
					break;
				}
			}
		}
	}
}

void IPS::parseRelations()
{
	depTable.clear();
	confTable.clear();

	QFile datFile(ipspath + currentGame + "/" + "assistant.txt");

	//parse ips dat and update the treewidget
	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");

		QString line;

		do
		{
			line = in.readLine();
			if (!line.startsWith("#"))
			{
				if (line.contains(">"))
				{
					QStringList depStrs = utils->split2Str(line, ">");
					const QString parent = depStrs.first().toLower().trimmed();
					QStringList children = depStrs.last().split(",");
					utils->lowerTrimmed(children);
					depTable.insert(parent, children);
				}
				else
				{
					QStringList conflicts = line.split(",");
					utils->lowerTrimmed(conflicts);
					confTable.append(conflicts);
				}
			}
		}
		while (!line.isNull());
	}
}

//check if is ips dat is avaiable for a game
bool IPS::checkAvailable(const QString &gameName)
{
	//set current ips dirpath
	if (ipspath == NULL && mameOpts.contains("ipspath"))
	{
		QString _dirpath = utils->getPath(mameOpts["ipspath"]->globalvalue);
		QDir dir(_dirpath);
		if (!_dirpath.isEmpty() && dir.exists())
			ipspath = utils->getPath(_dirpath);
	}

	if (ipspath == NULL)
		return false;

	//iterate all files in ips path
	QDir dir(ipspath + gameName);
	datFiles = dir.entryList((QStringList() << "*.dat"), QDir::Files | QDir::Readable);
	if (datFiles.isEmpty())
		return false;

	return true;
}

void IPS::parse(QTreeWidgetItem *current, QTreeWidgetItem *previous, const QString &_datName, const QString & fallbackLang)
{
	QString datName = _datName;
	
	if (ipspath == NULL)
		return;

	if (current != NULL)
		datName = current->text(1) + ".dat";

	//set category font
	static QFont boldFont(twList->font());
	boldFont.setBold(true);
	static const QBrush brushCat = QBrush(QColor(0, 21, 110, 255));

	QString lang;
	if (fallbackLang.isEmpty())
		lang = "[" + cmbLang->currentText() + "]";
	else
		lang = fallbackLang;

	QFile datFile(ipspath + currentGame + "/" + datName);
	QFileInfo fi(datFile);
	datName = fi.completeBaseName();

	//parse ips dat and update the treewidget
	if (datFile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream in(&datFile);
		in.setCodec("UTF-8");
	
		QString line;
		QString desc = "";
	
		//met category/short desc
		bool startCat = false;
		//met long desc
		bool startDesc = false;
		bool success = false;

		do
		{
			line = in.readLine().trimmed();

			//met another language
			if (startDesc && line.startsWith("[") && line.endsWith("]"))
			{
				startDesc = false;
			}

			//build ips tree
			if (current == NULL && startCat)
			{
				QStringList cats = line.split("/");
				//fixme: move to utils
				int sep;
				sep = line.indexOf("/");
	
				QTreeWidgetItem *parentItem = twList->invisibleRootItem();
				QTreeWidgetItem *item;
	
				QString shortDesc;
				if (sep > -1)
				{
					QString cat = line.left(sep);
					shortDesc = line.right(line.size() - sep - 1);

					QList<QTreeWidgetItem *> foundItems = twList->findItems(cat, Qt::MatchFixedString, 0);
					if (foundItems.isEmpty())
					{
						parentItem = new QTreeWidgetItem(parentItem, (QStringList() << cat));
						parentItem->setForeground (0, brushCat);
						parentItem->setFont(0, boldFont);
						parentItem->setFlags(parentItem->flags() 
							& ~Qt::ItemIsUserCheckable
							& ~Qt::ItemIsSelectable);
					}
					else
						parentItem = foundItems.first();
				}
				else
				{
					shortDesc = line;
					parentItem = twList->invisibleRootItem();
				}
	
				item = new QTreeWidgetItem(parentItem, (QStringList() 
						<< shortDesc << datName));
				item->setCheckState(0, Qt::Unchecked);

				if (!datName.isEmpty())
					itemStateTable.insert(datName.toLower(), 0);

				success = true;
			
				startCat = false;
			}
	
			if (current != NULL && startDesc)
			{
				desc.append(line);
				desc.append("<br>");
			}
	
			if (line == lang)
			{
				startCat = true;
				startDesc = true;
			}
		}
		while (!line.isNull());

		//use fallback language to parse again
		QString fallbackTag = "[" + ipsLangs[0] + "]";
		if (current == NULL && lang != fallbackTag && !success)
		{
//			win->log(QString("lang fallback: %1").arg(_datName));
			parse(current, previous, _datName, fallbackTag);
		}

		//update desc and snap
		if (current != NULL)
		{
			tbDesc->setHtml(desc);

			//fixme: hack
			QSize bounding = lblSnap->size();
			if (bounding.width() < 260 || bounding.height() < 240)
				bounding = QSize(260, 240);

			QPixmap pm = QPixmap(ipspath + currentGame + "/" + datName + ".png");
			if (pm.isNull())
			{
				GameInfo *gameinfo = mameGame->games[currentGame];
				if (!gameinfo->cloneof.isEmpty())
					pm = QPixmap(ipspath + gameinfo->cloneof + "/" + datName + ".png");
			}

			QSize scaledSize = utils->getScaledSize(pm.size(), bounding, true);
			lblSnap->setPixmap(pm.scaled(scaledSize,
					Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		}
	}
}

//update tree ctlr content and checkstate
void IPS::updateList()
{
	twList->clear();
	itemStateTable.clear();

	//build ips tree
	for (int i = 0; i < datFiles.count(); i++)
		parse(NULL, NULL, datFiles[i]);

	twList->expandAll();

	//update mameOpts
	optUtils->preUpdateModel(NULL, OPTLEVEL_CURR);

	//check the selected ips	
	if (mameOpts.contains("ips"))
	{
		QString ipsString = mameOpts["ips"]->currvalue;
		QStringList datNames = ipsString.split(",");

		foreach (QString datName, datNames)
		{
			//skip empty entries
			datName = datName.trimmed();
			if (datName.isEmpty())
				continue;

			QList<QTreeWidgetItem *> foundItems = twList->findItems(datName, Qt::MatchFixedString | Qt::MatchRecursive, 1);
			if (!foundItems.isEmpty())
			{
				QTreeWidgetItem *item = foundItems.first();
				item->setCheckState(0, Qt::Checked);
				if (itemStateTable.contains(datName))
					itemStateTable.insert(datName, 1);
			}
		}
	}

	//hack. auto resize the header, then release view to the user
	QHeaderView *header = twList->header();
	header->resizeSection(1, 20);
	header->setResizeMode(0, QHeaderView::ResizeToContents);
	qApp->processEvents();
	//qt bug? must add a little more space
	header->resizeSection(0, header->sectionSize(0) + 10);
	header->setResizeMode(0, QHeaderView::Interactive);

	QTreeWidgetItem *item = twList->itemAt(0, 0);
	if (item->childCount() > 0)
	{
		twList->setCurrentItem(item->child(0));
		twList->scrollToItem(item);
	}
	else
		twList->setCurrentItem(item);

	setWindowTitle(utils->getDesc(currentGame) + " - " + "IPS Settings");

	stopListenRelations = false;
	parseRelations();
}

void IPS::save()
{
	ipsValues.clear();

	QHashIterator<QString, int> i(itemStateTable);
	while (i.hasNext())
	{
		i.next();
		if (i.value() == 1)
			ipsValues.append(i.key() + ",");
	}

	if (ipsValues.endsWith(","))
		ipsValues.remove(ipsValues.length() - 1, 1);

	mameOpts["ips"]->currvalue = ipsValues;
	optUtils->saveIniFile(OPTLEVEL_CURR, mameIniPath + "ini/" + currentGame + INI_EXT);
}

void IPS::clear()
{
//	win->log("clear");
	iterateItems(twList->invisibleRootItem(), ITR_CLEAR);
}

//save tree ctrl values to private data: ipsValues
void IPS::iterateItems(QTreeWidgetItem *item, int method)
{
	int count = item->childCount();

	//it's a category
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			QTreeWidgetItem *childItem = item->child(i);
			iterateItems(childItem, method);
		}
	}
	// ips item
	else
	{
		const QString datName = item->text(1).toLower().trimmed();

		//process signals sent from a checked item
		if (item->checkState(0) == Qt::Checked)
		{
			if (method == ITR_CLEAR)
				item->setCheckState(0, Qt::Unchecked);
		}

		if (method == ITR_RELATIONS)
		{
			item->setCheckState(0, (itemStateTable[datName] == 1) ? Qt::Checked : Qt::Unchecked);
		}
	}
}

