CFLAGS=-Wall -Wno-deprecated-declarations -pthread -O2 -rdynamic
CFLAGS+=$(shell pkg-config --cflags-only-I gtkmm-2.4)
LDFLAGS+=$(shell pkg-config --libs gtkmm-2.4 gmodule-2.0)

all: build/demo build/dl

build/demo: demo.cpp build/demo.ui
	g++ -std=c++17 $(CFLAGS) $(LDFLAGS) $< -o $@

build/demo.so: demo.cpp
	g++ -std=c++17 -g $(CFLAGS) -fPIC -shared -export-dynamic $(LDFLAGS) $^ -o $@

build/dl: dl.c build/demo.so build/demo.ui
	gcc $(CFLAGS) -ldl $< -o $@

build/demo.ui: demo.ui
	cp $^ $@
