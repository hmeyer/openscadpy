#ifndef PRIMITIVES_H
#define PRIMITIVES_H

/*
 *  OpenSCAD (www.openscad.org)
 *  Copyright (C) 2009-2011 Clifford Wolf <clifford@clifford.at> and
 *                          Marius Kintel <marius@kintel.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  As a special exception, you have permission to link this program
 *  with the CGAL library and distribute executables, as long as you
 *  follow the requirements of the GNU GPL in regard to all of the
 *  software in the executable aside from CGAL.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "module.h"
#include "node.h"
#include <boost/concept_check.hpp>

class PolySet;



class PrimitiveNode : public AbstractPolyNode
{
public:
	struct Accuracy {
	  double fn;
	  double fs;
	  double fa;
	  Accuracy():fn(0),fs(3),fa(2) {}
	  Accuracy(double fn, double fs, double fa):fn(fn),fs(fs),fa(fa) {}
	};
	typedef shared_ptr<PrimitiveNode> Pointer;
	static const double F_MINIMUM = 0.01;
	int convexity;
	PrimitiveNode(int convex=1, const Props p=Props()) : AbstractPolyNode(p), convexity(convex) { }
	virtual QString dump(QString indent) const;
};

class CubeNode : public PrimitiveNode {
	bool center;
	Float3 dim;
public:
	CubeNode(const Float3 &dim, bool center=false, const Props p=Props())
	  :PrimitiveNode(1,p), center(center), dim(dim) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class SphereNode : public PrimitiveNode {
	double r;
	Accuracy acc;
public:
	SphereNode(double r, Accuracy acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), r(r), acc(acc) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class CylinderNode : public PrimitiveNode {
	bool center;
	double r1,r2;
	double h;
	Accuracy acc;
public:
	CylinderNode(double r1, double r2, double h, bool center=false, Accuracy acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), center(center), r1(r1), r2(r2), h(h), acc(acc) {}
	CylinderNode(double r, double h, bool center=false, Accuracy acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), center(center), r1(r), r2(r), h(h), acc(acc) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class PolyhedronNode : public PrimitiveNode {
	Vec3D points;
	VecTriangles triangles;
public:
	PolyhedronNode(const Vec3D &points, const VecTriangles &triangles, int convexity, const Props p=Props())
	  :PrimitiveNode(convexity,p), points(points), triangles(triangles) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class SquareNode : public PrimitiveNode {
	bool center;
	Float2 dim;
public:
	SquareNode(const Float2 &dim, bool center, const Props p=Props())
	  :PrimitiveNode(1,p), center(center), dim(dim) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class CircleNode : public PrimitiveNode {
	double r;
	Accuracy acc;
public:
	CircleNode(double r, Accuracy acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), r(r), acc(acc) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class PolygonNode : public PrimitiveNode {
	Vec2D points;
	VecPaths paths;
public:
	PolygonNode(const Vec2D &points, const VecPaths &paths, int convexity, const Props p=Props())
	  :PrimitiveNode(convexity,p), points(points), paths(paths) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};


#endif
