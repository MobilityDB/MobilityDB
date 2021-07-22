/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/*
 * temporal_spgist.sql
 * Quad-tree SP-GiST index for temporal types
 */

CREATE FUNCTION sptemporal_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE FUNCTION tbox_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_spgist_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_spgist_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_spgist_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_spgist_leaf_consistent(internal, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sptnumber_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tbool_ops
  DEFAULT FOR TYPE tbool USING spgist AS
  -- overlaps
  OPERATOR  3    && (tbool, period),
  OPERATOR  3    && (tbool, tbool),
    -- same
  OPERATOR  6    ~= (tbool, period),
  OPERATOR  6    ~= (tbool, tbool),
  -- contains
  OPERATOR  7    @> (tbool, period),
  OPERATOR  7    @> (tbool, tbool),
  -- contained by
  OPERATOR  8    <@ (tbool, period),
  OPERATOR  8    <@ (tbool, tbool),
  -- adjacent
  OPERATOR  17    -|- (tbool, period),
  OPERATOR  17    -|- (tbool, tbool),
  -- overlaps or before
  OPERATOR  28    &<# (tbool, period),
  OPERATOR  28    &<# (tbool, tbool),
  -- strictly before
  OPERATOR  29    <<# (tbool, period),
  OPERATOR  29    <<# (tbool, tbool),
  -- strictly after
  OPERATOR  30    #>> (tbool, period),
  OPERATOR  30    #>> (tbool, tbool),
  -- overlaps or after
  OPERATOR  31    #&> (tbool, period),
  OPERATOR  31    #&> (tbool, tbool),
  -- functions
  FUNCTION  1  spperiod_gist_config(internal, internal),
  FUNCTION  2  spperiod_gist_choose(internal, internal),
  FUNCTION  3  spperiod_gist_picksplit(internal, internal),
  FUNCTION  4  spperiod_gist_inner_consistent(internal, internal),
  FUNCTION  5  spperiod_gist_leaf_consistent(internal, internal),
  FUNCTION  6  sptemporal_gist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tbox_spgist_ops
  DEFAULT FOR TYPE tbox USING spgist AS
  -- strictly left
  OPERATOR  1    << (tbox, tbox),
  OPERATOR  1    << (tbox, tint),
  OPERATOR  1    << (tbox, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tbox, tbox),
  OPERATOR  2    &< (tbox, tint),
  OPERATOR  2    &< (tbox, tfloat),
  -- overlaps
  OPERATOR  3    && (tbox, tbox),
  OPERATOR  3    && (tbox, tint),
  OPERATOR  3    && (tbox, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tbox, tbox),
  OPERATOR  4    &> (tbox, tint),
  OPERATOR  4    &> (tbox, tfloat),
  -- strictly right
  OPERATOR  5    >> (tbox, tbox),
  OPERATOR  5    >> (tbox, tint),
  OPERATOR  5    >> (tbox, tfloat),
    -- same
  OPERATOR  6    ~= (tbox, tbox),
  OPERATOR  6    ~= (tbox, tint),
  OPERATOR  6    ~= (tbox, tfloat),
  -- contains
  OPERATOR  7    @> (tbox, tbox),
  OPERATOR  7    @> (tbox, tint),
  OPERATOR  7    @> (tbox, tfloat),
  -- contained by
  OPERATOR  8    <@ (tbox, tbox),
  OPERATOR  8    <@ (tbox, tint),
  OPERATOR  8    <@ (tbox, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tbox, tbox),
  OPERATOR  17    -|- (tbox, tint),
  OPERATOR  17    -|- (tbox, tfloat),
  -- overlaps or before
  OPERATOR  28    &<# (tbox, tbox),
  OPERATOR  28    &<# (tbox, tint),
  OPERATOR  28    &<# (tbox, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tbox, tbox),
  OPERATOR  29    <<# (tbox, tint),
  OPERATOR  29    <<# (tbox, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tbox, tbox),
  OPERATOR  30    #>> (tbox, tint),
  OPERATOR  30    #>> (tbox, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tbox, tbox),
  OPERATOR  31    #&> (tbox, tint),
  OPERATOR  31    #&> (tbox, tfloat),
  -- functions
  FUNCTION  1  tbox_spgist_config(internal, internal),
  FUNCTION  2  tbox_spgist_choose(internal, internal),
  FUNCTION  3  tbox_spgist_picksplit(internal, internal),
  FUNCTION  4  tbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  tbox_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tint_ops
  DEFAULT FOR TYPE tint USING spgist AS
  -- strictly left
  OPERATOR  1    << (tint, intrange),
  OPERATOR  1    << (tint, tbox),
  OPERATOR  1    << (tint, tint),
  OPERATOR  1    << (tint, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tint, intrange),
  OPERATOR  2    &< (tint, tbox),
  OPERATOR  2    &< (tint, tint),
  OPERATOR  2    &< (tint, tfloat),
  -- overlaps
  OPERATOR  3    && (tint, intrange),
  OPERATOR  3    && (tint, tbox),
  OPERATOR  3    && (tint, tint),
  OPERATOR  3    && (tint, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tint, intrange),
  OPERATOR  4    &> (tint, tbox),
  OPERATOR  4    &> (tint, tint),
  OPERATOR  4    &> (tint, tfloat),
  -- strictly right
  OPERATOR  5    >> (tint, intrange),
  OPERATOR  5    >> (tint, tbox),
  OPERATOR  5    >> (tint, tint),
  OPERATOR  5    >> (tint, tfloat),
    -- same
  OPERATOR  6    ~= (tint, intrange),
  OPERATOR  6    ~= (tint, tbox),
  OPERATOR  6    ~= (tint, tint),
  OPERATOR  6    ~= (tint, tfloat),
  -- contains
  OPERATOR  7    @> (tint, intrange),
  OPERATOR  7    @> (tint, tbox),
  OPERATOR  7    @> (tint, tint),
  OPERATOR  7    @> (tint, tfloat),
  -- contained by
  OPERATOR  8    <@ (tint, intrange),
  OPERATOR  8    <@ (tint, tbox),
  OPERATOR  8    <@ (tint, tint),
  OPERATOR  8    <@ (tint, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tint, intrange),
  OPERATOR  17    -|- (tint, tbox),
  OPERATOR  17    -|- (tint, tint),
  OPERATOR  17    -|- (tint, tfloat),
  -- overlaps or before
  OPERATOR  28    &<# (tint, tbox),
  OPERATOR  28    &<# (tint, tint),
  OPERATOR  28    &<# (tint, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tint, tbox),
  OPERATOR  29    <<# (tint, tint),
  OPERATOR  29    <<# (tint, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tint, tbox),
  OPERATOR  30    #>> (tint, tint),
  OPERATOR  30    #>> (tint, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tint, tbox),
  OPERATOR  31    #&> (tint, tint),
  OPERATOR  31    #&> (tint, tfloat),
  -- functions
  FUNCTION  1  tbox_spgist_config(internal, internal),
  FUNCTION  2  tbox_spgist_choose(internal, internal),
  FUNCTION  3  tbox_spgist_picksplit(internal, internal),
  FUNCTION  4  tbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  tbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  sptnumber_gist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tfloat_ops
  DEFAULT FOR TYPE tfloat USING spgist AS
  -- strictly left
  OPERATOR  1    << (tfloat, floatrange),
  OPERATOR  1    << (tfloat, tbox),
  OPERATOR  1    << (tfloat, tint),
  OPERATOR  1    << (tfloat, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tfloat, floatrange),
  OPERATOR  2    &< (tfloat, tbox),
  OPERATOR  2    &< (tfloat, tint),
  OPERATOR  2    &< (tfloat, tfloat),
  -- overlaps
  OPERATOR  3    && (tfloat, floatrange),
  OPERATOR  3    && (tfloat, tbox),
  OPERATOR  3    && (tfloat, tint),
  OPERATOR  3    && (tfloat, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tfloat, floatrange),
  OPERATOR  4    &> (tfloat, tbox),
  OPERATOR  4    &> (tfloat, tint),
  OPERATOR  4    &> (tfloat, tfloat),
  -- strictly right
  OPERATOR  5    >> (tfloat, floatrange),
  OPERATOR  5    >> (tfloat, tbox),
  OPERATOR  5    >> (tfloat, tint),
  OPERATOR  5    >> (tfloat, tfloat),
    -- same
  OPERATOR  6    ~= (tfloat, floatrange),
  OPERATOR  6    ~= (tfloat, tbox),
  OPERATOR  6    ~= (tfloat, tint),
  OPERATOR  6    ~= (tfloat, tfloat),
  -- contains
  OPERATOR  7    @> (tfloat, floatrange),
  OPERATOR  7    @> (tfloat, tbox),
  OPERATOR  7    @> (tfloat, tint),
  OPERATOR  7    @> (tfloat, tfloat),
  -- contained by
  OPERATOR  8    <@ (tfloat, floatrange),
  OPERATOR  8    <@ (tfloat, tbox),
  OPERATOR  8    <@ (tfloat, tint),
  OPERATOR  8    <@ (tfloat, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tfloat, floatrange),
  OPERATOR  17    -|- (tfloat, tbox),
  OPERATOR  17    -|- (tfloat, tint),
  OPERATOR  17    -|- (tfloat, tfloat),
  -- overlaps or before
  OPERATOR  28    &<# (tfloat, tbox),
  OPERATOR  28    &<# (tfloat, tint),
  OPERATOR  28    &<# (tfloat, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tfloat, tbox),
  OPERATOR  29    <<# (tfloat, tint),
  OPERATOR  29    <<# (tfloat, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tfloat, tbox),
  OPERATOR  30    #>> (tfloat, tint),
  OPERATOR  30    #>> (tfloat, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tfloat, tbox),
  OPERATOR  31    #&> (tfloat, tint),
  OPERATOR  31    #&> (tfloat, tfloat),
  -- functions
  FUNCTION  1  tbox_spgist_config(internal, internal),
  FUNCTION  2  tbox_spgist_choose(internal, internal),
  FUNCTION  3  tbox_spgist_picksplit(internal, internal),
  FUNCTION  4  tbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  tbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  sptnumber_gist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_ttext_ops
  DEFAULT FOR TYPE ttext USING spgist AS
  -- overlaps
  OPERATOR  3    && (ttext, period),
  OPERATOR  3    && (ttext, ttext),
    -- same
  OPERATOR  6    ~= (ttext, period),
  OPERATOR  6    ~= (ttext, ttext),
  -- contains
  OPERATOR  7    @> (ttext, period),
  OPERATOR  7    @> (ttext, ttext),
  -- contained by
  OPERATOR  8    <@ (ttext, period),
  OPERATOR  8    <@ (ttext, ttext),
  -- adjacent
  OPERATOR  17    -|- (ttext, period),
  OPERATOR  17    -|- (ttext, ttext),
  -- overlaps or before
  OPERATOR  28    &<# (ttext, period),
  OPERATOR  28    &<# (ttext, ttext),
  -- strictly before
  OPERATOR  29    <<# (ttext, period),
  OPERATOR  29    <<# (ttext, ttext),
  -- strictly after
  OPERATOR  30    #>> (ttext, period),
  OPERATOR  30    #>> (ttext, ttext),
  -- overlaps or after
  OPERATOR  31    #&> (ttext, period),
  OPERATOR  31    #&> (ttext, ttext),
  -- functions
  FUNCTION  1  spperiod_gist_config(internal, internal),
  FUNCTION  2  spperiod_gist_choose(internal, internal),
  FUNCTION  3  spperiod_gist_picksplit(internal, internal),
  FUNCTION  4  spperiod_gist_inner_consistent(internal, internal),
  FUNCTION  5  spperiod_gist_leaf_consistent(internal, internal),
  FUNCTION  6  sptemporal_gist_compress(internal);

/******************************************************************************/
