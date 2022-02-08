# SOIL makefile for linux (based on the AngelScript makefile)
# Type 'make' then 'make install' to complete the installation of the library

# For 'make install' to work, set LOCAL according to your system configuration
LOCAL = /usr/local

LIBNAME = libSOIL
VERSION = 1.07-20071110
MAJOR = 1

AFILE = $(LIBNAME).a

SOFILE_V = $(LIBNAME).so.$(VERSION)
SOFILE_M = $(LIBNAME).so.$(MAJOR)
SOFILE = $(LIBNAME).so

SRCDIR = ../../src
LIBDIR = ../../lib
INCDIR = ../../src
OBJDIR = .

CXX = gcc
CXXFLAGS = -O2 -s -Wall

LINK= gcc
LINKFLAG= -shared -Wl,-soname,$(LIBNAME).so.$(MAJOR)

LN = ln -s
DELETER = rm -f
COPIER = cp

SRCNAMES = \
  image_helper.c \
  stb_image_aug.c  \
  image_DXT.c \
  SOIL.c \


INC = SOIL.h
OBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.c=.o)))
LIBOBJ = $(addprefix $(OBJDIR)/, $(notdir $(SRCNAMES:.c=.lo)))

ALIB = $(LIBDIR)/$(AFILE)

SOLIB = $(LIBDIR)/$(SOFILE_V)

all: $(ALIB) $(SOLIB)

$(ALIB): $(OBJ) $(LIBDIR)
	ar r $(ALIB) $(OBJ)
	ranlib $(ALIB)

$(SOLIB): $(LIBOBJ)
	# create shared library
	$(LINK) $(LINKFLAG) -o $(SOLIB) $(LIBOBJ)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(OBJDIR)/%.lo: $(SRCDIR)/%.c
	$(CXX) $(CXXFLAGS) -fPIC -o $@ -c $<

$(LIBDIR):
	mkdir $(LIBDIR)

clean:
	$(DELETER) $(OBJ) $(ALIB) $(LIBOBJ) $(SOLIB)

install: $(ALIB) $(SOLIB)
	@echo Installing to: $(LOCAL)/lib and $(LOCAL)/include...
	@echo -------------------------------------------------------------------
	$(COPIER) $(ALIB) $(LOCAL)/lib
	$(COPIER) $(SOLIB) $(LOCAL)/lib
	$(COPIER) $(INCDIR)/$(INC) $(LOCAL)/include
	@echo Updating ldconfig cache
	@echo -------------------------------------------------------------------
	# create link to the full versionned shared objet (.so et .so.1) + ldconfig update
	( cd $(LOCAL)/lib && $(LN) $(SOFILE_V) $(SOFILE_M) && $(LN) $(SOFILE_V) $(SOFILE) )
	ldconfig
	@echo SOIL library installed. Enjoy!

uninstall:
	$(DELETER) $(LOCAL)/include/$(INC) $(LOCAL)/lib/$(AFILE) $(LOCAL)/lib/$(SOFILE_V) $(LOCAL)/lib/$(SOFILE_M) $(LOCAL)/lib/$(SOFILE)
	@echo -------------------------------------------------------------------
	@echo SOIL library uninstalled.

.PHONY: all clean install uninstall
