#ifndef NODE_H_
#define NODE_H_

#include <QCache>
#include <QVector>

#ifdef ENABLE_CGAL
#include "cgal.h"
#endif

#include <boost/shared_ptr.hpp>
#include "matrix.h"

using boost::shared_ptr;

extern int progress_report_count;
extern void (*progress_report_f)(const class AbstractNode&, void*, int);
extern void *progress_report_vp;

void progress_report_prep(AbstractNode &root, void (*f)(const class AbstractNode &node, void *vp, int mark), void *vp);
void progress_report_fin();

class AbstractNode
{
	static int idx_counter;   // Node instantiation index
public:
	struct Props {
	  bool root;
	  bool highlight;
	  bool background;
	  inline Props():root(false),highlight(false),background(false) {}
	  inline Props(bool root, bool highlight, bool background)
	    :root(root),highlight(highlight),background(background) {}
	};
	typedef shared_ptr<AbstractNode> Pointer;
	typedef QVector<Pointer> NodeList;
	static void resetIndexCounter() { idx_counter = 1; }

	NodeList children;
	Props props;

	int progress_mark;
	void progress_prepare();
	void progress_report() const;

	int idx;
	QString dump_cache;

	AbstractNode(const Props &p);
	AbstractNode(const Props &p, const NodeList &children);
	virtual ~AbstractNode();
	virtual QString mk_cache_id() const;
#ifdef ENABLE_CGAL
	struct cgal_nef_cache_entry {
		CGAL_Nef_polyhedron N;
		QString msg;
		cgal_nef_cache_entry(const CGAL_Nef_polyhedron &N);
	};
	static QCache<QString, cgal_nef_cache_entry> cgal_nef_cache;
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
	class CSGTerm *render_csg_term_from_nef(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, const char *statement, int convexity) const;
#endif
	virtual class CSGTerm *render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
};

class AbstractIntersectionNode : public AbstractNode
{
public:
	AbstractIntersectionNode(const Props &p) : AbstractNode(p) { };
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	virtual CSGTerm *render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
};

class AbstractPolyNode : public AbstractNode
{
public:
	enum render_mode_e {
		RENDER_CGAL,
		RENDER_OPENCSG
	};
	AbstractPolyNode(const Props &p) : AbstractNode(p) { };
	AbstractPolyNode(const Props &p, const NodeList &children): AbstractNode(p, children) { };
	virtual class PolySet *render_polyset(render_mode_e mode) const = 0;
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	virtual CSGTerm *render_csg_term(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	static CSGTerm *render_csg_term_from_ps(const Float20 &m, QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, PolySet *ps, const Props &p, int idx);
};

#endif
