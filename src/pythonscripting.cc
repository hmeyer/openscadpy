#define BOOST_PYTHON_MAX_ARITY 15
#define BOOST_PARAMETER_MAX_ARITY 9

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
#include "projection.h"
#include "cgaladv.h"
#include <boost/python.hpp>
#include <boost/make_shared.hpp>

#include <boost/parameter/keyword.hpp>
#include <boost/parameter/preprocessor.hpp>
#include <boost/parameter/python.hpp>
#include <boost/python.hpp>
#include <boost/mpl/vector.hpp>
#pragma GCC diagnostic ignored "-Wunused-parameter" // "'boost_parameter_enabler_argument'


BOOST_PARAMETER_KEYWORD(tag, dim)
BOOST_PARAMETER_KEYWORD(tag, length)
BOOST_PARAMETER_KEYWORD(tag, center)
BOOST_PARAMETER_KEYWORD(tag, ang)
BOOST_PARAMETER_KEYWORD(tag, vec)
BOOST_PARAMETER_KEYWORD(tag, children)
BOOST_PARAMETER_KEYWORD(tag, child)
BOOST_PARAMETER_KEYWORD(tag, h)
BOOST_PARAMETER_KEYWORD(tag, r)
BOOST_PARAMETER_KEYWORD(tag, r1)
BOOST_PARAMETER_KEYWORD(tag, r2)
BOOST_PARAMETER_KEYWORD(tag, file)
BOOST_PARAMETER_KEYWORD(tag, layer)
BOOST_PARAMETER_KEYWORD(tag, twist)
BOOST_PARAMETER_KEYWORD(tag, origin)
BOOST_PARAMETER_KEYWORD(tag, scale)
BOOST_PARAMETER_KEYWORD(tag, convexity)
BOOST_PARAMETER_KEYWORD(tag, slices)
BOOST_PARAMETER_KEYWORD(tag, cut_mode)
BOOST_PARAMETER_KEYWORD(tag, name)


using namespace boost::python;
using boost::make_shared;

list empty_list;
class PyAbstractNode;

class PyContext {
  Accuracy acc;
  object main_module;
  object openscad_namespace;
  
  public:
  static const std::string nsresult;
  static const std::string nsopenscad;
  PyContext() {};
    void init(object &openscad_module, double time=0.0){
      main_module = object((handle<>(borrowed(PyImport_AddModule("__main__")))));
      main_namespace = main_module.attr("__dict__");
      main_namespace[nsopenscad] = openscad_module;
      openscad_namespace = openscad_module.attr("__dict__");
      if (openscad_namespace.contains(nsresult)) openscad_namespace[nsresult].del();
      openscad_namespace["fn"] = acc.fn;
      openscad_namespace["fs"] = acc.fs;
      openscad_namespace["fa"] = acc.fa;     
      openscad_namespace["t"] = time;     
    }
    object getResult() {
      if (openscad_namespace.contains(nsresult))
	return openscad_namespace[nsresult];
      return object();
    }
    void setResult(const PyAbstractNode &n) {
	openscad_namespace[nsresult] = n;
    }
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

bool myIsNaN(double x) {
	return x!=x;
}


class PyAbstractNode {
protected:
  AbstractNode::Pointer node;
public:
  AbstractNode::Pointer getNode() const { return node; }
  void setHighlight(bool set=true) {
    node->props.highlight = set;
  }  
  bool getHighlight(void) const {
    return node->props.highlight;
  }  
  void setBackground(bool set=true) {
    node->props.background = set;
  }
  bool getBackground(void) const {
    return node->props.background;
  }
};

AbstractNode::NodeList list2NodeList(const list &l) {
  AbstractNode::NodeList nl;
  for(int i=0;i<len(l);++i) {
    extract<PyAbstractNode> x(l[i]);
    if (x.check()) nl.append(x().getNode());
  }
  return nl;
}

class PyNodeAccuracy {
public:
  typedef boost::shared_ptr<Accuracy> AccPtr;
protected:
  boost::shared_ptr<Accuracy> accp;
public:
  void initAcc(AccPtr p) { accp = p; }
  virtual void setfn(double fn) {accp->fn = fn;}
  virtual double getfn(void) const {return accp->fn;}
  virtual void setfs(double fs) {accp->fs = fs;}
  virtual double getfs(void) const {return accp->fs;}
  virtual void setfa(double fa) {accp->fa = fa;}
  virtual double getfa(void) const {return accp->fa;}
};

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

typedef PyTransformNode<TransformTranslateNode> PyTranslateNode;
typedef PyTransformNode<TransformScaleNode,1> PyScaleNode;
typedef PyTransformNode<TransformMirrorNode> PyMirrorNode;


class PyRotateNodeBase: public PyAbstractNode {
public:
    template <class ArgumentPack>
    PyRotateNodeBase(ArgumentPack const& args) {
      AbstractNode::NodeList childlist;
      const list &lchildren = args[children|empty_list];
      if (len(lchildren) > 0) childlist = list2NodeList(lchildren);
      else childlist.append(args[child|PyAbstractNode()].getNode());
      
      double a = args[ang|std::numeric_limits<double>::quiet_NaN()];
      if (myIsNaN( a )) {
	const list &lvec = args[vec|empty_list];
	node = make_shared<TransformRotateNode>(list2StdArray<Float3>(lvec), childlist);
      } else {
	const list &lvec = args[vec|empty_list];
	Float3 v = {{0,0,1}};
	if (len(lvec)>0) v = list2StdArray<Float3>(lvec);
	node = make_shared<TransformRotateAxisNode>(v, a, childlist);
      }
    }
};

class PyRotateNode: public PyRotateNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyRotateNode, (PyRotateNodeBase), tag,
      (optional (ang, (double))
	(vec, (list))
	(children, (list))
	(child, (PyAbstractNode)))
  )
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
    node = make_shared<TransformColorNode>(list2StdArray<Float4>(ca,1), AbstractNode::NodeList(1, n.getNode()));
  }
  PyColorNode(const list &ca, const list &a) {
    node = make_shared<TransformColorNode>(list2StdArray<Float4>(ca,1), list2NodeList(a));
  }
};

class PyCubeNodeBase: public PyAbstractNode {
public:
    template <class ArgumentPack>
    PyCubeNodeBase(ArgumentPack const& args) {
	const list &ldim = args[dim|empty_list];
	if (len(ldim) > 0)
	  node = make_shared<CubeNode>(list2StdArray<Float3>(ldim), args[center|false]);
	else {
	  double l = args[length|.0];
	  Float3 dim3 = {{ l,l,l }};
	  node = make_shared<CubeNode>(dim3, args[center|false]);
	}
    }
};
class PyCubeNode: public PyCubeNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyCubeNode, (PyCubeNodeBase), tag,
      (optional (dim, (list))
	(length, (double))
	(center, (bool)))
  )
};

class PySphereNode: public PyAbstractNode, public PyNodeAccuracy {
public:
  PySphereNode(double r) {
    SphereNode::Pointer p = make_shared<SphereNode>(r, ctx.getAcc());
    initAcc(p);
    node = p;
  }
};


class PyCylinderNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyCylinderNodeBase(ArgumentPack const& args) {
      double height = args[h|std::numeric_limits<double>::quiet_NaN()];
      double rad = args[r|std::numeric_limits<double>::quiet_NaN()];
      double rad1 = args[r1|std::numeric_limits<double>::quiet_NaN()];
      double rad2 = args[r2|std::numeric_limits<double>::quiet_NaN()];
      if (myIsNaN( rad )) {
	if (myIsNaN(rad1) && myIsNaN(rad2)) {
	  std::cerr << __func__ << " warning no valid radius - using 10" << std::endl;
	  rad1 = rad2 = 10.0;
	} else {
	  if (myIsNaN(rad1)) rad1 = rad2;
	  if (myIsNaN(rad2)) rad2 = rad1;
	}
      } else rad1 = rad2 = rad;
      CylinderNode::Pointer p = make_shared<CylinderNode>(rad1, rad2, height, args[center|false], ctx.getAcc());
      node = p;
      initAcc(p);      
    }
};

class PyCylinderNode: public PyCylinderNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyCylinderNode, (PyCylinderNodeBase), tag,
      (required (h, (double)))
      (optional (r, (double))
	(r1, (double))
	(r2, (double))
	(center, (bool)))
  )
};

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

class PyCircleNode: public PyAbstractNode, public PyNodeAccuracy {
public:
  PyCircleNode(double r) {
    CircleNode::Pointer p = make_shared<CircleNode>(r, ctx.getAcc());
    initAcc(p);
    node = p;
  }
};

class PyPolygonNode: public PyAbstractNode {
public:
  PyPolygonNode(const list &points, const list &paths=list(), unsigned int convexity=5) {
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

class PyDxfLinearExtrudeNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyDxfLinearExtrudeNodeBase(ArgumentPack const& args) {
      DxfLinearExtrudeNode::Pointer p(new DxfLinearExtrudeNode(
	AbstractNode::NodeList(), QString::fromStdString(args[file]), 
	QString::fromStdString(args[layer|std::string()]),
	args[h], args[twist|.0], list2StdArray<Float2>(args[origin|empty_list]), args[scale|1.0],
	args[convexity|5], args[slices|-1], args[center|false], ctx.getAcc()
      ));
      initAcc(p);
      node = p;     
    }
};

class PyDxfLinearExtrudeNode: public PyDxfLinearExtrudeNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyDxfLinearExtrudeNode, (PyDxfLinearExtrudeNodeBase), tag,
      (required (file, (std::string)))
      (optional (layer, (std::string)))
      (required (h, (double)))
      (optional (twist, (double))
	(origin, (list))
	(scale, (double))
	(convexity, (unsigned int))
	(slices, (int))
	(center, (bool)))
  )
};

class PyLinearExtrudeNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyLinearExtrudeNodeBase(ArgumentPack const& args) {
      AbstractNode::NodeList childlist;
      const list &lchildren = args[children|empty_list];
      if (len(lchildren) > 0) childlist = list2NodeList(lchildren);
      else childlist.append(args[child|PyAbstractNode()].getNode());
      
      DxfLinearExtrudeNode::Pointer p(new DxfLinearExtrudeNode(
	childlist, QString(), QString(),
	args[h], args[twist|.0], Float2(), 0,
	args[convexity|5], args[slices|-1], args[center|false], ctx.getAcc()
      ));
      initAcc(p);
      node = p;     
    }
};

class PyLinearExtrudeNode: public PyLinearExtrudeNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyLinearExtrudeNode, (PyLinearExtrudeNodeBase), tag,
      (required (h, (double)))
      (optional (twist, (double))
	(convexity, (unsigned int))
	(slices, (int))
	(center, (bool))
	(child, (PyAbstractNode))
	(children, (list)))
  )
};

class PyDxfRotateExtrudeNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyDxfRotateExtrudeNodeBase(ArgumentPack const& args) {
      DxfRotateExtrudeNode::Pointer p = make_shared<DxfRotateExtrudeNode>(
	AbstractNode::NodeList(), QString::fromStdString(args[file]), 
	QString::fromStdString(args[layer|std::string()]),
	list2StdArray<Float2>(args[origin|empty_list]), args[scale|1.0],
	args[convexity|5], ctx.getAcc()
      );
      initAcc(p);
      node = p;     
    }
};

class PyDxfRotateExtrudeNode: public PyDxfRotateExtrudeNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyDxfRotateExtrudeNode, (PyDxfRotateExtrudeNodeBase), tag,
      (required (file, (std::string)))
      (optional (layer, (std::string))
	(origin, (list))
	(scale, (double))
	(convexity, (unsigned int)))
  )
};

class PyRotateExtrudeNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyRotateExtrudeNodeBase(ArgumentPack const& args) {
      AbstractNode::NodeList childlist;
      const list &lchildren = args[children|empty_list];
      if (len(lchildren) > 0) childlist = list2NodeList(lchildren);
      else childlist.append(args[child|PyAbstractNode()].getNode());

      DxfRotateExtrudeNode::Pointer p = make_shared<DxfRotateExtrudeNode>(
	childlist, 
	QString(), QString(), Float2(), 1.0,
	args[convexity|5], ctx.getAcc()
      );
      initAcc(p);
      node = p;     
    }
};

class PyRotateExtrudeNode: public PyRotateExtrudeNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyRotateExtrudeNode, (PyRotateExtrudeNodeBase), tag,
      (optional (convexity, (unsigned int))
	(child, (PyAbstractNode))
	(children, (list)))
  )
};

class PySurfaceNodeBase: public PyAbstractNode {
public:
    template <class ArgumentPack>
    PySurfaceNodeBase(ArgumentPack const& args) {
      node = make_shared<SurfaceNode>(QString::fromStdString(args[file]), args[convexity|5], args[center|false]);    
    }
};

class PySurfaceNode: public PySurfaceNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PySurfaceNode, (PySurfaceNodeBase), tag,
      (required (file, (std::string)))
      (optional (convexity, (unsigned int))
	(center, (bool)))
  )
};

class PyImportSTLNode: public PyAbstractNode {
public:
  PyImportSTLNode(const std::string &filename, unsigned int convexity=5) {
      node = make_shared<ImportSTLNode>(QString::fromStdString(filename), convexity);
  }
};

class PyImportDXFNodeBase: public PyAbstractNode, public PyNodeAccuracy {
public:
    template <class ArgumentPack>
    PyImportDXFNodeBase(ArgumentPack const& args) {
      ImportDXFNode::Pointer p = make_shared<ImportDXFNode>(QString::fromStdString(args[file]),
	QString::fromStdString(args[layer|std::string()]), 
	list2StdArray<Float2>(args[origin|empty_list]), args[convexity|5], args[scale|1.0], ctx.getAcc());      
      initAcc(p);
      node = p;     
    }
};

class PyImportDXFNode: public PyImportDXFNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyImportDXFNode, (PyImportDXFNodeBase), tag,
      (required (file, (std::string)))
      (optional (layer, (std::string))
	(origin, (list))
	(scale, (double))
	(convexity, (unsigned int)))
  )
};

class PyProjectionNodeBase: public PyAbstractNode {
public:
    template <class ArgumentPack>
    PyProjectionNodeBase(ArgumentPack const& args) {
      AbstractNode::NodeList childlist;
      const list &lchildren = args[children|empty_list];
      if (len(lchildren) > 0) childlist = list2NodeList(lchildren);
      else childlist.append(args[child|PyAbstractNode()].getNode());
      node = make_shared<ProjectionNode>(childlist, args[cut_mode|false], args[convexity|5]);    
    }
};

class PyProjectionNode: public PyProjectionNodeBase {
public:
  BOOST_PARAMETER_CONSTRUCTOR(PyProjectionNode, (PyProjectionNodeBase), tag,
      (optional (child, (PyAbstractNode))
	(children, (list))
	(convexity, (unsigned int))
	(cut_mode, (bool)))
  )
};


class PyMinkowskiNode: public PyAbstractNode {
public:
  PyMinkowskiNode(const list &a, unsigned int convexity=5) {
    node = make_shared<CgaladvMinkowskiNode>(list2NodeList(a), convexity);
  }
};

/*
BOOST_PARAMETER_FUNCTION((double), pyDxfDim, tag,
    (required (file, *))
    (optional (layer, *, std::string())
      (name, *, std::string())
      (origin, *, empty_list)
      (scale, *, 1.0))
){
*/  
double pyDxfDim(const std::string &file, const std::string &layer=std::string(), const std::string &name=std::string(), list origin=empty_list, double scale=1.0) {
  return dxf_dim(QString::fromStdString(file), QString::fromStdString(layer), QString::fromStdString(name), list2StdArray<Float2>(origin), scale);
}
/*
struct pyDxfDim_fwd
{
    template <class A0, class A1, class A2, class A3, class A4>
    void operator()(boost::type<void>, A0 const& a0, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        return pyDxfDim(a0, a1, a2, a3, a4);
    }
};
*/

BOOST_PYTHON_FUNCTION_OVERLOADS(pyDxfDim_overloads, pyDxfDim, 1, 5)

list pyDxfCross(const std::string &filename, const std::string &layername=std::string(), list origin=list(), double scale=1.0) {
  list res;
  Float2 cross = dxf_cross(QString::fromStdString(filename), QString::fromStdString(layername), list2StdArray<Float2>(origin), scale);
  res.append<double>(cross[0]);
  res.append<double>(cross[1]);
  return res;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(pyDxfCross_overloads, pyDxfCross, 1, 4)

void assemble(const PyAbstractNode &n) {
	ctx.setResult(n);	
}


BOOST_PYTHON_MODULE(openscad) {
  namespace py = boost::parameter::python;
  namespace mpl = boost::mpl;
  class_<PyAbstractNode>("AbstractNode")
    .add_property("highlight", &PyAbstractNode::getHighlight, &PyAbstractNode::setHighlight)
    .add_property("background", &PyAbstractNode::getBackground, &PyAbstractNode::setBackground);    
  class_<PyNodeAccuracy>("NodeAccuracy")
    .add_property("_fn", &PyNodeAccuracy::getfn, &PyNodeAccuracy::setfn)
    .add_property("_fs", &PyNodeAccuracy::getfs, &PyNodeAccuracy::setfs)
    .add_property("_fa", &PyNodeAccuracy::getfa, &PyNodeAccuracy::setfa);
  class_<PyUnionNode, bases<PyAbstractNode> >("union", init<list>());
  class_<PyDifferenceNode, bases<PyAbstractNode> >("difference", init<list>());
  class_<PyIntersectionNode, bases<PyAbstractNode> >("intersection", init<list>());
  class_<PyScaleNode, bases<PyAbstractNode> >("scale", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyTranslateNode, bases<PyAbstractNode> >("translate", init<list, PyAbstractNode>()).def(init<list, list>());

  class_<PyRotateNodeBase, bases<PyAbstractNode> >("_RotateBase", no_init);
  class_<PyRotateNode, bases<PyRotateNodeBase> >("rotate", no_init)
    .def(py::init< mpl::vector< tag::ang(double), tag::vec*(list), tag::child(PyAbstractNode)> >())
    .def(py::init< mpl::vector< tag::ang(double), tag::vec*(list), tag::children(list)> >())
    .def(py::init< mpl::vector< tag::vec(list), tag::child(PyAbstractNode)> >())
    .def(py::init< mpl::vector< tag::vec(list), tag::children(list)> >());
  
  class_<PyMirrorNode, bases<PyAbstractNode> >("mirror", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyMatrixNode, bases<PyAbstractNode> >("matrix", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyColorNode, bases<PyAbstractNode> >("color", init<list, PyAbstractNode>()).def(init<list, list>());
  class_<PyCubeNodeBase, bases<PyAbstractNode> >("_CubeBase", no_init);
  class_<PyCubeNode, bases<PyCubeNodeBase> >("cube", no_init)
    .def(py::init< mpl::vector< tag::length(double), tag::center*(bool)> >())
    .def(py::init< mpl::vector< tag::dim(list), tag::center*(bool)> >());
  class_<PySphereNode, bases<PyAbstractNode, PyNodeAccuracy> >("sphere", init<double>());
  class_<PyCylinderNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_CylinderBase", no_init);
  class_<PyCylinderNode, bases<PyCylinderNodeBase> >("cylinder", no_init)
    .def(py::init< mpl::vector< tag::h(double), tag::r(double), tag::center*(bool)> >())
    .def(py::init< mpl::vector< tag::h(double), tag::r1(double), tag::r2(double), tag::center*(bool)> >());
  
  class_<PyPolyhedronNode, bases<PyAbstractNode> >("polyhedron", init<list, list, optional<unsigned int> >());
  class_<PySquareNode, bases<PyAbstractNode> >("square", init<list, optional<bool> >()).def(init<double, optional<bool> >());
  class_<PyCircleNode, bases<PyAbstractNode, PyNodeAccuracy> >("circle", init<double>());
  class_<PyPolygonNode, bases<PyAbstractNode> >("polygon", init<list, optional<list, unsigned int> >());
  class_<PyRenderNode, bases<PyAbstractNode> >("render", init<PyAbstractNode, optional<unsigned int> >()).def(init<list, optional<unsigned int> >());

  class_<PyDxfLinearExtrudeNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_DXFLinearExtrudeBase", no_init);
  class_<PyDxfLinearExtrudeNode, bases<PyDxfLinearExtrudeNodeBase> >("dxf_linear_extrude", no_init)
    .def(py::init< mpl::vector< tag::file(std::string), tag::layer*(std::string), tag::h(double),
	 tag::twist*(double), tag::origin*(list), tag::scale*(double),
	 tag::convexity*(unsigned int), tag::slices*(int), tag::center*(bool) > >());

  class_<PyLinearExtrudeNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_LinearExtrudeBase", no_init);
  class_<PyLinearExtrudeNode, bases<PyLinearExtrudeNodeBase> >("linear_extrude", no_init)
    .def(py::init< mpl::vector< tag::h(double), tag::twist*(double),
	 tag::convexity*(unsigned int), tag::slices*(int), tag::center*(bool), tag::child(PyAbstractNode) > >())
    .def(py::init< mpl::vector< tag::h(double), tag::twist*(double),
	 tag::convexity*(unsigned int), tag::slices*(int), tag::center*(bool), tag::children(list) > >());

    
  class_<PyDxfRotateExtrudeNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_DXFRotateExtrudeBase", no_init);
  class_<PyDxfRotateExtrudeNode, bases<PyDxfRotateExtrudeNodeBase> >("dxf_rotate_extrude", no_init)
    .def(py::init< mpl::vector< tag::file(std::string), tag::layer*(std::string),
	 tag::origin*(list), tag::scale*(double), tag::convexity*(unsigned int)> >());
    
  class_<PyRotateExtrudeNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_RotateExtrudeBase", no_init);
  class_<PyRotateExtrudeNode, bases<PyRotateExtrudeNodeBase> >("rotate_extrude", no_init)
    .def(py::init< mpl::vector< tag::convexity*(unsigned int), tag::child(PyAbstractNode) > >())
    .def(py::init< mpl::vector< tag::convexity*(unsigned int), tag::children(list) > >());

  class_<PySurfaceNodeBase, bases<PyAbstractNode> >("_SurfaceNodeBase", no_init);
  class_<PySurfaceNode, bases<PySurfaceNodeBase> >("surface", no_init)
    .def(py::init< mpl::vector< tag::file(std::string), tag::convexity*(unsigned int), tag::center*(bool) > >());

  class_<PyImportSTLNode, bases<PyAbstractNode> >("import_stl", init<std::string, optional<unsigned int> >());

  class_<PyImportDXFNodeBase, bases<PyAbstractNode, PyNodeAccuracy> >("_ImportDXFNodeBase", no_init);
  class_<PyImportDXFNode, bases<PyImportDXFNodeBase> >("import_dxf", no_init)
    .def(py::init< mpl::vector< tag::file(std::string), tag::layer*(std::string),
	 tag::origin*(list), tag::scale*(double), tag::convexity*(unsigned int)> >());


  class_<PyProjectionNodeBase, bases<PyAbstractNode> >("_ProjectionBase", no_init);
  class_<PyProjectionNode, bases<PyProjectionNodeBase> >("projection", no_init)
    .def(py::init< mpl::vector< tag::convexity*(unsigned int), tag::cut_mode*(bool), tag::child(PyAbstractNode) > >())
    .def(py::init< mpl::vector< tag::convexity*(unsigned int), tag::cut_mode*(bool), tag::children(list) > >());
  
  class_<PyMinkowskiNode, bases<PyAbstractNode> >("minkowski", init<list, optional<unsigned int> >());
    
    
/* doesn't work: error: no matching function for call to ´def(const char [7])´
  def< pyDxfDim_fwd
      , mpl::vector<
            double, tag::file(std::string), tag::layer*(std::string), tag::name*(std::string),
            tag::origin*(list), tag::scale*(double)
        >
    >("dxfdim");   
*/    
  def("DxfDim", pyDxfDim, pyDxfDim_overloads());
  def("DxfCross", pyDxfCross, pyDxfCross_overloads());
  def("assemble", assemble);
}

PythonScript::PythonScript(double time) {
  PyImport_AppendInittab(const_cast<char*>(PyContext::nsopenscad.c_str()), &initopenscad );
  Py_Initialize();
  object openscad_module( (handle<>(PyImport_ImportModule(PyContext::nsopenscad.c_str()))) );
  ctx.init(openscad_module, time);
  exec(
"def nodeAdd(self, other):\n"
"	return openscad.union([self, other])\n"
"def nodeSub(self, other):\n"
"	return openscad.difference([self, other])\n"
"def nodeAnd(self, other):\n"
"	return openscad.intersection([self, other])\n"
"def nodeMul(self, other):\n"
"	return openscad.minkowski([self, other])\n"
"openscad.AbstractNode.__add__ = nodeAdd\n"
"openscad.AbstractNode.__sub__ = nodeSub\n"
"openscad.AbstractNode.__and__ = nodeAnd\n"
"openscad.AbstractNode.__mul__ = nodeMul\n"
"def fn(self,val):\n"
"	self._fn = val\n"
"	return self\n"
"openscad.NodeAccuracy.fn = fn\n"
"def fs(self,val):\n"
"	self._fs = val\n"
"	return self\n"
"openscad.NodeAccuracy.fs = fs\n"
"def fa(self,val):\n"
"	self._fa = val\n"
"	return self\n"
"openscad.NodeAccuracy.fa = fa\n"
"def translate(self,v):\n"
"	o = translate(v,self)\n"
"	return o\n"
"openscad.AbstractNode.translate = translate\n"
"def scale(self,v):\n"
"	o = scale(v,self)\n"
"	return o\n"
"openscad.AbstractNode.scale = scale\n"
"def mirror(self,v):\n"
"	o = mirror(v,self)\n"
"	return o\n"
"openscad.AbstractNode.mirror = mirror\n"
"def matrix(self,v):\n"
"	o = matrix(v,self)\n"
"	return o\n"
"openscad.AbstractNode.matrix = matrix\n"
"def color(self,v):\n"
"	o = color(v,self)\n"
"	return o\n"
"openscad.AbstractNode.color = color\n"
"def rotate(self,ang=0,v=[0,0,1]):\n"
"	if type(ang).__name__=='list':\n"
"		v=ang\n"
"		ang=0\n"
"	if ang==0:\n"
"		o = rotate(v,child=self)\n"
"	else:\n"
"		o = rotate(ang,v,child=self)\n"
"	return o\n"
"openscad.AbstractNode.rotate = rotate\n"
  , ctx.main_namespace);
  //ImportOFFNode
  //CgaladvMinkowskiNode
  //CgaladvGlideNode
  //CgaladvSubdivNode
}

PythonScript::~PythonScript() {}

AbstractNode::Pointer PythonScript::evaluate(const std::string &code, const std::string &path) {
  try {
    str dir(path);
    if (len(dir)==0) dir =".";
    object sys = import("sys"); 
    sys.attr("path").attr("insert")(0, dir); 
    exec(code.c_str(), ctx.main_namespace);
    PyAbstractNode &resNode = extract<PyAbstractNode&>(ctx.getResult());
    sys.attr("path").attr("remove")(dir); 
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
