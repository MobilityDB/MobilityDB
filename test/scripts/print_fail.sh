#!/bin/bash

for file in $(find tmptest/log -type f) do
	echo "==========="
	echo "file: $file"
	echo "==========="
	echo
	cat "$file"
	echo
done

for file in $(find tmptest/out -name '*.diff' -type f -not -size 0) do
	echo "==========="
	echo "file: $file"
	echo "==========="
	echo
	cat "$file"
	echo
done
