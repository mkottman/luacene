CFLAGS=-I/usr/include/lua5.1 -fPIC -shared
LIBS=-lclucene-core

luacene.so: luacene.cpp
	gcc -o luacene.so $(CFLAGS) $(LIBS) luacene.cpp
