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

#include "import.h"
#include "polyset.h"
#include "dxfdata.h"
#include "dxftess.h"
#include "printutils.h"
#include "openscad.h" // handle_dep()

#include <QFile>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/make_shared.hpp>

enum import_type_e {
       TYPE_STL,
       TYPE_OFF,
       TYPE_DXF
};


PolySet *ImportSTLNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  p->convexity = convexity;
  handle_dep(filename);
  QFile f(filename);
  if (!f.open(QIODevice::ReadOnly)) {
    PRINTF("WARNING: Can't open import file `%s'.", filename.toAscii().data());
    return p;
  }

  QByteArray data = f.read(5);
  if (data.size() == 5 && QString(data) == QString("solid"))
  {
    int i = 0;
    double vdata[3][3];
    QRegExp splitre = QRegExp("\\s*(vertex)?\\s+");
    f.readLine();
    while (!f.atEnd())
    {
      QString line = QString(f.readLine()).remove("\n").remove("\r");
      if (line.contains("solid") || line.contains("facet") || line.contains("endloop"))
	      continue;
      if (line.contains("outer loop")) {
	      i = 0;
	      continue;
      }
      if (line.contains("vertex")) {
	QStringList tokens = line.split(splitre);
	bool ok[3] = { false, false, false };
	if (tokens.size() == 4) {
		vdata[i][0] = tokens[1].toDouble(&ok[0]);
		vdata[i][1] = tokens[2].toDouble(&ok[1]);
		vdata[i][2] = tokens[3].toDouble(&ok[2]);
	}
	if (!ok[0] || !ok[1] || !ok[2]) {
		PRINTF("WARNING: Can't parse vertex line `%s'.", line.toAscii().data());
		i = 10;
	} else if (++i == 3) {
		p->append_poly();
		p->append_vertex(vdata[0][0], vdata[0][1], vdata[0][2]);
		p->append_vertex(vdata[1][0], vdata[1][1], vdata[1][2]);
		p->append_vertex(vdata[2][0], vdata[2][1], vdata[2][2]);
	}
      }
    }
  } else {
    f.read(80-5+4);
    while (1) {
#ifdef _MSC_VER
#pragma pack(push,1)
#endif
      struct {
	float i, j, k;
	float x1, y1, z1;
	float x2, y2, z2;
	float x3, y3, z3;
	unsigned short acount;
      }
#ifdef __GNUC__
      __attribute__ ((packed))
#endif
      data;
#ifdef _MSC_VER
#pragma pack(pop)
#endif

      if (f.read((char*)&data, sizeof(data)) != sizeof(data))
	      break;
      p->append_poly();
      p->append_vertex(data.x1, data.y1, data.z1);
      p->append_vertex(data.x2, data.y2, data.z2);
      p->append_vertex(data.x3, data.y3, data.z3);
    }
  }
  return p;
}

PolySet *ImportDXFNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  p->convexity = convexity;
  DxfData dd(*this, filename, layername, origin[0], origin[1], scale);
  p->is2d = true;
  dxf_tesselate(p, &dd, 0, true, false, 0);
  dxf_border_to_ps(p, &dd);
  return p;
}

PolySet *ImportOFFNode::render_polyset(render_mode_e) const {
  PolySet *p = new PolySet();
  p->convexity = convexity;
  PRINTF("WARNING: OFF import is not implemented yet.");
  return p;
}

QString ImportSTLNode::dump(QString indent) const {
  if (dump_cache.isEmpty()) {
    QString text;
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    stat(filename.toAscii().data(), &st);
    text.sprintf("import_stl(file = \"%s\", cache = \"%x.%x\", convexity = %d);\n",
		    filename.toAscii().data(), (int)st.st_mtime, (int)st.st_size, convexity);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;  
}

QString ImportOFFNode::dump(QString indent) const {
  if (dump_cache.isEmpty()) {
    QString text;
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    stat(filename.toAscii().data(), &st);
    text.sprintf("import_off(file = \"%s\", cache = \"%x.%x\", convexity = %d);\n",
		    filename.toAscii().data(), (int)st.st_mtime, (int)st.st_size, convexity);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}

QString ImportDXFNode::dump(QString indent) const {
  if (dump_cache.isEmpty()) {
    QString text;
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    stat(filename.toAscii().data(), &st);
    text.sprintf("import_dxf(file = \"%s\", cache = \"%x.%x\", layer = \"%s\", "
		    "origin = [ %g %g ], scale = %g, convexity = %d, "
		    "$fn = %g, $fa = %g, $fs = %g);\n",
		    filename.toAscii().data(), (int)st.st_mtime, (int)st.st_size,
		    layername.toAscii().data(), origin[0], origin[1], scale, convexity,
		    fn, fa, fs);
    ((AbstractNode*)this)->dump_cache = indent + QString("n%1: ").arg(idx) + text;
  }
  return dump_cache;
}