#include "interpreter.h"

extern "C"
{
    #include "lua.h"
    #include "lualib.h"
}
#include <luabind/luabind.hpp>
#include "builtin.h"
#include "node.h"

using namespace luabind;

AbstractNode *lua_builtinPolygon(object const &lua_points, object const &lua_paths, int convexity, bool highlight = false, bool background = false, bool root = false);
AbstractNode *lua_builtinPolyhedron(object const &lua_points, object const &lua_triangles, int convexity, bool highlight = false, bool background = false, bool root = false);
AbstractNode *lua_builtinMultMatrix(AbstractNode *child, object const &lua_Matrix, bool highlight = false, bool background = false, bool root = false);

struct Interpreter_impl {
  lua_State *L; 
};

AbstractNode *root;

void internalSetRoot(AbstractNode *newRoot) { root = newRoot; }


Interpreter::Interpreter():impl(new Interpreter_impl()) {
  impl->L = lua_open();
  luaL_openlibs(impl->L);
  open(impl->L);
  module(impl->L)//, "openscad")
  [
      def("setroot", &internalSetRoot),
      def("_cube", &builtinCube),
      def("sphere", &builtinSphere),
      def("cylinder", &builtinCylinder),
      def("polyhedron", &builtinPolyhedron),
      def("square", &builtinSquare),
      def("circle", &builtinCircle),
      def("polygon", &lua_builtinPolygon),
      def("union", &builtinUnion),
      def("difference", &builtinDifference),
      def("intersection", &builtinIntersection),
      def("scale", &builtinScale),
      def("rotateXYZ", &builtinRotateXYZ),
      def("rotateAxis", &builtinRotateAxis),
      def("mirror", &builtinMirror),
      def("translate", &builtinTranslate),
      def("multmatrix", &lua_builtinMultMatrix),
      def("color", &builtinColor),
      
      class_<AbstractNode>("AbstractNode")
	  .def("append", &AbstractNode::append)
  ];
  interpret(
    "function cube(dim, center=false, highlight=false, background=false, root=false) return _cube(dim[0], dim[1], dim[2], center==true, highlight, background, root)  end"
  );
  root = new AbstractNode(true, false, false);
}

int Interpreter::interpret(const QString &code) {
  int result = luaL_dostring(impl->L,
    code.toStdString().c_str());
  if (result!= 0 && !cb.empty()) cb(lua_tostring(impl->L, -1));
  return result;
}

AbstractNode *Interpreter::getRoot() {
  return root;
}

Interpreter::~Interpreter() {
  lua_close(impl->L);
  delete impl;
}

AbstractNode *lua_builtinPolygon(object const &lua_points, object const &lua_paths, int convexity, bool highlight, bool background, bool root) {
  Vec2D points;
  if (type(lua_points) == LUA_TTABLE) {
    for (iterator lua_point(lua_points), end; lua_point != end; ++lua_point) {
      if (type(*lua_point) == LUA_TTABLE) {
	Point2D point;int ci=0;
	for (iterator c(*lua_point), end; c != end; ++c) {
	  switch (ci) {
	    case 0: point.x = object_cast<double>(*c);break;
	    case 1: point.y = object_cast<double>(*c);break;
	  } 
	  ++ci;
	}
	points.push_back(point);
      }
    }    
  }
  VecPaths paths;
  if (type(lua_paths) == LUA_TTABLE) {
    for (iterator lua_path(lua_paths), end; lua_path != end; ++lua_path) {
      if (type(*lua_path) == LUA_TTABLE) {
	VecPoints path;
	for (iterator lua_point(*lua_path), end; lua_point != end; ++lua_point) {
	  path.push_back(object_cast<unsigned int>(*lua_point));
	}
	paths.push_back(path);
      }
    }    
  }
  return builtinPolygon(points, paths, convexity, highlight, background, root);
}

AbstractNode *lua_builtinPolyhedron(object const &lua_points, object const &lua_triangles, int convexity, bool highlight, bool background, bool root) {
  Vec3D points;
  if (type(lua_points) == LUA_TTABLE) {
    for (iterator lua_point(lua_points), end; lua_point != end; ++lua_point) {
      if (type(*lua_point) == LUA_TTABLE) {
	Point3D point;int ci=0;
	for (iterator c(*lua_point), end; c != end; ++c) {
	  switch (ci) {
	    case 0: point.x = object_cast<double>(*c);break;
	    case 1: point.y = object_cast<double>(*c);break;
	    case 2: point.z = object_cast<double>(*c);break;
	  } 
	  ++ci;
	}
	points.push_back(point);
      }
    }    
  }
  VecTriangles triangles;
  if (type(lua_triangles) == LUA_TTABLE) {
    for (iterator lua_triangle(lua_triangles), end; lua_triangle != end; ++lua_triangle) {
      if (type(*lua_triangle) == LUA_TTABLE) {
	Triangle triangle;Triangle::iterator ti(triangle.begin());
	for (iterator lua_point(*lua_triangle), end; lua_point != end; ++lua_point) {
	  if (ti!=triangle.end()) {
	    *ti = object_cast<unsigned int>(*lua_point);
	    ++ti;
	  }
	}
	triangles.push_back(triangle);
      }
    }    
  }
  return builtinPolyhedron(points, triangles, convexity, highlight, background, root);  
}

AbstractNode *lua_builtinMultMatrix(AbstractNode *child, object const &lua_Matrix, bool highlight, bool background, bool root) {
  MatDouble4x4 m;
  if (type(lua_Matrix) == LUA_TTABLE) {
    int rowNum = 0;
    for (iterator lua_row(lua_Matrix), end; lua_row != end; ++lua_row) {
      if (type(*lua_row) == LUA_TTABLE) {
	int indexNum = 0;
	for (iterator lua_index(*lua_row), end; lua_index != end; ++lua_index) {
	  if (rowNum <= 4 && indexNum <= 4)
	    m[rowNum][indexNum] = object_cast<double>(*lua_index);
	}
      }
    }    
  }  
  return builtinMultMatrix(child,m,highlight,background,root);
}
