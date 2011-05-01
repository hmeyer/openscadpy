#ifndef RICE_H
#define RICE_H

#include <string>
#include <memory>
#include <boost/shared_ptr.hpp>

namespace Rice {
class VM;
}

class AbstractNode;

class RubyScript {
public:
  RubyScript();
  ~RubyScript();
  boost::shared_ptr<AbstractNode> evaluate(const std::string &code, const std::string &path);
  std::string getStatusText();
  bool error() const;
protected:
  int status;
  std::string statusText;
};


#endif