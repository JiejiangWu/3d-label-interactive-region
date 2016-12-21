#include "Mesh.h"
#include <qdebug.h>
#include <queue>
#include <algorithm>
#include <igl/per_vertex_normals.h>
#include <igl/per_face_normals.h>
#include <igl/harmonic.h>
#include <igl/colon.h>
#include <igl/mat_max.h>
#include <igl/mat_min.h>
#include <igl/slice.h>


GLint Mesh::_colorList[14][3] = {
	{ 255, 255, 255 }, //白色

{ 220, 20, 60 }, //浅粉红
{ 255, 215, 0 }, //金色
{ 128, 0, 128 }, //紫色
{ 165, 42, 42 }, //棕色
{ 0, 0, 205 }, //中蓝色
{ 160, 82, 45 }, //黄土赭色
{ 0, 196, 255 }, //天蓝色

{ 0, 255, 127 }, //春绿色
{ 255, 165, 0 }, //橙色
{ 128, 128, 128 },
{ 243, 208, 177 },//肉色
{ 215, 186, 130 },//中间色
{ 196, 196, 196 } //无分割信息情况下默认显示颜色
};

Mesh::Mesh(std::string filename, QObject *parent)
{
	igl::readOFF(filename, V, F);
	C = Eigen::RowVector3d(1.0, 1.0, 1.0).replicate(V.rows(), 1);
	needNormals();
	deletedV.resize(V.rows(), false);
	deletedF.resize(F.rows(), false);
	resetSelection();
	resetLabelAndRegion();
	id = 0;
	haveConnectionNet = false;
	Transform.setIdentity();
	dragVec.setZero();
}

Mesh::~Mesh()
{
}

void Mesh::write(std::string filename)
{
	igl::writeOFF(filename, V, F);
}

void Mesh::resetSelection()
{
	pickedF = -1;
	selectedV.clear();
	selectedF.clear();
	selectedV.resize(V.rows(), false);
	selectedF.resize(F.rows(), false);
	selectedV2.clear();
	selectedF2.clear();
	selectedV2.resize(V.rows(), false);
	selectedF2.resize(F.rows(), false);
	accumulate_s.clear();
	accumulate_s.resize(F.rows(), false);
	tempSelectedF.clear();
	tempSelectedF.resize(F.rows(), false);

	deselectedF.clear();
	deselectedF.resize(F.rows(), false);
	select1 = false;
	select2 = false;
}

void Mesh::resetSelection2()
{
	pickedF = -1;
	selectedV2.clear();
	selectedF2.clear();
	selectedV2.resize(V.rows(), false);
	selectedF2.resize(F.rows(), false);
	select2 = false;
}

void Mesh::needNormals()
{
	igl::per_face_normals(V, F, N_faces);
	igl::per_vertex_normals(V, F, N_vertices);
}

void Mesh::selectComponet()
{
	if (pickedF < 0 || pickedF >= F.rows())
		return;
	//resetSelection();
	std::queue<int> q;
	int t;
	while (!q.empty())
		q.pop();
	q.push(pickedF);
	selectedF[pickedF] = true;
	while (!q.empty())
	{
		t = q.front();
		q.pop();
		for (std::set<int>::iterator i = F_Fset[t].begin(); i != F_Fset[t].end(); i++)
		{
			if (selectedF[*i])
				continue;
			q.push(*i);
			selectedF[*i] = true;
		}
	}
	//pickedF = -1;
}

void Mesh::selectTopComponet()
{
	if (pickedF < 0 || pickedF >= F.rows())
		return;
	//resetSelection();
	std::queue<int> q;
	std::vector<bool> s;
	s.resize(F.rows(), false);
	

	int t;
	while (!q.empty())
		q.pop();
	q.push(pickedF);
	s[pickedF] = true;
	while (!q.empty())
	{
		t = q.front();
		q.pop();
		for (std::set<int>::iterator i = F_Fset[t].begin(); i != F_Fset[t].end(); i++)
		{
			if (s[*i])
				continue;
			if (!selectedF[*i])
				continue;
			q.push(*i);
			s[*i] = true;
		}
	}
	/*for (int i = 0; i < F.rows(); i++)
	{
		selectedF[i] = s[i];
	}*/
	for (int i = 0; i < F.rows(); i++){
		accumulate_s[i] = s[i] || accumulate_s[i];
		selectedF[i] = accumulate_s[i];
	}
	//for (int i = 0; i < F.rows(); i++)
	//{
	//	selectedF[i] = accumulate_s[i];
	//}
	//pickedF = -1;
}

void Mesh::selectTopComponet2()
{
	if (pickedF < 0 || pickedF >= F.rows())
		return;
	//resetSelection();
	std::queue<int> q;
	std::vector<bool> s;
	s.resize(F.rows(), false);

	int t;
	while (!q.empty())
		q.pop();
	q.push(pickedF);
	s[pickedF] = true;
	while (!q.empty())
	{
		t = q.front();
		q.pop();
		for (std::set<int>::iterator i = F_Fset[t].begin(); i != F_Fset[t].end(); i++)
		{
			if (s[*i])
				continue;
			if (!selectedF2[*i])
				continue;
			q.push(*i);
			s[*i] = true;
		}
	}
	for (int i = 0; i < F.rows(); i++)
	{
		selectedF2[i] = s[i];
	}
	//pickedF = -1;
}


void Mesh::Draw(int mode) const
{
	glPushMatrix();

	if (mode == SELECT_MODEL_MODE)
	{
		glLoadName(id);
	}

	QColor _color;

	GLfloat diffuseColor[4] = { 1.0, 1.0, 1.0, 0.0 };

	_color = QColor("red");
	GLfloat selectionColor[4] =
	{ _color.redF(), _color.greenF(), _color.blueF(), _color.alphaF() };

	_color = QColor("blue");
	GLfloat deformationColor[4] =
	{ _color.redF(), _color.greenF(), _color.blueF(), _color.alphaF() };

	GLfloat middleColor[4] =
	{ (selectionColor[0] + diffuseColor[0]) / 2, (selectionColor[1] + diffuseColor[1]) / 2, (selectionColor[2] + diffuseColor[2]) / 2, 0.0 };

	int count = F.rows();

	if (mode == RENDER_MODE || mode == REGION_MODE)
		glBegin(GL_TRIANGLES);
	for (int i = 0; i < count; ++i)
	{
		if (deletedF[i])
			continue;
		if (mode == SELECT_FACES_MODE)
		{
			glLoadName(i);
			glBegin(GL_TRIANGLES);
		}
		glNormal3f(N_faces(i, 0), N_faces(i, 1), N_faces(i, 2));
		for (int k = 0; k < 3; k++)
		{
			if (mode == RENDER_MODE){
				if (accumulate_s[i])
				{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, selectionColor);
				}
				else if (tempSelectedF[i]){
					glMaterialfv(GL_FRONT, GL_DIFFUSE, middleColor);
				}
				else
				{
					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
				}
			}
			else{
				GLfloat tempColor[4];
				tempColor[0] = _colorList[class_label[i]][0] / 255.0;
				tempColor[1] = _colorList[class_label[i]][1] / 255.0;
				tempColor[2] = _colorList[class_label[i]][2] / 255.0;
				tempColor[3] = 0.0;
				glMaterialfv(GL_FRONT, GL_DIFFUSE, tempColor);
			}
			if (selectedV[F(i, k)])
				glVertex3f(V(F(i, k), 0) + dragVec(0), V(F(i, k), 1) + dragVec(1), V(F(i, k), 2) + dragVec(2));
			else
				glVertex3f(V(F(i, k), 0), V(F(i, k), 1), V(F(i, k), 2));
		}
		//if (selectedF[i])
		//	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
		if (mode == SELECT_FACES_MODE)
		{
			glEnd();
		}
	}
	if (mode == RENDER_MODE || mode == REGION_MODE)
		glEnd();

	glPopMatrix();
}
//
//void Mesh::Draw(int mode) const
//{
//	glPushMatrix();
//
//	if (mode == SELECT_MODEL_MODE)
//	{
//		glLoadName(id);
//	}
//
//	QColor _color;
//
//	GLfloat diffuseColor[4] = { 1.0, 1.0, 1.0, 0.0 };
//
//	_color = QColor("red");
//	GLfloat selectionColor[4] =
//	{ _color.redF(), _color.greenF(), _color.blueF(), _color.alphaF() };
//
//	_color = QColor("blue");
//	GLfloat deformationColor[4] =
//	{ _color.redF(), _color.greenF(), _color.blueF(), _color.alphaF() };
//
//	int count = F.rows();
//
//	if (mode == RENDER_MODE || mode == REGION_MODE)
//		glBegin(GL_TRIANGLES);
//	for (int i = 0; i < count; ++i)
//	{
//		if (deletedF[i])
//			continue;
//		if (mode == SELECT_FACES_MODE)
//		{
//			glLoadName(i);
//			glBegin(GL_TRIANGLES);
//		}
//		glNormal3f(N_faces(i, 0), N_faces(i, 1), N_faces(i, 2));
//		for (int k = 0; k < 3; k++) 
//		{
//			if (mode == RENDER_MODE){
//				if (selectedF[i])
//				{
//					glMaterialfv(GL_FRONT, GL_DIFFUSE, selectionColor);
//				}
//				else
//				{
//					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
//				}
//			}
//			else{
//				if (selectedF[i])
//				{
//					glMaterialfv(GL_FRONT, GL_DIFFUSE, selectionColor);
//				}
//				else
//				{
//					glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
//				}
//				/*GLfloat tempColor[4];
//				tempColor[0] = _colorList[class_label[i]][0] / 255.0;
//				tempColor[1] = _colorList[class_label[i]][1] / 255.0;
//				tempColor[2] = _colorList[class_label[i]][2] / 255.0;
//				tempColor[3] = 0.0;
//				glMaterialfv(GL_FRONT, GL_DIFFUSE, tempColor);*/
//			}
//
//			glVertex3f(V(F(i, k), 0), V(F(i, k), 1), V(F(i, k), 2));
//		}
//		//if (selectedF[i])
//		//	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseColor);
//		if (mode == SELECT_FACES_MODE)
//		{
//			glEnd();
//		}
//	}
//	if (mode == RENDER_MODE)
//		glEnd();
//
//	glPopMatrix();
//}

bool Mesh::FNextToF(int f0, int f1)
{
	if (f0 == f1)
		return false;
	int t = 0;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if (F(f0,i) == F(f1,j))
				t++;
		}
	}
	return t > 1;
}

void Mesh::needConnectionNet()
{
	if (haveConnectionNet)
		return;
	F_Fset.resize(F.rows());
	V_Fset.resize(V.rows());
	V_Vset.resize(V.rows());
	for (int i = 0; i < F.rows(); i++)
		F_Fset[i].clear();
	for (int i = 0; i < V.rows(); i++)
	{
		V_Fset[i].clear();
		V_Vset[i].clear();
	}

	for (int i = 0; i < F.rows(); i++)
	{
		if (deletedF[i])
			continue;

		int v0 = F(i,0);
		int v1 = F(i,1);
		int v2 = F(i,2);
		V_Fset[v0].insert(i);
		V_Vset[v0].insert(v1);
		V_Vset[v0].insert(v2);
		V_Fset[v1].insert(i);
		V_Vset[v1].insert(v0);
		V_Vset[v1].insert(v2);
		V_Fset[v2].insert(i);
		V_Vset[v2].insert(v0);
		V_Vset[v2].insert(v1);
	}

	for (int i = 0; i < V.rows(); i++)
	{
		for (std::set<int>::iterator j = V_Fset[i].begin(); j != V_Fset[i].end(); j++)
		{
			std::set<int>::iterator k = j;
			for (k++; k != V_Fset[i].end(); k++)
			{
				if (FNextToF(*j, *k))
				{
					F_Fset[*j].insert(*k);
					F_Fset[*k].insert(*j);
				}
			}
		}
	}

}

void Mesh::applyChanges()
{

}

void Mesh::applyHarmonicDeformation()
{
	Eigen::MatrixXd U, V_bc, U_bc;
	Eigen::VectorXd Z;
	Eigen::VectorXi b;


	if (!select2)
	{
		std::set<int> s;
		s.empty();
		for (int i = 0; i < F.rows(); i++)
		{
			if (selectedF[i])
				for (int j = 0; j < 3; j++)
				{
					s.insert(F(i, j));
				}
		}

		b.resize(s.size());
		int t = 0;
		for (auto i : s)
		{
			b(t) = i;
			t++;
		}

		Eigen::MatrixXd sV;
		igl::slice(V, b, Eigen::Vector3i(0, 1, 2), sV);
		Mesh::bSphare sBS;
		Mesh::calcBoundingSphare(sV, sBS);
		double maxR = sBS.r + bs.r*0.2;

		bool valid = false;
		std::queue<int>q;
		std::vector<bool>visited;
		std::vector<bool>toBeDeformed;
		visited.clear();
		visited.resize(V.rows(), false);
		toBeDeformed.clear();
		toBeDeformed.resize(V.rows(), true);
		while (!q.empty())
			q.pop();
		for (int i = 0; i < b.rows(); i++)
		{
			q.push(b(i));
			visited[b(i)] = true;
		}
		while (!q.empty())
		{
			t = q.front();
			q.pop();
			for (std::set<int>::iterator i = V_Vset[t].begin(); i != V_Vset[t].end(); i++)
			{
				if (visited[*i])
					continue;
				visited[*i] = true;
				if ((V.row(*i).transpose() - sBS.c).norm() < maxR)
				{
					valid = true;
					q.push(*i);
					toBeDeformed[*i] = false;
				}
			}
		}

		if (valid)
		{
			t = 0;
			for (auto i : toBeDeformed)
				if (i)
					t++;
			b.resize(t);
			t = 0;
			for (int i = 0; i < V.rows(); i++)
			{
				if (toBeDeformed[i])
				{
					b(t) = i;
					t++;
				}
			}
			U_bc.resize(b.size(), V.cols());
			V_bc.resize(b.size(), V.cols());
			for (int bi = 0; bi < b.size(); bi++)
			{
				V_bc.row(bi) = V.row(b(bi));
				if (selectedV[b(bi)])
				{
					U_bc.row(bi) = V.row(b(bi)) + dragVec.transpose();
				}
				else
				{
					U_bc.row(bi) = V.row(b(bi));
				}
			}
			Eigen::MatrixXd D;
			Eigen::MatrixXd D_bc = U_bc - V_bc;
			igl::harmonic(V, F, b, D_bc, 2, D);
			V = V + D;
		}
		else
		{
			for (int i = 0; i < V.rows(); i++)
			{
				if (selectedV[i])
					V.row(i) += dragVec.transpose();
			}
		}
		needNormals();
		bs.valid = false;
	}
	else
	{
		int t = 0;
		for (int i = 0; i < V.rows(); i++)
		{
			if (!selectedV2[i] || selectedV[i])
				t++;
		}
		b.resize(t);
		t = 0;
		for (int i = 0; i < V.rows(); i++)
		{
			if (!selectedV2[i] || selectedV[i])
			{
				b(t) = i;
				t++;
			}
		}
		U_bc.resize(b.size(), V.cols());
		V_bc.resize(b.size(), V.cols());
		for (int bi = 0; bi < b.size(); bi++)
		{
			V_bc.row(bi) = V.row(b(bi));
			if (selectedV[b(bi)])
			{
				U_bc.row(bi) = V.row(b(bi)) + dragVec.transpose();
			}
			else
			{
				U_bc.row(bi) = V.row(b(bi));
			}
		}
		Eigen::MatrixXd D;
		Eigen::MatrixXd D_bc = U_bc - V_bc;
		igl::harmonic(V, F, b, D_bc, 2, D);
		V = V + D;
	}
}

void Mesh::needSelectedV()
{
	for (int i = 0; i < F.rows(); i++)
	{
		if (selectedF[i])
			for (int j = 0; j < 3; j++)
			{
				selectedV[F(i, j)] = true;
			}
	}
}

void Mesh::needSelectedV2()
{
	for (int i = 0; i < F.rows(); i++)
	{
		if (selectedF2[i])
			for (int j = 0; j < 3; j++)
			{
				selectedV2[F(i, j)] = true;
			}
	}
}

void Mesh::needBoundingSphare()
{
	if (bs.valid)
		return;
	calcBoundingSphare(V, bs);
}

void Mesh::calcBoundingSphare(const Eigen::MatrixXd &v, bSphare &b)
{
	//Find the max and min along the x-axie, y-axie, z-axie
	Eigen::VectorXd d;
	Eigen::VectorXi maxV, minV;
	igl::mat_max(v, 1, d, maxV);
	igl::mat_min(v, 1, d, minV);

	double x, y, z;
	x = (v.row(maxV(0)) - v.row(minV(0))).norm();
	y = (v.row(maxV(1)) - v.row(minV(1))).norm();
	z = (v.row(maxV(2)) - v.row(minV(2))).norm();

	double dia = 0;
	int max = maxV(0), min = minV(0);
	if (z > x && z > y)
	{
		max = maxV(2);
		min = minV(2);
		dia = z;
	}
	else if (y > x && y > z)
	{
		max = maxV(1);
		min = minV(1);
		dia = y;
	}

	//Compute the center point  
	Eigen::VectorXd center = (v.row(max) + v.row(min))*0.5;

	//Compute the radious  
	double radius = dia*0.5;

	//Fix it  
	for (int i = 0; i < v.rows(); i++)
	{
		d = v.row(i).transpose() - center;
		double dist = d.norm();

		if (dist > radius)
		{
			double newRadious = (dist + radius) * 0.5;
			double k = (newRadious - radius) / dist;
			radius = newRadious;
			center += d*k;
		}// end if  
	}// end for vertex_num  

	b.c = center;
	b.r = radius;
	b.valid = true;
}

void Mesh::applyNormalize()
{
	needBoundingSphare();

	V.rowwise() -= bs.c.transpose();
	V /= bs.r;
	bs.c = Eigen::Vector3d(0, 0, 0);
	bs.r = 1;
}


void Mesh::resetLabelAndRegion(){
	class_label.clear();
	class_label.resize(F.rows(), 0);
	region_number.clear();
	region_number.resize(F.rows(), 0);
}

void Mesh::addregion(int label,int no){
	for (int i = 0; i < F.rows(); i++){
		if (accumulate_s[i]){
			class_label[i] = label;
			region_number[i] = no;
		}
	}
}