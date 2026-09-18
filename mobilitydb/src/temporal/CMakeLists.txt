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

add_library(pg_temporal OBJECT
  doublen.c
  geo_constructors.c
  index_sortsupport.c
  meos_catalog.c
  set.c
  set_gin.c
  set_aggfuncs.c
  set_ops.c
  skiplist.c
  span.c
  span_aggfuncs.c
  span_analyze.c
  span_gist.c
  span_spgist.c
  span_ops.c
  span_selfuncs.c
  spanset.c
  spanset_ops.c
  tbool_ops.c
  tbox.c
  temporal.c
  temporal_aggfuncs.c
  temporal_analytics.c
  temporal_analyze.c
  temporal_boxops.c
  temporal_compops.c
  temporal_index.c
  temporal_posops.c
  temporal_selfuncs.c
  temporal_supportfn.c
  temporal_tile.c
  temporal_waggfuncs.c
  tnumber_distance.c
  tnumber_gist.c
  tnumber_mathfuncs.c
  tnumber_spgist.c
  ttext_funcs.c
  type_in.c
  type_out.c
  type_util.c
)
