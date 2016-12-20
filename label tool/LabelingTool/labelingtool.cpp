#include "labelingtool.h"
#include "qdebug.h"

ScanningAndFusion::ScanningAndFusion(QWidget *parent)
	: QMainWindow(parent)
{
	editWidget = new EditWidget();
	
	actionOpen = new QAction(tr("&Open"), this);
	actionSave = new QAction(tr("&Save"), this);
	actionSaveAs = new QAction(tr("Save As"), this);
	actionExit = new QAction(tr("&Exit"), this);

	menuFile = menuBar()->addMenu(tr("&File"));
	menuFile->addAction(actionOpen);
	menuFile->addAction(actionSave);
	menuFile->addAction(actionSaveAs);
	menuFile->addAction(actionExit);
	menuAbout = menuBar()->addMenu(tr("&About"));

	connect(actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(actionOpen, SIGNAL(triggered()), editWidget, SLOT(load()));
	connect(actionSave, SIGNAL(triggered()), editWidget, SLOT(saveToFile()));
	connect(actionSaveAs, SIGNAL(triggered()), editWidget, SLOT(saveAs()));

	setCentralWidget(editWidget);
	setWindowTitle("Labeling Tool");
} 

ScanningAndFusion::~ScanningAndFusion()
{
	 
}
