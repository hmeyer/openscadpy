#include "pythonscripting.h"
#include "node.h"
#include "csgops.h"
#include "primitives.h"
#include "transform.h"
#include "render.h"
#include "dxflinextrude.h"
#include "dxfrotextrude.h"
#include "dxfdim.h"
#include "surface.h"
#include "import.h"
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

using namespace boost::python;
using boost::make_shared;


class PyContext {
  Accuracy acc;
  object main_module;
  object openscad_namespace;
  
  std::string path;
  public:
  static const std::string nsresult;
  static const std::string nsopenscad;
  PyContext() {};
    void init(object &openscad_module){
      path.clear();
      main_module = object((handle<>(borrowed(PyImport_AddModule("__main__")))));
      main_namespace = main_module.attr("__dict__");
      main_namespace[nsopenscad] = openscad_module;
      openscad_namespace = openscad_module.attr("__dict__");
      if (openscad_namespace.contains(nsresult)) openscad_namespace[nsresult].del();
      openscad_namespace["fn"] = acc.fn;
      openscad_namespace["fs"] = acc.fs;
      openscad_namespace["fa"] = acc.fa;      
    }
    object getResult() {
      if (openscad_namespace.contains(nsresult))
	return openscad_namespace[nsresult];
      return object();
    }
    const std::string &getPath() const { return path;}
    void setPath(const std::string &p) { path = p;}
    Accuracy &getAcc() {
      acc.fn = extract<double>(openscad_namespace["fn"]);
      acc.fa = extract<double>(openscad_namespace["fa"]);
      acc.fs = extract<double>(openscad_namespace["fs"]);
      return acc;
    }
    object main_namespace;
};

const std::string PyContext::nsresult("result");
const std::string PyContext::nsopenscad("openscad");


PyContext ctx;

template<class StdArray>
StdArray list2StdArray(const list &l, double defval=0.0) {
  StdArray p;
  int i;
  for(i=0;i<len(l);++i) {
    extract<double> x(l[i]);
    if (x.check()) p[i] = x();
    else p[i] = defval;
  }
  for(;i<p.static_size;++i) p[i] = defval;
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
  void background(bool set=true) {
    node->props.background = set;
  }
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(PyAbstractNode_hi_overloads, highlight, 0, 1)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(PyAbstractNode_bg_overloads, background, 0, 1)

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

template<class NodeClass, int defval=0>
class PyTransformNode: public PyAbstractNode {
public:
  PyTransformNode(const list &va, const PyAbstractNode &n) {
    node = make_shared<NodeClass>(list2StdArray<Float3>(va, defval), AbstractNode::NodeList(1, n.getNode()));
  }
  PyTransformNode(const list &va, const list &n) {
    node = make_shared<NodeClass>(list2StdArray<Float3>(va, defval), list2NodeList(n));
  }
};

typedef PyTransformNode<TransformScaleNode,1> PyScaleNode;
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
  PyRotateAxisNode(double ang, const PyAbstractNode &n) {
    Float3 v = {{0,0,1}};
    node = make_shared<TransformRotateAxisNode>(v, ang, AbstractNode::NodeList(1, n.getNode()));
  }
  PyRotateAxisNode(double ang, const list &a) {
    Float3 v = {{0,0,1}};
    node = make_shared<TransformRotateAxisNode>(v, ang, list2NodeList(a));
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
      list2VecVec<VecPaths>(triangles), convexity);
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
	double height, double twist=.0, double origin_x=.0, double origin_y=.0, double scale=1.0, 
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
	double height, double twist=.0,
	unsigned int convexity=5, int slices = -1, bool center = false) {
      node.reset(new DxfLinearExtrudeNode(
	AbstractNode::NodeList(1, n.getNode()), QString(), QString(),
	height, twist, 0, 0, 0,
	convexity, slices, center, ctx.getAcc()
      ));
  }
  PyLinearExtrudeNode(const list &a,
	double height, double twist=.0,
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
	double origin_x=.0, double origin_y=.0, double scale=1.0, 
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

class PySurfaceNode: public PyAbstractNode {
public:
  PySurfaceNode(const std::string &filename, unsigned int convexity=5, bool center=false) {
      node = make_shared<SurfaceNode>(QString::fromStdString(filename), convexity, center);
  }
};

class PyImportSTLNode: public PyAbstractNode {
public:
  PyImportSTLNode(const std::string &filename, unsigned int convexity=5) {
      node = make_shared<ImportSTLNode>(QString::fromStdString(filename), convexity);
  }
};

class PyImportDXFNode: public PyAbstractNode {
public:
  PyImportDXFNode(const std::string &filename, const std::string &layer,
	double origin_x=0.0, double origin_y=0.0, double scale=1.0, unsigned int convexity=5) {
      node = make_shared<ImportDXFNode>(QString::fromStdString(filename),
	QString::fromStdString(layer), origin_x, origin_y, convexity, scale, ctx.getAcc());
  }
};

double pyDxfDim(const std::string &filename, const std::string &layername=std::string(), const std::string &name=std::string(), double xorigin=0.0, double yorigin=0.0, double scale=1.0) {
  return dxf_dim(QString::fromStdString(filename), QString::fromStdString(layername), QString::fromStdString(name), xorigin, yorigin, scale);
}

BOOST_PYTHON_FUNCTION_OVERLOADS(pyDxfDim_overloads, pyDxfDim, 1, 6)

list pyDxfCross(const std::string &filename, const std::string &layername=std::string(), double xorigin=0.0, double yorigin=0.0, double scale=1.0) {
  list res;
  Float2 cross = dxf_cross(QString::fromStdString(filename), QString::fromStdString(layername), xorigin, yorigin, scale);
  res.append<double>(cross[0]);
  res.append<double>(cross[1]);
  return res;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(pyDxfCross_overloads, pyDxfCross, 1, 5)


BOOST_PYTHON_MODULE(openscad) {
  class_<PyAbstractNode>("AbstractNode")
    .def("highlight", &PyAbstractNode::highlight, PyAbstractNode_hi_overloads())
    .def("background", &PyAbstractNode::background, PyAbstractNode_bg_overloads());
  class_<PyUnionNode, bases<PyAbstractNode> >("Union", init<list>());
  class_<PyDifferenceNode, bases<PyAbstractNode> >("Difference", init<list>());
  class_<PyIntersectionNode, bases<PyAbstractNode> >("Intersection", init<list>());
  class_<PyScaleNode, bases<PyAbstractNode> >("Scale", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyTranslateNode, bases<PyAbstractNode> >("Translate", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyRotateNode, bases<PyAbstractNode> >("Rotate", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyMirrorNode, bases<PyAbstractNode> >("Mirror", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyRotateAxisNode, bases<PyAbstractNode> >("RotateAxis", init<double, list, PyAbstractNode>()).def(init<double, list, list>())
    .def(init<double, PyAbstractNode>()).def(init<double, list>());
  class_<PyMatrixNode, bases<PyAbstractNode> >("Matrix", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyColorNode, bases<PyAbstractNode> >("Color", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyCubeNode, bases<PyAbstractNode> >("Cube", init<list, optional<bool> >()).def(init<double, optional<bool> >());
  class_<PySphereNode, bases<PyAbstractNode> >("Sphere", init<double>());
  class_<PyCylinderNode, bases<PyAbstractNode> >("Cylinder", init<double, double, double, optional<bool> >()).def(init<double, double, optional< bool> >());
  class_<PyPolyhedronNode, bases<PyAbstractNode> >("Polyhedron", init<list, list, optional<unsigned int> >());
  class_<PySquareNode, bases<PyAbstractNode> >("Square", init<list, optional<bool> >()).def(init<double, optional<bool> >());
  class_<PyCircleNode, bases<PyAbstractNode> >("Circle", init<double>());
  class_<PyPolygonNode, bases<PyAbstractNode> >("Polygon", init<list, list, optional<unsigned int> >());
  class_<PyRenderNode, bases<PyAbstractNode> >("Render", init<PyAbstractNode, optional<unsigned int> >()).def(init<list, optional<unsigned int> >());
  class_<PyDxfLinearExtrudeNode, bases<PyAbstractNode> >("DxfLinearExtrude", 
    init<std::string, std::string,
      double, optional< double, double, double, double,
      unsigned int, int, bool > >());
  class_<PyLinearExtrudeNode, bases<PyAbstractNode> >("LinearExtrude", 
    init<PyAbstractNode,
      double, optional< double,
      unsigned int, int, bool > >()).def(
    init<list, 
      double, optional< double,
      unsigned int, int, bool > >());
  class_<PyDxfRotateExtrudeNode, bases<PyAbstractNode> >("DxfRotateExtrude", 
    init<std::string, std::string,
      optional< double, double, double, unsigned int > >());
  class_<PyRotateExtrudeNode, bases<PyAbstractNode> >("RotateExtrude", 
    init<PyAbstractNode, optional< unsigned int> >()).def(
    init<list, optional< unsigned int > >());
  class_<PySurfaceNode, bases<PyAbstractNode> >("Surface", init<std::string, optional<unsigned int, bool> >());
  class_<PyImportSTLNode, bases<PyAbstractNode> >("ImportSTL", init<std::string, optional<unsigned int> >());
  class_<PyImportDXFNode, bases<PyAbstractNode> >("ImportDXF", 
    init<std::string, std::string, optional< double, double, double,unsigned int> >());						  
   
  def("DxfDim", pyDxfDim, pyDxfDim_overloads());
  def("DxfCross", pyDxfCross, pyDxfCross_overloads());
}

PythonScript::PythonScript() {
  PyImport_AppendInittab(const_cast<char*>(PyContext::nsopenscad.c_str()), &initopenscad );
  Py_Initialize();
  object openscad_module( (handle<>(PyImport_ImportModule(PyContext::nsopenscad.c_str()))) );
  ctx.init(openscad_module);
  //SurfaceNode
  //ImportSTLNode
  //ImportDXFNode
  //ImportOFFNode
  //ProjectionNode
  //CgaladvMinkowskiNode
  //CgaladvGlideNode
  //CgaladvSubdivNode
}

PythonScript::~PythonScript() {}

AbstractNode::Pointer PythonScript::evaluate(const std::string &code, const std::string &path) {
  ctx.setPath(path);
  try {
    exec(code.c_str(), ctx.main_namespace);
    PyAbstractNode &resNode = extract<PyAbstractNode&>(ctx.getResult());
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
