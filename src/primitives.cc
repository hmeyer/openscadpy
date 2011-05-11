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

#include "primitives.h"
#include "polyset.h"
#include "context.h"
#include "dxfdata.h"
#include "dxftess.h"
#include "builtin.h"
#include "printutils.h"
#include <assert.h>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include "tostring.h"

using boost::make_shared;

enum primitive_type_e {
	CUBE,
	SPHERE,
	CYLINDER,
	POLYHEDRON,
	SQUARE,
	CIRCLE,
	POLYGON
};

class PrimitiveModule : public AbstractModule
{
public:
	primitive_type_e type;
	PrimitiveModule(primitive_type_e type) : type(type) { }
	virtual AbstractNode::Pointer evaluate(const Context *ctx, const ModuleInstantiation *inst) const;
};





AbstractNode::Pointer PrimitiveModule::evaluate(const Context *ctx, const ModuleInstantiation *inst) const
{
	QVector<QString> argnames;
	QVector<Expression*> argexpr;

	if (type == CUBE) {
		argnames = QVector<QString>() << "size" << "center";
	}
	if (type == SPHERE) {
		argnames = QVector<QString>() << "r";
	}
	if (type == CYLINDER) {
		argnames = QVector<QString>() << "h" << "r1" << "r2" << "center";
	}
	if (type == POLYHEDRON) {
		argnames = QVector<QString>() << "points" << "triangles" << "convexity";
	}
	if (type == SQUARE) {
		argnames = QVector<QString>() << "size" << "center";
	}
	if (type == CIRCLE) {
		argnames = QVector<QString>() << "r";
	}
	if (type == POLYGON) {
		argnames = QVector<QString>() << "points" << "paths" << "convexity";
	}

	Context c(ctx);
	c.args(argnames, argexpr, inst->argnames, inst->argvalues);
	
	Accuracy acc;

	acc.fn = c.lookup_variable("$fn").num;
	acc.fs = c.lookup_variable("$fs").num;
	acc.fa = c.lookup_variable("$fa").num;

	if (acc.fs < PrimitiveNode::F_MINIMUM) {
		PRINTF("WARNING: $fs too small - clamping to %f", PrimitiveNode::F_MINIMUM);
		acc.fs = PrimitiveNode::F_MINIMUM;
	}
	if (acc.fa < PrimitiveNode::F_MINIMUM) {
		PRINTF("WARNING: $fa too small - clamping to %f", PrimitiveNode::F_MINIMUM);
		acc.fa = PrimitiveNode::F_MINIMUM;
	}
	int convexity = c.lookup_variable("convexity", true).num;
	if (convexity < 1)
		convexity = 1;


	if (type == CUBE) {
		Value size = c.lookup_variable("size");
		Value center = c.lookup_variable("center");
		Float3 dim;
		size.getnum(dim[0]);
		size.getnum(dim[1]);
		size.getnum(dim[2]);
		size.getv3(dim[0], dim[1], dim[2]);
		bool c(false);
		if (center.type == Value::BOOL) {
			c = center.b;
		}
		return make_shared<CubeNode>(dim,c,inst);
	}

	if (type == SPHERE) {
		Value val_r = c.lookup_variable("r");
		double r = 1;
		if (val_r.type == Value::NUMBER) {
			r = val_r.num;
		}
		return make_shared<SphereNode>(r,acc,inst);
	}

	if (type == CYLINDER) {
		Value val_h = c.lookup_variable("h");
		Value val_r, val_r1, val_r2;
		val_r1 = c.lookup_variable("r1");
		val_r2 = c.lookup_variable("r2");
		if (val_r1.type != Value::NUMBER && val_r2.type != Value::NUMBER)
			val_r = c.lookup_variable("r", true); // silence warning since r has no default value
		Value val_center = c.lookup_variable("center");
		bool center = false;
		double r1 = 1,r2 = 1,h = 1;
		if (val_h.type == Value::NUMBER) {
			h = val_h.num;
		}
		if (val_r.type == Value::NUMBER) {
			r1 = val_r.num;
			r2 = val_r.num;
		}
		if (val_r1.type == Value::NUMBER) {
			r1 = val_r1.num;
		}
		if (val_r2.type == Value::NUMBER) {
			r2 = val_r2.num;
		}
		if (val_center.type == Value::BOOL) {
			center = val_center.b;
		}
		return make_shared<CylinderNode>(r1,r2,h,center,acc,inst);
	}

	if (type == POLYHEDRON) {
		Vec3D points;
		Value vpoints = c.lookup_variable("points");
		value2points3d(vpoints,points);

		VecPaths triangles;
		Value vtriangles = c.lookup_variable("triangles");
		value2paths(vtriangles,triangles);
		
		return make_shared<PolyhedronNode>(points,triangles,convexity,inst);
	}

	if (type == SQUARE) {
		Value val_size = c.lookup_variable("size");
		Value val_center = c.lookup_variable("center");
		Float2 dim;
		bool center = false;
		val_size.getnum(dim[0]);
		val_size.getnum(dim[1]);
		val_size.getv2(dim[0], dim[1]);
		if (val_center.type == Value::BOOL) {
			center = val_center.b;
		}
		return make_shared<SquareNode>(dim,center,inst);
	}

	if (type == CIRCLE) {
		Value val_r = c.lookup_variable("r");
		double r=1;
		if (val_r.type == Value::NUMBER) {
			r = val_r.num;
		}
		return make_shared<CircleNode>(r,acc,inst);
	}

	if (type == POLYGON) {
		Vec2D points;
		Value vpoints = c.lookup_variable("points");
		value2points2d(vpoints,points);

		VecPaths paths;
		Value vpaths = c.lookup_variable("paths");
		value2paths(vpaths,paths);
		
		return make_shared<PolygonNode>(points,paths,convexity,inst);
	}
	return AbstractNode::Pointer();
}

void register_builtin_primitives()
{
	builtin_modules["cube"] = new PrimitiveModule(CUBE);
	builtin_modules["sphere"] = new PrimitiveModule(SPHERE);
	builtin_modules["cylinder"] = new PrimitiveModule(CYLINDER);
	builtin_modules["polyhedron"] = new PrimitiveModule(POLYHEDRON);
	builtin_modules["square"] = new PrimitiveModule(SQUARE);
	builtin_modules["circle"] = new PrimitiveModule(CIRCLE);
	builtin_modules["polygon"] = new PrimitiveModule(POLYGON);
}

static void generate_circle(Vec2D &circle, double r)
{
	for (unsigned int i=0; i<circle.size(); i++) {
		double phi = (M_PI*2* (i + 0.5)) / circle.size();
		circle[i][0] = r*cos(phi);
		circle[i][1] = r*sin(phi);
	}
}

struct ring_s {
	Vec2D points;
	double z;
};

PolySet *CubeNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  const double &x = dim[0], &y = dim[1], &z = dim[2];
  if (x > 0 && y > 0 && z > 0)
  {
    double x1, x2, y1, y2, z1, z2;
    if (center) {
      x1 = -x/2;
      x2 = +x/2;
      y1 = -y/2;
      y2 = +y/2;
      z1 = -z/2;
      z2 = +z/2;
    } else {
      x1 = y1 = z1 = 0;
      x2 = x;
      y2 = y;
      z2 = z;
    }

    p->append_poly(); // top
    p->append_vertex(x1, y1, z2);
    p->append_vertex(x2, y1, z2);
    p->append_vertex(x2, y2, z2);
    p->append_vertex(x1, y2, z2);

    p->append_poly(); // bottom
    p->append_vertex(x1, y2, z1);
    p->append_vertex(x2, y2, z1);
    p->append_vertex(x2, y1, z1);
    p->append_vertex(x1, y1, z1);

    p->append_poly(); // side1
    p->append_vertex(x1, y1, z1);
    p->append_vertex(x2, y1, z1);
    p->append_vertex(x2, y1, z2);
    p->append_vertex(x1, y1, z2);

    p->append_poly(); // side2
    p->append_vertex(x2, y1, z1);
    p->append_vertex(x2, y2, z1);
    p->append_vertex(x2, y2, z2);
    p->append_vertex(x2, y1, z2);

    p->append_poly(); // side3
    p->append_vertex(x2, y2, z1);
    p->append_vertex(x1, y2, z1);
    p->append_vertex(x1, y2, z2);
    p->append_vertex(x2, y2, z2);

    p->append_poly(); // side4
    p->append_vertex(x1, y2, z1);
    p->append_vertex(x1, y1, z1);
    p->append_vertex(x1, y1, z2);
    p->append_vertex(x1, y2, z2);
  }
  return p;
}

PolySet *SphereNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  if (r > 0) {
    int fragments = get_fragments_from_r(r, *this);
    int rings = fragments/2;
// Uncomment the following three lines to enable experimental sphere tesselation
//		if (rings % 2 == 0) rings++; // To ensure that the middle ring is at phi == 0 degrees
    typedef std::vector< ring_s > RingVector;
    RingVector ring(rings);

//		double offset = 0.5 * ((fragments / 2) % 2);
    for (int i = 0; i < rings; i++) {
//			double phi = (M_PI * (i + offset)) / (fragments/2);
      double phi = (M_PI * (i + 0.5)) / rings;
      double rr = r * sin(phi);
      ring[i].z = r * cos(phi);
      ring[i].points.resize(fragments);
      generate_circle(ring[i].points, rr);
    }

    p->append_poly();
    for (int i = 0; i < fragments; i++)
      p->append_vertex(ring[0].points[i][0], ring[0].points[i][1], ring[0].z);

    for (int i = 0; i < rings-1; i++) {
      ring_s &r1(ring[i]);
      ring_s &r2(ring[i+1]);
      int r1i = 0, r2i = 0;
      while (r1i < fragments || r2i < fragments)
      {
	if (r1i >= fragments)
		goto sphere_next_r2;
	if (r2i >= fragments)
		goto sphere_next_r1;
	if ((double)r1i / fragments <
			(double)r2i / fragments)
	{
sphere_next_r1:
	  p->append_poly();
	  int r1j = (r1i+1) % fragments;
	  p->insert_vertex(r1.points[r1i][0], r1.points[r1i][1], r1.z);
	  p->insert_vertex(r1.points[r1j][0], r1.points[r1j][1], r1.z);
	  p->insert_vertex(r2.points[r2i % fragments][0], r2.points[r2i % fragments][1], r2.z);
	  r1i++;
	} else {
sphere_next_r2:
	  p->append_poly();
	  int r2j = (r2i+1) % fragments;
	  p->append_vertex(r2.points[r2i][0], r2.points[r2i][1], r2.z);
	  p->append_vertex(r2.points[r2j][0], r2.points[r2j][1], r2.z);
	  p->append_vertex(r1.points[r1i % fragments][0], r1.points[r1i % fragments][1], r1.z);
	  r2i++;
	}
      }
    }
    p->append_poly();
    for (int i = 0; i < fragments; i++)
      p->insert_vertex(ring[rings-1].points[i][0], ring[rings-1].points[i][1], ring[rings-1].z);
  }

  return p;
}

PolySet *CylinderNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  if (h > 0 && r1 >=0 && r2 >= 0 && (r1 > 0 || r2 > 0)) {
    int fragments = get_fragments_from_r(std::max(r1, r2), *this);

    double z1, z2;
    if (center) {
      z1 = -h/2;
      z2 = +h/2;
    } else {
      z1 = 0;
      z2 = h;
    }

    Vec2D circle1(fragments);
    Vec2D circle2(fragments);

    generate_circle(circle1, r1);
    generate_circle(circle2, r2);
    
    for (int i=0; i<fragments; i++) {
      int j = (i+1) % fragments;
      if (r1 == r2) {
	p->append_poly();
	p->insert_vertex(circle1[i][0], circle1[i][1], z1);
	p->insert_vertex(circle2[i][0], circle2[i][1], z2);
	p->insert_vertex(circle2[j][0], circle2[j][1], z2);
	p->insert_vertex(circle1[j][0], circle1[j][1], z1);
      } else {
	if (r1 > 0) {
	  p->append_poly();
	  p->insert_vertex(circle1[i][0], circle1[i][1], z1);
	  p->insert_vertex(circle2[i][0], circle2[i][1], z2);
	  p->insert_vertex(circle1[j][0], circle1[j][1], z1);
	}
	if (r2 > 0) {
	  p->append_poly();
	  p->insert_vertex(circle2[i][0], circle2[i][1], z2);
	  p->insert_vertex(circle2[j][0], circle2[j][1], z2);
	  p->insert_vertex(circle1[j][0], circle1[j][1], z1);
	}
      }
    }

    if (r1 > 0) {
      p->append_poly();
      for (int i=0; i<fragments; i++)
	p->insert_vertex(circle1[i][0], circle1[i][1], z1);
    }

    if (r2 > 0) {
      p->append_poly();
      for (int i=0; i<fragments; i++)
	p->append_vertex(circle2[i][0], circle2[i][1], z2);
    }
  }
  return p;
}

PolySet *PolyhedronNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  p->convexity = convexity;
  BOOST_FOREACH(const VecPoints &t, triangles) {
    p->append_poly();
    BOOST_FOREACH(unsigned int pt, t) {
      if (pt < points.size()) {
	const Float3 &point(points[pt]);
	p->insert_vertex(point[0], point[1], point[2]);
      }
    }
  }
  return p;
}

PolySet *SquareNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  const double &x=dim[0],&y=dim[1];
  double x1, x2, y1, y2;
  if (center) {
	  x1 = -x/2;
	  x2 = +x/2;
	  y1 = -y/2;
	  y2 = +y/2;
  } else {
	  x1 = y1 = 0;
	  x2 = x;
	  y2 = y;
  }

  p->is2d = true;
  p->append_poly();
  p->append_vertex(x1, y1);
  p->append_vertex(x2, y1);
  p->append_vertex(x2, y2);
  p->append_vertex(x1, y2);
  return p;
}

PolySet *CircleNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  int fragments = get_fragments_from_r(r, *this);

  p->is2d = true;
  p->append_poly();

  for (int i=0; i < fragments; i++) {
	  double phi = (M_PI*2*i) / fragments;
	  p->append_vertex(r*cos(phi), r*sin(phi));
  }
  return p;
}

PolySet *PolygonNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  DxfData dd;
  BOOST_FOREACH(const Float2 &p, points) {
	  dd.points.append(DxfData::Point(p[0], p[1]));
  }

  if (paths.size() == 0) {
    dd.paths.append(DxfData::Path());
    for (unsigned int i=0; i<points.size(); i++) {
      DxfData::Point *p = &dd.points[i];
      dd.paths.last().points.append(p);
    }
    if (dd.paths.last().points.size() > 0) {
	dd.paths.last().points.append(dd.paths.last().points.first());
	dd.paths.last().is_closed = true;
    }
  }
  else {
    BOOST_FOREACH(const VecPoints &p, paths) {
      dd.paths.append(DxfData::Path());
      BOOST_FOREACH(unsigned int idx, p) {
	if (idx < (unsigned int)dd.points.size()) {
	  DxfData::Point *p = &dd.points[idx];
	  dd.paths.last().points.append(p);
	}
      }
      if (dd.paths.last().points.isEmpty()) {
	dd.paths.removeLast();
      } else {
	dd.paths.last().points.append(dd.paths.last().points.first());
	dd.paths.last().is_closed = true;
      }
    }
  }
  p->is2d = true;
  p->convexity = convexity;
  dxf_tesselate(p, &dd, 0, true, false, 0);
  dxf_border_to_ps(p, &dd);
  return p;
}

QString PrimitiveNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("PrimitiveNode");
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}


QString CubeNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("cube(size = [%g, %g, %g], center = %s);\n", dim[0], dim[1], dim[2], center ? "true" : "false");
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString SphereNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("sphere($fn = %g, $fa = %g, $fs = %g, r = %g);\n", fn, fa, fs, r);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}


QString CylinderNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("cylinder($fn = %g, $fa = %g, $fs = %g, h = %g, r1 = %g, r2 = %g, center = %s);\n", fn, fa, fs, h, r1, r2, center ? "true" : "false");
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString PolyhedronNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("polyhedron(points = %s, triangles = %s, convexity = %d);\n", toString(points).c_str(), toString(triangles).c_str(), convexity);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}


QString SquareNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("square(size = [%g, %g], center = %s);\n", dim[0], dim[1], center ? "true" : "false");
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}


QString CircleNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("circle($fn = %g, $fa = %g, $fs = %g, r = %g);\n", fn, fa, fs, r);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}


QString PolygonNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
    QString text;
    text.sprintf("polygon(points = %s, paths = %s, convexity = %d);\n", toString(points).c_str(), toString(paths).c_str(), convexity);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}