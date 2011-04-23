#ifndef NODE_H_
#define NODE_H_

#include <QCache>
#include <QVector>

#ifdef ENABLE_CGAL
#include "cgal.h"
#endif

extern int progress_report_count;
extern void (*progress_report_f)(const class AbstractNode*, void*, int);
extern void *progress_report_vp;

void progress_report_prep(AbstractNode *root, void (*f)(const class AbstractNode *node, void *vp, int mark), void *vp);
void progress_report_fin();

class AbstractNode
{
	static int idx_counter;   // Node instantiation index
public:
	static void resetIndexCounter() { idx_counter = 1; }

	QVector<AbstractNode*> children;
	const bool tag_root;
	const bool tag_highlight;
	const bool tag_background;

	int progress_mark;
	void progress_prepare();
	void progress_report() const;

	int idx;
	QString dump_cache;


	AbstractNode(bool root, bool highlight, bool background);
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
	class CSGTerm *render_csg_term_from_nef(double m[20], QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, const char *statement, int convexity) const;
#endif
	virtual class CSGTerm *render_csg_term(double m[20], QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
	void append(AbstractNode *child) { children.append(child); }
};

class AbstractIntersectionNode : public AbstractNode
{
public:
	AbstractIntersectionNode(bool root, bool highlight, bool background) : AbstractNode(root, highlight, background) { };
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	virtual CSGTerm *render_csg_term(double m[20], QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	virtual QString dump(QString indent) const;
};

class AbstractPolyNode : public AbstractNode
{
public:
	enum render_mode_e {
		RENDER_CGAL,
		RENDER_OPENCSG
	};
	AbstractPolyNode(bool root, bool highlight, bool background) : AbstractNode(root, highlight, background) { };
	virtual class PolySet *render_polyset(render_mode_e mode) const = 0;
#ifdef ENABLE_CGAL
	virtual CGAL_Nef_polyhedron render_cgal_nef_polyhedron() const;
#endif
	virtual CSGTerm *render_csg_term(double m[20], QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background) const;
	static CSGTerm *render_csg_term_from_ps(double m[20], QVector<CSGTerm*> *highlights, QVector<CSGTerm*> *background, PolySet *ps, bool tag_highlight, bool tag_background, int idx);
};

#endif
