#include "intersect_with_half_space.h"
#include "mesh_boolean.h"
#include "half_space_box.h"

template <
  typename DerivedV,
  typename DerivedF,
  typename Derivedp,
  typename Derivedn,
  typename DerivedVC,
  typename DerivedFC,
  typename DerivedJ>
IGL_INLINE bool igl::copyleft::cgal::intersect_with_half_space(
  const Eigen::PlainObjectBase<DerivedV > & V,
  const Eigen::PlainObjectBase<DerivedF > & F,
  const Eigen::PlainObjectBase<Derivedp > & p,
  const Eigen::PlainObjectBase<Derivedn > & n,
  Eigen::PlainObjectBase<DerivedVC > & VC,
  Eigen::PlainObjectBase<DerivedFC > & FC,
  Eigen::PlainObjectBase<DerivedJ > & J)
{
  Eigen::Matrix<CGAL::Epeck::FT,8,3> BV;
  Eigen::Matrix<int,12,3> BF;
  half_space_box(p,n,V,BV,BF);
  // Disturbingly, (BV,BF) must be first argument
  return mesh_boolean(BV,BF,V,F,MESH_BOOLEAN_TYPE_INTERSECT,VC,FC,J);
}

template <
  typename DerivedV,
  typename DerivedF,
  typename Derivedequ,
  typename DerivedVC,
  typename DerivedFC,
  typename DerivedJ>
IGL_INLINE bool igl::copyleft::cgal::intersect_with_half_space(
  const Eigen::PlainObjectBase<DerivedV > & V,
  const Eigen::PlainObjectBase<DerivedF > & F,
  const Eigen::PlainObjectBase<Derivedequ > & equ,
  Eigen::PlainObjectBase<DerivedVC > & VC,
  Eigen::PlainObjectBase<DerivedFC > & FC,
  Eigen::PlainObjectBase<DerivedJ > & J)
{
  Eigen::Matrix<CGAL::Epeck::FT,8,3> BV;
  Eigen::Matrix<int,12,3> BF;
  half_space_box(equ,V,BV,BF);
  // Disturbingly, (BV,BF) must be first argument
  return mesh_boolean(BV,BF,V,F,MESH_BOOLEAN_TYPE_INTERSECT,VC,FC,J);
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template specialization
template bool igl::copyleft::cgal::intersect_with_half_space<Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, 1, 3, 1, 1, 3>, Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, 1, 0, -1, 1> >(Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> > const&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, 1, 3, 1, 1, 3> > const&, Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, -1, 0, -1, -1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&);
#endif
