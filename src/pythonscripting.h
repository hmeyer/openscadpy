#ifndef PYTHONSCRIPTING_H
#define PYTHONSCRIPTING_H

#include <boost/shared_ptr.hpp>
#include <string>
#include "accuracy.h"

class AbstractNode;


class PythonScript {
public:
  PythonScript(double time=0.0);
  ~PythonScript();
  boost::shared_ptr<AbstractNode> evaluate(const std::string &code, const std::string &path);
  std::string error(void);
protected:
  class PythonScriptImpl;
  boost::shared_ptr<PythonScriptImpl> impl;
};

#endif