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
typedef boost::array< FloatType, 2 > Float2;

typedef boost::array< Float4, 4> MatFloat4x4;
typedef boost::array< unsigned int, 3> UInt3;
typedef std::vector<unsigned int> VecPoints;
typedef std::vector<VecPoints> VecPaths;
typedef std::vector<UInt3> VecTriangles;
typedef std::vector<Float2> Vec2D;
typedef std::vector<Float3> Vec3D;

void value2points2d(const Value &v, Vec2D &points);
void value2paths(const Value &v, VecPaths &paths);
void value2points3d(const Value &v, Vec3D &points);
void value2triangles(const Value &v, VecTriangles &triangles);

#endif
