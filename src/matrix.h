#ifndef MATRIX_H
#define MATRIX_H

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>

typedef double FloatType;
typedef boost::array< FloatType, 20 > Float20;
typedef boost::shared_ptr<Float20> Float20Ptr;
typedef boost::array< FloatType, 16 > Float16;
typedef boost::array< FloatType, 4 > Float4;
typedef boost::array< FloatType, 3 > Float3;

#endif
