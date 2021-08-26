#/**********************************************************************
# *
# * PostGIS - Spatial Types for PostgreSQL
# * http://postgis.net
# *
# * PostGIS is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 2 of the License, or
# * (at your option) any later version.
# *
# * PostGIS is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with PostGIS.  If not, see <http://www.gnu.org/licenses/>.
# *
# **********************************************************************
# *
# * Copyright 2008 Mark Cave-Ayland
# *
# **********************************************************************/

CC = gcc
RYU_INCLUDE = -I../deps/ryu/..
CPPFLAGS =  $(RYU_INCLUDE)  -I/usr/include   -I/usr/include/libxml2  -I/usr/include/json-c  -DNDEBUG 
CFLAGS = -std=gnu99 -g -O0 -fno-math-errno -fno-signed-zeros  -fPIC -DPIC
LDFLAGS =  -lm -L/usr/lib/x86_64-linux-gnu -lgeos_c -lproj -ljson-c
top_builddir = ..
prefix = /usr/local/pgsql/13
exec_prefix = ${prefix}
libdir = ${exec_prefix}/lib
includedir = ${prefix}/include
SHELL = /bin/bash
INSTALL = /usr/bin/install -c
LIBTOOL = $(SHELL) $(top_builddir)/libtool

SOVER = 3.1

IFACE_CURRENT = 0
IFACE_AGE = 0
IFACE_REV = 0

VERSION_INFO = $(IFACE_CURRENT):$(IFACE_REV):$(IFACE_AGE)


YACC=/usr/bin/bison
LEX=flex

# Standalone LWGEOM objects
SA_OBJS = \
	stringbuffer.o \
	bytebuffer.o \
	measures.o \
	measures3d.o \
	ptarray.o \
	lookup3.o \
	lwgeom_api.o \
	lwgeom.o \
	lwpoint.o \
	lwline.o \
	lwpoly.o \
	lwtriangle.o \
	lwmpoint.o \
	lwmline.o \
	lwmpoly.o \
	lwboundingcircle.o \
	lwcollection.o \
	lwcircstring.o \
	lwcompound.o \
	lwcurvepoly.o \
	lwmcurve.o \
	lwmsurface.o \
	lwpsurface.o \
	lwtin.o \
	lwout_wkb.o \
	lwin_geojson.o \
	lwin_wkb.o \
	lwin_twkb.o \
	lwiterator.o \
	lwgeom_median.o \
	lwout_wkt.o \
	lwout_twkb.o \
	lwin_wkt_parse.o \
	lwin_wkt_lex.o \
	lwin_wkt.o \
	lwin_encoded_polyline.o \
	lwutil.o \
	lwhomogenize.o \
	lwalgorithm.o \
	lwstroke.o \
	lwlinearreferencing.o \
	lwprint.o \
	gbox.o \
	gserialized.o \
	gserialized1.o \
	gserialized2.o \
	lwgeodetic.o \
	lwgeodetic_tree.o \
	lwrandom.o \
	lwtree.o \
	lwout_gml.o \
	lwout_kml.o \
	lwout_geojson.o \
	lwout_svg.o \
	lwout_x3d.o \
	lwout_encoded_polyline.o \
	lwgeom_debug.o \
	lwgeom_geos.o \
	lwgeom_geos_clean.o \
	lwgeom_geos_cluster.o \
	lwgeom_geos_node.o \
	lwgeom_geos_split.o \
	lwgeom_topo.o \
	lwgeom_transform.o \
	lwgeom_wrapx.o \
	lwunionfind.o \
	effectivearea.o \
	lwchaikins.o \
	lwmval.o \
	lwkmeans.o \
	varint.o

NM_OBJS = \
	lwspheroid.o

ifeq (,sfcgal)
CFLAGS += 
LDFLAGS += 
SA_OBJS += lwgeom_sfcgal.o
endif

LDFLAGS += -no-undefined

LT_SA_OBJS = $(SA_OBJS:.o=.lo)
LT_NM_OBJS = $(NM_OBJS:.o=.lo)
LT_OBJS = $(LT_SA_OBJS) $(LT_NM_OBJS)

SA_HEADERS = \
	bytebuffer.h \
	effectivearea.h \
	liblwgeom.h \
	liblwgeom_internal.h \
	lwgeodetic.h \
	lwgeodetic_tree.h \
	liblwgeom_topo.h \
	liblwgeom_topo_internal.h \
	lwgeom_log.h \
	lwgeom_geos.h \
	lwgeom_log.h \
	lwgeom_sfcgal.h \
	lwinline.h \
	lwin_wkt.h \
	lwin_wkt_parse.h \
	lwout_twkb.h \
	lwrandom.h \
	lwtree.h \
	measures3d.h \
	measures.h \
	stringbuffer.h \
	varint.h

all: liblwgeom.la

install:

uninstall:

# Make all objects depend upon postgis_config.h and postgis_revision.h
$(LT_OBJS): ../postgis_config.h ../postgis_revision.h $(SA_HEADERS)

../postgis_revision.h:
	$(MAKE) -C .. postgis_revision.h


RYU_LIBPATH = ../deps/ryu/libryu.la
$(RYU_LIBPATH): ../deps/ryu/d2s.c
	$(MAKE) -C ../deps/ryu libryu.la

liblwgeom.la: $(LT_OBJS) $(RYU_LIBPATH)
	$(LIBTOOL) --tag=CC --mode=link $(CC) -rpath $(libdir) $(LT_OBJS) $(RYU_LIBPATH)\
             -release $(SOVER) -version-info $(VERSION_INFO) $(LDFLAGS) -static -o $@

maintainer-clean: clean
	rm -f lwin_wkt_lex.c
	rm -f lwin_wkt_parse.h
	rm -f lwin_wkt_parse.c

clean:
	$(MAKE) -C cunit clean
	$(MAKE) -C ../deps/ryu clean
	rm -f $(LT_OBJS) $(SA_OBJS) $(NM_OBJS)
	rm -f liblwgeom.la
	rm -rf .libs

distclean: clean
	$(MAKE) -C cunit distclean
	rm -f liblwgeom.h Makefile

check: check-unit

check-regress:

.PHONY: check-unit
check-unit: liblwgeom.la
	$(MAKE) -C cunit check

# Command to build each of the .lo files
$(LT_SA_OBJS): %.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

$(LT_NM_OBJS): %.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

lwin_wkt_parse.c lwin_wkt_parse.h: lwin_wkt_parse.y
	@test -n "$(YACC)" || { \
			echo "No yacc found, cannot build parser" >&2; \
			false; \
	}
	$(YACC) -p wkt_yy -o'lwin_wkt_parse.c' -d lwin_wkt_parse.y

lwin_wkt_lex.c: lwin_wkt_lex.l
	@test -n "$(LEX)" || { \
			echo "No lex found, cannot build parser" >&2; \
			false; \
	}
	$(LEX) -i lwin_wkt_lex.l

liblwgeom.h: liblwgeom.h.in
	cd .. && ./config.status
