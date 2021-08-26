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
# * Copyright 2019 Raúl Marín
# *
# **********************************************************************/

CC=gcc
CFLAGS = -I..  -I/usr/include   -I/usr/include/libxml2  -I/usr/include/json-c  -DNDEBUG  -std=gnu99 -g -O0 -fno-math-errno -fno-signed-zeros  -fPIC -DPIC -DRYU_NO_TRAILING_ZEROS
top_builddir = ../..
SHELL = /bin/bash
INSTALL = /usr/bin/install -c
LIBTOOL = $(SHELL) $(top_builddir)/libtool

RYU_OBJS = d2s.o
LT_RYU_OBJS =  $(RYU_OBJS:.o=.lo)

all: libryu.la

libryu.la: $(LT_RYU_OBJS)
	$(LIBTOOL) --tag=CC --mode=link $(CC) $(LT_RYU_OBJS) -static -o $@


$(LT_RYU_OBJS): %.lo: %.c
	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f libryu.la $(RYU_OBJS) $(LT_RYU_OBJS)
	rm -rf .libs

distclean: clean
	rm -f Makefile


install:

uninstall:

check:

.PHONY: clean distclean install uninstall check
