#!/bin/bash

if [ "$#" != "1" ]; then
  echo "Usage: $0 <language_code>"
  exit 1
fi

if [[ ! -d "$1/" ]]
then
  echo "There is no language subdirectory $1."
  exit 1
fi

if [[ -d "$1_new/" ]]
then
  echo "There is already a new language subdirectory $1_new."
  exit 1
fi

echo "Creating directory $1_new ...";
mkdir "$1_new"
echo "Copying files required to produce the documentation into directory $1_new ...";
cp docbook.css texstyle.sty "$1_new"

while IFS= read -r -d '' file
do
  echo "Processing $file file...";
  filename=$(basename "$file" .po)
  po2xml "${filename}.xml" "$1/$filename.po" | \
    sed 's/images\//..\/images\//g' | \
    tidy -quiet --show-warnings no --preserve-entities yes -w -xml -indent > "$1_new/${filename}.xml";
done <  <(find "$1" -maxdepth 1 -name '*.po' -print0)

exit 0
