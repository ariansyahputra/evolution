/* 
 * Copyright 2000 HelixCode (http://www.helixcode.com).
 *
 * A simple, extensible s-exp evaluation engine.
 *
 * Author : 
 *  Michael Zucchi <notzed@helixcode.com>

 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
  The following built-in s-exp's are supported:

  list = (and list*)
	perform an intersection of a number of lists, and return that.

  bool = (and bool*)
	perform a boolean AND of boolean values.

  list = (or list*)
	perform a union of a number of lists, returning the new list.

  bool = (or bool*)
	perform a boolean OR of boolean values.

  int = (+ int*)
	Add integers.

  string = (+ string*)
	Concat strings.

  int = (- int int*)
	Subtract integers from the first.

  Comparison operators:

  bool = (< int int)
  bool = (> int int)
  bool = (= int int)

  bool = (< string string)
  bool = (> string string)
  bool = (= string string)
	Perform a comparision of 2 integers, or 2 string values.

  Function flow:

  type = (if bool function)
  type = (if bool function function)
  	Choose a flow path based on a boolean value

  type = (begin  func func func)
        Execute a sequence.  The last function return is the return type.
*/

#include <glib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "filter-sexp.h"

#define p(x)			/* parse debug */
#define r(x)			/* run debug */
#define d(x)			/* general debug */


static struct _FilterSEXPTerm * parse_list(FilterSEXP *f, int gotbrace);
static struct _FilterSEXPTerm * parse_value(FilterSEXP *f);

static void parse_dump_term(struct _FilterSEXPTerm *t, int depth);

static GtkObjectClass *parent_class;



static GScannerConfig scanner_config =
{
	( " \t\r\n")		/* cset_skip_characters */,
	( G_CSET_a_2_z
	  "_+<=>"
	  G_CSET_A_2_Z)		/* cset_identifier_first */,
	( G_CSET_a_2_z
	  "_0123456789-<>"
	  G_CSET_A_2_Z
	  G_CSET_LATINS
	  G_CSET_LATINC	)	/* cset_identifier_nth */,
	( ";\n" )		/* cpair_comment_single */,
  
	FALSE			/* case_sensitive */,
  
	TRUE			/* skip_comment_multi */,
	TRUE			/* skip_comment_single */,
	TRUE			/* scan_comment_multi */,
	TRUE			/* scan_identifier */,
	TRUE			/* scan_identifier_1char */,
	FALSE			/* scan_identifier_NULL */,
	TRUE			/* scan_symbols */,
	FALSE			/* scan_binary */,
	TRUE			/* scan_octal */,
	TRUE			/* scan_float */,
	TRUE			/* scan_hex */,
	FALSE			/* scan_hex_dollar */,
	TRUE			/* scan_string_sq */,
	TRUE			/* scan_string_dq */,
	TRUE			/* numbers_2_int */,
	FALSE			/* int_2_float */,
	FALSE			/* identifier_2_string */,
	TRUE			/* char_2_token */,
	FALSE			/* symbol_2_token */,
	FALSE			/* scope_0_fallback */,
};

struct _FilterSEXPResult *
filter_sexp_result_new(int type)
{
	struct _FilterSEXPResult *r = g_malloc0(sizeof(*r));
	r->type = type;
	return r;
}

void
filter_sexp_result_free(struct _FilterSEXPResult *t)
{
	if (t == NULL)
		return;

	switch(t->type) {
	case FSEXP_RES_ARRAY_PTR:
		g_ptr_array_free(t->value.ptrarray, TRUE);
		break;
	case FSEXP_RES_BOOL:
	case FSEXP_RES_INT:
		break;
	case FSEXP_RES_STRING:
		g_free(t->value.string);
		break;
	case FSEXP_RES_UNDEFINED:
		break;
	}
	g_free(t);
}

/* implementations for the builtin functions */

/* can you tell, i dont like glib? */
/* we can only itereate a hashtable from a called function */
struct _glib_sux_donkeys {
	int count;
	GPtrArray *uids;
};

/* ok, store any values that are in all sets */
static void
g_lib_sux_htand(char *key, int value, struct _glib_sux_donkeys *fuckup)
{
	if (value == fuckup->count) {
		g_ptr_array_add(fuckup->uids, key);
	}
}

/* or, store all unique values */
static void
g_lib_sux_htor(char *key, int value, struct _glib_sux_donkeys *fuckup)
{
	g_ptr_array_add(fuckup->uids, key);
}

static FilterSEXPResult *
term_eval_and(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;
	GHashTable *ht = g_hash_table_new(g_str_hash, g_str_equal);
	struct _glib_sux_donkeys lambdafoo;
	int type=-1;
	int bool = TRUE;
	int i;
	
	r(printf("( and\n"));

	r = filter_sexp_result_new(FSEXP_RES_UNDEFINED);
	
	for (i=0;bool && i<argc;i++) {
		r1 = filter_sexp_term_eval(f, argv[i]);
		if (type == -1)
			type = r1->type;
		if (type != r1->type) {
			printf("invalid types in and operation, all types must be the same\n");
		} else if ( r1->type == FSEXP_RES_ARRAY_PTR ) {
			char **a1;
			int l1, j;
			
			a1 = (char **)r1->value.ptrarray->pdata;
			l1 = r1->value.ptrarray->len;
			for (j=0;i<l1;j++) {
				int n;
				n = (int)g_hash_table_lookup(ht, a1[i]);
				g_hash_table_insert(ht, a1[i], (void *)n+1);
			}
		} else if ( r1->type == FSEXP_RES_BOOL ) {
			bool &= r1->value.bool;
		}
		filter_sexp_result_free(r1);
	}
	
	if (type == FSEXP_RES_ARRAY_PTR) {
		lambdafoo.count = argc;
		lambdafoo.uids = g_ptr_array_new();
		g_hash_table_foreach(ht, (GHFunc)g_lib_sux_htand, &lambdafoo);
		r->type = FSEXP_RES_ARRAY_PTR;
		r->value.ptrarray = lambdafoo.uids;
	} else if (type == FSEXP_RES_BOOL) {
		r->type = FSEXP_RES_BOOL;
		r->value.bool = bool;
	}
	
	g_hash_table_destroy(ht);
	
	return r;
}

static FilterSEXPResult *
term_eval_or(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;
	GHashTable *ht = g_hash_table_new(g_str_hash, g_str_equal);
	struct _glib_sux_donkeys lambdafoo;
	int type = -1;
	int bool = FALSE;
	int i;
	
	r(printf("(or \n"));

	r = filter_sexp_result_new(FSEXP_RES_UNDEFINED);
	
	for (i=0;!bool && i<argc;i++) {
		r1 = filter_sexp_term_eval(f, argv[i]);
		if (type == -1)
			type = r1->type;
		if (r1->type != type) {
			printf("wrong types in or operation\n");
		} else if (r1->type == FSEXP_RES_ARRAY_PTR) {
			char **a1;
			int l1, j;
			
			a1 = (char **)r1->value.ptrarray->pdata;
			l1 = r1->value.ptrarray->len;
			for (j=0;i<l1;j++) {
				g_hash_table_insert(ht, a1[j], (void *)1);
			}
		} else if (r1->type == FSEXP_RES_BOOL) {
			bool |= r1->value.bool;				
		}
		filter_sexp_result_free(r1);
	}
	
	if (type == FSEXP_RES_ARRAY_PTR) {
		lambdafoo.count = argc;
		lambdafoo.uids = g_ptr_array_new();
		g_hash_table_foreach(ht, (GHFunc)g_lib_sux_htor, &lambdafoo);
		r->type = FSEXP_RES_ARRAY_PTR;
		r->value.ptrarray = lambdafoo.uids;
	} else if (type == FSEXP_RES_BOOL) {
		r->type = FSEXP_RES_BOOL;
		r->value.bool = bool;
	}
	g_hash_table_destroy(ht);
	
	return r;
}

/* this should support all arguments ...? */
static FilterSEXPResult *
term_eval_lt(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;

	r = filter_sexp_result_new(FSEXP_RES_UNDEFINED);
	
	if (argc == 2) {
		r1 = filter_sexp_term_eval(f, argv[0]);
		r2 = filter_sexp_term_eval(f, argv[1]);
		if (r1->type != r2->type) {
			printf("error, invalid types in compare\n");
		} else if (r1->type == FSEXP_RES_INT) {
			r->type = FSEXP_RES_BOOL;
			r->value.bool = r1->value.number < r2->value.number;
		} else if (r1->type == FSEXP_RES_STRING) {
			r->type = FSEXP_RES_BOOL;
			r->value.bool = strcmp(r1->value.string, r2->value.string) < 0;
		}
	}
	return r;
}

/* this should support all arguments ...? */
static FilterSEXPResult *
term_eval_gt(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;

	r = filter_sexp_result_new(FSEXP_RES_UNDEFINED);
	
	if (argc == 2) {
		r1 = filter_sexp_term_eval(f, argv[0]);
		r2 = filter_sexp_term_eval(f, argv[1]);
		if (r1->type != r2->type) {
			printf("error, invalid types in compare\n");
		} else if (r1->type == FSEXP_RES_INT) {
			r->type = FSEXP_RES_BOOL;
			r->value.bool = r1->value.number > r2->value.number;
		} else if (r1->type == FSEXP_RES_STRING) {
			r->type = FSEXP_RES_BOOL;
			r->value.bool = strcmp(r1->value.string, r2->value.string) > 0;
		}
	}
	return r;
}

/* this should support all arguments ...? */
static FilterSEXPResult *
term_eval_eq(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;

	r = filter_sexp_result_new(FSEXP_RES_BOOL);
	
	if (argc == 2) {
		r1 = filter_sexp_term_eval(f, argv[0]);
		r2 = filter_sexp_term_eval(f, argv[1]);
		if (r1->type != r2->type) {
			r->value.bool = FALSE;
		} else if (r1->type == FSEXP_RES_INT) {
			r->value.bool = r1->value.number == r2->value.number;
		} else if (r1->type == FSEXP_RES_BOOL) {
			r->value.bool = r1->value.bool == r2->value.bool;
		} else if (r1->type == FSEXP_RES_STRING) {
			r->value.bool = strcmp(r1->value.string, r2->value.string) == 0;
		}
	}
	return r;
}

static FilterSEXPResult *
term_eval_plus(struct _FilterSEXP *f, int argc, struct _FilterSEXPResult **argv, void *data)
{
	struct _FilterSEXPResult *r=NULL, *r1, *r2;
	int type;
	int i;

	if (argc>0) {
		type = argv[0]->type;
		switch(type) {
		case FSEXP_RES_INT: {
			int total = argv[0]->value.number;
			for (i=1;i<argc && argv[i]->type == FSEXP_RES_INT;i++) {
				total += argv[i]->value.number;
			}
			if (i<argc) {
				g_warning("Wrong type trying to add integers: ignored");
			}
			r = filter_sexp_result_new(FSEXP_RES_INT);
			r->value.number = total;
			break; }
		case FSEXP_RES_STRING: {
			GString *s = g_string_new(argv[0]->value.string);
			for (i=1;i<argc && argv[i]->type == FSEXP_RES_STRING;i++) {
				g_string_append(s, argv[i]->value.string);
			}
			if (i<argc) {
				g_warning("Wrong type trying to concat strings: ignored");
			}
			r = filter_sexp_result_new(FSEXP_RES_STRING);
			r->value.string = s->str;
			g_string_free(s, FALSE);
			break; }
			
		}
	}

	if (!r) {
		r = filter_sexp_result_new(FSEXP_RES_INT);
		r->value.number = 0;
	}
	return r;
}

static FilterSEXPResult *
term_eval_sub(struct _FilterSEXP *f, int argc, struct _FilterSEXPResult **argv, void *data)
{
	struct _FilterSEXPResult *r=NULL, *r1, *r2;
	int type;
	int i;

	if (argc>0) {
		type = argv[0]->type;
		switch(type) {
		case FSEXP_RES_INT: {
			int total = argv[0]->value.number;
			for (i=1;i<argc && argv[i]->type == FSEXP_RES_INT;i++) {
				total -= argv[i]->value.number;
			}
			if (i<argc) {
				g_warning("Wrong type trying to subtract numbers: ignored");
			}
			r = filter_sexp_result_new(FSEXP_RES_INT);
			r->value.number = total;
			break; }
		}
	}

	if (!r) {
		r = filter_sexp_result_new(FSEXP_RES_INT);
		r->value.number = 0;
	}
	return r;
}

/* implements 'if' function */
static FilterSEXPResult *
term_eval_if(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r, *r1, *r2;
	int doit;

	if (argc >=2 && argc<=3) {
		r = filter_sexp_term_eval(f, argv[0]);
		doit = (r->type == FSEXP_RES_BOOL && r->value.bool);
		filter_sexp_result_free(r);
		if (doit) {
			return filter_sexp_term_eval(f, argv[1]);
		} else if (argc>2) {
			return filter_sexp_term_eval(f, argv[2]);
		}
	}
	return filter_sexp_result_new(FSEXP_RES_UNDEFINED);
}

/* implements 'begin' statement */
static FilterSEXPResult *
term_eval_begin(struct _FilterSEXP *f, int argc, struct _FilterSEXPTerm **argv, void *data)
{
	struct _FilterSEXPResult *r=NULL, *r1, *r2;
	int i;

	for (i=0;i<argc;i++) {
		if (r)
			filter_sexp_result_free(r);
		r = filter_sexp_term_eval(f, argv[i]);
	}
	if (r)
		return r;
	else
		return filter_sexp_result_new(FSEXP_RES_UNDEFINED);
}


struct _FilterSEXPResult *
filter_sexp_term_eval(struct _FilterSEXP *f, struct _FilterSEXPTerm *t)
{
	struct _FilterSEXPResult *r, *r1, *r2;
	int i;

	r(printf("eval term :\n"));
	r(parse_dump_term(t, 0));

	r = g_malloc0(sizeof(*r));
	r->type = FSEXP_RES_UNDEFINED;

	switch (t->type) {
	case FSEXP_TERM_STRING:
		r(printf(" (string \"%s\")\n", t->value.string));
		r->type = FSEXP_RES_STRING;
		/* erk, this shoul;dn't need to strdup this ... */
		r->value.string = g_strdup(t->value.string);
		break;
	case FSEXP_TERM_INT:
		r(printf(" (int %d)\n", t->value.number));
		r->type = FSEXP_RES_INT;
		r->value.number = t->value.number;
		break;
	case FSEXP_TERM_BOOL:
		r(printf(" (int %d)\n", t->value.number));
		r->type = FSEXP_RES_BOOL;
		r->value.bool = t->value.bool;
		break;
	case FSEXP_TERM_IFUNC: {
		if (t->value.func.sym->f.ifunc) {
			r1 = t->value.func.sym->f.ifunc(f, t->value.func.termcount, t->value.func.terms, t->value.func.sym->data);
			if (r1) {
				filter_sexp_result_free(r);
				r = r1;
			}
		}
		break; }
	case FSEXP_TERM_FUNC: {
		struct _FilterSEXPResult **argv;

		/* first evaluate all arguments to result types */
		argv = alloca(sizeof(argv[0]) * t->value.func.termcount);
		for (i=0;i<t->value.func.termcount;i++) {
			argv[i] = filter_sexp_term_eval(f, t->value.func.terms[i]);
		}
		/* call the function */
		if (t->value.func.sym->f.func) {
			r1 = t->value.func.sym->f.func(f, t->value.func.termcount, argv, t->value.func.sym->data);
			if (r1) {
				filter_sexp_result_free(r);
				r = r1;
			}
		}
		for (i=0;i<t->value.func.termcount;i++) {
			filter_sexp_result_free(argv[i]);
		}
		break; }
	default:
		printf("Warning: Unknown type encountered in parse tree: %d\n", t->type);
		r->type = FSEXP_RES_UNDEFINED;
	}

	return r;
}


static void
eval_dump_result(FilterSEXPResult *r, int depth)
{
	int dumpvals = FALSE;
	int i;
	
	if (r==NULL) {
		printf("null result???\n");
		return;
	}

	for (i=0;i<depth;i++)
		printf("   ");

	switch (r->type) {
	case FSEXP_RES_ARRAY_PTR:
		printf("array pointers\n");
		break;
	case FSEXP_RES_INT:
		printf("int: %d\n", r->value.number);
		break;
	case FSEXP_RES_STRING:
		printf("string: '%s'\n", r->value.string);
		break;
	case FSEXP_RES_BOOL:
		printf("bool: %c\n", r->value.bool?'t':'f');
		break;
	case FSEXP_RES_UNDEFINED:
		printf(" <undefined>\n");
		break;
	}
	printf("\n");
}

static void
parse_dump_term(struct _FilterSEXPTerm *t, int depth)
{
	int dumpvals = FALSE;
	int i;

	if (t==NULL) {
		printf("null term??\n");
		return;
	}

	for (i=0;i<depth;i++)
		printf("   ");
	
	switch (t->type) {
	case FSEXP_TERM_STRING:
		printf(" \"%s\"", t->value.string);
		break;
	case FSEXP_TERM_INT:
		printf(" %d", t->value.number);
		break;
	case FSEXP_TERM_BOOL:
		printf(" #%c", t->value.bool?'t':'f');
		break;
	case FSEXP_TERM_IFUNC:
	case FSEXP_TERM_FUNC:
		printf(" (function %s\n", t->value.func.sym->name);
		/*printf(" [%d] ", t->value.func.termcount);*/
		for (i=0;i<t->value.func.termcount;i++) {
			parse_dump_term(t->value.func.terms[i], depth+1);
		}
		for (i=0;i<depth;i++)
			printf("   ");
		printf(" )");
		break;
	case FSEXP_TERM_VAR:
		printf(" (variable %s )\n", t->value.var->name);
		break;
	default:
		printf("unknown type: %d\n", t->type);
	}

	printf("\n");
}

/*
  PARSER
*/

static struct _FilterSEXPTerm *
parse_new_term(int type)
{
	struct _FilterSEXPTerm *s = g_malloc0(sizeof(*s));
	s->type = type;
	return s;
}

static void
parse_term_free(struct _FilterSEXPTerm *t)
{
	int i;

	if (t==NULL) {
		return;
	}
	
	switch (t->type) {
	case FSEXP_TERM_FUNC:
	case FSEXP_TERM_IFUNC:
		for (i=0;i<t->value.func.termcount;i++) {
			parse_term_free(t->value.func.terms[i]);
		}
		g_free(t->value.func.terms);
		break;
	case FSEXP_TERM_VAR:
		break;
	case FSEXP_TERM_STRING:
		g_free(t->value.string);
		break;
	case FSEXP_TERM_INT:
		break;
	default:
		printf("parse_term_free: unknown type: %d\n", t->type);
	}
	g_free(t);
}

static struct _FilterSEXPTerm **
parse_values(FilterSEXP *f, int *len)
{
	int token;
	struct _FilterSEXPTerm **terms;
	int i=0;
	GScanner *gs = f->scanner;

	p(printf("parsing values\n"));

	/* FIXME: This hardcoded nonsense!!! :) */
	terms = g_malloc0(20*sizeof(*terms));

	while ( (token = g_scanner_peek_next_token(gs)) != G_TOKEN_EOF
		&& token != ')') {
		terms[i]=parse_value(f);
		i++;
	}

	p(printf("found %d subterms\n", i));
	*len = i;
	
	p(printf("dont parsing values\n"));
	return terms;
}

static struct _FilterSEXPTerm *
parse_value(FilterSEXP *f)
{
	int token;
	struct _FilterSEXPTerm *t = NULL;
	GScanner *gs = f->scanner;
	struct _FilterSEXPSymbol *s;

	p(printf("parsing value\n"));

	token = g_scanner_get_next_token(gs);
	switch(token) {
	case G_TOKEN_LEFT_PAREN:
		p(printf("got brace, its a list!\n"));
		return parse_list(f, TRUE);
	case G_TOKEN_STRING:
		p(printf("got string\n"));
		t = parse_new_term(FSEXP_TERM_STRING);
		t->value.string = g_strdup(g_scanner_cur_value(gs).v_string);
		break;
	case G_TOKEN_INT:
		t = parse_new_term(FSEXP_TERM_INT);
		t->value.number = g_scanner_cur_value(gs).v_int;
		p(printf("got int\n"));
		break;
	case '#':
		printf("got bool?\n");
		token = g_scanner_get_next_token(gs);
		t = parse_new_term(FSEXP_TERM_BOOL);
		t->value.bool = token=='t';
		break;
	case G_TOKEN_SYMBOL:
		s = g_scanner_cur_value(gs).v_symbol;
		switch (s->type) {
		case FSEXP_TERM_FUNC:
		case FSEXP_TERM_IFUNC:
				/* this is basically invalid, since we can't use function
				   pointers, but let the runtime catch it ... */
			t = parse_new_term(s->type);
			t->value.func.sym = s;
			t->value.func.terms = parse_values(f, &t->value.func.termcount);
			break;
		case FSEXP_TERM_VAR:
			t = parse_new_term(s->type);
			t->value.var = s;
			break;
		default:
			printf("Invalid symbol type: %d\n", s->type);
		}
		break;
	case G_TOKEN_IDENTIFIER:
		printf("Unknown identifier encountered: %s\n", g_scanner_cur_value(gs).v_identifier);
		break;
	default:
		printf("Innvalid token trying to parse a list of values\n");
	}
	p(printf("done parsing value\n"));
	return t;
}

/* FIXME: this needs some robustification */
static struct _FilterSEXPTerm *
parse_list(FilterSEXP *f, int gotbrace)
{
	int token;
	struct _FilterSEXPTerm *t = NULL;
	GScanner *gs = f->scanner;

	p(printf("parsing list\n"));
	if (gotbrace)
		token = '(';
	else
		token = g_scanner_get_next_token(gs);
	if (token =='(') {
		token = g_scanner_get_next_token(gs);
		switch(token) {
		case G_TOKEN_SYMBOL: {
			struct _FilterSEXPSymbol *s;

			s = g_scanner_cur_value(gs).v_symbol;
			p(printf("got funciton: %s\n", s->name));
			t = parse_new_term(s->type);
			p(printf("created new list %p\n", t));
			/* if we have a variable, find out its base type */
			while (s->type == FSEXP_TERM_VAR) {
				s = ((FilterSEXPTerm *)(s->data))->value.var;
			}
			if (s->type == FSEXP_TERM_FUNC
			    || s->type == FSEXP_TERM_IFUNC) {
				t->value.func.sym = s;
				t->value.func.terms = parse_values(f, &t->value.func.termcount);
			} else {
				printf("Error, trying to call variable as function\n");
			}
			break; }
		case G_TOKEN_IDENTIFIER:
			printf("Unknown identifier: %s\n", g_scanner_cur_value(gs).v_identifier);
			break;
		default:
			printf("unknown sequence encountered, type = %d\n", token);
		}
		token = g_scanner_get_next_token(gs);
		if (token != ')') {
			printf("Error, expected ')' not found\n");
		}
	} else {
		printf("Error, list term without opening (\n");
	}

	p(printf("returning list %p\n", t));
	return t;
}

#if 0
GList *
camel_mbox_folder_search_by_expression(CamelFolder *folder, char *expression, CamelException *ex)
{
	GScanner *gs;
	int i;
	struct _FilterSEXPTerm *t;
	struct _searchcontext *ctx;
	struct _FilterSEXPResult *r;
	GList *matches = NULL;

	gs = g_scanner_new(&scanner_config);
	for(i=0;i<sizeof(symbols)/sizeof(symbols[0]);i++)
		g_scanner_scope_add_symbol(gs, 0, symbols[i].name, &symbols[i]);

	g_scanner_input_text(gs, expression, strlen(expression));
	t = parse_list(gs, 0);

	if (t) {
		ctx = g_malloc0(sizeof(*ctx));
		ctx->folder = folder;
		ctx->summary = camel_folder_get_summary(folder, ex);
		ctx->message_info = camel_folder_summary_get_message_info_list(ctx->summary);
#ifdef HAVE_IBEX
		ctx->index = ibex_open(CAMEL_MBOX_FOLDER(folder)->index_file_path, FALSE);
		if (!ctx->index) {
			perror("Cannot open index file, body searches will be ignored\n");
		}
#endif
		r = term_eval(ctx, t);

		/* now create a folder summary to return?? */
		if (r
		    && r->type == FSEXP_RES_ARRAY_PTR) {
			d(printf("got result ...\n"));
			for (i=0;i<r->value.ptrarray->len;i++) {
				d(printf("adding match: %s\n", (char *)g_ptr_array_index(r->value.ptrarray, i)));
				matches = g_list_prepend(matches, g_strdup(g_ptr_array_index(r->value.ptrarray, i)));
			}
			filter_sexp_result_free(r);
		}

		if (ctx->index)
			ibex_close(ctx->index);

		gtk_object_unref((GtkObject *)ctx->summary);
		g_free(ctx);
		parse_term_free(t);
	} else {
		printf("Warning, Could not parse expression!\n %s\n", expression);
	}

	g_scanner_destroy(gs);

	return matches;
}
#endif


static void
filter_sexp_class_init (FilterSEXPClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;
	parent_class = gtk_type_class (gtk_object_get_type ());
}

/* 'builtin' functions */
static struct {
	char *name;
	FilterSEXPFunc *func;
	int type;		/* set to 1 if a function can perform shortcut evaluation, or
				   doesn't execute everything, 0 otherwise */
} symbols[] = {
	{ "and", (FilterSEXPFunc *)term_eval_and, 1 },
	{ "or", (FilterSEXPFunc *)term_eval_or, 1 },
	{ "<", (FilterSEXPFunc *)term_eval_lt, 1 },
	{ ">", (FilterSEXPFunc *)term_eval_gt, 1 },
	{ "=", (FilterSEXPFunc *)term_eval_eq, 1 },
	{ "+", (FilterSEXPFunc *)term_eval_plus, 0 },
	{ "-", (FilterSEXPFunc *)term_eval_sub, 0 },
	{ "if", (FilterSEXPFunc *)term_eval_if, 1 },
	{ "begin", (FilterSEXPFunc *)term_eval_begin, 1 },
};

static void
filter_sexp_init (FilterSEXP *s)
{
	int i;

	s->scanner = g_scanner_new(&scanner_config);

	/* load in builtin symbols? */
	for(i=0;i<sizeof(symbols)/sizeof(symbols[0]);i++) {
		if (symbols[i].type == 1) {
			filter_sexp_add_ifunction(s, 0, symbols[i].name, (FilterSEXPIFunc *)symbols[i].func, &symbols[i]);
		} else {
			filter_sexp_add_function(s, 0, symbols[i].name, symbols[i].func, &symbols[i]);
		}
	}
}

guint
filter_sexp_get_type (void)
{
	static guint type = 0;
	
	if (!type) {
		GtkTypeInfo type_info = {
			"FilterSEXP",
			sizeof (FilterSEXP),
			sizeof (FilterSEXPClass),
			(GtkClassInitFunc) filter_sexp_class_init,
			(GtkObjectInitFunc) filter_sexp_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		
		type = gtk_type_unique (gtk_object_get_type (), &type_info);
	}
	
	return type;
}

FilterSEXP *
filter_sexp_new (void)
{
	FilterSEXP *f = FILTER_SEXP ( gtk_type_new (filter_sexp_get_type ()));

	return f;
}

void
filter_sexp_add_function(FilterSEXP *f, int scope, char *name, FilterSEXPFunc *func, void *data)
{
	struct _FilterSEXPSymbol *s;

	g_return_if_fail(FILTER_IS_SEXP(f));
	g_return_if_fail(name != NULL);

	s = g_malloc0(sizeof(*s));
	s->name = g_strdup(name);
	s->f.func = func;
	s->type = FSEXP_TERM_FUNC;
	s->data = data;
	g_scanner_scope_add_symbol(f->scanner, scope, s->name, s);
}

void
filter_sexp_add_ifunction(FilterSEXP *f, int scope, char *name, FilterSEXPIFunc *ifunc, void *data)
{
	struct _FilterSEXPSymbol *s;

	g_return_if_fail(FILTER_IS_SEXP(f));
	g_return_if_fail(name != NULL);

	s = g_malloc0(sizeof(*s));
	s->name = g_strdup(name);
	s->f.ifunc = ifunc;
	s->type = FSEXP_TERM_IFUNC;
	s->data = data;
	g_scanner_scope_add_symbol(f->scanner, scope, s->name, s);
}

void
filter_sexp_add_variable(FilterSEXP *f, int scope, char *name, FilterSEXPTerm *value)
{
	struct _FilterSEXPSymbol *s;

	g_return_if_fail(FILTER_IS_SEXP(f));
	g_return_if_fail(name != NULL);

	s = g_malloc0(sizeof(*s));
	s->name = g_strdup(name);
	s->type = FSEXP_TERM_VAR;
	s->data = value;
	g_scanner_scope_add_symbol(f->scanner, scope, s->name, s);
}

void
filter_sexp_remove_symbol(FilterSEXP *f, int scope, char *name)
{
	int oldscope;
	struct _FilterSEXPSymbol *s;

	g_return_if_fail(FILTER_IS_SEXP(f));
	g_return_if_fail(name != NULL);

	oldscope = g_scanner_set_scope(f->scanner, scope);
	s = g_scanner_lookup_symbol(f->scanner, name);
	g_scanner_scope_remove_symbol(f->scanner, scope, name);
	g_scanner_set_scope(f->scanner, oldscope);
	if (s) {
		g_free(s->name);
		g_free(s);
	}
}

int
filter_sexp_set_scope(FilterSEXP *f, int scope)
{
	g_return_val_if_fail(FILTER_IS_SEXP(f), 0);

	return g_scanner_set_scope(f->scanner, scope);
}

void
filter_sexp_input_text(FilterSEXP *f, char *text, int len)
{
	g_return_if_fail(FILTER_IS_SEXP(f));
	g_return_if_fail(text != NULL);

	g_scanner_input_text(f->scanner, text, len);
}

void
filter_sexp_input_file (FilterSEXP *f, int fd)
{
	g_return_if_fail(FILTER_IS_SEXP(f));

	g_scanner_input_file(f->scanner, fd);
}

/* needs some error return? */
void
filter_sexp_parse(FilterSEXP *f)
{
	g_return_if_fail(FILTER_IS_SEXP(f));

	f->tree = parse_list(f, FALSE);

	if (f->tree)
		parse_dump_term(f->tree, 0);
}

struct _FilterSEXPResult *
filter_sexp_eval(FilterSEXP *f)
{
	g_return_val_if_fail(FILTER_IS_SEXP(f), NULL);
	g_return_val_if_fail(f->tree != NULL, NULL);

	return filter_sexp_term_eval(f, f->tree);
}

#ifdef TESTER
int main(int argc, char **argv)
{
	FilterSEXP *f;
	char *t = "(+ \"foo\" \"\\\"\" \"bar\" \"\\\\ blah \\x \")";
	FilterSEXPResult *r;

	gtk_init(&argc, &argv);

	f = filter_sexp_new();

	filter_sexp_add_variable(f, 0, "test", NULL);

	filter_sexp_input_text(f, t, strlen(t));
	filter_sexp_parse(f);

	if (f->tree) {
		parse_dump_term(f->tree, 0);
	}

	r = filter_sexp_eval(f);
	if (r) {
		eval_dump_result(r, 0);
	} else {
		printf("no result?|\n");
	}

	return 0;
}
#endif
