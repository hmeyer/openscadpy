#ifndef CSGTERM_H_
#define CSGTERM_H_

#include <QString>
#include <QVector>
#include "matrix.h"
#include <boost/shared_ptr.hpp>

class CSGTerm
{
public:
	enum type_e {
		TYPE_PRIMITIVE,
		TYPE_UNION,
		TYPE_INTERSECTION,
		TYPE_DIFFERENCE
	};

	type_e type;
	class PolySet *polyset;
	QString label;
	CSGTerm *left;
	CSGTerm *right;
	Float20 m;
	int refcounter;

	CSGTerm(PolySet *polyset, const Float20 &m, QString label);
	CSGTerm(type_e type, CSGTerm *left, CSGTerm *right);

	CSGTerm *normalize();
	CSGTerm *normalize_tail();

	CSGTerm *link();
	void unlink();
	QString dump();
};

class CSGChain
{
public:
	QVector<PolySet*> polysets;
	QVector<Float20Ptr> matrices;
	QVector<CSGTerm::type_e> types;
	QVector<QString> labels;

	CSGChain();

	void add(PolySet *polyset, const Float20 &m, CSGTerm::type_e type, QString label);
	void import(CSGTerm *term, CSGTerm::type_e type = CSGTerm::TYPE_UNION);
	QString dump();
};

#endif
