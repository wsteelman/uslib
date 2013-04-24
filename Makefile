# makefile

CC=g++
CFLAGS=-Wall
LDFLAGS=-pthread -lm -lfftw3 -lGL -lGLU -framework GLUT -framework OpenGL
LIBPATH=-L"/System/Library/Frameworks/OpenGl.framework/Libraries"
DEBUGFLAGS=-DDEBUG -g3
OPTFLAGS=-O5

BUILDDIR=bin
SOURCEDIR=uslib
HEADERDIR=include
OBJDIR=.obj
EXTRASOURCE=
EXTRAINCLUDE=./

SOURCES := $(wildcard $(SOURCEDIR)/*.cpp)
OBJS := $(patsubst $(SOURCEDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))

test: PROGRAM=DisplayDemo
test: APP=apps/$(PROGRAM).cpp
test: exec

class: PROGRAM=class
class: APP=apps/$(PROGRAM).cpp
class: LDFLAGS += -lusb-1.0
class: EXTRAINCLUDE=../462_interface/blocking_api
class: EXTRASOURCE=../462_interface/blocking_api/usapi.c ../462_interface/blocking_api/usapi.h
class: exec

exec: $(OBJS)
	$(CC) $(APP) $(EXTRASOURCE) $(CFLAGS) $(OPTFLAGS) $(OBJS) $(LIBPATH) $(LDFLAGS) -I$(EXTRAINCLUDE) -I$(HEADERDIR) -I$(SOURCEDIR) -I$(OBJDIR) -o $(BUILDDIR)/$(PROGRAM)

$(OBJDIR)/%.o: $(SOURCEDIR)/%.cpp
	$(CC) $(CFLAGS) $(OPTFLAGS) -I$(HEADERDIR) -I$(SOURCEDIR) -I$(EXTRAINCLUDE) -c $< -o $@

clean:
	rm -f $(OBJDIR)/* $(BUILDDIR)/*
