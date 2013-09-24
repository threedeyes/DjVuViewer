#C- This program is free software; you can redistribute it and/or 
#C- modify it under the terms of the GNU General Public License, 
#C- either Version 2 of the License or (at your option) any later
#C- version.  The license should have accompanied the program 
#C- or you may obtain a copy of the license from the Free Software
#C- Foundation at http://www.fsf.org.
#C- 
#C- This program is distributed in the hope that it will be useful,
#C- but WITHOUT ANY WARRANTY; without even the implied warranty of
#C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#C- GNU General Public License for more details.


SHELL = /bin/sh

PACKAGE_NAME = djvulibre
PACKAGE_VERSION = 3.5.25

srcdir = .
top_srcdir = ..
top_builddir = ..

prefix = /boot/common
exec_prefix = ${prefix}
bindir = ${exec_prefix}/bin
datarootdir = ${prefix}/share
datadir = ${datarootdir}
libdir = ${exec_prefix}/lib
mandir = ${datarootdir}/man
includedir = ${prefix}/include

CC = gcc
CXX = g++
RM = /bin/rm -f
AR = /boot/develop/tools/gnupro/bin/ar
LN_S = ln -s
TOUCH = /bin/touch
INSTALL = /bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
LIBTOOL = $(SHELL) $(top_builddir)/libtool

DEFS = -DHAVE_CONFIG_H
OPTS = -DNDEBUG -Wall -O3 -Wno-non-virtual-dtor -mcpu=i586
CXXRPOFLAGS = 
JPEG_LIBS=-ljpeg
JPEG_CFLAGS=
THREAD_LIBS = 
THREAD_CFLAGS =  -DTHREADMODEL=POSIXTHREADS

LIBDJVU_LA = libdjvulibre.la
LIBDJVU_VER = 25:0:4

INCS = -I${top_builddir} -I${srcdir}
FLAGS = ${DEFS} ${INCS} ${OPTS} ${JPEG_CFLAGS} ${THREAD_CFLAGS}
LIBS=  ${JPEG_LIBS} ${THREAD_LIBS} 
CFLAGS = ${FLAGS}   
CXXFLAGS =  ${FLAGS} ${CXXPICFLAGS} ${CXXRPOFLAGS}   
DLLFLAGS = 

all: ${LIBDJVU_LA} 

install: install-lib install-data install-include install-pkgconfig

OBJS =  DjVuGlobal.lo DjVuGlobalMemory.lo GOS.lo GException.lo GContainer.lo \
        GString.lo GThreads.lo GRect.lo atomic.lo GSmartPointer.lo MMX.lo    \
        Arrays.lo GBitmap.lo GPixmap.lo GScaler.lo DjVuPalette.lo	     \
        ByteStream.lo IFFByteStream.lo BSByteStream.lo BSEncodeByteStream.lo \
        ZPCodec.lo JB2Image.lo JB2EncodeCodec.lo IW44Image.lo		     \
        IW44EncodeCodec.lo MMRDecoder.lo JPEGDecoder.lo DataPool.lo GURL.lo  \
        DjVuFileCache.lo DjVuPort.lo GMapAreas.lo DjVuAnno.lo GIFFManager.lo \
        DjVuText.lo DjVuInfo.lo DjVuFile.lo DjVuImage.lo DjVuDocument.lo     \
        DjVmDir.lo DjVmDoc.lo DjVmDir0.lo DjVmNav.lo DjVuNavDir.lo	     \
        DjVuErrorList.lo DjVuDocEditor.lo DjVuToPS.lo DjVuDumpHelper.lo	     \
        DjVuMessageLite.lo DjVuMessage.lo GUnicode.lo UnicodeByteStream.lo   \
        XMLParser.lo XMLTags.lo miniexp.lo ddjvuapi.lo debug.lo

${LIBDJVU_LA} : ${OBJS}
	@echo "[ generating $@ ... ]"
	${LIBTOOL} --mode=link ${CXX} ${CXXFLAGS} -no-undefined \
	    -rpath ${libdir} -version-info ${LIBDJVU_VER} ${DLLFLAGS} \
	    ${OBJS} ${LIBS} -o ${LIBDJVU_LA}

#${LIBDJVU_LA} : libdjvu-rpo
#libdjvu-rpo : ${OBJS}
#	@echo "[ performing library closure ... ]"
#	@echo "int main() { return 0; }" > $@.cpp
#	${LIBTOOL} --mode=compile ${CXX} ${CXXFLAGS} -c $@.cpp
#	${LIBTOOL} --mode=link ${CXX} ${CXXFLAGS} -o $@ $@.lo ${OBJS} ${LIBS}

install-data: FORCE
	${INSTALL} -d ${DESTDIR}${datadir}/djvu
	for n in `cd ${top_srcdir}/share && \
	          find djvu -name '*.xml' -print` ; do \
	  src="${top_srcdir}/share/$$n" ; \
	  dir="`dirname ${datadir}/$$n`" ;\
	  test -d ${DESTDIR}$$dir || ${INSTALL} -d ${DESTDIR}$$dir ; \
	  ${INSTALL_DATA} $$src ${DESTDIR}$$dir || exit ; done

install-lib: ${LIBDJVU_LA} FORCE
	${INSTALL} -d ${DESTDIR}${libdir}
	${LIBTOOL} --mode=install \
	  ${INSTALL_DATA} ${LIBDJVU_LA} ${DESTDIR}${libdir}

install-include: FORCE
	${INSTALL} -d ${DESTDIR}${includedir}/libdjvu
	${INSTALL_DATA} ${srcdir}/ddjvuapi.h ${DESTDIR}${includedir}/libdjvu
	${INSTALL_DATA} ${srcdir}/miniexp.h ${DESTDIR}${includedir}/libdjvu

install-pkgconfig: FORCE
	${INSTALL} -d ${DESTDIR}${libdir}/pkgconfig
	${INSTALL_DATA} ddjvuapi.pc ${DESTDIR}${libdir}/pkgconfig

depend: FORCE
	for n in ${srcdir}/*.cpp ; do \
	  ${CXX} -MM ${CXXFLAGS} $$n | \
	    sed -e 's/^\([^.]*\)\.o:/\1.lo \1.o:/'; \
	done > Makefile.dep

clean: FORCE
	-${LIBTOOL} --mode=clean ${RM} 2>/dev/null *.lo *.la 
	-${RM} 2>/dev/null libdjvu-rpo

distclean: clean 
	-${RM} -r 2>/dev/null Makefile *.rpo ii_files

FORCE:
.PHONY: FORCE
.SUFFIXES: .c .cpp .lo .o

.c.o:
	${CC} ${CFLAGS} -c $<

.cpp.o:
	${CXX} ${CXXFLAGS} -c $<

.c.lo:
	${LIBTOOL} --mode=compile ${CC} ${CFLAGS} -c $<

.cpp.lo:
	${LIBTOOL} --mode=compile ${CXX} ${CXXFLAGS} -c $<

# Dependencies

Makefile.dep: ${srcdir}/Makefile.dep
	cp ${srcdir}/Makefile.dep $@

include Makefile.dep

