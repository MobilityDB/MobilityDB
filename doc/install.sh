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

mkdir -p $installpath/workshop

xsltproc --stringparam html.stylesheet "docbook.css" --xinclude -o $installpath/workshop/index.html /usr/share/xml/docbook/stylesheet/docbook-xsl-ns/html/chunk.xsl mobilitydb-workshop.xml
dblatex -s texstyle.sty -T native -t pdf -o $installpath/workshop/workshop.pdf mobilitydb-workshop.xml
dbtoepub -o $installpath/workshop/workshop.epub mobilitydb-workshop.xml

cp docbook.css $installpath/
cp docbook.css $installpath/workshop/

cp -r images $installpath/
cp -r images $installpath/workshop/
cp -r workshopimages $installpath/workshop/


