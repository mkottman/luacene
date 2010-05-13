CFLAGS=-I/usr/include/lua5.1 -fPIC -shared -g
LIBS=-lclucene-core

luacene.so: luacene.cpp
	gcc -o luacene.so $(CFLAGS) $(LIBS) luacene.cpp
