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

#include "printutils.h"
#include "node.h"
#include "csgterm.h"
#include "progress.h"
#include "polyset.h"
#include <QRegExp>

int AbstractNode::idx_counter;

AbstractNode::AbstractNode(const Props &p):props(p)
{
	idx = idx_counter++;
}

AbstractNode::AbstractNode(const Props &p, const NodeList &children):children(children),props(p)
{
	idx = idx_counter++;
}


AbstractNode::~AbstractNode() {}

QString AbstractNode::mk_cache_id() const
{
	QString cache_id = dump("");
	cache_id.remove(QRegExp("[a-zA-Z_][a-zA-Z_0-9]*:"));
	cache_id.remove(' ');
	cache_id.remove('\t');
	cache_id.remove('\n');
	return cache_id;
}

#ifdef ENABLE_CGAL

AbstractNode::cgal_nef_cache_entry::cgal_nef_cache_entry(const CGAL_Nef_polyhedron &N) :
		N(N), msg(print_messages_stack.last()) { };

QCache<QString, AbstractNode::cgal_nef_cache_entry> AbstractNode::cgal_nef_cache(100000);

static CGAL_Nef_polyhedron render_cgal_nef_polyhedron_backend(const AbstractNode *that, bool intersect)
{
	QString cache_id = that->mk_cache_id();
	if (that->cgal_nef_cache.contains(cache_id)) {
		that->progress_report();
		PRINT(that->cgal_nef_cache[cache_id]->msg);
		return that->cgal_nef_cache[cache_id]->N;
	}

	print_messages_push();

	bool first = true;
	CGAL_Nef_polyhedron N;
	foreach (AbstractNode::Pointer v, that->children) {
		if (v->props.background)
			continue;
		if (first) {
			N = v->render_cgal_nef_polyhedron();
			if (N.dim != 0)
				first = false;
		} else if (N.dim == 2) {
			if (intersect)
				N.p2 *= v->render_cgal_nef_polyhedron().p2;
			else
				N.p2 += v->render_cgal_nef_polyhedron().p2;
		} else {
			if (intersect)
				N.p3 *= v->render_cgal_nef_polyhedron().p3;
			else
				N.p3 += v->render_cgal_nef_polyhedron().p3;
		}
		v->progress_report();
	}

	that->cgal_nef_cache.insert(cache_id, new AbstractNode::cgal_nef_cache_entry(N), N.weight());
	that->progress_report();
	print_messages_pop();

	return N;
}

CGAL_Nef_polyhedron AbstractNode::render_cgal_nef_polyhedron() const
{
	return render_cgal_nef_polyhedron_backend(this, false);
}

CGAL_Nef_polyhedron AbstractIntersectionNode::render_cgal_nef_polyhedron() const
{
	return render_cgal_nef_polyhedron_backend(this, true);
}

#endif /* ENABLE_CGAL */

static CSGTerm *render_csg_term_backend(const AbstractNode *that, bool intersect, const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background)
{
	CSGTerm *t1 = NULL;
	foreach(AbstractNode::Pointer v, that->children) {
		CSGTerm *t2 = v->render_csg_term(m, highlights, background);
		if (t2 && !t1) {
			t1 = t2;
		} else if (t2 && t1) {
			if (intersect)
				t1 = new CSGTerm(CSGTerm::TYPE_INTERSECTION, t1, t2);
			else
				t1 = new CSGTerm(CSGTerm::TYPE_UNION, t1, t2);
		}
	}
	if (t1 && that->props.highlight && highlights)
		highlights->append(t1->link());
	if (t1 && that->props.background && background) {
		background->append(t1);
		return NULL;
	}
	return t1;
}

CSGTerm *AbstractNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	return render_csg_term_backend(this, false, m, highlights, background);
}

CSGTerm *AbstractIntersectionNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	return render_csg_term_backend(this, true, m, highlights, background);
}

QString AbstractNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text = indent + QString("n%1: group() {\n").arg(idx);
		foreach (AbstractNode::Pointer v, children)
			text += v->dump(indent + QString("\t"));
		((AbstractNode*)this)->dump_cache = text + indent + "}\n";
	}
	return dump_cache;
}

QString AbstractIntersectionNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text = indent + QString("n%1: intersection() {\n").arg(idx);
		foreach (AbstractNode::Pointer v, children)
			text += v->dump(indent + QString("\t"));
		((AbstractNode*)this)->dump_cache = text + indent + "}\n";
	}
	return dump_cache;
}

void AbstractNode::progress_prepare()
{
	foreach (AbstractNode::Pointer v, children)
		v->progress_prepare();
	this->progress_mark = ++progress_report_count;
}

void AbstractNode::progress_report() const
{
	progress_update(*this, this->progress_mark);
}

#ifdef ENABLE_CGAL

CGAL_Nef_polyhedron AbstractPolyNode::render_cgal_nef_polyhedron() const
{
	QString cache_id = mk_cache_id();
	if (cgal_nef_cache.contains(cache_id)) {
		progress_report();
		PRINT(cgal_nef_cache[cache_id]->msg);
		return cgal_nef_cache[cache_id]->N;
	}

	print_messages_push();

	PolySet *ps = render_polyset(RENDER_CGAL);
	try {
		CGAL_Nef_polyhedron N = ps->render_cgal_nef_polyhedron();
		cgal_nef_cache.insert(cache_id, new cgal_nef_cache_entry(N), N.weight());
		print_messages_pop();
		progress_report();
		
		ps->unlink();
		return N;
	}
	catch (...) { // Don't leak the PolySet on ProgressCancelException
		ps->unlink();
		throw;
	}
}

#endif /* ENABLE_CGAL */

CSGTerm *AbstractPolyNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	PolySet *ps = render_polyset(RENDER_OPENCSG);
	return render_csg_term_from_ps(m, highlights, background, ps, props, idx);
}

CSGTerm *AbstractPolyNode::render_csg_term_from_ps(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, PolySet *ps, const AbstractNode::Props &p, int idx)
{
	CSGTerm *t = new CSGTerm(ps, m, QString("n%1").arg(idx));
	if (p.highlight && highlights)
		highlights->append(t->link());
	if (p.background && background) {
		background->append(t);
		return NULL;
	}
	return t;
}

