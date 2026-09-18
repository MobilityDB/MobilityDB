#!/bin/bash
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

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
