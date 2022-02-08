
INSTALLDIR = /usr/local

LIBFOLDER= ./lib
INCFOLDER= ./include


all: install

install:
	@echo "====== Installing gfl to $(INSTALLDIR) ========== " 
	cp $(LIBFOLDER)/* $(INSTALLDIR)/lib
	cp $(INCFOLDER)/* $(INSTALLDIR)/include
	ldconfig
	
	
