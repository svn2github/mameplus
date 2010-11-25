#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_

#include <QtGui>

class Screenshot : public QDockWidget
{
    Q_OBJECT

public:
    Screenshot(QString, QWidget *parent = 0);
	void setPixmap(QPixmap pm);
	void setPixmap(const QByteArray &, bool);
    void updateScreenshotLabel(bool = false);

protected:
    void resizeEvent(QResizeEvent *);

private slots:
	void rotateImage();

private:
	QPushButton *screenshotLabel;
	QPixmap originalPixmap;
	QGridLayout *mainLayout;
	QWidget *dockWidgetContents;
	bool forceAspect;
};

#endif
