#!/bin/bash

while IFS= read -r -d '' file
do
	echo "==========="
	echo "file: $file"
	echo "==========="
	echo
	cat "$file"
	echo
done <  <(find tmptest/log -type f -print0)

while IFS= read -r -d '' file
do
	echo "==========="
	echo "file: $file"
	echo "==========="
	echo
	cat "$file"
	echo
done <  <(find find tmptest/out -name '*.diff' -type f -not -size 0 -print0)
