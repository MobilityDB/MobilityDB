#!/bin/bash

if [ "$#" != "1" ]; then 
	echo "Usage: $0 <install-path>"
	exit 1
fi

installpath=$1

cd `dirname $0`

mkdir -p $installpath

xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o $installpath/index.html /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl mobilitydb-manual.xml
dblatex -s texstyle.sty -T native -t pdf -o $installpath/mobilitydb.pdf mobilitydb-manual.xml
dbtoepub -o $installpath/mobilitydb.epub mobilitydb-manual.xml

cp docbook.css $installpath/
cp -r images $installpath/

## doxygen

cd ..
( cat Doxyfile ; echo "OUTPUT_DIRECTORY=$installpath/api" ) | doxygen -
