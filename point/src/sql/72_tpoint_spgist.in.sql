/*****************************************************************************
 *
 * tpoint_spist.c
 *    Oct-tree SP-GiST index for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION >= 110000

CREATE FUNCTION stbox_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_spgist_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_spgist_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_spgist_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_spgist_leaf_consistent(internal, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sptpoint_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS stbox_spgist_ops
  DEFAULT FOR TYPE stbox USING spgist AS
  -- strictly left
  OPERATOR  1    << (stbox, stbox),
  OPERATOR  1    << (stbox, tgeompoint),
  -- overlaps or left
  OPERATOR  2    &< (stbox, stbox),
  OPERATOR  2    &< (stbox, tgeompoint),
  -- overlaps
  OPERATOR  3    && (stbox, stbox),
  OPERATOR  3    && (stbox, tgeompoint),
  -- overlaps or right
  OPERATOR  4    &> (stbox, stbox),
  OPERATOR  4    &> (stbox, tgeompoint),
    -- strictly right
  OPERATOR  5    >> (stbox, stbox),
  OPERATOR  5    >> (stbox, tgeompoint),
    -- same
  OPERATOR  6    ~= (stbox, stbox),
  OPERATOR  6    ~= (stbox, tgeompoint),
  -- contains
  OPERATOR  7    @> (stbox, stbox),
  OPERATOR  7    @> (stbox, tgeompoint),
  -- contained by
  OPERATOR  8    <@ (stbox, stbox),
  OPERATOR  8    <@ (stbox, tgeompoint),
  -- overlaps or below
  OPERATOR  9    &<| (stbox, stbox),
  OPERATOR  9    &<| (stbox, tgeompoint),
  -- strictly below
  OPERATOR  10    <<| (stbox, stbox),
  OPERATOR  10    <<| (stbox, tgeompoint),
  -- strictly above
  OPERATOR  11    |>> (stbox, stbox),
  OPERATOR  11    |>> (stbox, tgeompoint),
  -- overlaps or above
  OPERATOR  12    |&> (stbox, stbox),
  OPERATOR  12    |&> (stbox, tgeompoint),
  -- adjacent
  OPERATOR  17    -|- (stbox, stbox),
  OPERATOR  17    -|- (stbox, tgeompoint),
  -- overlaps or before
  OPERATOR  28    &<# (stbox, stbox),
  OPERATOR  28    &<# (stbox, tgeompoint),
  -- strictly before
  OPERATOR  29    <<# (stbox, stbox),
  OPERATOR  29    <<# (stbox, tgeompoint),
  -- strictly after
  OPERATOR  30    #>> (stbox, stbox),
  OPERATOR  30    #>> (stbox, tgeompoint),
  -- overlaps or after
  OPERATOR  31    #&> (stbox, stbox),
  OPERATOR  31    #&> (stbox, tgeompoint),
  -- overlaps or front
  OPERATOR  32    &</ (stbox, stbox),
  OPERATOR  32    &</ (stbox, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (stbox, stbox),
  OPERATOR  33    <</ (stbox, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (stbox, stbox),
  OPERATOR  34    />> (stbox, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (stbox, stbox),
  OPERATOR  35    /&> (stbox, tgeompoint),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_spgist_choose(internal, internal),
  FUNCTION  3  stbox_spgist_picksplit(internal, internal),
  FUNCTION  4  stbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeompoint_ops
  DEFAULT FOR TYPE tgeompoint USING spgist AS
  -- strictly left
  OPERATOR  1    << (tgeompoint, geometry),  
  OPERATOR  1    << (tgeompoint, stbox),  
  OPERATOR  1    << (tgeompoint, tgeompoint),  
  -- overlaps or left
  OPERATOR  2    &< (tgeompoint, geometry),  
  OPERATOR  2    &< (tgeompoint, stbox),  
  OPERATOR  2    &< (tgeompoint, tgeompoint),  
  -- overlaps  
  OPERATOR  3    && (tgeompoint, geometry),  
  OPERATOR  3    && (tgeompoint, stbox),  
  OPERATOR  3    && (tgeompoint, tgeompoint),  
  -- overlaps or right
  OPERATOR  4    &> (tgeompoint, geometry),  
  OPERATOR  4    &> (tgeompoint, stbox),  
  OPERATOR  4    &> (tgeompoint, tgeompoint),  
    -- strictly right
  OPERATOR  5    >> (tgeompoint, geometry),  
  OPERATOR  5    >> (tgeompoint, stbox),  
  OPERATOR  5    >> (tgeompoint, tgeompoint),  
    -- same
  OPERATOR  6    ~= (tgeompoint, geometry),  
  OPERATOR  6    ~= (tgeompoint, stbox),  
  OPERATOR  6    ~= (tgeompoint, tgeompoint),  
  -- contains
  OPERATOR  7    @> (tgeompoint, geometry),  
  OPERATOR  7    @> (tgeompoint, stbox),  
  OPERATOR  7    @> (tgeompoint, tgeompoint),  
  -- contained by
  OPERATOR  8    <@ (tgeompoint, geometry),  
  OPERATOR  8    <@ (tgeompoint, stbox),  
  OPERATOR  8    <@ (tgeompoint, tgeompoint),  
  -- overlaps or below
  OPERATOR  9    &<| (tgeompoint, geometry),  
  OPERATOR  9    &<| (tgeompoint, stbox),  
  OPERATOR  9    &<| (tgeompoint, tgeompoint),  
  -- strictly below
  OPERATOR  10    <<| (tgeompoint, geometry),  
  OPERATOR  10    <<| (tgeompoint, stbox),  
  OPERATOR  10    <<| (tgeompoint, tgeompoint),  
  -- strictly above
  OPERATOR  11    |>> (tgeompoint, geometry),  
  OPERATOR  11    |>> (tgeompoint, stbox),  
  OPERATOR  11    |>> (tgeompoint, tgeompoint),  
  -- overlaps or above
  OPERATOR  12    |&> (tgeompoint, geometry),  
  OPERATOR  12    |&> (tgeompoint, stbox),  
  OPERATOR  12    |&> (tgeompoint, tgeompoint),  
  -- adjacent
  OPERATOR  17    -|- (tgeompoint, geometry),
  OPERATOR  17    -|- (tgeompoint, stbox),
  OPERATOR  17    -|- (tgeompoint, tgeompoint),
#if MOBDB_PGSQL_VERSION >= 120000
  -- distance
  OPERATOR  25    |=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
#endif
  -- overlaps or before
  OPERATOR  28    &<# (tgeompoint, stbox),
  OPERATOR  28    &<# (tgeompoint, tgeompoint),
  -- strictly before
  OPERATOR  29    <<# (tgeompoint, stbox),
  OPERATOR  29    <<# (tgeompoint, tgeompoint),
  -- strictly after
  OPERATOR  30    #>> (tgeompoint, stbox),
  OPERATOR  30    #>> (tgeompoint, tgeompoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeompoint, stbox),
  OPERATOR  31    #&> (tgeompoint, tgeompoint),
  -- overlaps or front
  OPERATOR  32    &</ (tgeompoint, geometry),
  OPERATOR  32    &</ (tgeompoint, stbox),
  OPERATOR  32    &</ (tgeompoint, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (tgeompoint, geometry),
  OPERATOR  33    <</ (tgeompoint, stbox),
  OPERATOR  33    <</ (tgeompoint, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (tgeompoint, geometry),
  OPERATOR  34    />> (tgeompoint, stbox),
  OPERATOR  34    />> (tgeompoint, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (tgeompoint, geometry),
  OPERATOR  35    /&> (tgeompoint, stbox),
  OPERATOR  35    /&> (tgeompoint, tgeompoint),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_spgist_choose(internal, internal),
  FUNCTION  3  stbox_spgist_picksplit(internal, internal),
  FUNCTION  4  stbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  sptpoint_gist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeogpoint_ops
  DEFAULT FOR TYPE tgeogpoint USING spgist AS
  -- overlaps
  OPERATOR  3    && (tgeogpoint, geography),  
  OPERATOR  3    && (tgeogpoint, stbox),  
  OPERATOR  3    && (tgeogpoint, tgeogpoint),  
    -- same
  OPERATOR  6    ~= (tgeogpoint, geography),  
  OPERATOR  6    ~= (tgeogpoint, stbox),  
  OPERATOR  6    ~= (tgeogpoint, tgeogpoint),  
  -- contains
  OPERATOR  7    @> (tgeogpoint, geography),  
  OPERATOR  7    @> (tgeogpoint, stbox),  
  OPERATOR  7    @> (tgeogpoint, tgeogpoint),  
  -- contained by
  OPERATOR  8    <@ (tgeogpoint, geography),  
  OPERATOR  8    <@ (tgeogpoint, stbox),  
  OPERATOR  8    <@ (tgeogpoint, tgeogpoint),  
  -- adjacent
  OPERATOR  17    -|- (tgeogpoint, geography),
  OPERATOR  17    -|- (tgeogpoint, stbox),
  OPERATOR  17    -|- (tgeogpoint, tgeogpoint),
#if MOBDB_PGSQL_VERSION >= 120000
  -- distance
  OPERATOR  25    |=| (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
#endif
  -- overlaps or before
  OPERATOR  28    &<# (tgeogpoint, stbox),
  OPERATOR  28    &<# (tgeogpoint, tgeogpoint),
  -- strictly before
  OPERATOR  29    <<# (tgeogpoint, stbox),
  OPERATOR  29    <<# (tgeogpoint, tgeogpoint),
  -- strictly after
  OPERATOR  30    #>> (tgeogpoint, stbox),
  OPERATOR  30    #>> (tgeogpoint, tgeogpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeogpoint, stbox),
  OPERATOR  31    #&> (tgeogpoint, tgeogpoint),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_spgist_choose(internal, internal),
  FUNCTION  3  stbox_spgist_picksplit(internal, internal),
  FUNCTION  4  stbox_spgist_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  sptpoint_gist_compress(internal);
#endif
  
/******************************************************************************/
