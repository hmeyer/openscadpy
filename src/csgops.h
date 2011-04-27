#ifndef CSGOPS_H
#define CSGOPS_H

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
#include "csgterm.h"
#ifdef ENABLE_CGAL
#  include "cgal.h"
#endif
#include <boost/concept_check.hpp>


enum csg_type_e {
	CSG_TYPE_UNION,
	CSG_TYPE_DIFFERENCE,
	CSG_TYPE_INTERSECTION
};

class ModuleInstantiation;

class CsgNode : public AbstractNode
{
public:
	typedef shared_ptr<CsgNode> Pointer;
	csg_type_e type;
	CsgNode(csg_type_e type, const NodeList &children, const Props p=Props()) : AbstractNode(p,children), type(type) { }
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	CSGTerm *render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
};


#endif