 #ifndef SCANNINGANDFUSION_H
#define SCANNINGANDFUSION_H

#include <QtWidgets/QMainWindow>
#include <qdebug.h>
#include <qtabwidget.h>
#include "EditWidget.h"
#include <QAction>
#include <QMenu>


class ScanningAndFusion : public QMainWindow
{
	Q_OBJECT

public:
	ScanningAndFusion(QWidget *parent = 0);
	~ScanningAndFusion();

private:
	EditWidget *editWidget;
	QAction *actionOpen, *actionSave, *actionSaveAs, *actionExit;
	QMenu *menuFile,*menuAbout;
};

#endif // SCANNINGANDFUSION_H
