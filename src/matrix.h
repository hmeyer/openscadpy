#ifndef MATRIX_H
#define MATRIX_H

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "value.h"

typedef double FloatType;
typedef boost::array< FloatType, 20 > Float20;
typedef boost::shared_ptr<Float20> Float20Ptr;
typedef boost::array< FloatType, 16 > Float16;
typedef boost::array< FloatType, 4 > Float4;
typedef boost::array< FloatType, 3 > Float3;

struct Point2D {
	double x, y;
	Point2D():x(1),y(1) {}
};
struct Point3D {
	double x, y, z;
	Point3D():x(1),y(1),z(1) {}
};

typedef boost::array< double, 4> VecDouble4;
typedef boost::array< VecDouble4, 4> MatDouble4x4;
typedef boost::array< unsigned int, 3> Triangle;
typedef std::vector<unsigned int> VecPoints;
typedef std::vector<VecPoints> VecPaths;
typedef std::vector<Triangle> VecTriangles;
typedef std::vector<Point2D> Vec2D;
typedef std::vector<Point3D> Vec3D;

void value2points2d(const Value &v, Vec2D &points);
void value2paths(const Value &v, VecPaths &paths);
void value2points3d(const Value &v, Vec3D &points);
void value2triangles(const Value &v, VecTriangles &triangles);

#endif
