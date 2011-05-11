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


#include "dxfrotextrude.h"
#include "module.h"
#include "context.h"
#include "printutils.h"
#include "builtin.h"
#include "polyset.h"
#include "dxfdata.h"
#include "progress.h"
#include "openscad.h" // get_fragments_from_r()

#include <QTime>
#include <QApplication>
#include <QProgressDialog>
#include <QDateTime>
#include <QFileInfo>
#include <boost/make_shared.hpp>

class DxfRotateExtrudeModule : public AbstractModule
{
public:
	DxfRotateExtrudeModule() { }
	virtual AbstractNode::Pointer evaluate(const Context *ctx, const ModuleInstantiation *inst) const;
};

AbstractNode::Pointer DxfRotateExtrudeModule::evaluate(const Context *ctx, const ModuleInstantiation *inst) const
{
  AbstractNode::NodeList children;
  foreach (ModuleInstantiation *v, inst->children) {
	  AbstractNode::Pointer n(v->evaluate(inst->ctx));
	  if (n) children.append(n);
  }
  QVector<QString> argnames = QVector<QString>() << "file" << "layer" << "origin" << "scale";
  QVector<Expression*> argexpr;

  Context c(ctx);
  c.args(argnames, argexpr, inst->argnames, inst->argvalues);

  Accuracy acc;
  acc.fn = c.lookup_variable("$fn").num;
  acc.fs = c.lookup_variable("$fs").num;
  acc.fa = c.lookup_variable("$fa").num;

  Value vfile = c.lookup_variable("file");
  Value vlayer = c.lookup_variable("layer", true);
  Value vconvexity = c.lookup_variable("convexity", true);
  Value vorigin = c.lookup_variable("origin", true);
  Value vscale = c.lookup_variable("scale", true);  

  int convexity = (int)vconvexity.num;

  if (convexity <= 0)
	  convexity = 1;
  
  QString file;
  if(!vfile.text.isNull())
    file = c.get_absolute_path(vfile.text);
  else
    file = vfile.text;
  double ox=0,oy=0;
  vorigin.getv2(ox, oy);
  
  double scale = vscale.num;
  if (scale <= 0.0)
    scale = 1.0;

  return boost::make_shared<DxfRotateExtrudeNode>(children, file, vlayer.text, ox, oy, scale, convexity, acc, inst);	
}

void register_builtin_dxf_rotate_extrude()
{
	builtin_modules["dxf_rotate_extrude"] = new DxfRotateExtrudeModule();
	builtin_modules["rotate_extrude"] = new DxfRotateExtrudeModule();
}

PolySet *DxfRotateExtrudeNode::render_polyset(render_mode_e) const
{
	QString key = mk_cache_id();
	if (PolySet::ps_cache.contains(key)) {
		PRINT(PolySet::ps_cache[key]->msg);
		return PolySet::ps_cache[key]->ps->link();
	}

	print_messages_push();
	DxfData *dxf;

	if (filename.isEmpty())
	{
#ifdef ENABLE_CGAL
		CGAL_Nef_polyhedron N;
		N.dim = 2;
		foreach(AbstractNode::Pointer v, children) {
			if (v->props.background)
				continue;
			N.p2 += v->render_cgal_nef_polyhedron().p2;
		}
		dxf = new DxfData(N);

#else // ENABLE_CGAL
		PRINT("WARNING: Found rotate_extrude() statement without dxf file but compiled without CGAL support!");
		dxf = new DxfData();
#endif // ENABLE_CGAL
	} else {
		dxf = new DxfData(*this, filename, layername, origin_x, origin_y, scale);
	}

	PolySet *ps = new PolySet();
	ps->convexity = convexity;

	for (int i = 0; i < dxf->paths.count(); i++)
	{
		double max_x = 0;
		for (int j = 0; j < dxf->paths[i].points.count(); j++) {
			max_x = fmax(max_x, dxf->paths[i].points[j]->x);
		}

		int fragments = get_fragments_from_r(max_x, *this);

        double ***points;
        points = new double**[fragments];
        for (int j=0; j < fragments; j++) {
            points[j] = new double*[dxf->paths[i].points.count()];
            for (int k=0; k < dxf->paths[i].points.count(); k++)
                points[j][k] = new double[3];
        }

		for (int j = 0; j < fragments; j++) {
			double a = (j*2*M_PI) / fragments;
			for (int k = 0; k < dxf->paths[i].points.count(); k++) {
				if (dxf->paths[i].points[k]->x == 0) {
					points[j][k][0] = 0;
					points[j][k][1] = 0;
				} else {
					points[j][k][0] = dxf->paths[i].points[k]->x * sin(a);
					points[j][k][1] = dxf->paths[i].points[k]->x * cos(a);
				}
				points[j][k][2] = dxf->paths[i].points[k]->y;
			}
		}

		for (int j = 0; j < fragments; j++) {
			int j1 = j + 1 < fragments ? j + 1 : 0;
			for (int k = 0; k < dxf->paths[i].points.count(); k++) {
				int k1 = k + 1 < dxf->paths[i].points.count() ? k + 1 : 0;
				if (points[j][k][0] != points[j1][k][0] ||
						points[j][k][1] != points[j1][k][1] ||
						points[j][k][2] != points[j1][k][2]) {
					ps->append_poly();
					ps->append_vertex(points[j ][k ][0],
							points[j ][k ][1], points[j ][k ][2]);
					ps->append_vertex(points[j1][k ][0],
							points[j1][k ][1], points[j1][k ][2]);
					ps->append_vertex(points[j ][k1][0],
							points[j ][k1][1], points[j ][k1][2]);
				}
				if (points[j][k1][0] != points[j1][k1][0] ||
						points[j][k1][1] != points[j1][k1][1] ||
						points[j][k1][2] != points[j1][k1][2]) {
					ps->append_poly();
					ps->append_vertex(points[j ][k1][0],
							points[j ][k1][1], points[j ][k1][2]);
					ps->append_vertex(points[j1][k ][0],
							points[j1][k ][1], points[j1][k ][2]);
					ps->append_vertex(points[j1][k1][0],
							points[j1][k1][1], points[j1][k1][2]);
				}
			}
		}

        for (int j=0; j < fragments; j++) {
            for (int k=0; k < dxf->paths[i].points.count(); k++)
                delete[] points[j][k];
            delete[] points[j];
        }
        delete[] points;
	}

	PolySet::ps_cache.insert(key, new PolySet::ps_cache_entry(ps->link()));
	print_messages_pop();
	delete dxf;

	return ps;
}

QString DxfRotateExtrudeNode::dump(QString indent) const
{
	if (dump_cache.isEmpty()) {
		QString text;
		QFileInfo fileInfo(filename);
		text.sprintf("rotate_extrude(file = \"%s\", cache = \"%x.%x\", layer = \"%s\", "
				"origin = [ %g %g ], scale = %g, convexity = %d, "
				"$fn = %g, $fa = %g, $fs = %g) {\n",
				filename.toAscii().data(), (int)fileInfo.lastModified().toTime_t(),
				(int)fileInfo.size(),layername.toAscii().data(), origin_x, origin_y, 
				scale, convexity, fn, fa, fs);
		foreach (AbstractNode::Pointer v, children)
			text += v->dump(indent + QString("\t"));
		text += indent + "}\n";
		((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
	}
	return dump_cache;
}

