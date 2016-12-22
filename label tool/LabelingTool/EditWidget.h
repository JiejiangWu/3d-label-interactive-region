#pragma once
#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QTextBrowser>
#include "Mesh.h"
#include "GlWidget.h"
#include "qdebug.h"


class EditWidget :
	public QWidget
{
	Q_OBJECT
public:
	EditWidget();
	~EditWidget();

protected:

	GlWidget *glWidget;
	QTextBrowser *textBrowser;
	Mesh *mm;
	QPushButton *openButton, *saveAsButton, *saveButton, *clearButton, *PreButton, *NextButton, *addRegionButton, *resetButton;
	QRadioButton* mouseRightButtonMode[2];
	QButtonGroup*  group;
	QSlider *selectDepth;
	QLabel *cntLabel;

	QLineEdit *line1, *line2;
	bool loadFromFile;
	QString saveName, filePath,fileName;
	QString tempModelName, tempModelID, finfoName, finfoPath;
	GLdouble zmin, zmax;
	QString modelFolder, labelFolder;
	QFileInfoList modelList;
	QFileInfoList::iterator modelIter;
	int modelCnt, modelIdx;

	int rightButtonState;  // 0 - select, 1 - deselect

	void init();
	void save();
	void reloadModel();
	void normalLoad();

	void getptsLabel();
	void writePTS();
	void writePInfo();
	void writeFInfo();


public slots:
	void load();
	void saveAs();
	void saveToFile();
	void PreModel();
	void clear();
	void NextModel();
	void addRegion();
	void resetAll();
	void changeRightButtonMode1();
	void changeRightButtonMode2();
	void updateDepthMaxMin(GLdouble max, GLdouble min);
	void changeSelectDepth();

};

