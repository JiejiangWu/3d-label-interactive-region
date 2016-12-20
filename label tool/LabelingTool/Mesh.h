#pragma once
#include <QObject>
#include <QtOpenGL>
#include <Eigen/Eigen>
#include <set>
#include <igl/readOFF.h>
#include <igl/writeOFF.h>

//#define FACE_ITEM 0
//#define VERTEX_ITEM 1

#define RENDER_MODE 0
#define SELECT_MODEL_MODE 1
#define SELECT_FACES_MODE 2
#define REGION_MODE 3

class Mesh :
	public QObject
{
	Q_OBJECT

public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	struct bSphare
	{
		Eigen::Vector3d c;
		double r;
		bool valid=false;

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	Mesh(std::string filename, QObject *parent = NULL);
	~Mesh();
	//Eigen::Matrix<float, -1, -1, Eigen::RowMajor>
	Eigen::MatrixXd V;
	Eigen::MatrixXi F;
	Eigen::MatrixXd C;
	Eigen::MatrixXd N_vertices;
	Eigen::MatrixXd N_faces;
	Eigen::Matrix<float, 4, 4, Eigen::RowMajor> Transform;
	bSphare bs;

	std::vector<std::set<int>> F_Fset, V_Fset, V_Vset;
	std::vector<bool> selectedV, selectedF, deletedV, deletedF, selectedV2, selectedF2;
	std::vector<bool> accumulate_s;
	std::vector<int> class_label;
	std::vector<int> region_number;
	Eigen::Vector3d dragVec;

	int id, pickedF;
	bool selected, haveConnectionNet, select1, select2;

	static GLint _colorList[14][3];

	void resetSelection();
	void resetSelection2();

	void needConnectionNet();
	void needSelectedV();
	void needSelectedV2();
	bool FNextToF(int, int);
	void selectComponet();
	void selectTopComponet();
	void selectTopComponet2();
	void needNormals();
	void needBoundingSphare();
	static void calcBoundingSphare(const Eigen::MatrixXd &v,bSphare &bs);
	void write(std::string filename);
	void applyChanges();
	void applyNormalize();
	void applyHarmonicDeformation();
	void Draw(int mode = RENDER_MODE) const;

	void resetLabelAndRegion();
	void addregion(int label, int no);
};

