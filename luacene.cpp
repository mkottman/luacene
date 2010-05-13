/*
 Copyright (c) 2010 Michal Kottman

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>

#include "CLucene.h"
#include "CLucene/analysis/standard/StandardAnalyzer.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexWriter.h"

using namespace lucene::analysis;
using namespace lucene::analysis::standard;
using namespace lucene::document;
using namespace lucene::index;
using namespace lucene::queryParser;
using namespace lucene::search;

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

static char * toUtf8(const TCHAR * str) {
	int len = wcstombs(NULL, str, 0) + 1;
	char * res = (char*) malloc(len);
	wcstombs(res, str, len);
	return res;
}

static TCHAR * fromUtf8(const char * str) {
	int len = mbstowcs(NULL, str, 0) + 1;
	TCHAR * res = (TCHAR*) malloc(sizeof(TCHAR) * len);
	mbstowcs(res, str, len);
	return res;
}

#define MT "Luacene"

#define LUADEF(type) \
static void push_##type(lua_State *L, type *v) {             \
	type ** x = (type **) lua_newuserdata(L, sizeof(type*)); \
	*x = v;                                                  \
	luaL_getmetatable(L, MT #type);                          \
	lua_setmetatable(L, -2);                                 \
}                                                            \
static type * get_##type(lua_State *L, int idx) {            \
	type ** x = (type **) luaL_checkudata(L, idx, MT #type); \
	return *x;                                               \
}

/*********** Document ************/

static int push_Document(lua_State *L, Document *d) {
	const Document::FieldsType * fields = d->getFields();
	lua_createtable(L, 0, fields->size());
	for (Document::FieldsType::const_iterator i = fields->begin();
			i != fields->end(); i++) {
		Field * f = *i;
		char * name = toUtf8(f->name());
		char * value = toUtf8(f->stringValue());
		lua_pushstring(L, name);
		lua_pushstring(L, value);
		lua_settable(L, -3);
		free(name);
		free(value);
	}
	return 1;
}

static Document * get_Document(lua_State *L, int idx) {
	Document * doc = _CLNEW Document();
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) {
		TCHAR * name = fromUtf8(lua_tostring(L, -2));
		TCHAR * value = fromUtf8(lua_tostring(L, -1));
		Field * f = _CLNEW Field(name, value, Field::STORE_YES | Field::INDEX_TOKENIZED);
		doc->add(*f);
		lua_pop(L, 1);
	}
	return doc;
}

/*********** IndexWriter ************/

LUADEF(IndexWriter)

static int iw_add(lua_State *L) {
	IndexWriter *iw = get_IndexWriter(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);

	Document *doc = get_Document(L, 2);
	iw->addDocument(doc);

	return 0;
}

static int iw_optimize(lua_State *L) {
	IndexWriter *iw = get_IndexWriter(L, 1);
	iw->optimize();
	lua_pushboolean(L, 1);
	return 1;
}

static int iw_flush(lua_State *L) {
	IndexWriter *iw = get_IndexWriter(L, 1);
	iw->flush();
	return 0;
}

static int iw_gc(lua_State *L) {
	IndexWriter *iw = get_IndexWriter(L, 1);
	Analyzer *an = iw->getAnalyzer();
	iw->close();
	_CLDELETE(an);
	_CLDELETE(iw);
	return 0;
}


static luaL_Reg IndexWriter_methods[] = {
	{"addDocument", iw_add},
	{"optimize", iw_optimize},
	{"flush", iw_flush},
	{"__gc", iw_gc},
	{NULL, NULL}
};

/*********** Hits ************/

LUADEF(Hits)

static int hits_index(lua_State *L) {
	Hits *h = get_Hits(L, 1);
	int type = lua_type(L, 2);
	if (type == LUA_TNUMBER) {
		int n = lua_tointeger(L, 2);
		Document * doc = &h->doc(n);
		return push_Document(L, doc);
	} else if (type == LUA_TSTRING) {
		const char * opts[] = {"length"};
		int idx = luaL_checkoption(L, 2, NULL, opts);
		if (idx == 0) {
			lua_pushnumber(L, h->length());
			return 1;
		}
	} else {
		luaL_argerror(L, 2, "expecting string or number");
	}
	return 0;
}

static int hits_gc(lua_State *L) {
	Hits *h = get_Hits(L, 1);
	_CLDELETE(h);
	return 0;
}

static luaL_Reg Hits_methods[] = {
	{"__index", hits_index},
	{"__gc", hits_gc},
	{NULL, NULL}
};

/*********** IndexSearcher ************/

LUADEF(IndexSearcher)

static int is_search(lua_State *L) {
	IndexSearcher* s = get_IndexSearcher(L, 1);
	size_t queryLen;
	const char * query = luaL_checklstring(L, 2, &queryLen);

	TCHAR * tquery = fromUtf8(query);
	StandardAnalyzer analyzer;

	Query* q = QueryParser::parse(tquery, _T("contents"), &analyzer);
	Hits* h = s->search(q);

	_CLDELETE(q);
	free(tquery);

	push_Hits(L, h);

	return 1;
}

static int is_gc(lua_State *L) {
	IndexSearcher *is = get_IndexSearcher(L, 1);
	_CLDELETE(is);
	return 0;
}

static luaL_Reg IndexSearcher_methods[] = {
	{"search", is_search},
	//{"__tostring", is_tostring},
	{"__gc", is_gc},
	{NULL, NULL}
};

/*********** Luacene ************/

static int l_searcher(lua_State *L) {
	const char * path = luaL_checkstring(L, 1);
	try {
		IndexSearcher *searcher = _CLNEW IndexSearcher(path);
		push_IndexSearcher(L, searcher);
		return 1;
	} catch (CLuceneError e) {
		lua_pushnil(L);
		char * what = toUtf8(e.twhat());
		lua_pushstring(L, what);
		free(what);
		return 2;
	}
}

static int l_writer(lua_State *L) {
	const char * path = luaL_checkstring(L, 1);
	try {
		StandardAnalyzer * analyzer = _CLNEW StandardAnalyzer();
		IndexWriter *writer = _CLNEW IndexWriter(path, analyzer, true);
		push_IndexWriter(L, writer);
		return 1;
	} catch (CLuceneError e) {
		lua_pushnil(L);
		char * what = toUtf8(e.twhat());
		lua_pushstring(L, what);
		free(what);
		return 2;
	}
}


static luaL_Reg luacene_functions[] = {
	{"searcher", l_searcher},
	{"writer", l_writer},
	{NULL, NULL}
};

static void createMetatable(lua_State *L, const char *name, luaL_Reg *methods) {
	luaL_newmetatable(L, name);
	luaL_register(L, NULL, methods);
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
}

extern "C" {
LUA_API int luaopen_luacene(lua_State *L) {
	// copy locale from environment
	setlocale(LC_ALL, "");

#define METADEF(name) \
	createMetatable(L, MT #name, name##_methods);

	METADEF(IndexSearcher);
	METADEF(IndexWriter);
	METADEF(Hits);

	luaL_register(L, "luacene", luacene_functions);
	return 1;
}
}
