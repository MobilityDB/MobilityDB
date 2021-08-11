#!/usr/bin/env bash

# This test checks that all source and documentation files correctly have license headers

SRC_EXCLUDE_LIST="control.in|\.control|\.out|\.xz|\.cmake|\.md|\.txt|\.sh"
DOC_EXCLUDE_LIST="\.png|\.svg|\.po|\.pot|\.pdf|\.sh|\.sty|\.vsdx|\.tx"

mylicensecheck() {
  licensecheck -r -l 30 --tail 0 -i "$1" "$2"
}

DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit

missing_src=""
while IFS= read -r ROW; do
  if [[ "$ROW" == *"No copyright"* || "$ROW" == *"UNKNOWN"* ]]; then
    missing_src+="$ROW"$'\n'
  fi
done < <(! { mylicensecheck "${SRC_EXCLUDE_LIST}" include ; mylicensecheck "${SRC_EXCLUDE_LIST}" src ; \
  mylicensecheck "${SRC_EXCLUDE_LIST}" sql ; mylicensecheck "${SRC_EXCLUDE_LIST}" test ; } )

missing_doc1=""
while IFS= read -a ROW; do
  if [[ "$ROW" == *"No copyright"* ]]; then
    missing_doc1+="$ROW"$'\n'
  fi
done < <(! { mylicensecheck "${DOC_EXCLUDE_LIST}" doc ; } )

# TODO
#missing_doc2=$(grep --files-without-match 'Creative Commons' doc/*.xml)
popd > /dev/null || exit

error=0
if [[ $missing_src ]]; then
  echo " ****************************************************"
  echo " *** Found source files without valid license headers"
  echo " ****************************************************"
  echo "$missing_src"
  error=1
fi

if [[ $missing_doc1 ]]; then
  echo " ****************************************************"
  echo " *** Found documentation files without copyright"
  echo " ****************************************************"
  echo "$missing_doc1"
  error=1
fi

if [[ $missing_doc2 ]]; then
  echo " ****************************************************"
  echo " *** Found documentation files without valid license headers"
  echo " ****************************************************"
  echo "$missing_doc2"
  error=1
fi
exit $error

