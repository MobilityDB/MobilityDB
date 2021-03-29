#!/bin/bash

if [ "$#" != "1" ]; then
	echo "Usage: $0 <install-path>"
	exit 1
fi

installpath=$1

cd `dirname $0`

mkdir -p $installpath

xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o $installpath/index.html /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl mobilitydb-manual.xml
dblatex -s texstyle.sty -T native -t pdf -o $installpath/mobilitydb-manual.pdf mobilitydb-manual.xml
dbtoepub -o $installpath/mobilitydb-manual.epub mobilitydb-manual.xml

cp docbook.css $installpath/
cp -r images $installpath/

## doxygen
cd ..
( cat Doxyfile ; echo "OUTPUT_DIRECTORY=$installpath/api" ) | doxygen -

cd doc

############# Spanish Traduction ##############


installpathES="${installpath}/es"

mkdir -p $installpathES

xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o $installpathES/index.html /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl po/es/mobilitydb-manual.xml
dblatex -s texstyle.sty -T native -t pdf -o $installpathES/mobilitydb-manual.pdf po/es/mobilitydb-manual.xml
dbtoepub -o $installpathES/mobilitydb-manual.epub po/es/mobilitydb-manual.xml

cp docbook.css $installpathES/
cp -r images $installpathES/