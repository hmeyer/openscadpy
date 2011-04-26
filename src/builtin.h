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
typedef boost::array< double, 4> VecDouble4;
typedef boost::array< VecDouble4, 4> MatDouble4x4;
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
AbstractNode *builtinUnion(AbstractNode *child, AbstractNode *b, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinDifference(AbstractNode *child, AbstractNode *b, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinIntersection(AbstractNode *child, AbstractNode *b, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinScale(AbstractNode *child, double x, double y, double z, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinRotateXYZ(AbstractNode *child, double x, double y, double z, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinRotateAxis(AbstractNode *child, double x, double y, double z, double a, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinMirror(AbstractNode *child, double x, double y, double z, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinTranslate(AbstractNode *child, double x, double y, double z, bool highlight = false, bool background = false, bool root = false) ;
AbstractNode *builtinMultMatrix(AbstractNode *child, const MatDouble4x4 &m, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinColor(AbstractNode *child, double r, double g, double b, double a, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinSurface(const QString &file, int convexity, bool center = false, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinRender(AbstractNode *n, int convexity,  bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinImportSTL(const QString &file, int convexity,  bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinImportDXF(const QString &file, const QString &layer, double origin_x, double origin_y, double scale, int convexity,  double fn, double fs, double fa, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinProjection(AbstractNode *child, bool cut, int convexity, bool highlight = false, bool background = false, bool root = false);
AbstractNode *builtinMinkowski(AbstractNode *a, AbstractNode *b, int convexity, bool highlight = false, bool background = false, bool root = false) ;

#endif
