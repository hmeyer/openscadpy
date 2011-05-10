#include "matrix.h"

void value2points2d(const Value &v, Vec2D &points) {
  points.clear();
  Float2 p;
  for (int i=0; i<v.vec.size(); i++) {
    v.vec[i]->getv2(p[0], p[1]);
    points.push_back(p);
  }  
}

void value2paths(const Value &v, VecPaths &paths) {
  paths.clear();
  VecPoints p;
  for (int i=0; i<v.vec.size(); i++) {
    for (int j=0; j<v.vec[i]->vec.size(); j++)
      p.push_back(v.vec[i]->vec[j]->num);
    paths.push_back(p);
    p.clear();
  }
}

void value2points3d(const Value &v, Vec3D &points) {
  points.clear();
  Float3 p;
  for (int i=0; i<v.vec.size(); i++) {
    v.vec[i]->getv3(p[0], p[1], p[2]);
    points.push_back(p);
  }  
}