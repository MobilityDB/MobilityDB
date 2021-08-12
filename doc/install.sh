#!/bin/bash

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
