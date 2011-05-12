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

#include "transform.h"
#include "dxfdata.h"
#include "csgterm.h"
#include "polyset.h"
#include "dxftess.h"
#include "printutils.h"
#include <boost/make_shared.hpp>
using boost::make_shared;


TransformNode::TransformNode(const NodeList &children, const Props p) 
  :AbstractNode(p,children) {
  for (int i = 0; i < 16; i++)
    m[i] = i % 5 == 0 ? 1.0 : 0.0;
  for (int i = 16; i < 20; i++)
    m[i] = -1;
}

TransformScaleNode::TransformScaleNode(const Float3 &scale, const NodeList &children, const Props p) 
  :TransformNode(children,p) {
    m[0] = scale[0];
    m[5] = scale[1];
    m[10] = scale[2];
    if (m[10] <= 0) m[10] = 1;
}

TransformRotateNode::TransformRotateNode(const Float3 &rotation, const NodeList &children, const Props p) 
  :TransformNode(children,p) {
  for (int i = 0; i < rotation.static_size; i++) {
	  double c = cos(rotation[i]*M_PI/180.0);
	  double s = sin(rotation[i]*M_PI/180.0);
	  double x = i == 0, y = i == 1, z = i == 2;
	  double mr[16] = {
		  x*x*(1-c)+c,
		  y*x*(1-c)+z*s,
		  z*x*(1-c)-y*s,
		  0,
		  x*y*(1-c)-z*s,
		  y*y*(1-c)+c,
		  z*y*(1-c)+x*s,
		  0,
		  x*z*(1-c)+y*s,
		  y*z*(1-c)-x*s,
		  z*z*(1-c)+c,
		  0,
		  0, 0, 0, 1
	  };
	  double mt[16];
	  for (int x = 0; x < 4; x++)
	  for (int y = 0; y < 4; y++)
	  {
		  mt[x+y*4] = 0;
		  for (int i = 0; i < 4; i++)
			  mt[x+y*4] += m[i+y*4] * mr[x+i*4];
	  }
	  for (int i = 0; i < 16; i++)
		  m[i] = mt[i];
  }
}

TransformRotateAxisNode::TransformRotateAxisNode(const Float3 &axis, FloatType angle, const NodeList &children, const Props p) 
  :TransformNode(children,p) {
  FloatType x = axis[0], y = axis[1], z =axis[2];
  if (x != 0.0 || y != 0.0 || z != 0.0) {
      double sn = 1.0 / sqrt(x*x + y*y + z*z);
      x *= sn, y *= sn, z *= sn;
      double c = cos(angle*M_PI/180.0);
      double s = sin(angle*M_PI/180.0);

      m[ 0] = x*x*(1-c)+c;
      m[ 1] = y*x*(1-c)+z*s;
      m[ 2] = z*x*(1-c)-y*s;

      m[ 4] = x*y*(1-c)-z*s;
      m[ 5] = y*y*(1-c)+c;
      m[ 6] = z*y*(1-c)+x*s;

      m[ 8] = x*z*(1-c)+y*s;
      m[ 9] = y*z*(1-c)-x*s;
      m[10] = z*z*(1-c)+c;
  }
}

TransformMirrorNode::TransformMirrorNode(const Float3 &axis, const NodeList &children, const Props p)
  :TransformNode(children,p) {
  FloatType x = axis[0], y = axis[1], z =axis[2];
  if (x != 0.0 || y != 0.0 || z != 0.0) {
    double sn = 1.0 / sqrt(x*x + y*y + z*z);
    x *= sn, y *= sn, z *= sn;
    m[ 0] = 1-2*x*x;
    m[ 1] = -2*y*x;
    m[ 2] = -2*z*x;

    m[ 4] = -2*x*y;
    m[ 5] = 1-2*y*y;
    m[ 6] = -2*z*y;

    m[ 8] = -2*x*z;
    m[ 9] = -2*y*z;
    m[10] = 1-2*z*z;
  }
}

TransformTranslateNode::TransformTranslateNode(const Float3 &v, const NodeList &children, const Props p)
  :TransformNode(children,p) {
  m[12] = v[0]; m[13] = v[1]; m[14] = v[2];
}

TransformMatrixNode::TransformMatrixNode(const Float16 &mat, const NodeList &children, const Props p)
  :TransformNode(children,p) {
  for (int i = 0; i < mat.static_size; i++)
    m[i] = mat[i];
}

TransformColorNode::TransformColorNode(const Float4 &color, const NodeList &children, const Props p)
  :TransformNode(children,p) {
  for (int i = 0; i < color.static_size; i++)
    m[16+i] = color[i];
}

#ifdef ENABLE_CGAL

CGAL_Nef_polyhedron TransformNode::render_cgal_nef_polyhedron() const
{
	QString cache_id = mk_cache_id();
	if (cgal_nef_cache.contains(cache_id)) {
		progress_report();
		PRINT(cgal_nef_cache[cache_id]->msg);
		return cgal_nef_cache[cache_id]->N;
	}

	print_messages_push();

	bool first = true;
	CGAL_Nef_polyhedron N;

	foreach (AbstractNode::Pointer v, children) {
		if (v->props.background)
			continue;
		if (first) {
			N = v->render_cgal_nef_polyhedron();
			if (N.dim != 0)
				first = false;
		} else if (N.dim == 2) {
			N.p2 += v->render_cgal_nef_polyhedron().p2;
		} else if (N.dim == 3) {
			N.p3 += v->render_cgal_nef_polyhedron().p3;
		}
		v->progress_report();
	}

	if (N.dim == 2)
	{
		// Unfortunately CGAL provides no transform method for CGAL_Nef_polyhedron2
		// objects. So we convert in to our internal 2d data format, transform it,
		// tesselate it and create a new CGAL_Nef_polyhedron2 from it.. What a hack!
		
		CGAL_Aff_transformation2 t(
				m[0], m[4], m[12],
				m[1], m[5], m[13], m[15]);

		DxfData dd(N);
		for (int i=0; i < dd.points.size(); i++) {
			CGAL_Kernel2::Point_2 p = CGAL_Kernel2::Point_2(dd.points[i].x, dd.points[i].y);
			p = t.transform(p);
			dd.points[i].x = to_double(p.x());
			dd.points[i].y = to_double(p.y());
		}

		PolySet ps;
		ps.is2d = true;
		dxf_tesselate(&ps, &dd, 0, true, false, 0);

		N = ps.render_cgal_nef_polyhedron();
		ps.refcount = 0;
	}
	if (N.dim == 3) {
		CGAL_Aff_transformation t(
				m[0], m[4], m[ 8], m[12],
				m[1], m[5], m[ 9], m[13],
				m[2], m[6], m[10], m[14], m[15]);
		N.p3.transform(t);
	}

	cgal_nef_cache.insert(cache_id, new cgal_nef_cache_entry(N), N.weight());
	print_messages_pop();
	progress_report();

	return N;
}

#endif /* ENABLE_CGAL */

CSGTerm *TransformNode::render_csg_term(const Float20 &c, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	Float20 x;

	for (int i = 0; i < 16; i++)
	{
		int c_row = i%4;
		int m_col = i/4;
		x[i] = 0;
		for (int j = 0; j < 4; j++)
			x[i] += c[c_row + j*4] * m[m_col*4 + j];
	}

	for (int i = 16; i < 20; i++)
		x[i] = m[i] < 0 ? c[i] : m[i];

	CSGTerm *t1 = NULL;
	foreach(AbstractNode::Pointer v, children)
	{
		CSGTerm *t2 = v->render_csg_term(x, highlights, background);
		if (t2 && !t1) {
			t1 = t2;
		} else if (t2 && t1) {
			t1 = new CSGTerm(CSGTerm::TYPE_UNION, t1, t2);
		}
	}
	if (t1 && props.highlight && highlights)
		highlights->append(t1->link());
	if (t1 && props.background && background) {
		background->append(t1);
		return NULL;
	}
	return t1;
}

QString TransformNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text;
		if (m[16] >= 0 || m[17] >= 0 || m[18] >= 0 || m[19] >= 0)
			text.sprintf("n%d: color([%g, %g, %g, %g])", idx,
					m[16], m[17], m[18], m[19]);
		else
			text.sprintf("n%d: multmatrix([[%g, %g, %g, %g], [%g, %g, %g, %g], "
					"[%g, %g, %g, %g], [%g, %g, %g, %g]])", idx,
					m[0], m[4], m[ 8], m[12],
					m[1], m[5], m[ 9], m[13],
					m[2], m[6], m[10], m[14],
					m[3], m[7], m[11], m[15]);
		text = indent + text + " {\n";
		foreach (AbstractNode::Pointer v, children)
			text += v->dump(indent + QString("\t"));
		((AbstractNode*)this)->dump_cache = text + indent + "}\n";
	}
	return dump_cache;
}
