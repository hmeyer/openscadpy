#ifndef DXFROTEXTRUDE_H
#define DXFROTEXTRUDE_H

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

class DxfRotateExtrudeNode : public AbstractPolyNode
{
public:
	typedef shared_ptr<DxfRotateExtrudeNode> Pointer;
	int convexity;
	double origin_x, origin_y, scale;
	QString filename, layername;
	Accuracy acc;
	DxfRotateExtrudeNode(const AbstractNode::NodeList &children, const QString &filename, const QString &layer,
	  double origin_x, double origin_y, double scale, 
	  int convexity, const Accuracy &acc=Accuracy(), const Props p=Props())
	    :AbstractPolyNode(p,children), convexity(convexity),
	    origin_x(origin_x), origin_y(origin_y), scale(scale), filename(filename), layername(layer), acc(acc) {}
	virtual PolySet *render_polyset(render_mode_e mode) const;
	virtual QString dump(QString indent) const;
};


#endif