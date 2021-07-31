#!/bin/bash

# Validate the manual and exit if it is not valid
echo "Validating the manual...";
xmllint --valid mobilitydb-manual.xml > /dev/null
retVal=$?
if [ $retVal -ne 0 ]; then
  echo "The manual is not valid. Please correct it."
  exit $retVal
else
  echo "The manual is valid."
fi

while IFS= read -r -d '' file
do
  echo "Processing $file file...";
  filename=$(basename "$file" .xml)
  xml2pot "${file}" > "$filename.pot";
done <  <(find . -maxdepth 1 -name '*.xml' -print0)

exit 0
