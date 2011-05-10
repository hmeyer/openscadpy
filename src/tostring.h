#ifndef TOSTRING_H
#define TOSTRING_H

#include <sstream>
#include <string>

template<typename T>
std::string toString(const T & val)
{
    std::ostringstream ostr;
    ostr << val;
    return ostr.str();
}

#endif
