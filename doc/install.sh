#!/bin/bash
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

if [ "$#" != "1" ]; then
	echo "Usage: $0 <install-path>"
	exit 1
fi

installpath=$1

cd "$(dirname "$0")" || exit

mkdir -p "$installpath"

xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o "$installpath"/index.html /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl mobilitydb-manual.xml
dblatex -s texstyle.sty -T native -t pdf -o "$installpath"/mobilitydb-manual.pdf mobilitydb-manual.xml
dbtoepub -o "$installpath"/mobilitydb-manual.epub mobilitydb-manual.xml

cp docbook.css "$installpath/"
cp -r images "$installpath/"

## doxygen
cd ../doxygen || exit
( cat Doxyfile ; echo "OUTPUT_DIRECTORY=$installpath/api" ) | doxygen -
