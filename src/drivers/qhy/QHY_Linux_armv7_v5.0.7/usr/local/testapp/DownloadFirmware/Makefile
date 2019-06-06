# Makefile for zlib, derived from Makefile.dj2.
# Modified for mingw32 by C. Spieler, 6/16/98.
# Updated for zlib 1.2.x by Christian Spieler and Cosmin Truta, Mar-2003.
# Last updated: Mar 2012.
# Tested under Cygwin and MinGW.

# Copyright (C) 1995-2003 Jean-loup Gailly.
# For conditions of distribution and use, see copyright notice in zlib.h

# To compile, or to compile and test, type from the top level zlib directory:
#
#   make -fwin32/Makefile.gcc;  make test testdll -fwin32/Makefile.gcc
#
# To use the asm code, type:
#   cp contrib/asm?86/match.S ./match.S
#   make LOC=-DASMV OBJA=match.o -fwin32/Makefile.gcc
#
# To install libz.a, zconf.h and zlib.h in the system directories, type:
#
#   make install -fwin32/Makefile.gcc
#
# BINARY_PATH, INCLUDE_PATH and LIBRARY_PATH must be set.
#
# To install the shared lib, append SHARED_MODE=1 to the make command :
#
#   make install -fwin32/Makefile.gcc SHARED_MODE=1

# Note:
# If the platform is *not* MinGW (e.g. it is Cygwin or UWIN),
# the DLL name should be changed from "zlib1.dll".

EXEC = downfirmware

#
# Set to 1 if shared object needs to be installed
#
SHARED_MODE=0





COMP_INC1 = /usr/include/
COMP_INC2 = /usr/local/include/

COMP_LIB = /usr/local/lib/


CXX = g++


CXXFLAGS = -Wall -Wsign-compare -std=c++11 -I. -I $(COMP_INC1)  -I$(COMP_INC2)

EXTRALIBS = -lqhyccd -lusb-1.0






CP = cp -f

OBJA = downfirmware.o




all: $(EXEC) 

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<


$(EXEC): $(OBJA) 
	$(CXX) -o downfirmware $(OBJA) $(EXTRALIBS)



install:
#	$(CP) $(EXEC)

clean:
	-$(RM) $(EXEC)
	-$(RM) *.o
	-$(RM) *~
	-$(RM) *.orig
