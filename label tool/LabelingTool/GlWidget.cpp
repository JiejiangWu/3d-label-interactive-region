#include "GlWidget.h"
#include <QtOpenGL>
#include <QDebug>
#include <GL/glu.h>
#include <stdarg.h>

const float PI2 = 2.0*3.1415926535f;								// PI Squared

#define MAX_SELECT_BUFFER 400000

GlWidget::GlWidget(QWidget *parent)
	:QOpenGLWidget(parent),
	_rx(0), _ry(0), _rz(0)
{
	_refreshTimer.setSingleShot(false);
	QObject::connect(
		&_refreshTimer, SIGNAL(timeout()),
		this, SLOT(update())
		);
	_refreshTimer.start(20);

	setAttribute(Qt::WA_NoSystemBackground);
	setMinimumSize(600, 600);
	//xOffset = 0;
	//yOffset = 0;
	//side = 600;
	nRange = 1.5f;
	setMouseTracking(true);
	
	selectBuf = new GLuint[MAX_SELECT_BUFFER];

	_arcBall = new ArcBallT(600, 600);
	
	MousePt.s.X = 0;
	MousePt.s.Y = 0;
	for (int i = 0; i < 16; i++)
		Transform.M[i] = 0.f;
	for (int i = 0; i < 16; i+=5)
		Transform.M[i] = 1.f;
	for (int i = 0; i < 9; i++)
	{
		LastRot.M[i] = 0.f;
		ThisRot.M[i] = 0.f;
	}
	for (int i = 0; i < 9; i += 4)
	{
		LastRot.M[i] = 1.f;
		ThisRot.M[i] = 1.f;
	}

	isSelectMode = false;
	isSelecting = false;
	isRotateMode = false;
	isDragMode = false;

	//selectTopMode = false;
	selectAreaMode = false;

	isClicked = false;
	firstDrag = true;
	deformed = false;

	curMesh = NULL;
	nowDrawMode = RENDER_MODE;
	sMode = 0;
}

GlWidget::~GlWidget()
{
	makeCurrent();

	QVectorIterator<Mesh*> cube(_meshes);
	while (cube.hasNext())
		delete cube.next();
}

void GlWidget::setModel(Mesh *m)
{
	_meshes.clear();
	_meshes.append(m);
	curMesh = m;
	m->selected = true;
}

void GlWidget::initializeGL()
{
	//qglClearColor(Qt::black);
	glClearColor(255.f, 255.f, 255.f, 255.f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);
}

void GlWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	resizeGL(width(), height());

	glEnable(GL_DEPTH_TEST);

	//qglClearColor(Qt::black);
	glClearColor(0.f, 0.f, 0.f, 255.f);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	static GLfloat lightPosition[4] = { 0.2, 0.3, 6.0, 3.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	if (isSelectMode == true)
		draw(nowDrawMode);
	else
		draw(nowDrawMode);
	glFlush();
}

void GlWidget::draw(int mode)
{
	glPushMatrix();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(255.0, 255.0, 255.0, 255.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glMultMatrixf(Transform.M);
	glTranslated(0.0, 0.0, -0.0);
	glRotated(_rx / 16.0, 1.0, 0.0, 0.0);
	glRotated(_ry / 16.0, 0.0, 1.0, 0.0);
	glRotated(_rz / 16.0, 0.0, 0.0, 1.0);
	QVectorIterator<Mesh*> mesh(_meshes);
	while (mesh.hasNext())
		mesh.next()->Draw(mode);

	if (isSelecting && mode==RENDER_MODE)
	{
		glPushMatrix();
		glLoadIdentity();
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0., 0., 0., 1.);
		glBegin(GL_LINE_LOOP);
		glVertex3f(((GLfloat)_currPos.x() * 2 / width() - 1.)*xScale*nRange, (1. - (GLfloat)_currPos.y() * 2 / height())*yScale*nRange, 0);
		glVertex3f(((GLfloat)_currPos.x() * 2 / width() - 1.)*xScale*nRange, (1. - (GLfloat)_lastPos.y() * 2 / height())*yScale*nRange, 0);
		glVertex3f(((GLfloat)_lastPos.x() * 2 / width() - 1.)*xScale*nRange, (1. - (GLfloat)_lastPos.y() * 2 / height())*yScale*nRange, 0);
		glVertex3f(((GLfloat)_lastPos.x() * 2 / width() - 1.)*xScale*nRange, (1. - (GLfloat)_currPos.y() * 2 / height())*yScale*nRange, 0);
		glEnd();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glPopMatrix();
	}

	glPopMatrix();
}

void GlWidget::resizeGL(int width, int height)
{
	//side = qMin(width, height);
	//xOffset = (width - side) >> 1;
	//yOffset = (height - side) >> 1;
	//glViewport(xOffset, yOffset, side, side);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(-1, +1, -1, +1, -1.0, 15.0);
	//glMatrixMode(GL_MODELVIEW);

	
	if (height == 0) {    // Prevent A Divide By Zero By  
		height = 1;    // Making Height Equal One  
	}
	glViewport(0, 0, width, height);    // Reset The Current Viewport  
	glMatrixMode(GL_PROJECTION);       // Select The Projection Matrix  
	glLoadIdentity();                  // Reset The Projection Matrix  

	xScale = (GLfloat)width / qMin(width, height);
	yScale = (GLfloat)height / qMin(width, height);
	glOrtho(-nRange*xScale, nRange*xScale, -nRange*yScale, nRange*yScale, -nRange, nRange);
	glMatrixMode(GL_MODELVIEW);      // Select The Modelview Matrix  
	glLoadIdentity();


	_arcBall->setBounds((GLfloat)width, (GLfloat)height);
}

void GlWidget::paintEvent(QPaintEvent */*event*/)
{
	QPainter painter;
	painter.begin(this);

	paintGL();

	painter.end();
}

void GlWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (!curMesh)
		return;

	isClicked = false;
	_currPos = event->pos();

	if (isRotateMode)
	{
		MousePt.s.X = (GLfloat)event->x();
		MousePt.s.Y = (GLfloat)event->y();
		Quat4fT     ThisQuat;
		_arcBall->drag(&MousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
		Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
		Matrix3fMulMatrix3f(&ThisRot, &LastRot);				// Accumulate Last Rotation Into This One
		Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);	// Set Our Final Transform's Rotation From This One
	}

	if (isDragMode)
	{
		_dragVec = _currPos - _lastPos;
		Eigen::Vector4d v2D(_dragVec.x(), -_dragVec.y(), 0, 1);
		v2D /= 0.5*qMin(width(), height());

		Eigen::Matrix4d T;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				T(i, j) = Transform.M[i * 4 + j];
		v3D = T*v2D;
		for (int i = 0; i < 3; i++)
			curMesh->dragVec(i) = v3D(i);
	}

	if (isSelectMode)
	{
		isSelecting = true;
		selectAreaMode = true;
	}

	update();

	//isClicked = false;
	//_currPos = event->pos();
	//MousePt.s.X = (GLfloat)event->x();
	//MousePt.s.Y = (GLfloat)event->y();

	////selectTopMode = false;
	//selectAreaMode = true;

	//if (isRotateMode && event->buttons() & Qt::LeftButton)
	//{
	//	Quat4fT     ThisQuat;

	//	_arcBall->drag(&MousePt, &ThisQuat);						// Update End Vector And Get Rotation As Quaternion
	//	Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);		// Convert Quaternion Into Matrix3fT
	//	Matrix3fMulMatrix3f(&ThisRot, &LastRot);				// Accumulate Last Rotation Into This One
	//	Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);	// Set Our Final Transform's Rotation From This One
	//}

	//if (isDragMode)
	//{
	//	_dragVec = _currPos - _lastPos;
	//	Eigen::Vector4d v2D(_dragVec.x(), -_dragVec.y(), 0, 1), v3D;
	//	v2D /= 0.5*qMin(width(), height());

	//	Eigen::Matrix4d T;
	//	for (int i = 0; i < 4; i++)
	//		for (int j = 0; j < 4; j++)
	//			T(i, j) = Transform.M[i * 4 + j];
	//	v3D = T*v2D;
	//	for (int i = 0; i < 3; i++)
	//		curMesh->dragVec(i) = v3D(i);
	//}

	//update();
}

void GlWidget::mousePressEvent(QMouseEvent *event)
{
	if (!curMesh)
		return;

	isSelectMode = false;
	isSelecting = false;
	isRotateMode = false;
	isDragMode = false;
	selectAreaMode = false;
	isClicked = true;
	curMesh->dragVec.setZero();

	_lastPos = event->pos();
	_currPos = event->pos();

	if (event->button() == Qt::LeftButton)
	{
		isRotateMode = true;
		MousePt.s.X = (GLfloat)event->x();
		MousePt.s.Y = (GLfloat)event->y();
		LastRot = ThisRot;										// Set Last Static Rotation To Last Dynamic One
		_arcBall->click(&MousePt);								// Update Start Vector And Prepare For Dragging
	}

	if (event->button() == Qt::MidButton)
	{
		prepareNewSelect();
		curMesh->tempSelectedF.clear();
		curMesh->tempSelectedF.resize(curMesh->F.rows(), false);
	}

	if (event->button() == Qt::RightButton)
	{
		isSelectMode = true;
		nowDrawMode = RENDER_MODE;
		tempSelectedFIdx.clear();
		tempDepth.clear();

	}


	//isClicked = true;
	//_lastPos = event->pos();
	//_currPos = event->pos();
	//MousePt.s.X = (GLfloat)event->x();
	//MousePt.s.Y = (GLfloat)event->y();

	//if (isRotateMode)
	//{
	//	LastRot = ThisRot;										// Set Last Static Rotation To Last Dynamic One
	//	_arcBall->click(&MousePt);								// Update Start Vector And Prepare For Dragging
	//}

	//if (event->button() == Qt::RightButton)
	//{
	//	isSelectMode = true;
	//	isSelecting = true;
	//}

	//if (event->button() == Qt::MidButton)
	//{
	//	isDragMode = true;
	//}
}

void GlWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (!curMesh)
		return;

	_currPos = event->pos();

	if (isDragMode)
	{
		if (curMesh->select1)
		{
			curMesh->applyHarmonicDeformation();
			curMesh->dragVec.setZero();
		}
		deformed = true;
		//outputAppend("%lf %lf %lf", v3D(0), v3D(1), v3D(2));
	}

	if (isSelectMode)
	{
		isSelecting = false;
		pick(event->pos());

	//	if (curMesh->select1 && !curMesh->select2)
		if (curMesh->select1)
		{
			QStringList s;
			for (int i = 0; i < curMesh->F.rows(); i++)
			{
				if (curMesh->selectedF[i])
					s.append(QString::number(i));
			}
			outputSet("#Number of faces in deform handle");
			outputAppend(QString::number(s.size()));
			outputAppend("#Index of faces");
			outputAppend(s.join('\n'));
			firstDrag = true;
		}
		/*if (curMesh->select2)
		{
			QStringList s;
			for (int i = 0; i < curMesh->F.rows(); i++)
			{
				if (curMesh->selectedF2[i])
					s.append(QString::number(i));
			}
			outputAppend("#Number of faces in deform area");
			outputAppend(QString::number(s.size()));
			outputAppend("#Index of faces");
			outputAppend(s.join('\n'));
		}*/
	}

	isSelectMode = false;
	isSelecting = false;
	isRotateMode = false;
	isDragMode = false;
	selectAreaMode = false;
	isClicked = true;

	//_currPos = event->pos();
	//isSelecting = false;
	//if (isClicked)
	//{
	//	//selectTopMode = true;
	//	selectAreaMode = false;
	//}

	//if (isSelectMode)
	//{
	//	pick(event->pos());
	//	QStringList s;
	//	for (int i = 0; i < curMesh->F.rows(); i++)
	//	{
	//		if (curMesh->selectedF[i])
	//			s.append(QString::number(i));
	//	}
	//	outputSet(QString::number(s.size()));
	//	outputAppend(s.join('\n'));
	//}

	//if (isDragMode)
	//{
	//	curMesh->applyHarmonicDeformation();
	//	curMesh->dragVec.setZero();
	//}
	//
	//_dragVec = QPoint(0, 0);

	//isDragMode = false;
	//isSelectMode = false;
	//isClicked = false;
}

void GlWidget::mouseDoubleClickEvent(QMouseEvent *event)
{

	//_currPos = event->pos();
	//if (event->button() == Qt::RightButton)
	//{
	//	curMesh->selectComponet();
	//	curMesh->needSelectedV();
	//}
}

void GlWidget::pick(const QPoint &pos)
{
	/*if (curMesh->select2 || deformed)
		curMesh->resetSelection();*/
	
	deformed = false;

	GLint hits;
	GLint viewport[4];
	viewport[0] = 0;
	viewport[1] = 0;
	viewport[2] = width();
	viewport[3] = height();
	//glGetIntegerv(GL_VIEWPORT, viewport);
	glSelectBuffer(MAX_SELECT_BUFFER, selectBuf);
	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//if (selectTopMode)
	//{
	//	qreal x = pos.x();
	//	qreal y = pos.y();
	//	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 1.f, 1.f, viewport);
	//}
	if (selectAreaMode)
	{
		qreal lx = _lastPos.x();
		qreal ly = _lastPos.y();
		qreal x = pos.x();
		qreal y = pos.y();
		//glGetIntegerv(GL_VIEWPORT, viewport);
		gluPickMatrix((GLdouble)((lx+x)/2), (GLdouble)(viewport[3] - (ly+y)/2), (GLdouble)qMax(qAbs(lx - x), 1.), (GLdouble)qMax(qAbs(ly - y), 1.), viewport);

		//gluPickMatrix(300, 300, 100, 100, viewport);
	}
	else
	{
		qreal x = pos.x();
		qreal y = pos.y();
		gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), 1.f, 1.f, viewport);
	}

	//glOrtho(-1, +1, -1, +1, -1.0, 1.0);
	glOrtho(-nRange*xScale, nRange*xScale, -nRange*yScale, nRange*yScale, -nRange, nRange);

	draw(SELECT_FACES_MODE);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	hits = glRenderMode(GL_RENDER);

	//On traite les sommets cliqués
	processHits(hits, selectBuf, sMode); //Fonction appelée ci-dessous

	glPopMatrix();

	update();
}

void GlWidget::processHits(GLint hits, GLuint buffer[],int mode)
{
	qDebug() << "Hits:" << hits;
	if (hits > 0)
	{
		bool s1;
		//if (!curMesh->select1)
		//{
		//	s1 = true;
		//	curMesh->select1 = true;
		//}
		//else
		//{
		//	s1 = false; 
		//	curMesh->select2 = true;
		//}
		curMesh->select1 = true;
		s1 = true;

		GLuint nb_names, name, *ptr;
		int n1 = -1;
		GLdouble z1, z2, zMin = 1000, zMax = -1;;

		qDebug() << "Hits:" << hits;

		

		ptr = (GLuint *)buffer;

		/*  store the hit faces index and depth  */
		for (GLint i = 0; i < hits; ++i)
		{
			nb_names = ptr[0];
			if (nb_names == 1)
			{
				z1 = (double)ptr[1] / 0x7fffffff;
				z2 = (double)ptr[2] / 0x7fffffff;
				tempDepth.push_back(z1);

				name = ptr[3];
				tempSelectedFIdx.push_back(name);

				//if (selectAreaMode)
				//{
				//	curMesh->selectedF[name] = true;
				//	if (mode == 1) // 
				//		curMesh->accumulate_s[name] = true;
				////}
				if (z1 < zMin)
				{
					zMin = z1;
					n1 = name;
				}
				if (z1 > zMax)
				{
					zMax = z1;
				}
				selectDepth = zMax;

				emit(depthChanged(zMax, zMin));
				//qDebug() << name;
			}
			ptr += 3 + nb_names;
		}

		//处理所有框选内面片
		for (int i = 0; i < tempSelectedFIdx.size(); i++){
			if (selectAreaMode){
				updateDepthSelect();
			}
			else{
				/*if (mode == 0)*/
					curMesh->tempSelectedF[n1] = true;
				/*if (mode == 1)
					curMesh->accumulate_s[n1] = false;*/
			}
		}
		//
		//if (n1 >= 0)
		//{
		//	if (s1)
		//	{
		//		curMesh->selectedF[n1] = true;
		//		curMesh->pickedF = n1;

		//		if (selectAreaMode && mode == 0)// right button
		//			curMesh->selectTopComponet();

		//		curMesh->needSelectedV();
		//	}
		//	else
		//	{
		//		curMesh->selectedF2[n1] = true;
		//		curMesh->pickedF = n1;

		//		if (selectAreaMode)
		//			curMesh->selectTopComponet2();

		//		curMesh->needSelectedV2();
		//	}
		//}
	}
	else
	{
		/*if (!selectAreaMode)
			curMesh->resetSelection();*/
	}
}

int GlWidget::normalizeAngle(int angle)
{
	while (angle < 0) angle += (360 << 4);
	while (angle >(360 << 4)) angle -= (360 << 4);
	return angle;
}

void GlWidget::outputAppend(const char * f, ...)
{
	QString s;
	va_list args;
	va_start(args, f);
	emit(append(QString().vsprintf(f, args)));
	va_end(args);
}

void GlWidget::outputAppend(const QString &s)
{
	emit(append(s));
}

void GlWidget::outputSet(const QString &s)
{
	emit(setText(s));
}

void GlWidget::selectReset(){
	curMesh->resetSelection();
}

void GlWidget::addRegion(int label,int no){
	curMesh->addregion(label, no);
	curMesh->resetSelection();
}

void GlWidget::resetAllRegion(){
	curMesh->resetLabelAndRegion();
	curMesh->resetSelection();
}

void GlWidget::prepareNewSelect(){
	// save this time's faces

	if (sMode == 0) // select
		for (int i = 0; i < curMesh->F.rows(); i++)
			curMesh->accumulate_s[i] = curMesh->accumulate_s[i] || curMesh->tempSelectedF[i];
	if (sMode == 1) // deselect
		for (int i = 0; i < curMesh->F.rows(); i++)
			curMesh->accumulate_s[i] = curMesh->accumulate_s[i] && (!curMesh->tempSelectedF[i]);
	tempSelectedFIdx.clear();
	tempDepth.clear();
}

void GlWidget::updateDepthSelect(){
	curMesh->tempSelectedF.clear();
	curMesh->tempSelectedF.resize(curMesh->F.rows(), false);

	for (int i = 0; i < tempSelectedFIdx.size(); i++){
		if (tempDepth[i] <= selectDepth){ // select faces in depth
			curMesh->tempSelectedF[tempSelectedFIdx[i]] = true;
		}
	}
}