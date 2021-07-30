#!/bin/bash

while IFS= read -r -d '' file
do
  echo "Processing $file file...";
  xml2pot "${file}" > "$file.pot";
done <  <(find . -name '*.xml' -print0)

exit 0
