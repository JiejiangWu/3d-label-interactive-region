#include "EditWidget.h"
#include <fstream>
#include "QSlim.h"

#define MAX_FACES 20000

using namespace std;

static void qslim_init()
{
	int i;
	qDebug() << "Reading input ..." << endl;
	qDebug() << "Cleaning up initial input ..." << endl;
	int initialVertCount = M0.vertCount();
	int initialEdgeCount = M0.edgeCount();
	int initialFaceCount = M0.faceCount();
	for (i = 0; i<M0.faceCount(); i++)
		if (!M0.face(i)->plane().isValid())
			M0.killFace(M0.face(i));
	M0.removeDegeneracy(M0.allFaces());
	for (i = 0; i<M0.vertCount(); i++)
	{
		if (M0.vertex(i)->edgeUses().length() == 0)
			M0.vertex(i)->kill();
	}
	qDebug() << "Input model summary:" << endl;
	qDebug() << "    Vertices    : " << initialVertCount << endl;
	qDebug() << "    Edges       : " << initialEdgeCount << endl;
	int man = 0, non = 0, bndry = 0, bogus = 0;
	for (i = 0; i<M0.edgeCount(); i++)
		switch (M0.edge(i)->faceUses().length())
	{
		case 0:
			bogus++;
			break;
		case 1:
			bndry++;
			break;
		case 2:
			man++;
			break;
		default:
			non++;
			break;
	}
	if (bogus)
		qDebug() << "        Bogus       : " << bogus << endl;
	qDebug() << "        Boundary    : " << bndry << endl;
	qDebug() << "        Manifold    : " << man << endl;
	qDebug() << "        Nonmanifold : " << non << endl;

	qDebug() << "    Faces       : " << initialFaceCount << endl;
}
static void qslim_run()
{
	decimate_init(M0, pair_selection_tolerance);
	while (M0.validFaceCount > face_target&& decimate_min_error() < error_tolerance)
		decimate_contract(M0);
	qDebug() << "Done!" << endl;
}
static void InitM0(Mesh *m)
{
	for (size_t i = 0; i<m->V.rows(); i++)
	{
		Vec3 v(m->V(i,0), m->V(i,1), m->V(i,2));
		M0.in_Vertex(v);
	}
	for (size_t i = 0; i<m->F.rows(); i++)
	{
		M0.in_Face(m->F(i,0), m->F(i,1), m->F(i,2));
	}
}
static void ReplaceM(Mesh *m)
{
	int vCnt = 0, fCnt = 0;
	for (int i = 0; i < M0.vertCount(); i++)
	{
		if (M0.vertex(i)->isValid())
			vCnt++;
	}
	for (int i = 0; i < M0.faceCount(); i++)
	{
		if (M0.face(i)->isValid())
			fCnt++;
	}
	m->V.resize(vCnt, 3);
	m->F.resize(fCnt, 3);
	int* map = new int[M0.vertCount()];
	int t = 0;
	for (int i = 0; i<M0.vertCount(); i++)
		map[i] = -1;
	for (int i = 0; i<M0.vertCount(); i++)
	{
		if (M0.vertex(i)->isValid())
		{
			map[i] = t;
			double* data = M0.vertex(i)->raw();
			for (int j = 0; j < 3;j++)
				m->V(t,j)=data[j];
			t++;
		}
	}
	t = 0;
	for (int i = 0; i<M0.faceCount(); i++)
	{
		if (M0.face(i)->isValid())
		{
			for (int j = 0; j < 3; j++)
				m->F(t, j) = map[M0.face(i)->vertex(j)->uniqID];
			t++;
		}
	}
	delete[] map;
}

EditWidget::EditWidget()
{
	loadFromFile = true;
	QGridLayout *layout = new QGridLayout();

	glWidget = new GlWidget();

	textBrowser = new QTextBrowser();

	//按钮们
	QVBoxLayout *vLayout = new QVBoxLayout();
	openButton = new QPushButton(tr("Load"));
	saveButton = new QPushButton(tr("Save"));

	PreButton = new QPushButton(tr("Pre Model"));
	NextButton = new QPushButton(tr("Next Model"));

	clearButton = new QPushButton(tr("clear Selected faces"));
	addRegionButton = new QPushButton(tr("add new Region"));
	resetButton = new QPushButton(tr("reset all Region"));

	//右键模式 选取\去选取
	QString RightButtonMode[] = { "select", "deselect"};
	mouseRightButtonMode[0] = new QRadioButton(RightButtonMode[0], this);
	mouseRightButtonMode[1] = new QRadioButton(RightButtonMode[1], this);
	group = new QButtonGroup;
	group->addButton(mouseRightButtonMode[0]);
	group->addButton(mouseRightButtonMode[1]);
	mouseRightButtonMode[0]->setChecked(true);
	
	//选取深度滑动条
	selectDepth = new QSlider(Qt::Horizontal);
	selectDepth->setMinimum(0);
	selectDepth->setMaximum(1000);
	selectDepth->setValue(500);

	//类别、区域号码输入框
	line1 = new QLineEdit;
	line1->setValidator(new QIntValidator(1, glWidget->classNames.size(), this));
	line2 = new QLineEdit;
	line2->setValidator(new QIntValidator(1, 500, this));

	QLabel * label1 = new QLabel(tr("class label"));
	QLabel * label2 = new QLabel(tr("region number"));


	// 类别注释
	QTextEdit *explain = new QTextEdit;
	QPalette pl = explain->palette();
	pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
	explain->setPalette(pl);
	explain->setFontPointSize(15);
	explain->setFixedSize(150, 240);
	explain->setLineWrapMode(QTextEdit::NoWrap);

	for (int i = 0; i < glWidget->classNames.size(); i++){
		QString className = glWidget->classNames[i];
		className = className + "\t" + QString::number(i + 1);
		explain->append(className);
	}
	explain->setReadOnly(true);
	explain->setDocumentTitle("class");
	

	vLayout->addWidget(openButton);
	vLayout->addWidget(saveButton);

	vLayout->addSpacing(10);
	vLayout->addWidget(PreButton);
	vLayout->addWidget(NextButton);
	vLayout->addSpacing(10);
	vLayout->addWidget(mouseRightButtonMode[0]);
	vLayout->addWidget(mouseRightButtonMode[1]);
	vLayout->addWidget(selectDepth);
	vLayout->addSpacing(10);
	vLayout->addWidget(clearButton);
	vLayout->addWidget(resetButton);
	vLayout->addWidget(addRegionButton);
	vLayout->addSpacing(10);
	vLayout->addWidget(label1);
	vLayout->addWidget(line1);
	vLayout->addSpacing(10);
	//vLayout->addWidget(label2);
	//vLayout->addWidget(line2);
	vLayout->addSpacing(10);
	vLayout->addWidget(explain);
	



	vLayout->addStretch();


	layout->addLayout(vLayout, 0, 0);
	layout->addWidget(glWidget, 0, 1);
	layout->addWidget(textBrowser, 0, 2);
	layout->setColumnStretch(1, 1);
	layout->setRowStretch(0, 1);

	setLayout(layout);

	glWidget->setFocusPolicy(Qt::StrongFocus);

	connect(glWidget, SIGNAL(setText(const QString &)), textBrowser, SLOT(setText(const QString &)));
	connect(glWidget, SIGNAL(append(const QString &)), textBrowser, SLOT(append(const QString &)));
	connect(openButton, SIGNAL(clicked()), this, SLOT(load()));
	
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveToFile()));
	connect(PreButton, SIGNAL(clicked()), this, SLOT(PreModel()));
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(NextButton, SIGNAL(clicked()), this, SLOT(NextModel()));
	connect(addRegionButton, SIGNAL(clicked()), this, SLOT(addRegion()));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAll()));
	connect(mouseRightButtonMode[0], SIGNAL(pressed()), this, SLOT(changeRightButtonMode1()));
	connect(mouseRightButtonMode[1], SIGNAL(pressed()), this, SLOT(changeRightButtonMode2()));
	connect(glWidget, SIGNAL(depthChanged(GLdouble, GLdouble)), this, SLOT(updateDepthMaxMin(GLdouble, GLdouble)));
	connect(selectDepth, SIGNAL(valueChanged(int)), this, SLOT(changeSelectDepth()));

	mm = NULL;

}

void EditWidget::load()
{
	loadFromFile = true;
	init();
}

void EditWidget::init()
{
	if (loadFromFile)
	{
		QString path = QFileDialog::getOpenFileName(this, tr("Open 3D Object"), ".", tr("3D Object Files(*.ply *.ray *.obj *.off *.sm *.smf *.stl *.dae)"));
		if (path.length() == 0)
			return;
		saveName = path;
		QFileInfo fi = QFileInfo(saveName);
		
		fileName = fi.fileName();
		filePath = fi.absolutePath();
		
		mm = new Mesh(path.toStdString());
		if (mm->F.rows() > MAX_FACES)
		{
			InitM0(mm);
			qslim_init();
			face_target = MAX_FACES;
			error_tolerance = HUGE;
			will_use_plane_constraint = true;
			will_use_vertex_constraint = false;
			will_preserve_boundaries = true;
			will_preserve_mesh_quality = true;
			will_constrain_boundaries = true;
			boundary_constraint_weight = 1.0;
			will_weight_by_area = false;
			placement_policy = 1;
			pair_selection_tolerance = 0.0;
			qslim_run();
			ReplaceM(mm);
			mm->resetSelection();
			mm->write(saveName.toStdString());
		}
		loadFromFile = false;
	}
	else
		mm = new Mesh("temp.off");

	mm->id=0;
	mm->applyNormalize();
	mm->needNormals();
	glWidget->setModel(mm);
}

void EditWidget::saveAs()
{
	QString path = QFileDialog::getSaveFileName(this, tr("Save 3D Object"), saveName, tr("3D Object Files(*.ply *.ray *.obj *.off *.sm *.smf *.stl *.dae)"));
	if (path.length() == 0)
		return;
	saveName = path;
	save();
	mm->write(saveName.toStdString());
}

void EditWidget::saveToFile()
{
	save();
	//mm->write(saveName.toStdString());


}

void EditWidget::save()
{
	if (glWidget->curMesh != NULL){
		getptsLabel();
		writeFInfo();
		writePTS();
		writePInfo();
	}
}

void EditWidget::PreModel()
{

}



void EditWidget::NextModel(){

}

void EditWidget::clear(){
	textBrowser->clear();
	glWidget->selectReset();
	glWidget->nowDrawMode = REGION_MODE;
}

void EditWidget::addRegion(){
	int tempClassLabel = line1->text().toInt();
	//int tempRegionNumber = line2->text().toInt();
	glWidget->RegionCount++;
	glWidget->addRegion(tempClassLabel, glWidget->RegionCount);
	glWidget->nowDrawMode = REGION_MODE;
	glWidget->outputInfo();
}
void EditWidget::resetAll(){
	glWidget->resetAllRegion();
	glWidget->nowDrawMode = RENDER_MODE;
	glWidget->outputInfo();
}

void EditWidget::changeRightButtonMode1(){
	rightButtonState = 0;
	glWidget->sMode = rightButtonState;
	glWidget->outputInfo();
}

void EditWidget::changeRightButtonMode2(){
	rightButtonState = 1;
	glWidget->sMode = rightButtonState;
	glWidget->outputInfo();
}

void EditWidget::changeSelectDepth(){
	glWidget->selectDepth = selectDepth->value() / 1000.0 * (zmax - zmin) + zmin;
	qDebug() << "selecting: zmax" << zmax << "  zmin" << zmin << "  depth" << glWidget->selectDepth;

	glWidget->updateDepthSelect();
	glWidget->update();
	glWidget->outputInfo();
}

void EditWidget::updateDepthMaxMin(GLdouble max, GLdouble min){
	
	//selectDepth->setMinimum(min);
	//selectDepth->setMaximum(max);
	
	zmin = min;
	zmax = max;
	selectDepth->setValue(1000);
}


void EditWidget::getptsLabel(){
	glWidget->curMesh->pointLabel.clear();
	glWidget->curMesh->pointLabel.resize(glWidget->curMesh->V.rows(), 0);
	glWidget->curMesh->pointRegion.clear();
	glWidget->curMesh->pointRegion.resize(glWidget->curMesh->V.rows(), 0);
	
	for (int i = 0; i < glWidget->curMesh->F.rows(); i++){
		for (int k = 0; k < 3; k++){
			glWidget->curMesh->pointLabel[glWidget->curMesh->F(i, k)] = glWidget->curMesh->class_label[i];
			glWidget->curMesh->pointRegion[glWidget->curMesh->F(i, k)] = glWidget->curMesh->region_number[i];
		}
	}
}

void EditWidget::writeFInfo(){
	QString FInfoFile;
	FInfoFile.resize(fileName.size() - 4);
	for (int i = 0; i <= fileName.size() - 4; i++){
		FInfoFile[i] = fileName[i];
	}
	FInfoFile.append("finfo");
	QFile f(filePath + "/labelInfo/finfo/" + FInfoFile);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		cout << "Open failed." << endl;
		return;
	}

	QTextStream txtOutput(&f);
	for (int i = 0; i < glWidget->curMesh->F.rows(); i++){
		txtOutput << glWidget->curMesh->region_number[i] << " " << glWidget->curMesh->class_label[i] << endl;
	}
	f.close();
}

void EditWidget::writePTS(){
	QString PTSFile;
	PTSFile.resize(fileName.size() - 4);
	for (int i = 0; i <= fileName.size() - 4; i++){
		PTSFile[i] = fileName[i];
	}
	PTSFile.append("pts");
	QFile f(filePath + "/labelInfo/pts/" + PTSFile);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		cout << "Open failed." << endl;
		return;
	}

	QTextStream txtOutput(&f);
	for (int i = 0; i < glWidget->curMesh->V.rows(); i++){
		if (glWidget->curMesh->pointRegion[i] != 0)
			txtOutput << glWidget->curMesh->V(i, 0) << " " << glWidget->curMesh->V(i, 1) << " " << glWidget->curMesh->V(i, 2) << endl;
	}
	f.close();
}

void EditWidget::writePInfo(){
	QString PInfoFile;
	PInfoFile.resize(fileName.size() - 4);
	for (int i = 0; i <= fileName.size() - 4; i++){
		PInfoFile[i] = fileName[i];
	}
	PInfoFile.append("pinfo");
	QFile f(filePath + "/labelInfo/pinfo/" + PInfoFile);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		cout << "Open failed." << endl;
		return;
	}

	QTextStream txtOutput(&f);
	for (int i = 0; i < glWidget->curMesh->V.rows(); i++){
		if (glWidget->curMesh->pointRegion[i] != 0)
			txtOutput<< glWidget->curMesh->pointRegion[i] << " " << glWidget->curMesh->pointLabel[i] << endl;
	}
	f.close();
}

EditWidget::~EditWidget()
{
}
 