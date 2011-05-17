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

#include "node.h"
#include "accuracy.h"

class PolySet;



class PrimitiveNode : public AbstractPolyNode {
public:
    typedef shared_ptr<PrimitiveNode> Pointer;
    static const double F_MINIMUM;
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

class SphereNode : public PrimitiveNode, public Accuracy {
	double r;
public:
	typedef shared_ptr<SphereNode> Pointer;
	SphereNode(double r, const Accuracy &acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), Accuracy(acc), r(r) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class CylinderNode : public PrimitiveNode, public Accuracy {
	bool center;
	double r1,r2;
	double h;
public:
	typedef shared_ptr<CylinderNode> Pointer;
	CylinderNode(double r1, double r2, double h, bool center=false, const Accuracy &acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), Accuracy(acc), center(center), r1(r1), r2(r2), h(h) {}
	CylinderNode(double r, double h, bool center=false, const Accuracy &acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), Accuracy(acc), center(center), r1(r), r2(r), h(h) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};

class PolyhedronNode : public PrimitiveNode {
	Vec3D points;
	VecPaths triangles;
public:
	PolyhedronNode(const Vec3D &points, const VecPaths &triangles, int convexity, const Props p=Props())
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

class CircleNode : public PrimitiveNode, public Accuracy {
	double r;
public:
	typedef shared_ptr<CircleNode> Pointer;
	CircleNode(double r, const Accuracy &acc=Accuracy(), const Props p=Props())
	  :PrimitiveNode(1,p), Accuracy(acc), r(r) {}
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
