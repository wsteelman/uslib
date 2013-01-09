# Generic GNUMakefile

# Just a snippet to stop executing under other make(1) commands
# that won't understand these lines
ifneq (,)
This makefile requires GNU Make.
endif

CC = g++
CFLAGS = -Wall
LDFLAGS = -pthread -lm -lfftw3 -lGL -lGLU -framework GLUT -framework OpenGL
LIBPATH = -L"/System/Library/Frameworks/OpenGl.framework/Libraries"
DEBUGFLAGS = -DDEBUG -g3
OPTFLAGS = -O5

BUILDDIR = bin
SOURCEDIR = uslib
HEADERDIR = include
OBJDIR = .obj

PROGRAM = test
C_FILES := $(wildcard $(SOURCEDIR)/*.cpp)
OBJS := $(patsubst $(SOURCEDIR)/%.cpp, $(OBJDIR)/%.o, $(C_FILES))

all: $(PROGRAM)

debug: CC += $(DEBUGFLAGS)
debug: OPTFLAGS = 
debug: $(PROGRAM)

$(PROGRAM): .depend $(OBJS)
	$(CC) $(CFLAGS) $(OPTFLAGS) $(OBJS) $(LIBPATH) $(LDFLAGS) -I$(HEADERDIR) -I$(SOURCEDIR) -I$(OBJDIR) -o $(BUILDDIR)/$(PROGRAM)

depend: .depend

.depend: cmd = gcc -I$(HEADERDIR) -I$(SOURCEDIR) -MM -MF depend $(var); cat depend >> .depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
$(OBJDIR)/%.o: $(SOURCEDIR)/%.cpp
	$(CC) $(CFLAGS) $(OPTFLAGS) -I$(HEADERDIR) -I$(SOURCEDIR) -c $< -o $@

%: $(SOURCEDIR)/%.cpp
	$(CC) $(CFLAGS) $(OPTFLAGS) -I$(HEADERDIR) -I$(SOURCEDIR) -o $@ $<

clean:
	rm -f .depend $(OBJDIR)/*.o $(BUILDDIR)/*

.PHONY: clean depend

