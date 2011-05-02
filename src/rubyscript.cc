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

using namespace Rice;
using boost::make_shared;

const std::string FNVarName("$fn");
const std::string FSVarName("$fs");
const std::string FAVarName("$fa");
VALUE fn = 0.0;
VALUE fs = 1.0;
VALUE fa = 1.0;

class RBAbstractNode {
protected:
  AbstractNode::Pointer node;
public:
  AbstractNode::Pointer getNode() const;
};

AbstractNode::Pointer RBAbstractNode::getNode() const {
  return node;
}

AbstractNode::NodeList Array2NodeList(const Array &a) {
  AbstractNode::NodeList list;
  for(Array::const_iterator it = a.begin(); it != a.end(); ++it) {
    try {
      list.push_back(from_ruby<RBAbstractNode>(it->value()).getNode());
    } catch( std::exception &e) {
      std::cerr << "Array2NodeList warning:" << e.what() << std::endl;
    }        
  }
  return list;
}

template<csg_type_e type> 
class RBCSGNode: public RBAbstractNode {
public:
  RBCSGNode(const Array &a) {
    node = make_shared<CsgNode>(type, Array2NodeList(a));
  }
};

typedef RBCSGNode< CSG_TYPE_UNION > RBUnionNode;
typedef RBCSGNode< CSG_TYPE_DIFFERENCE > RBDifferenceNode;
typedef RBCSGNode< CSG_TYPE_INTERSECTION > RBIntersectionNode;


Float3 Array2Float3(const Array &a) {
  Float3 p;
  try {
    int i=0;
    for(Array::const_iterator it = a.begin(); it!=a.end() && i < p.static_size; ++it,++i)
      p[i] = from_ruby<double>(it->value());
  } catch( std::exception &e) {
    std::cerr << "Array to Float3 warning:" << e.what() << std::endl;
  }
  return p;
}

class RBTranslateNode: public RBAbstractNode {
public:
  RBTranslateNode(const Array &va, const Array &a) {
    node = make_shared<TransformTranslateNode>(Array2Float3(va), Array2NodeList(a));
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
    node = make_shared<CubeNode>(Array2Float3(adim), center);  
  }
};

class RBSphereNode: public RBAbstractNode {
public:
  RBSphereNode(double r) {
    node = make_shared<SphereNode>(r, getAcc());
  }
};


RubyScript::RubyScript():status(0) {
  ruby_init();
  Data_Type<RBAbstractNode> rb_AbstractNode =
    define_class<RBAbstractNode>("AbstractNode");
  Data_Type<RBUnionNode> rb_UnionNode =
    define_class<RBUnionNode, RBAbstractNode>("Union")
    .define_constructor(Constructor<RBUnionNode, const Array>(),(Arg("children")));
  Data_Type<RBDifferenceNode> rb_DifferenceNode =
    define_class<RBDifferenceNode, RBAbstractNode>("Difference")
    .define_constructor(Constructor<RBDifferenceNode, const Array>(),(Arg("children")));
  Data_Type<RBIntersectionNode> rb_IntersectionNode =
    define_class<RBIntersectionNode, RBAbstractNode>("Intersection")
    .define_constructor(Constructor<RBIntersectionNode, const Array>(),(Arg("children")));    
  Data_Type<RBTranslateNode> rb_TranslateNode =
    define_class<RBTranslateNode, RBAbstractNode>("Translate")
    .define_constructor(Constructor<RBTranslateNode, const Array, const Array>(),(Arg("vector"),Arg("children")));    
  Data_Type<RBCubeNode> rb_CubeNode =
    define_class<RBCubeNode, RBAbstractNode>("Cube")
    .define_constructor(Constructor<RBCubeNode, const Array, bool>(),(Arg("dim")=Array(),Arg("center")=false));
  Data_Type<RBSphereNode> rb_SphereNode =
    define_class<RBSphereNode, RBAbstractNode>("Sphere")
    .define_constructor(Constructor<RBSphereNode, double>(),(Arg("r")=1));
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

