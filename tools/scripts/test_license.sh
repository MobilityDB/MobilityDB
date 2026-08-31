#!/usr/bin/env bash

# This test checks that all source files correctly have license headers

DOC_EXCLUDE_LIST="\.png|\.svg|\.po|\.pot|\.pdf|\.sh|\.sty|\.vsdx|\.xsl|\.tx|\.md"

# The first-party trees, and the extensions that carry code. A file holding
# data -- a .csv fixture, a .json schema -- states no licence of its own and
# the repository's licence covers it.
SOURCE_TREES="meos mobilitydb tools"
SOURCE_EXTENSIONS='\.(c|h|cpp|sql|py)$'

mylicensecheck() {
  licensecheck -r -l 30 --tail 0 -i "$1" "$2"
}

DIR=$(git rev-parse --show-toplevel)

# WITHOUT THE READER THERE IS NO READING. licensecheck absent leaves every file
# unexamined while the run still prints no finding and exits 0, which is the
# answer a clean tree gives — the same silence this check was rewritten to stop
# giving. Say what is missing instead.
if ! command -v licensecheck > /dev/null; then
  echo " *** licensecheck is not installed, so the licence of no file was read"
  echo " *** install it (apt-get install licensecheck) and run this again"
  exit 1
fi

pushd "${DIR}" > /dev/null || exit
# ASK GIT WHICH FILES THE REPOSITORY CARRIES, NEVER THE DIRECTORY. The four
# names this read carried -- include, src, sql, test -- are not directories of
# this tree, whose sources live under meos/ and mobilitydb/, so it matched
# nothing: the check passed on empty input while sixty files carried no banner,
# which is the whole reason it exists. Reading git also keeps out what a build
# leaves behind, such as a __pycache__ .pyc or a staged install prefix.
sources=$(git ls-files ${SOURCE_TREES} |
  grep -E "${SOURCE_EXTENSIONS}" |
  grep -vxF -f tools/scripts/license_other_projects.txt)
read_count=$(printf '%s\n' "${sources}" | grep -c .)
missing=$(printf '%s\n' "${sources}" | xargs -r licensecheck -l 30 --tail 0 |
  grep "No copyright\|UNKNOWN")
missing1=$(mylicensecheck ${DOC_EXCLUDE_LIST} doc | grep "No copyright")
missing2=$(find doc -type f -name "*.xml" -exec grep -H -i -c 'Creative Commons' {} \; | grep :0$ | cut -d':' -f1)
popd > /dev/null || exit

error=0
# THE CHECK STATES ITS OWN DENOMINATOR. A run that reads nothing prints no
# finding and exits 0, which is what a run over a clean tree prints too, and
# that is how a read of four directories this tree does not have went unnoticed.
# Saying how many files were read tells the two apart at a glance.
echo "license: read ${read_count} source file(s) under ${SOURCE_TREES// /, }"
if [[ ${read_count} -eq 0 ]]; then
  echo " *** The licence check read no file at all, so it asserts nothing"
  error=1
fi
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

