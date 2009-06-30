/*
 *  OpenSCAD (www.openscad.at)
 *  Copyright (C) 2009  Clifford Wolf <clifford@clifford.at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#define INCLUDE_ABSTRACT_NODE_DETAILS

#include "openscad.h"

enum control_type_e {
	ASSIGN,
	FOR,
	IF
};

class ControlModule : public AbstractModule
{
public:
	control_type_e type;
	ControlModule(control_type_e type) : type(type) { }
	virtual AbstractNode *evaluate(const Context *ctx, const QVector<QString> &call_argnames, const QVector<Value> &call_argvalues, const QVector<ModuleInstanciation*> arg_children, const Context *arg_context) const;
};

void for_eval(AbstractNode *node, int l, const QVector<QString> &call_argnames, const QVector<Value> &call_argvalues, const QVector<ModuleInstanciation*> arg_children, const Context *arg_context)
{
	if (call_argnames.size() > l) {
		QString it_name = call_argnames[l];
		Value it_values = call_argvalues[l];
		Context c(arg_context);
		if (it_values.type == Value::RANGE) {
			for (double i = it_values.r_begin; i <= it_values.r_end; i += it_values.r_step) {
				fprintf(stderr, "%f\n", i);
				c.set_variable(it_name, Value(i));
				for_eval(node, l+1, call_argnames, call_argvalues, arg_children, &c);
			}
		}
		else {
			for_eval(node, l+1, call_argnames, call_argvalues, arg_children, &c);
		}
	} else {
		foreach (ModuleInstanciation *v, arg_children) {
			AbstractNode *n = v->evaluate(arg_context);
			if (n != NULL)
				node->children.append(n);
		}
	}
}

AbstractNode *ControlModule::evaluate(const Context*, const QVector<QString> &call_argnames, const QVector<Value> &call_argvalues, const QVector<ModuleInstanciation*> arg_children, const Context *arg_context) const
{
	AbstractNode *node = new AbstractNode();

	if (type == ASSIGN)
	{
		Context c(arg_context);
		for (int i = 0; i < call_argnames.size(); i++) {
			if (!call_argnames[i].isEmpty())
				c.set_variable(call_argnames[i], call_argvalues[i]);
		}
		foreach (ModuleInstanciation *v, arg_children) {
			AbstractNode *n = v->evaluate(&c);
			if (n != NULL)
				node->children.append(n);
		}
	}

	if (type == FOR)
	{
		for_eval(node, 0, call_argnames, call_argvalues, arg_children, arg_context);
	}

	if (type == IF)
	{
		if (call_argvalues.size() > 0 && call_argvalues[0].type == Value::BOOL && call_argvalues[0].b)
			foreach (ModuleInstanciation *v, arg_children) {
				AbstractNode *n = v->evaluate(arg_context);
				if (n != NULL)
					node->children.append(n);
			}
	}

	return node;
}

void register_builtin_control()
{
	builtin_modules["assign"] = new ControlModule(ASSIGN);
	// builtin_modules["for"] = new ControlModule(FOR);
	builtin_modules["if"] = new ControlModule(IF);
}
