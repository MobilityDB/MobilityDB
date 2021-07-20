#!/usr/bin/env bash

# This test checks that all source files correctly have license headers

EXCLUDE_LIST="control.in|\.out|\.xz|\.cmake|\.md|\.txt"
DOC_EXCLUDE_LIST="\.png|\.svg|\.po|\.pot|\.pdf|\.sh|\.sty|\.vsdx|\.tx|po/es"

mylicensecheck() {
    licensecheck -r --copyright -l 30 --tail 0 -i "$1" "$2"
}

DIR=$(git rev-parse --show-toplevel)

pushd "${DIR}" > /dev/null || exit
missing=$(! { mylicensecheck ${EXCLUDE_LIST} src & mylicensecheck ${EXCLUDE_LIST}  point & mylicensecheck ${EXCLUDE_LIST} npoint & mylicensecheck ${EXCLUDE_LIST} include;}  | grep "No copyright\|UNKNOWN")
missing1=$(mylicensecheck ${DOC_EXCLUDE_LIST} doc  | grep "No copyright")
#missing2=$(grep --files-without-match 'Creative Commons' doc/*.xml)
popd > /dev/null || exit

error=0
if [[ $missing ]]; then
  echo " ****************************************************"
  echo " *** Found source files without valid license headers"
  echo " ****************************************************"
  echo "$missing"
  error=1
fi

if [[ $missing1 ]]; then
  echo " ****************************************************"
  echo " *** Found documentation files without copyright"
  echo " ****************************************************"
  echo "$missing1"
  error=1
fi

if [[ $missing2 ]]; then
  echo " ****************************************************"
  echo " *** Found documentation files without valid license headers"
  echo " ****************************************************"
  echo "$missing2"
  error=1
fi
exit $error

