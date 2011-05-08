#include "pythonscripting.h"
#include "node.h"
#include "csgops.h"
#include "primitives.h"
#include "transform.h"
#include "render.h"
#include "dxflinextrude.h"
#include "dxfrotextrude.h"
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

using namespace boost::python;
using boost::make_shared;


class PyContext {
  Accuracy acc;
  object main_module;
  std::string path;
  public:
    PyContext() {};
    void init(){
      path.clear();
      main_module = object((handle<>(borrowed(PyImport_AddModule("__main__")))));
      main_namespace = main_module.attr("__dict__");
      main_namespace["fn"] = acc.fn;
      main_namespace["fs"] = acc.fs;
      main_namespace["fa"] = acc.fa;
      if (main_namespace.contains("result")) main_namespace["result"].del();
    }
    const std::string &getPath() const { return path;}
    void setPath(const std::string &p) { path = p;}
    Accuracy &getAcc() {
      acc.fn = extract<double>(main_namespace["fn"]);
      acc.fa = extract<double>(main_namespace["fa"]);
      acc.fs = extract<double>(main_namespace["fs"]);
      return acc;
    }
  object main_namespace;
};

PyContext ctx;

template<class StdArray>
StdArray list2StdArray(const list &l) {
  StdArray p;
  for(int i=0;i<len(l);++i) {
    extract<double> x(l[i]);
    if (x.check()) p[i] = x();
  }
  return p;
}



class PyAbstractNode {
protected:
  AbstractNode::Pointer node;
public:
  AbstractNode::Pointer getNode() const { return node; }
  void highlight(bool set=true) {
    node->props.highlight = set;
  }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(PyAbstractNode_overloads, highlight, 0, 1)

AbstractNode::NodeList list2NodeList(const list &l) {
  AbstractNode::NodeList nl;
  for(int i=0;i<len(l);++i) {
    extract<PyAbstractNode> x(l[i]);
    if (x.check()) nl.append(x().getNode());
  }
  return nl;
}


template<csg_type_e type> 
class PyCSGNode: public PyAbstractNode {
public:
  PyCSGNode(const list &l) {
    node = make_shared<CsgNode>(type, list2NodeList(l));
  }
};

typedef PyCSGNode< CSG_TYPE_UNION > PyUnionNode;
typedef PyCSGNode< CSG_TYPE_DIFFERENCE > PyDifferenceNode;
typedef PyCSGNode< CSG_TYPE_INTERSECTION > PyIntersectionNode;

template<class NodeClass>
class PyTransformNode: public PyAbstractNode {
public:
  PyTransformNode(const list &va, const PyAbstractNode &n) {
    node = make_shared<NodeClass>(list2StdArray<Float3>(va), AbstractNode::NodeList(1, n.getNode()));
  }
  PyTransformNode(const list &va, const list &n) {
    node = make_shared<NodeClass>(list2StdArray<Float3>(va), list2NodeList(n));
  }
};

typedef PyTransformNode<TransformScaleNode> PyScaleNode;
typedef PyTransformNode<TransformTranslateNode> PyTranslateNode;
typedef PyTransformNode<TransformRotateNode> PyRotateNode;
typedef PyTransformNode<TransformMirrorNode> PyMirrorNode;

class PyRotateAxisNode: public PyAbstractNode {
public:
  PyRotateAxisNode(double ang, const list &va, const PyAbstractNode &n) {
    node = make_shared<TransformRotateAxisNode>(list2StdArray<Float3>(va), ang, AbstractNode::NodeList(1, n.getNode()));
  }
  PyRotateAxisNode(double ang, const list &va, const list &a) {
    node = make_shared<TransformRotateAxisNode>(list2StdArray<Float3>(va), ang, list2NodeList(a));
  }
};
class PyMatrixNode: public PyAbstractNode {
public:
  PyMatrixNode(const list &va, const PyAbstractNode &n) {
    node = make_shared<TransformMatrixNode>(list2StdArray<Float16>(va), AbstractNode::NodeList(1, n.getNode()));
  }
  PyMatrixNode(const list &ma, const list &a) {
    node = make_shared<TransformMatrixNode>(list2StdArray<Float16>(ma), list2NodeList(a));
  }
};
class PyColorNode: public PyAbstractNode {
public:
  PyColorNode(const list &ca, const PyAbstractNode &n) {
    node = make_shared<TransformColorNode	>(list2StdArray<Float4>(ca), AbstractNode::NodeList(1, n.getNode()));
  }
  PyColorNode(const list &ca, const list &a) {
    node = make_shared<TransformColorNode>(list2StdArray<Float4>(ca), list2NodeList(a));
  }
};

class PyCubeNode: public PyAbstractNode {
public:
  PyCubeNode(const list &dim, bool center=false) {
    node = make_shared<CubeNode>(list2StdArray<Float3>(dim), center);  
  }
  PyCubeNode(double l, bool center=false) {
    Float3 dim = {{ l,l,l }};
    node = make_shared<CubeNode>(dim, center);  
  }  
};

class PySphereNode: public PyAbstractNode {
public:
  PySphereNode(double r) {
    node = make_shared<SphereNode>(r, ctx.getAcc());
  }
};

class PyCylinderNode: public PyAbstractNode {
public:
  PyCylinderNode(double r1, double r2, double h, bool center=false) {
    node = make_shared<CylinderNode>(r1, r2, h, center, ctx.getAcc());
  }
  PyCylinderNode(double r, double h, bool center=false) {
    node = make_shared<CylinderNode>(r, r, h, center, ctx.getAcc());
  }
};

template<class ArrayVec>
ArrayVec list2ArrayVec(const list &l) {
  ArrayVec v;
  for(int i=0;i<len(l);++i) {
    extract<list> x(l[i]);
    if (x.check()) v.push_back(list2StdArray<typename ArrayVec::value_type>(x()));
  }  
  return v;
}

template<class Vec>
Vec list2Vec(const list &l) {
  Vec v;
  for(int i=0;i<len(l);++i) {
    extract<typename Vec::value_type> x(l[i]);
    if (x.check()) v.push_back(x());
  }  
  return v;
}

template<class VecVec>
VecVec list2VecVec(const list &l) {
  VecVec v;
  for(int i=0;i<len(l);++i) {
    extract<list> x(l[i]);
    if (x.check()) v.push_back(list2Vec<typename VecVec::value_type>(x()));
  }  
  return v;
}

class PyPolyhedronNode: public PyAbstractNode {
public:
  PyPolyhedronNode(const list &points, const list &triangles, unsigned int convexity=5) {
    node = make_shared<PolyhedronNode>(
      list2ArrayVec<Vec3D>(points), 
      list2ArrayVec<VecTriangles>(triangles), convexity);
  }
};


class PySquareNode: public PyAbstractNode {
public:
  PySquareNode(const list &dim, bool center=false) {
    node = make_shared<SquareNode>(list2StdArray<Float2>(dim), center);  
  }
  PySquareNode(double l, bool center=false) {
    Float2 dim = {{ l,l }};
    node = make_shared<SquareNode>(dim, center);  
  }
};

class PyCircleNode: public PyAbstractNode {
public:
  PyCircleNode(double r) {
    node = make_shared<CircleNode>(r, ctx.getAcc());
  }
};

class PyPolygonNode: public PyAbstractNode {
public:
  PyPolygonNode(const list &points, const list &paths, unsigned int convexity=5) {
    node = make_shared<PolygonNode>(
      list2ArrayVec<Vec2D>(points), 
      list2VecVec<VecPaths>(paths), convexity);
  }
};

class PyRenderNode: public PyAbstractNode {
public:
  PyRenderNode(const PyAbstractNode &n, unsigned int convexity=5) {
    node = make_shared<RenderNode>(AbstractNode::NodeList(1, n.getNode()), convexity);
  }
  PyRenderNode(const list &a, unsigned int convexity=5) {
    node = make_shared<RenderNode>(list2NodeList(a), convexity);
  }
};

class PyDxfLinearExtrudeNode: public PyAbstractNode {
public:
  PyDxfLinearExtrudeNode(const std::string &filename, const std::string &layer,
	double height, double twist, double origin_x, double origin_y, double scale, 
	unsigned int convexity=5, int slices = -1, bool center = false) {
      node.reset(new DxfLinearExtrudeNode(
	AbstractNode::NodeList(), QString::fromStdString(filename), QString::fromStdString(layer),
	height, twist, origin_x, origin_y, scale,
	convexity, slices, center, ctx.getAcc()
      ));
  }
};

class PyLinearExtrudeNode: public PyAbstractNode {
public:
  PyLinearExtrudeNode(const PyAbstractNode &n,
	double height, double twist,
	unsigned int convexity=5, int slices = -1, bool center = false) {
      node.reset(new DxfLinearExtrudeNode(
	AbstractNode::NodeList(1, n.getNode()), QString(), QString(),
	height, twist, 0, 0, 0,
	convexity, slices, center, ctx.getAcc()
      ));
  }
  PyLinearExtrudeNode(const list &a,
	double height, double twist,
	unsigned int convexity=5, int slices = -1, bool center = false) {
      node.reset(new DxfLinearExtrudeNode(
	list2NodeList(a), QString(), QString(),
	height, twist, 0, 0, 0,
	convexity, slices, center, ctx.getAcc()
      ));
  }
};


class PyDxfRotateExtrudeNode: public PyAbstractNode {
public:
  PyDxfRotateExtrudeNode(const std::string &filename, const std::string &layer,
	double origin_x, double origin_y, double scale, 
	unsigned int convexity=5) {
      node = make_shared<DxfRotateExtrudeNode>(
	AbstractNode::NodeList(), QString::fromStdString(filename), QString::fromStdString(layer),
	origin_x, origin_y, scale,
	convexity, ctx.getAcc()
      );
  }
};
class PyRotateExtrudeNode: public PyAbstractNode {
public:
  PyRotateExtrudeNode(const PyAbstractNode &n,
	unsigned int convexity=5) {
      node = make_shared<DxfRotateExtrudeNode>(
	AbstractNode::NodeList(1, n.getNode()), QString(), QString(),
	0, 0, 0,
	convexity, ctx.getAcc()
      );
  }
  PyRotateExtrudeNode(const list &a,
	unsigned int convexity=5) {
      node = make_shared<DxfRotateExtrudeNode>(
	list2NodeList(a), QString(), QString(),
	0, 0, 0,
	convexity, ctx.getAcc()
      );
  }
};


PythonScript::PythonScript() {
  Py_Initialize();
  ctx.init();
  ctx.main_namespace["AbstractNode"] = class_<PyAbstractNode>("AbstractNode").def("highlight", &PyAbstractNode::highlight, PyAbstractNode_overloads());
  ctx.main_namespace["Union"] = class_<PyUnionNode, bases<PyAbstractNode> >("Union", init<list>());
  ctx.main_namespace["Difference"] = class_<PyDifferenceNode, bases<PyAbstractNode> >("Difference", init<list>());
  ctx.main_namespace["Intersection"] = class_<PyIntersectionNode, bases<PyAbstractNode> >("Intersection", init<list>());
  ctx.main_namespace["Scale"] = class_<PyScaleNode, bases<PyAbstractNode> >("Scale", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["Translate"] = class_<PyTranslateNode, bases<PyAbstractNode> >("Translate", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["Rotate"] = class_<PyRotateNode, bases<PyAbstractNode> >("Rotate", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["Mirror"] = class_<PyMirrorNode, bases<PyAbstractNode> >("Mirror", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["RotateAxis"] = class_<PyRotateAxisNode, bases<PyAbstractNode> >("RotateAxis", init<double, list, PyAbstractNode>()).def(init<double, list, list>());
  ctx.main_namespace["Matrix"] = class_<PyMatrixNode, bases<PyAbstractNode> >("Matrix", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["Color"] = class_<PyColorNode, bases<PyAbstractNode> >("Color", init<list, PyAbstractNode>()).def(init<list, list>());
  ctx.main_namespace["Cube"] = class_<PyCubeNode, bases<PyAbstractNode> >("Cube", init<list, optional<bool> >()).def(init<double, optional<bool> >());
  ctx.main_namespace["Sphere"] = class_<PySphereNode, bases<PyAbstractNode> >("Sphere", init<double>());
  ctx.main_namespace["Cylinder"] = class_<PyCylinderNode, bases<PyAbstractNode> >("Cylinder", init<double, double, double, optional<bool> >()).def(init<double, double, optional< bool> >());
  ctx.main_namespace["Polyhedron"] = class_<PyPolyhedronNode, bases<PyAbstractNode> >("Polyhedron", init<list, list, optional<unsigned int> >());
  ctx.main_namespace["Square"] = class_<PySquareNode, bases<PyAbstractNode> >("Square", init<list, optional<bool> >()).def(init<double, optional<bool> >());
  ctx.main_namespace["Circle"] = class_<PyCircleNode, bases<PyAbstractNode> >("Circle", init<double>());
  ctx.main_namespace["Polygon"] = class_<PyPolygonNode, bases<PyAbstractNode> >("Polygon", init<list, list, optional<unsigned int> >());
  ctx.main_namespace["Render"] = class_<PyRenderNode, bases<PyAbstractNode> >("Render", init<PyAbstractNode, optional<unsigned int> >()).def(init<list, optional<unsigned int> >());
  ctx.main_namespace["DxfLinearExtrude"] = class_<PyDxfLinearExtrudeNode, bases<PyAbstractNode> >("DxfLinearExtrude", 
					    init<std::string, std::string,
					      double, double, double, double, double,
					      optional< unsigned int, int, bool > >());
  ctx.main_namespace["LinearExtrude"] = class_<PyLinearExtrudeNode, bases<PyAbstractNode> >("LinearExtrude", 
					    init<PyAbstractNode,
					      double, double,
					      optional< unsigned int, int, bool > >()).def(
					    init<list, 
					      double, double,
					      optional< unsigned int, int, bool > >());
  ctx.main_namespace["DxfRotateExtrude"] = class_<PyDxfRotateExtrudeNode, bases<PyAbstractNode> >("DxfRotateExtrude", 
					    init<std::string, std::string,
					      double, double, double, optional< unsigned int > >());
  ctx.main_namespace["RotateExtrude"] = class_<PyRotateExtrudeNode, bases<PyAbstractNode> >("RotateExtrude", 
					    init<PyAbstractNode, optional< unsigned int> >()).def(
					    init<list, optional< unsigned int > >());
  //SurfaceNode
  //ImportSTLNode
  //ImportDXFNode
  //ImportOFFNode
  //ProjectionNode
  //CgaladvMinkowskiNode
  //CgaladvGlideNode
  //CgaladvSubdivNode
  //dxf_dim
  //dxf_
}

PythonScript::~PythonScript() {}

AbstractNode::Pointer PythonScript::evaluate(const std::string &code, const std::string &path) {
  ctx.setPath(path);
  try {
    exec(code.c_str(), ctx.main_namespace);
    PyAbstractNode &resNode = extract<PyAbstractNode&>(ctx.main_namespace["result"]);
    return resNode.getNode();
  } catch(error_already_set) {
//    PyErr_Print();
  }
  return AbstractNode::Pointer();
}

std::string PythonScript::error() {
  namespace py = boost::python;
  try {
    PyObject *exc,*val,*tb;
    PyErr_Fetch(&exc,&val,&tb);
    handle<> hexc(exc),hval(val),htb(tb);
    if(!htb || !hval)
    {
      return extract<std::string>(str(hexc));
    }
    else
    {
      object traceback(py::import("traceback"));
      object format_exception(traceback.attr("format_exception"));
      object formatted_list(format_exception(hexc,hval,htb));    
      object formatted(str("\n").join(formatted_list));
      return extract<std::string>(formatted);
    }
  } catch(error_already_set) {
    PyErr_Print();
  }
  return "Could not catch error - see command line";
}
