#ifndef MATRIX_H
#define MATRIX_H

#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <vector>

typedef double FloatType;
typedef boost::array< FloatType, 20 > Float20;
typedef boost::shared_ptr<Float20> Float20Ptr;
typedef boost::array< FloatType, 16 > Float16;
typedef boost::array< FloatType, 4 > Float4;
typedef boost::array< FloatType, 3 > Float3;
typedef boost::array< FloatType, 2 > Float2;

typedef boost::array< Float4, 4> MatFloat4x4;
typedef boost::array< unsigned int, 3> UInt3;
typedef std::vector<unsigned int> VecPoints;
typedef std::vector<VecPoints> VecPaths;
typedef std::vector<Float2> Vec2D;
typedef std::vector<Float3> Vec3D;

template<typename T>
std::ostream& operator<<(std::ostream &out, const std::vector<T> &rhs){
  out << "[";
  typename std::vector<T>::const_iterator it = rhs.begin();
  if (it != rhs.end())
	out << *it++;
  while(it != rhs.end())
    out << ", " << *it++; 
  out << "]";
  return out;
}

template<typename T, std::size_t N>
std::ostream& operator<<(std::ostream &out, const boost::array<T,N> &rhs){
  out << "[";
  typename boost::array<T,N>::const_iterator it = rhs.begin();
  if (it != rhs.end()) 
	out << *it++;
  while(it != rhs.end())
    out << ", " << *it++; 
  out << "]";
  return out;
}


#endif
