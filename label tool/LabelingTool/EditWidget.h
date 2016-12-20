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
	QPushButton *openButton, *saveAsButton, *saveButton, *clearButton, *copyButton, *generateButton, *addRegionButton, *resetButton;
	QLineEdit *line1, *line2;
	bool loadFromFile;
	QString saveName;

	void init();
	void save();

public slots:
	void load();
	void saveAs();
	void saveToFile();
	void copyAll();
	void clear();
	void generateText();
	void addRegion();
	void resetAll();
};

