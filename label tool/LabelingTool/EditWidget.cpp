#include "EditWidget.h"
#include <fstream>
#include "QSlim.h"

#define MAX_FACES 50000

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

	QVBoxLayout *vLayout = new QVBoxLayout();
	openButton = new QPushButton(tr("Load"));
	saveAsButton = new QPushButton(tr("Save As"));
	saveButton = new QPushButton(tr("Save"));
	copyButton = new QPushButton(tr("Copy"));
	clearButton = new QPushButton(tr("Clear"));
	generateButton = new QPushButton(tr("generateText"));
	addRegionButton = new QPushButton(tr("add new Region"));
	resetButton = new QPushButton(tr("reset all Region"));
	
	line1 = new QLineEdit;
	line2 = new QLineEdit;

	QLabel * label1 = new QLabel(tr("class label"));
	QLabel * label2 = new QLabel(tr("region number"));

	QTextEdit *explain = new QTextEdit;
	//explain->setStyleSheet("QTextEdit { background: grey }");
	QPalette pl = explain->palette();
	pl.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
	explain->setPalette(pl);
	explain->setFontPointSize(15);
	explain->setFixedSize(150, 240);
	explain->setLineWrapMode(QTextEdit::NoWrap);

	explain->append("sit\t 1");
	explain->append("rely\t 2");
	explain->append("grounding\t 3");
	explain->append("handput\t 4");
	explain->append("grasp\t 5");
	explain->append("pedal\t 6");
	explain->append("carry\t 7");
	explain->setReadOnly(true);
	explain->setDocumentTitle("class");
	

	vLayout->addWidget(openButton);
	vLayout->addWidget(saveAsButton);
	vLayout->addWidget(saveButton);
	vLayout->addSpacing(20);
	vLayout->addWidget(copyButton);
	vLayout->addWidget(clearButton);
	vLayout->addWidget(generateButton);
	vLayout->addSpacing(10);
	vLayout->addWidget(resetButton);
	vLayout->addWidget(addRegionButton);
	vLayout->addSpacing(10);
	vLayout->addWidget(label1);
	vLayout->addWidget(line1);
	vLayout->addSpacing(10);
	vLayout->addWidget(label2);
	vLayout->addWidget(line2);
	vLayout->addSpacing(10);
	vLayout->addWidget(explain);
	



	vLayout->addStretch();


	layout->addLayout(vLayout, 0, 0);
	layout->addWidget(glWidget, 0, 1);
	layout->addWidget(textBrowser, 0, 2);
	layout->setColumnStretch(1, 1);
	layout->setRowStretch(0, 1);

	setLayout(layout);

	connect(glWidget, SIGNAL(setText(const QString &)), textBrowser, SLOT(setText(const QString &)));
	connect(glWidget, SIGNAL(append(const QString &)), textBrowser, SLOT(append(const QString &)));
	connect(openButton, SIGNAL(clicked()), this, SLOT(load()));
	connect(saveAsButton, SIGNAL(clicked()), this, SLOT(saveAs()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveToFile()));
	connect(copyButton, SIGNAL(clicked()), this, SLOT(copyAll()));
	connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
	connect(generateButton, SIGNAL(clicked()), this, SLOT(generateText()));
	connect(addRegionButton, SIGNAL(clicked()), this, SLOT(addRegion()));
	connect(resetButton, SIGNAL(clicked()), this, SLOT(resetAll()));

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
			mm->write("temp.off");
		}
		loadFromFile = false;
	}
	else
		mm = new Mesh("temp.off");

	mm->id=0;
	//mm->needConnectionNet();
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
	mm->write(saveName.toStdString());
}

void EditWidget::save()
{
	mm->write("temp.off");
}

void EditWidget::copyAll()
{
	textBrowser->selectAll();
	textBrowser->copy();
}

void EditWidget::clear(){
	textBrowser->clear();
	glWidget->selectReset();
	glWidget->nowDrawMode = REGION_MODE;
}

void EditWidget::generateText(){

}

void EditWidget::addRegion(){
	int tempClassLabel = line1->text().toInt();
	int tempRegionNumber = line2->text().toInt();
	glWidget->addRegion(tempClassLabel, tempRegionNumber);
	glWidget->nowDrawMode = REGION_MODE;
}
void EditWidget::resetAll(){
	glWidget->resetAllRegion();
	glWidget->nowDrawMode = RENDER_MODE;
}

EditWidget::~EditWidget()
{
}
 