Luacene
=======

Luacene is a Lua binding for the C++ implementation of Lucene - [CLucene](http://clucene.sourceforge.net/).

It is not meant to be a complete binding (better use something like SWIG
for that), but just a handful of classes and functions that are most needed
(for me at least :).

It tries to do thing Lua way - i.e. a Document with Fields is just a Lua table
with keys, Hits instance has custom __index etc.

Luacene was written for the [2\_3\_2 git master HEAD](http://clucene.sourceforge.net/download.shtml#2_3_2)
version of CLucene.

Luacene uses the ANSI `mbstowcs` and `wcstombs` functions for Lua &lt;-&gt; Unicode conversion.
They depend on the current locale, specifically on the LC\_CTYPE settings, i.e. for Utf8,
you should check the output of the `locale` command (I have: LC\_CTYPE="en_US.UTF-8"),
or modify it through [os.setlocale](http://www.lua.org/manual/5.1/manual.html#pdf-os.setlocale).

Overview
--------

Library is loaded using require:

	require 'luacene'

The luacene table has two functions:

- luacene.writer(path) - returns an IndexWriter for the specified path.
- luacene.searcher(path) - returns an IndexSearcher for the specified path.

`RAMDirectory` is not supported (yet). Both use the `StandardAnalyzer` tokenizer.

IndexWriter has these functions:

- IndexWriter:addDocument(doc)

	Adds a document `doc` to the index. `doc` is a Lua table with string keys. These keys
	are interpreted as `Field` names. Values can be:
	- string - the text is stored as field value as `Field::STORE_YES | Field::INDEX_TOKENIZED`
		(a sensible default)
	- table - must be of this structure { value, store, index, termvector }
		- value - field text
		- store - "yes", "no", "compress"
		- index - "no", "tokenized", "untokenized", "nonorms"
		- termvector - "no", "yes", "positions", "offsets", "positions+offsets"

- IndexWriter:optimize()

	Optimizes the index. Returns `true` when everything went OK, or `nil, error` in case of
	error.

- IndexWriter:flush()

	Flushes the index. Returns `true` when everything went OK, or `nil, error` in case of
	error.

IndexSearcher has one method:

- IndexSearcher:search(query)

	Searches the index for `query`. The query is a string, which is parsed by `QueryParser`.
	Returns an instance of `Hits`. Uses "contents" as default field name.

Hits is the search result set. It has overriden __index, so that access to documents is
straight-forward.

- Hits.lenght - the number of search results

- Hits[i] - retrieves the `i`-th document. `i` is one based, like tables in Lua (1 &lt;= `i`
&lt;= Hits.lenght). Documents are returned as Lua tables, with keys being `Field` names and
values being their string content, which is encoded using current locale.

License
-------

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
