# System information (ca peut tjs servir)
platform=$(shell uname -s 2>/dev/null)
platform_lcase=$(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])
arch=$(shell uname -m)

MAKE_DIR= "$(shell pwd)"

# -------------------------- Paramétrage ----------------
# répertoir de sortie / Copie des fichiers
ifndef OUTDIR
	OUTDIR= ../LinuxSublimeSolution/bin
endif

# nom du projet à compiler
PROJECT = SampleProject
# -------------------------- Paramétrage ----------------


INSTALLDIR= /usr/local/lib

# nom de l'executable du core et du projet à compiler
CORE = gobLim
CORE_FOLDER=../Core


# option: target_arch - if set to 32, force compilation for 32 bit architecture
ifeq ($(target_arch), 32)
	arch=i686
endif


# -------------------------- CFLAGS section -------------------------- #
# Flags pour le debug
CFLAGS_DEBUG = -g -D_DEBUG_
CFLAGS_STANDARD = -O3

#ifeq ($(dbg), 1)
	CXXFLAGS = $(CFLAGS_DEBUG)
#else
#	CXXFLAGS = $(CFLAGS_STANDARD)
#endif

# Flags pour la version opengl par defaut 4, exemple : make opengl=3
ifndef opengl
	opengl=4
endif

CXXFLAGS += -DOPENGL_VERSION=$(opengl)
# ------------------------ CFLAGS section end ---------------------- #

# -------------------------- ECHO section -------------------------- #
# option: echo - 0 silent, 1 show commands
ifeq ($(echo), 1)
	CMDECHO :=
else
	CMDECHO := @
endif

# tool to clear current line
#CARRIAGE_RETURN=printf "\r                                 \r"
# use the following instead to output on multiple lines
CARRIAGE_RETURN=printf "\n"

# ------------------------ ECHO section end ---------------------- #

CC=g++
MOVE=mv
COPY=cp
LIBLINK=ldconfig



SNAPSHOT_FILE = ./GobLim.tgz

SO_EXT= .so
SO_V= 1.0

# required installed package :
# libglew-dev
# libassimp-dev
# libxrandr-dev
# gflw3 (from source)
# SOIL (from source)
# GFL SDK
# AntTweakBar

LDFLAGS = -L../Libraries/linux/
LIBS = -lGLEW -x11 -lGL -lSOIL -lassimp -lXrandr -lXxf86vm -lrt -lAntTweakBar -lgfl -l:libglfw.so.3 

#libraries included in goblim
LDFLAGS += $(LIBS)

CXXFLAGS += -I$(CORE_FOLDER)
CXXFLAGS += -I../Libraries/include
CXXFLAGS += -std=c++11

# pour compilation de l'executable de goblim
SRC= $(shell find $(CORE_FOLDER) -name \*.cpp -printf '%p ')
OBJ= $(SRC:.cpp=.o)
EXEC=$(CORE)

# pour compilation de la librairie (exclusion du main de test)
LIBSRC= $(shell find $(CORE_FOLDER) -name \*.cpp ! -path $(CORE_FOLDER)/Test/* -printf '%p ')
LIBOBJ= $(LIBSRC:.cpp=.lo)
LIBEXEC= lib$(CORE)$(SO_EXT).$(SO_V)
LIBLDFLAGS = $(LDFLAGS)
LIBLDFLAGS+= -shared -Wl,-soname,lib$(CORE)$(SO_EXT).$(SO_V)
LIBCXXFLAGS = $(CXXFLAGS)
LIBCXXFLAGS+= -Wall -fPIC


PROJSRC= $(shell find ../$(PROJECT) -name \*.cpp -printf '%p ')
LIBPROJSRC = $(LIBSRC)
# Version Moche : recompilation des sources avec le code
#PROJSRC+= $(LIBSRC)
# project and core source specialy compiled for project
PROJOBJ= $(PROJSRC:.cpp=.po)
LIBPROJOBJ= $(LIBPROJSRC:.cpp=.po)

PROJCXXFLAGS= $(CXXFLAGS)
PROJCXXFLAGS+= -I../$(PROJECT)/
PROJCXXFLAGS+= -I../$(PROJECT)/Sources/
PROJBIN= $(PROJECT)

# headers
HEADERS = $(shell find ../ -name \*.h -printf '%p ')

# name of the top-level Makefile (this file)
MFILE_NAME = $(firstword $(MAKEFILE_LIST))

# Doxygen configuration
DOXYCONF = ./Doxygen_settings
DOCSDIR = ./docs

GOBEXE= $(OUTDIR)/$(EXEC)
GOBLIB= $(OUTDIR)/$(LIBEXEC)
PROJEXE= $(OUTDIR)/$(PROJBIN)

#List shader files for copy
SHADERS= $(shell cd $(CORE_FOLDER) && find -name \*.glsl -printf '%p ')
SHADERS += $(shell cd ../$(PROJECT) && find -name \*.glsl -printf '%p ')

# Par défaut : compilation du projet
all: $(PROJEXE)

# ------------------------- Options de compilation des projets
# Il est possible de compiler le moteur en mode TEST (Rendu d'une scène par défaut en utilisant la configuration de l'engine de base)
goblim: $(GOBEXE)

library: $(GOBLIB)

# install the goblim core lib to $(INSTALLDIR) : may require sudo
install: $(GOBLIB)
	$(COPY) $(GOBLIB) $(INSTALLDIR)
	$(LIBLINK)

update_assets:
	$(COPY) -u -r $(CORE_FOLDER)/Objets $(OUTDIR)
	$(COPY) -u -r ../$(PROJECT)/Objets $(OUTDIR)
	cd $(CORE_FOLDER) && find -name \*.glsl -exec cp -u -r --parents '{}' $(OUTDIR) \;
	cd ../$(PROJECT) && find -name \*.glsl -exec cp -u -r --parents '{}' $(OUTDIR) \;

update_project: clean_project
	make -j 8

update_all: clean
	make -j 8

# Juste pour tester
run: $(PROJEXE)
	cd $(OUTDIR) && ./$(PROJBIN)

# -------------------------------------------------------------
# ---------------------------- Targets -----------------------
$(PROJEXE): $(PROJBIN) $(OUTDIR)
	$(MOVE) $(PROJBIN) $(OUTDIR)
	$(COPY) -u -r $(CORE_FOLDER)/Objets $(OUTDIR)
	$(COPY) -u -r ../$(PROJECT)/Objets $(OUTDIR)
	cd $(CORE_FOLDER) && find -name \*.glsl -exec cp -u -r --parents '{}' $(OUTDIR) \;
	cd ../$(PROJECT) && find -name \*.glsl -exec cp -u -r --parents '{}' $(OUTDIR) \;



$(PROJBIN): $(PROJOBJ) $(LIBPROJOBJ)
	@echo "Project compiled"
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[LINK] $(PROJBIN)\n" && \
	$(CC) -o $(PROJBIN) $(PROJOBJ) $(LIBPROJOBJ) $(LDFLAGS) $(PRECOMPILE)

%.po: %.cpp
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[CPP] $(<F)\n" && \
	$(CC) -o $@ -c $< $(PROJCXXFLAGS) $(PRECOMPILE)


$(GOBLIB): $(OUTDIR) $(LIBEXEC)
	$(MOVE) $(LIBEXEC) $(OUTDIR)


$(GOBEXE): $(OUTDIR) $(EXEC)
	$(MOVE) $(EXEC)	$(OUTDIR)
	$(COPY) -u -r $(CORE_FOLDER)/Objets $(OUTDIR)
	cd $(CORE_FOLDER) && find -name \*.glsl -exec cp -u -r --parents '{}' $(OUTDIR) \;
	@echo "Generated target : $(GOBEXE)"


$(EXEC): $(OBJ)
	@echo
	@echo "Core sources compiled"
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[LINK] $(EXEC)\n" && \
	$(CC) -o $(EXEC) $^ $(LDFLAGS) $(PRECOMPILE)
	

%.o: %.cpp
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[CPP] $(<F)\n" && \
	$(CC) -o $@ -c $< $(CXXFLAGS) $(PRECOMPILE)



$(LIBEXEC): $(LIBOBJ)
	@echo
	@echo "Core sources compiled"
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[LINK] $(LIBEXEC)\n" && \
	$(CC) -o $(LIBEXEC) $^ $(LIBLDFLAGS) $(PRECOMPILE)

%.lo: %.cpp
	$(CMDECHO)$(CARRIAGE_RETURN) && \
	printf "[CPP] $(<F)\n" && \
	$(CC) -o $@ -c $< $(LIBCXXFLAGS) $(PRECOMPILE)

$(OUTDIR):
	@echo
	mkdir $(OUTDIR)


#%.o: %.cpp
#	$(CMDECHO)$(CARRIAGE_RETURN) && \
#	printf "[CPP] $(<F)\n" && \
#	 $(CC) -o $@ -c $< $(CXXFLAGS) $(PRECOMPILE)



.PHONY: clean mrproper

clean_project:
	rm -rf $(PROJOBJ)

clean:
	rm -rf $(OBJ) $(LIBOBJ) $(PROJOBJ) $(LIBPROJOBJ) 

mrproper: clean
	rm -rf $(EXEC) $(LIBEXEC) $(PROJEXE) $(OUTDIR)

# target: snapshot - Make a snapshot of current sourcecode in $(SNAPSHOT_FILE)
# it seems tar option --totals doesn't work
# use $(shell date +%F_%T) to include date and time in filename
snapshot:
	$(CMDECHO)tar czf $(SNAPSHOT_FILE) ./$(MFILE_NAME) $(DOXYCONF) $(SRC) $(HEADERS) && echo "Created $(SNAPSHOT_FILE)"

# target: show - Show platform info and compiling options
show:
	@echo "Platform:        $(platform)"
	@echo "Architecture:    $(arch)"
	@echo "Current dir:     $(CURDIR)"
	@echo "This Makefile:   $(MFILE_NAME)"
	@echo "Snapshot file:   $(SNAPSHOT_FILE)"
	@echo "Verbose:         $(verbose)"
	@echo "Debug:           $(dbg)"
	@echo "CC:              $(CC)"
	@echo "OPENGL_VERSION:	$(opengl)"
	@echo "EXEC:		$(EXEC)"
	@echo
	@echo "LDFLAGS:         $(LDFLAGS)"
	@echo "CXXFLAGS:        $(CXXFLAGS)"
	@echo "SRC:		$(SRC)"
	@echo "HEADERS:		$(HEADERS)"
#	@echo "Suffixes:        $(SUFFIXES)"

# target: docs - Generate Doxygen documentation in $(DOCSDIR);
# target:        to produce refman.pdf, run "make pdf" in $(DOCSDIR)/latex/.
docs: $(DOXYCONF)
	$(CMDECHO)mkdir -p $(DOCSDIR)
	@echo Running Doxygen...
	$(CMDECHO)doxygen $(DOXYCONF) > /dev/null && \
	echo Generated Doxygen documentation in $(DOCSDIR)

# target: docsclean - Remove $(DOCSDIR)
docsclean:
	$(CMDECHO)rm -rf $(DOCSDIR)

# target: help - Display help
help:
	@echo "Type"
	@echo
	@echo "  $$ make"
	@echo
	@echo "to compile with OpenGL$(opengl)"
	@echo
	@echo "  $$ make opengl=X"
	@echo
	@echo "to compile with OpenGLX"
	@echo
	@echo "  $$ make dgb=1"
	@echo
	@echo "to compile with debug options"
	@echo
	@echo "  $$ make show"
	@echo
	@echo "to show Makefile variables"
	@echo
	@echo "  $$ make dgb=1"
	@echo
	@echo "to compile with debug options"
	@echo
	@echo "  $$ make snapshot"
	@echo
	@echo "to create a snapshot of the solution"

