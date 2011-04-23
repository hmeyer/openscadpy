#ifndef BUILTIN_H_
#define BUILTIN_H_

#include <QHash>
#include "node.h"
#include <boost/array.hpp>

extern QHash<QString, class AbstractFunction*> builtin_functions;
extern void initialize_builtin_functions();
extern void destroy_builtin_functions();

extern QHash<QString, class AbstractModule*> builtin_modules;
extern void initialize_builtin_modules();
extern void destroy_builtin_modules();

extern void register_builtin_csgops();
extern void register_builtin_transform();
extern void register_builtin_primitives();
extern void register_builtin_surface();
extern void register_builtin_control();
extern void register_builtin_render();
extern void register_builtin_import();
extern void register_builtin_projection();
extern void register_builtin_cgaladv();
extern void register_builtin_dxf_linear_extrude();
extern void register_builtin_dxf_rotate_extrude();
extern void initialize_builtin_dxf_dim();

struct Point2D {
	double x, y;
};
struct Point3D {
	double x, y, z;
};
typedef boost::array< unsigned int, 3> Triangle;
typedef std::vector<unsigned int> VecPoints;
typedef std::vector<VecPoints> VecPaths;
typedef std::vector<Triangle> VecTriangles;
typedef std::vector<Point2D> Vec2D;
typedef std::vector<Point3D> Vec3D;



AbstractNode *builtinCube(double x, double y, double z, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinSphere(double r, double fn, double fs, double fa, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinCylinder(double r1, double r2, double h, double fn, double fs, double fa, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinPolyhedron(const Vec3D &points, const VecTriangles &triangles, int convexity, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinSquare(double x, double y, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinCircle(double r, double fn, double fs, double fa, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinPolygon(const Vec2D &points, const VecPaths &paths, int convexity, bool highlight = false, bool background = false, bool root = false);

#endif
