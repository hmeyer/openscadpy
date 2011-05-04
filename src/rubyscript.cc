#include "rubyscript.h"

#include "node.h"
#include "primitives.h"
#include "csgops.h"
#include "transform.h"
#include <rice/Array.hpp>
#include <rice/Hash.hpp>
#include <rice/Symbol.hpp>
#include "rice/Data_Type.hpp"
#include "rice/Constructor.hpp"
#include <ruby.h>
#include <boost/make_shared.hpp>
#include <boost/typeof/typeof.hpp>

using namespace Rice;
using boost::make_shared;

const std::string FNVarName("$fn");
const std::string FSVarName("$fs");
const std::string FAVarName("$fa");
VALUE fn = (double)0.0;
VALUE fs = (double)1.0;
VALUE fa = (double)12.0;

class RBAbstractNode {
protected:
  AbstractNode::Pointer node;
public:
  AbstractNode::Pointer getNode() const { return node; }
  std::string to_s() const {
    return node->dump("").toStdString();
  }
};

template<>
AbstractNode::NodeList from_ruby<AbstractNode::NodeList>(Object x) {
  AbstractNode::NodeList list;
  if (x.class_of() == Array().class_of()) {
    const Array &a(x);
    for(Array::const_iterator it = a.begin(); it != a.end(); ++it) {
      try {
	list.push_back(from_ruby<RBAbstractNode>(it->value()).getNode());
      } catch( std::exception &e) {
	std::cerr << "from_ruby<AbstractNode::NodeList> warning:" << e.what() << std::endl;
      }        
    }
  } else {
      try {
	list.push_back(from_ruby<RBAbstractNode>(x.value()).getNode());
      } catch( std::exception &e) {
	std::cerr << "from_ruby<AbstractNode::NodeList> warning:" << e.what() << std::endl;
      }  
  }
  return list;
}


template<csg_type_e type> 
class RBCSGNode: public RBAbstractNode {
public:
  RBCSGNode(const Array &a) {
    node = make_shared<CsgNode>(type, from_ruby<AbstractNode::NodeList>(a));
  }
};

typedef RBCSGNode< CSG_TYPE_UNION > RBUnionNode;
typedef RBCSGNode< CSG_TYPE_DIFFERENCE > RBDifferenceNode;
typedef RBCSGNode< CSG_TYPE_INTERSECTION > RBIntersectionNode;


template<class Iterator>
void Array2Container(const Array &a, Iterator begin, Iterator end) {
  try {
    for(Array::const_iterator it = a.begin(); it!=a.end() && begin != end; ++it,++begin)
      *begin = from_ruby<BOOST_TYPEOF(*begin)>(it->value());
  } catch( std::exception &e) {
    std::cerr << "Array to Array2Container warning:" << e.what() << std::endl;
  }
}

template<class Iterator>
void Array2ContainerContainer(const Array &a, Iterator begin, Iterator end) {
  try {
    for(Array::const_iterator it = a.begin(); it!=a.end() && begin != end; ++it,++begin) {
      const Array &aa(*it);
      Array2Container(aa, begin->begin(), begin->end());
    }
  } catch( std::exception &e) {
    std::cerr << "Array to Array2Container warning:" << e.what() << std::endl;
  }
}

template<class StdArray>
StdArray Array2StdArray(const Array &a) {
  StdArray p;
  Array2Container(a,p.begin(),p.end());
  return p;
}

template<class NodeClass>
class RBTransformNode: public RBAbstractNode {
public:
  RBTransformNode(const Array &va, const Object &a) {
    node = make_shared<NodeClass>(Array2StdArray<Float3>(va), from_ruby<AbstractNode::NodeList>(a));
  }
};

typedef RBTransformNode<TransformScaleNode> RBScaleNode;
typedef RBTransformNode<TransformTranslateNode> RBTranslateNode;
typedef RBTransformNode<TransformRotateNode> RBRotateNode;
typedef RBTransformNode<TransformMirrorNode> RBMirrorNode;
class RBRotateAxisNode: public RBAbstractNode {
public:
  RBRotateAxisNode(const Array &va, double ang, const Object &a) {
    node = make_shared<TransformRotateAxisNode>(Array2StdArray<Float3>(va), ang, from_ruby<AbstractNode::NodeList>(a));
  }
};
class RBMatrixNode: public RBAbstractNode {
public:
  RBMatrixNode(const Array &ma, const Object &a) {
    node = make_shared<TransformMatrixNode>(Array2StdArray<Float16>(ma), from_ruby<AbstractNode::NodeList>(a));
  }
};
class RBColorNode: public RBAbstractNode {
public:
  RBColorNode(const Array &ca, const Object &a) {
    node = make_shared<TransformColorNode>(Array2StdArray<Float4>(ca), from_ruby<AbstractNode::NodeList>(a));
  }
};





PrimitiveNode::Accuracy getAcc(void) {
  PrimitiveNode::Accuracy acc;
  try {
    acc = PrimitiveNode::Accuracy(fn, fs, fa);
  } catch( std::exception &e) {
    std::cerr << "global variable warning:" << e.what() << std::endl;
  }
  return acc;
}



class RBCubeNode: public RBAbstractNode {
public:
  RBCubeNode(const Array &adim, bool center) {  
    node = make_shared<CubeNode>(Array2StdArray<Float3>(adim), center);  
  }
};

class RBSphereNode: public RBAbstractNode {
public:
  RBSphereNode(double r) {
    node = make_shared<SphereNode>(r, getAcc());
  }
};

class RBCylinderNode: public RBAbstractNode {
public:
  RBCylinderNode(const Object &r, double h, bool center) {
    Float2 rr={{1.0,1.0}};
    try {
      if (r.class_of() == Array().class_of()) {
	const Array &a(r);
	int i=0;
	for(Array::const_iterator it = a.begin(); it != a.end() && i<rr.static_size; ++it,++i)
	    rr[i] = from_ruby<double>(it->value());
      }
      else rr[0] = rr[1] = from_ruby<double>(r.value());    
    } catch( std::exception &e) {
      std::cerr << "CylinderNode warning:" << e.what() << std::endl;
    }        
    node = make_shared<CylinderNode>(rr[0], rr[1], h, center, getAcc());
  }
};

class RBPolyhedronNode: public RBAbstractNode {
public:
  RBPolyhedronNode(const Array &rpoints, const Array &rtriangles, unsigned int convexity) {
    Vec3D points(rpoints.size());
    VecTriangles triangles(rtriangles.size());
    Array2ContainerContainer(rpoints, points.begin(), points.end());
    Array2ContainerContainer(rtriangles, triangles.begin(), triangles.end());
    node = make_shared<PolyhedronNode>(points, triangles, convexity);
  }
};

RubyScript::RubyScript():status(0) {
  ruby_init();
  Data_Type<RBAbstractNode> rb_AbstractNode =
    define_class<RBAbstractNode>("AbstractNode")
    .define_method("to_s", &RBAbstractNode::to_s);
  Data_Type<RBUnionNode> rb_UnionNode =
    define_class<RBUnionNode, RBAbstractNode>("Union")
    .define_constructor(Constructor<RBUnionNode, const Array>(),(Arg("children")));
  Data_Type<RBDifferenceNode> rb_DifferenceNode =
    define_class<RBDifferenceNode, RBAbstractNode>("Difference")
    .define_constructor(Constructor<RBDifferenceNode, const Array>(),(Arg("children")));
  Data_Type<RBIntersectionNode> rb_IntersectionNode =
    define_class<RBIntersectionNode, RBAbstractNode>("Intersection")
    .define_constructor(Constructor<RBIntersectionNode, const Array>(),(Arg("children")));    
  Data_Type<RBScaleNode> rb_ScaleNode =
    define_class<RBScaleNode, RBAbstractNode>("Scale")
    .define_constructor(Constructor<RBScaleNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBRotateNode> rb_RotateNode =
    define_class<RBRotateNode, RBAbstractNode>("Rotate")
    .define_constructor(Constructor<RBRotateNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBRotateAxisNode> rb_RotateAxisNode =
    define_class<RBRotateAxisNode, RBAbstractNode>("RotateAxis")
    .define_constructor(Constructor<RBRotateAxisNode, const Array, double, const Object>(),(Arg("vector"),Arg("ang"),Arg("children")));    
  Data_Type<RBMirrorNode> rb_MirrorNode =
    define_class<RBMirrorNode, RBAbstractNode>("Mirror")
    .define_constructor(Constructor<RBMirrorNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBTranslateNode> rb_TranslateNode =
    define_class<RBTranslateNode, RBAbstractNode>("Translate")
    .define_constructor(Constructor<RBTranslateNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBMatrixNode> rb_MatrixTransformNode =
    define_class<RBMatrixNode, RBAbstractNode>("MatrixTransform")
    .define_constructor(Constructor<RBMatrixNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBColorNode> rb_ColorNode =
    define_class<RBColorNode, RBAbstractNode>("Color")
    .define_constructor(Constructor<RBColorNode, const Array, const Object>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBCubeNode> rb_CubeNode =
    define_class<RBCubeNode, RBAbstractNode>("Cube")
    .define_constructor(Constructor<RBCubeNode, const Array, bool>(),(Arg("dim"),Arg("center")=(bool)false));
  Data_Type<RBSphereNode> rb_SphereNode =
    define_class<RBSphereNode, RBAbstractNode>("Sphere")
    .define_constructor(Constructor<RBSphereNode, double>(),(Arg("r")=1));
  Data_Type<RBCylinderNode> rb_CylinderNode =
    define_class<RBCylinderNode, RBAbstractNode>("Cylinder")
    .define_constructor(Constructor<RBCylinderNode, const Object, double, bool>(),(Arg("r"),Arg("h"),Arg("center")=(bool)false));
  Data_Type<RBPolyhedronNode> rb_PolyhedronNode =
    define_class<RBPolyhedronNode, RBAbstractNode>("Polyhedron")
    .define_constructor(Constructor<RBPolyhedronNode, const Array, const Array, unsigned int>(),(Arg("points"),Arg("triangles"),Arg("convexity")=(unsigned int)5));
  rb_define_variable(FNVarName.c_str(), &fn);
  rb_define_variable(FSVarName.c_str(), &fs);
  rb_define_variable(FAVarName.c_str(), &fa);    
}

RubyScript::~RubyScript() {
}


void error_code(int state, std::string &statustext) {
  statustext.clear();
#ifndef DYNAMIC_RUBY
#if !(defined(RUBY_VERSION) && RUBY_VERSION >= 19) \
    && !(defined(DYNAMIC_RUBY_VER) && DYNAMIC_RUBY_VER >= 19)
    extern VALUE ruby_errinfo;
#endif
#endif
    VALUE eclass;
    VALUE einfo;

#define TAG_RETURN      0x1
#define TAG_BREAK       0x2
#define TAG_NEXT        0x3
#define TAG_RETRY       0x4
#define TAG_REDO        0x5
#define TAG_RAISE       0x6
#define TAG_THROW       0x7
#define TAG_FATAL       0x8
#define TAG_MASK        0xf

    switch (state) {
    case TAG_RETURN:
        statustext += "E267: unexpected return";
        break;
    case TAG_NEXT:
        statustext += "E268: unexpected next";
        break;
    case TAG_BREAK:
        statustext += "E269: unexpected break";
        break;
    case TAG_REDO:
        statustext += "E270: unexpected redo";
        break;
    case TAG_RETRY:
        statustext += "E271: retry outside of rescue clause";
        break;
    case TAG_RAISE:
    case TAG_FATAL:
#ifdef RUBY19_OR_LATER
        eclass = CLASS_OF(rb_errinfo());
        einfo = rb_obj_as_string(rb_errinfo());
#else
        eclass = CLASS_OF(ruby_errinfo);
        einfo = rb_obj_as_string(ruby_errinfo);
#endif
        if (eclass == rb_eRuntimeError && RSTRING_LEN(einfo) == 0) {
            statustext += "E272: unhandled exception";
        }
        else {
            VALUE epath;
            epath = rb_class_path(eclass);
            statustext += RSTRING_PTR(epath);
	    statustext + "\n";
	    statustext += RSTRING_PTR(einfo);
        }
        break;
    default:
        statustext+="E273: unknown longjmp status " + state;
        break;
    }
}

std::string RubyScript::getStatusText() {
  if (statusText.empty()) error_code(status,statusText);
  return statusText;
}


AbstractNode::Pointer RubyScript::evaluate(const std::string &code, const std::string &path) {
  statusText.clear();
  VALUE val = rb_eval_string_protect(code.c_str(),&status);
  try {
    RBAbstractNode *rba = from_ruby<RBAbstractNode *>(val);
    return rba->getNode();
  } catch (std::exception &e) {
    statusText = e.what();
    status = 1;
    return AbstractNode::Pointer();
  }
}

bool RubyScript::error() const {
  return status != 0;
}

