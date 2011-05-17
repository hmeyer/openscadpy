#ifndef DXFDIM_H_
#define DXFDIM_H_

#include <QHash>
#include "matrix.h"

extern QHash<QString,Float2> dxf_cross_cache;
extern QHash<QString,double> dxf_dim_cache;

double dxf_dim(const QString &filename, const QString &layername=QString(), const QString &name=QString(), Float2 origin=Float2(), double scale=1.0);
Float2 dxf_cross(const QString &filename, const QString &layername=QString(), Float2 origin=Float2(), double scale=1.0);

#endif
