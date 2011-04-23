#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <boost/function.hpp>
#include <QString>


class Interpreter_impl;
class AbstractNode;


class Interpreter {
public:
  Interpreter();
  ~Interpreter();
  typedef boost::function<void (const QString &message)> ErrorCallback;  
  int interpret(const QString &code);
  void setCallback(ErrorCallback callback) { cb = callback; }
  AbstractNode *getRoot();
private:
  Interpreter_impl *impl;
  ErrorCallback cb;
};




#endif
