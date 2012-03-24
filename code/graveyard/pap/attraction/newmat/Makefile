#############################################################################
# auto-generated Makefile for building: libnewmat.a
# Template: static library
#############################################################################

####### Compiler, tools and options

CC       = gcc
CXX      = g++
LEX      = flex
YACC     = yacc
CFLAGS   = -pipe -Wall -W -O2 -DQT_NO_DEBUG
CXXFLAGS = -pipe -Wall -W -O2 -DQT_NO_DEBUG
#LCFLAGS   = -pipe -Wall -W -O2 -march=athlon-xp  -DQT_NO_DEBUG
#CXXFLAGS = -pipe -Wall -W -O2 -march=athlon-xp  -DQT_NO_DEBUG
LEXFLAGS = 
YACCFLAGS= -d
INCPATH  =
AR       = ar cqs
RANLIB   = 
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### Output directory

OBJECTS_DIR = ./

####### Files

HEADERS = boolean.h \
		include.h \
		newmat.h \
		newmatio.h \
		newmatrc.h \
		precisio.h \
		controlw.h \
		myexcept.h \
		newmatap.h \
		newmatnl.h \
		newmatrm.h \
		solution.h
SOURCES = bandmat.cpp \
		hholder.cpp \
		newmat2.cpp \
		newmat7.cpp \
		newmatrm.cpp \
		cholesky.cpp \
		jacobi.cpp \
		newmat3.cpp \
		newmat8.cpp \
		solution.cpp \
		evalue.cpp \
		myexcept.cpp \
		newmat4.cpp \
		newmat9.cpp \
		sort.cpp \
		newfft.cpp \
		newmat5.cpp \
		newmatex.cpp \
		submat.cpp \
		fft.cpp \
		newmat1.cpp \
		newmat6.cpp \
		newmatnl.cpp \
		svd.cpp
OBJECTS = bandmat.o \
		hholder.o \
		newmat2.o \
		newmat7.o \
		newmatrm.o \
		cholesky.o \
		jacobi.o \
		newmat3.o \
		newmat8.o \
		solution.o \
		evalue.o \
		myexcept.o \
		newmat4.o \
		newmat9.o \
		sort.o \
		newfft.o \
		newmat5.o \
		newmatex.o \
		submat.o \
		fft.o \
		newmat1.o \
		newmat6.o \
		newmatnl.o \
		svd.o
DIST	   = project.pro
DESTDIR  = 
TARGET   = libnewmat.a

first: all
####### Implicit rules

.SUFFIXES: .c .o .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: $(TARGET) 

staticlib: $(TARGET)

$(TARGET):  $(OBJECTS)
	-$(DEL_FILE) $(TARGET)
	$(AR) $(TARGET) $(OBJECTS)


dist: 
	@mkdir -p .tmp/newmat && $(COPY_FILE) --parents $(SOURCES) $(HEADERS) $(DIST) .tmp/newmat/ && ( cd `dirname .tmp/newmat` && $(TAR) newmat.tar newmat && $(GZIP) newmat.tar ) && $(MOVE) `dirname .tmp/newmat`/newmat.tar.gz . && $(DEL_FILE) -r .tmp/newmat

yaccclean:
lexclean:
clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) $(TARGET)


FORCE:

####### Compile

bandmat.o: bandmat.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

hholder.o: hholder.cpp include.h \
		newmatap.h \
		newmat.h \
		boolean.h \
		myexcept.h

newmat2.o: newmat2.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmat7.o: newmat7.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmatrm.o: newmatrm.cpp newmat.h \
		newmatrm.h \
		include.h \
		boolean.h \
		myexcept.h

cholesky.o: cholesky.cpp include.h \
		newmat.h \
		boolean.h \
		myexcept.h

jacobi.o: jacobi.cpp include.h \
		newmatap.h \
		precisio.h \
		newmatrm.h \
		newmat.h \
		boolean.h \
		myexcept.h

newmat3.o: newmat3.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmat8.o: newmat8.cpp include.h \
		newmat.h \
		newmatrc.h \
		precisio.h \
		boolean.h \
		myexcept.h \
		controlw.h

solution.o: solution.cpp include.h \
		boolean.h \
		myexcept.h \
		solution.h

evalue.o: evalue.cpp include.h \
		newmatap.h \
		newmatrm.h \
		precisio.h \
		newmat.h \
		boolean.h \
		myexcept.h

myexcept.o: myexcept.cpp include.h \
		boolean.h \
		myexcept.h

newmat4.o: newmat4.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmat9.o: newmat9.cpp include.h \
		newmat.h \
		newmatio.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

sort.o: sort.cpp include.h \
		newmatap.h \
		newmat.h \
		boolean.h \
		myexcept.h

newfft.o: newfft.cpp newmatap.h \
		newmat.h \
		include.h \
		boolean.h \
		myexcept.h

newmat5.o: newmat5.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmatex.o: newmatex.cpp include.h \
		newmat.h \
		boolean.h \
		myexcept.h

submat.o: submat.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

fft.o: fft.cpp include.h \
		newmatap.h \
		newmat.h \
		boolean.h \
		myexcept.h

newmat1.o: newmat1.cpp newmat.h \
		include.h \
		boolean.h \
		myexcept.h

newmat6.o: newmat6.cpp include.h \
		newmat.h \
		newmatrc.h \
		boolean.h \
		myexcept.h \
		controlw.h

newmatnl.o: newmatnl.cpp newmatap.h \
		newmatnl.h \
		newmat.h \
		include.h \
		boolean.h \
		myexcept.h

svd.o: svd.cpp include.h \
		newmatap.h \
		newmatrm.h \
		precisio.h \
		newmat.h \
		boolean.h \
		myexcept.h

####### Install

install:  

uninstall:  

