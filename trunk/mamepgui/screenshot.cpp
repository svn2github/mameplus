// screenshot widget
#include "screenshot.h"
#include "utils.h"
#include "mainwindow.h"

Screenshot::Screenshot(QString title, QWidget *parent) : 
	QDockWidget(parent),
	forceAspect(false)
{
	setObjectName("dockWidget_" + title);
	setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::NoDockWidgetFeatures);
	dockWidgetContents = new QWidget(this);
	dockWidgetContents->setObjectName("dockWidgetContents_" + title);
	mainLayout = new QGridLayout(dockWidgetContents);
	mainLayout->setObjectName("mainLayout_" + title);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	screenshotLabel = new QPushButton(dockWidgetContents);
	screenshotLabel->setObjectName("label_" + title);
	screenshotLabel->setCursor(QCursor(Qt::PointingHandCursor));
	screenshotLabel->setFlat(true);

	//so that we can shrink image
	screenshotLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	
	mainLayout->addWidget(screenshotLabel);

	setWidget(dockWidgetContents);
	setWindowTitle(MainWindow::tr(qPrintable(title)));

	connect(screenshotLabel, SIGNAL(clicked()), this, SLOT(rotateImage()));
}

void Screenshot::resizeEvent(QResizeEvent * /* event */)
{
    QSize scaledSize = originalPixmap.size();
	scaledSize.scale(screenshotLabel->size(), Qt::KeepAspectRatio);

	updateScreenshotLabel();
}

void Screenshot::setPixmap(QPixmap pm)
{
	originalPixmap = pm;
	forceAspect = false;
	updateScreenshotLabel();
}

void Screenshot::setPixmap(const QByteArray &pmdata, bool _forceAspect)
{
	QPixmap pm;
	pm.loadFromData(pmdata);
	originalPixmap = pm;

	forceAspect = _forceAspect;
	updateScreenshotLabel();
}

//click screenshot area to rotate dockwidgets
void Screenshot::rotateImage()
{
	QString objName = ((QWidget* )sender())->objectName();
	objName.remove(0, 6);	//remove "label_"

	//there's no API in Qt to access docked widget tabbar
	QList<QTabBar *> tabs = win->findChildren<QTabBar *>();
	foreach (QTabBar *tab, tabs)
	{
		bool isDock = false;

		// if the dock widget contains any of screenshot/history widgets
		for (int i = 0; i < win->dockCtrlNames.size(); i++)
		{
			if (MainWindow::tr(qPrintable(win->dockCtrlNames[i])) == tab->tabText(0))
			{
				isDock = true;
				break;
			}
		}

		// select the next tab
		if (isDock && MainWindow::tr(qPrintable(objName)) == tab->tabText(tab->currentIndex()))
		{
			int i = tab->currentIndex();
			if (++i > tab->count() - 1)
				i = 0;
			tab->setCurrentIndex(i);
			break;
		}
	}
}

void Screenshot::updateScreenshotLabel(bool isLoading)
{
	if (originalPixmap.isNull())
		return;

    QSize scaledSize = utils->getScaledSize(originalPixmap.size(), screenshotLabel->size(), forceAspect);

	screenshotLabel->setIconSize(scaledSize);
	QPixmap pm = originalPixmap.scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	if (isLoading)
	{
		QPainter p;
		p.begin(&pm);
		p.setPen(Qt::black);
		p.drawText(5, -3, pm.width(), pm.height(), Qt::AlignBottom, tr("Loading..."));
		p.setPen(Qt::white);
		p.drawText(4, -4, pm.width(), pm.height(), Qt::AlignBottom, tr("Loading..."));
		p.end();
	}
	
	screenshotLabel->setIcon(pm);
}
