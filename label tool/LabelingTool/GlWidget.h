#ifndef __GLWIDGET_H
#define __GLWIDGET_H

#include <QOpenGLWidget>
#include <QTimer>
#include <Mesh.h>
#include <ArcBall.h>


class GlWidget : public QOpenGLWidget
{
	Q_OBJECT

protected:
	QTimer _refreshTimer;
	QVector<Mesh*> _meshes;
	Mesh *curMesh;
	QPoint _lastPos, _currPos,_dragVec;
	Eigen::Vector4d v3D;
	int _rx;
	int _ry;
	int _rz;
	int _dx;
	int _dy;
	int _dz;
	ArcBallT *_arcBall;
	Point2fT MousePt;
	Matrix4fT Transform;
	Matrix3fT LastRot;
	Matrix3fT ThisRot;
	GLuint *selectBuf;
	//int xOffset, yOffset, side;
	GLfloat nRange,xScale,yScale;

	bool isSelectMode,isDragMode, isRotateMode,selectAreaMode,isSelecting,deformed,firstDrag;
	bool isClicked;
	
	QVector <int> tempSelectedFIdx;
	QVector <GLdouble> tempDepth;


public:
	GlWidget(QWidget *parent = NULL);
	virtual ~GlWidget();

	static int normalizeAngle(int angle);
	void setModel(Mesh *);
	void selectReset();
	void addRegion(int label, int no);
	void resetAllRegion();

	void prepareNewSelect();
	void updateDepthSelect();

	int nowDrawMode;

	int sMode;  // 0 ,select  1, deselect
	GLdouble minDepth, maxDepth, selectDepth;

signals:
	void append(const QString &);
	void setText(const QString &);
	void depthChanged(GLdouble max, GLdouble min);

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int width, int height);
	virtual void paintEvent(QPaintEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent * event);
	virtual void pick(const QPoint &pos);
	virtual void processHits(GLint hits, GLuint buffer[], int mode);
	virtual void draw(int mode = RENDER_MODE);
	void outputSet(const QString &);
	void outputAppend(const QString &);
	void outputAppend(const char * f, ...);
};

#endif
