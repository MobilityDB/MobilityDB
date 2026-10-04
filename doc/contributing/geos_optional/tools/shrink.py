#!/usr/bin/env python3
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

"""Shrink a declining union row to a minimal declining subset.

One member at a time is dropped while the head still declines, calling the
dumper the corpus was answered with.

usage: shrink.py <dumper> < rows
"""
import subprocess, sys

dumper = sys.argv[1]

def head_declines(members):
    """Return True where the dumper answers this array with DECLINED."""
    if len(members) < 2:
        return False
    out = subprocess.run([dumper], input=";".join(members) + "\n",
                         capture_output=True, text=True).stdout
    return out.startswith("DECLINED")

for line in sys.stdin:
    ms = line.strip().split(";")
    if not ms or not head_declines(ms):
        continue
    changed = True
    while changed:
        changed = False
        for i in range(len(ms)):
            trial = ms[:i] + ms[i + 1:]
            if head_declines(trial):
                ms, changed = trial, True
                break
    print(";".join(ms))
