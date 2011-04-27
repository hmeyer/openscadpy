#ifndef TRANSFORM_H
#define TRANSFORM_H

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
#include "matrix.h"
#ifdef ENABLE_CGAL
#  include "cgal.h"
#endif

enum transform_type_e {
	SCALE,
	ROTATE,
	MIRROR,
	TRANSLATE,
	MULTMATRIX,
	COLOR
};

class ModuleInstantiation;

class TransformNode : public AbstractNode
{
public:
	typedef shared_ptr< TransformNode > Pointer;
	Float20 m;
	TransformNode(const NodeList &children, const Props p=Props());
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	virtual CSGTerm *render_csg_term(const Float20 &c, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
};

class TransformScaleNode : public TransformNode {
public:	TransformScaleNode(const Float3 &scale, const NodeList &children, const Props p=Props());
};

class TransformRotateNode : public TransformNode {
public: TransformRotateNode(const Float3 &rotation, const NodeList &children, const Props p=Props());
};

class TransformRotateAxisNode : public TransformNode {
public: TransformRotateAxisNode(const Float3 &axis, FloatType angle, const NodeList &children, const Props p=Props());
};

class TransformMirrorNode : public TransformNode {
public: TransformMirrorNode(const Float3 &axis, const NodeList &children, const Props p=Props());
};

class TransformTranslateNode : public TransformNode {
public: TransformTranslateNode(const Float3 &v, const NodeList &children, const Props p=Props());
};

class TransformMatrixNode : public TransformNode {
public: TransformMatrixNode(const Float16 &mat, const NodeList &children, const Props p=Props());
};

class TransformColorNode : public TransformNode {
public: TransformColorNode(const Float4 &color, const NodeList &children, const Props p=Props());
};
#endif