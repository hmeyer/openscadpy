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

#include "render.h"
#include "polyset.h"
#include "dxfdata.h"
#include "dxftess.h"
#include "csgterm.h"
#include "printutils.h"
#include "progress.h"


#include <QProgressDialog>
#include <QApplication>
#include <QTime>
#include <boost/make_shared.hpp>


#ifdef ENABLE_CGAL

CGAL_Nef_polyhedron RenderNode::render_cgal_nef_polyhedron() const
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
	foreach(AbstractNode::Pointer v, children)
	{
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

	cgal_nef_cache.insert(cache_id, new cgal_nef_cache_entry(N), N.weight());
	print_messages_pop();
	progress_report();

	return N;
}

CSGTerm *AbstractNode::render_csg_term_from_nef(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, const char *statement, int convexity) const
{
	QString key = mk_cache_id();
	if (PolySet::ps_cache.contains(key)) {
		PRINT(PolySet::ps_cache[key]->msg);
		return AbstractPolyNode::render_csg_term_from_ps(m, highlights, background,
				PolySet::ps_cache[key]->ps->link(), props, idx);
	}

	print_messages_push();
	CGAL_Nef_polyhedron N;

	QString cache_id = mk_cache_id();
	if (cgal_nef_cache.contains(cache_id))
	{
		PRINT(cgal_nef_cache[cache_id]->msg);
		N = cgal_nef_cache[cache_id]->N;
	}
	else
	{
		PRINTF_NOCACHE("Processing uncached %s statement...", statement);
		// PRINTA("Cache ID: %1", cache_id);
		QApplication::processEvents();

		QTime t;
		t.start();

		N = this->render_cgal_nef_polyhedron();

		int s = t.elapsed() / 1000;
		PRINTF_NOCACHE("..rendering time: %d hours, %d minutes, %d seconds", s / (60*60), (s / 60) % 60, s % 60);
	}

	PolySet *ps = NULL;

	if (N.dim == 2)
	{
		DxfData dd(N);
		ps = new PolySet();
		ps->is2d = true;
		dxf_tesselate(ps, &dd, 0, true, false, 0);
		dxf_border_to_ps(ps, &dd);
	}

	if (N.dim == 3)
	{
		if (!N.p3.is_simple()) {
			PRINTF("WARNING: Result of %s() isn't valid 2-manifold! Modify your design..", statement);
			return NULL;
		}

		ps = new PolySet();

		CGAL_Polyhedron P;
		N.p3.convert_to_Polyhedron(P);

		typedef CGAL_Polyhedron::Vertex Vertex;
		typedef CGAL_Polyhedron::Vertex_const_iterator VCI;
		typedef CGAL_Polyhedron::Facet_const_iterator FCI;
		typedef CGAL_Polyhedron::Halfedge_around_facet_const_circulator HFCC;

		for (FCI fi = P.facets_begin(); fi != P.facets_end(); ++fi) {
			HFCC hc = fi->facet_begin();
			HFCC hc_end = hc;
			ps->append_poly();
			do {
				Vertex v = *VCI((hc++)->vertex());
				double x = CGAL::to_double(v.point().x());
				double y = CGAL::to_double(v.point().y());
				double z = CGAL::to_double(v.point().z());
				ps->append_vertex(x, y, z);
			} while (hc != hc_end);
		}
	}

	if (ps)
	{
		ps->convexity = convexity;
		PolySet::ps_cache.insert(key, new PolySet::ps_cache_entry(ps->link()));

		CSGTerm *term = new CSGTerm(ps, m, QString("n%1").arg(idx));
		if (props.highlight && highlights)
			highlights->append(term->link());
		if (props.background && background) {
			background->append(term);
			return NULL;
		}
		return term;
	}
	print_messages_pop();

	return NULL;
}

CSGTerm *RenderNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	return render_csg_term_from_nef(m, highlights, background, "render", this->convexity);
}

#else

CSGTerm *RenderNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	CSGTerm *t1 = NULL;
	PRINT("WARNING: Found render() statement but compiled without CGAL support!");
	foreach(AbstractNode * v, children) {
		CSGTerm *t2 = v->render_csg_term(m, highlights, background);
		if (t2 && !t1) {
			t1 = t2;
		} else if (t2 && t1) {
			t1 = new CSGTerm(CSGTerm::TYPE_UNION, t1, t2);
		}
	}
	if (modinst->tag_highlight && highlights)
		highlights->append(t1->link());
	if (t1 && modinst->tag_background && background) {
		background->append(t1);
		return NULL;
	}
	return t1;
}

#endif

QString RenderNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text = indent + QString("n%1: ").arg(idx) + QString("render(convexity = %1) {\n").arg(QString::number(convexity));
		foreach (AbstractNode::Pointer v, children)
			text += v->dump(indent + QString("\t"));
		((AbstractNode*)this)->dump_cache = text + indent + "}\n";
	}
	return dump_cache;
}

