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

#include "cgaladv.h"
#include "module.h"
#include "context.h"
#include "builtin.h"
#include "printutils.h"
#include "cgal.h"
#include <boost/make_shared.hpp>

using boost::make_shared;

#ifdef ENABLE_CGAL
extern CGAL_Nef_polyhedron3 minkowski3(CGAL_Nef_polyhedron3 a, CGAL_Nef_polyhedron3 b);
extern CGAL_Nef_polyhedron2 minkowski2(CGAL_Nef_polyhedron2 a, CGAL_Nef_polyhedron2 b);
extern CGAL_Nef_polyhedron2 convexhull2(std::list<CGAL_Nef_polyhedron2> a);
#endif

enum cgaladv_type_e {
	MINKOWSKI,
	GLIDE,
	SUBDIV,
	HULL
};

class CgaladvModule : public AbstractModule
{
public:
	cgaladv_type_e type;
	CgaladvModule(cgaladv_type_e type) : type(type) { }
	virtual AbstractNode::Pointer evaluate(const Context *ctx, const ModuleInstantiation *inst) const;
};

AbstractNode::Pointer CgaladvModule::evaluate(const Context *ctx, const ModuleInstantiation *inst) const
{
  AbstractNode::NodeList children;
  foreach (ModuleInstantiation *v, inst->children) {
	  AbstractNode::Pointer n(v->evaluate(inst->ctx));
	  if (n) children.append(n);
  }
  QVector<QString> argnames;
  QVector<Expression*> argexpr;

  if (type == MINKOWSKI)
	  argnames = QVector<QString>() << "convexity";

  if (type == GLIDE)
	  argnames = QVector<QString>() << "path" << "convexity";

  if (type == SUBDIV)
	  argnames = QVector<QString>() << "type" << "level" << "convexity";

  Context c(ctx);
  c.args(argnames, argexpr, inst->argnames, inst->argvalues);

  Value vconvexity, path, vsubdiv_type, vlevel;

  if (type == MINKOWSKI) {
	  vconvexity = c.lookup_variable("convexity", true);
    return make_shared<CgaladvMinkowskiNode>(children, (int)vconvexity.num, inst);
  }

  if (type == GLIDE) {
	  vconvexity = c.lookup_variable("convexity", true);
	  path = c.lookup_variable("path", false);
    return make_shared<CgaladvGlideNode>(children, path, (int)vconvexity.num, inst);
  }

  if (type == SUBDIV) {
    vconvexity = c.lookup_variable("convexity", true);
    vsubdiv_type = c.lookup_variable("type", false);
    vlevel = c.lookup_variable("level", true);
    return make_shared<CgaladvSubdivNode>(children, vsubdiv_type.text, std::min(1,(int)vlevel.num), (int)vconvexity.num, inst);
  }
  return make_shared<CgaladvHullNode>(children, (int)vconvexity.num, inst);
}

void register_builtin_cgaladv()
{
	builtin_modules["minkowski"] = new CgaladvModule(MINKOWSKI);
	builtin_modules["glide"] = new CgaladvModule(GLIDE);
	builtin_modules["subdiv"] = new CgaladvModule(SUBDIV);
	builtin_modules["hull"] = new CgaladvModule(HULL);
}

#ifdef ENABLE_CGAL

CGAL_Nef_polyhedron CgaladvMinkowskiNode::render_cgal_nef_polyhedron() const {
  QString cache_id = mk_cache_id();
  if (cgal_nef_cache.contains(cache_id)) {
	  progress_report();
	  PRINT(cgal_nef_cache[cache_id]->msg);
	  return cgal_nef_cache[cache_id]->N;
  }

  print_messages_push();
  CGAL_Nef_polyhedron N;

  bool first = true;
  foreach(AbstractNode::Pointer v, children) {
	  if (v->props.background)
		  continue;
	  if (first) {
		  N = v->render_cgal_nef_polyhedron();
		  if (N.dim != 0)
			  first = false;
	  } else {
		  CGAL_Nef_polyhedron tmp = v->render_cgal_nef_polyhedron();
		  if (N.dim == 3 && tmp.dim == 3) {
			  N.p3 = minkowski3(N.p3, tmp.p3);
		  }
		  if (N.dim == 2 && tmp.dim == 2) {
			  N.p2 = minkowski2(N.p2, tmp.p2);
		  }
	  }
	  v->progress_report();
  }
  cgal_nef_cache.insert(cache_id, new cgal_nef_cache_entry(N), N.weight());
  print_messages_pop();
  progress_report();

  return N;
}

CGAL_Nef_polyhedron CgaladvGlideNode::render_cgal_nef_polyhedron() const {
  PRINT("WARNING: glide() is not implemented yet!");
  return CGAL_Nef_polyhedron();
}

CGAL_Nef_polyhedron CgaladvSubdivNode::render_cgal_nef_polyhedron() const {
  PRINT("WARNING: subdiv() is not implemented yet!");
  return CGAL_Nef_polyhedron();
}

CGAL_Nef_polyhedron CgaladvHullNode::render_cgal_nef_polyhedron() const {
  QString cache_id = mk_cache_id();
  if (cgal_nef_cache.contains(cache_id)) {
	  progress_report();
	  PRINT(cgal_nef_cache[cache_id]->msg);
	  return cgal_nef_cache[cache_id]->N;
  }

  print_messages_push();
  CGAL_Nef_polyhedron N;

  std::list<CGAL_Nef_polyhedron2> polys;
  bool all2d = true;
  foreach(AbstractNode::Pointer v, children) {
	  if (v->props.background)
      continue;
	  N = v->render_cgal_nef_polyhedron();
	  if (N.dim == 3) {
      //polys.push_back(tmp.p3);
		  PRINT("WARNING: hull() is not implemented yet for 3D objects!");
      all2d=false;
	  }
	  if (N.dim == 2) {
      polys.push_back(N.p2);
	  }
	  v->progress_report();
  }

  if (all2d)
	  N.p2 = convexhull2(polys);

  cgal_nef_cache.insert(cache_id, new cgal_nef_cache_entry(N), N.weight());
  print_messages_pop();
  progress_report();

  return N;
}

CSGTerm *CgaladvMinkowskiNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const {
  return render_csg_term_from_nef(m, highlights, background, "minkowski", this->convexity);
}

CSGTerm *CgaladvGlideNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const {
  return render_csg_term_from_nef(m, highlights, background, "glide", this->convexity);
}

CSGTerm *CgaladvSubdivNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const {
  return render_csg_term_from_nef(m, highlights, background, "subdiv", this->convexity);
}

CSGTerm *CgaladvHullNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const {
  return render_csg_term_from_nef(m, highlights, background, "hull", this->convexity);
}

#else // ENABLE_CGAL

CSGTerm *CgaladvNode::render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const
{
	PRINT("WARNING: Found minkowski(), glide(), subdiv() or hull() statement but compiled without CGAL support!");
	return NULL;
}

#endif // ENABLE_CGAL

QString CgaladvNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
      QString text;
      text.sprintf("CgaladvNode() {\n");
      foreach (AbstractNode::Pointer v, this->children)
	      text += v->dump(indent + QString("\t"));
      text += indent + "}\n";
      ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}



QString CgaladvMinkowskiNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
      QString text;
      text.sprintf("minkowski(convexity = %d) {\n", this->convexity);
      text += static_cast<const CgaladvNode*>(this)->dump(indent + QString("\t"));
      text += indent + "}\n";
      ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString CgaladvGlideNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
      QString text;
      text.sprintf(", convexity = %d) {\n", this->convexity);
      text = QString("glide(path = ") + this->path.dump() + text;
      text += static_cast<const CgaladvNode*>(this)->dump(indent + QString("\t"));
      text += indent + "}\n";
      ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString CgaladvSubdivNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
      QString text;
      text.sprintf("subdiv(level = %d, convexity = %d) {\n", this->level, this->convexity);
      text += static_cast<const CgaladvNode*>(this)->dump(indent + QString("\t"));
      text += indent + "}\n";
      ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString CgaladvHullNode::dump(QString indent) const
{
  if (dump_cache.isEmpty()) {
      QString text;
      text.sprintf("hull() {\n");
      text += static_cast<const CgaladvNode*>(this)->dump(indent + QString("\t"));
      text += indent + "}\n";
      ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

