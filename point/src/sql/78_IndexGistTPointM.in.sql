/*****************************************************************************
 *
 * IndexGistTemporalPointM.c
 *	  R-tree GiST index for temporal geometry points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tpoint_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_union(internal, internal)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gist_tpoint_union'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_penalty'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_picksplit'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_same(gbox, gbox, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_same'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tgeompoint_distance(internal, tgeompoint, smallint, oid, internal)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'gist_tpoint_distance'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING gist AS
	STORAGE gbox,
	-- strictly left
	OPERATOR	1		<< (tgeompoint, geometry),  
	OPERATOR	1		<< (tgeompoint, gbox),  
	OPERATOR	1		<< (tgeompoint, tgeompoint),  
	-- overlaps or left
	OPERATOR	2		&< (tgeompoint, geometry),  
	OPERATOR	2		&< (tgeompoint, gbox),  
	OPERATOR	2		&< (tgeompoint, tgeompoint),  
	-- overlaps	
	OPERATOR	3		&& (tgeompoint, timestamptz),  
	OPERATOR	3		&& (tgeompoint, timestampset),  
	OPERATOR	3		&& (tgeompoint, period),
	OPERATOR	3		&& (tgeompoint, periodset),
	OPERATOR	3		&& (tgeompoint, geometry),  
	OPERATOR	3		&& (tgeompoint, gbox),  
	OPERATOR	3		&& (tgeompoint, tgeompoint),  
	-- overlaps or right
	OPERATOR	4		&> (tgeompoint, geometry),  
	OPERATOR	4		&> (tgeompoint, gbox),  
	OPERATOR	4		&> (tgeompoint, tgeompoint),  
  	-- strictly right
	OPERATOR	5		>> (tgeompoint, geometry),  
	OPERATOR	5		>> (tgeompoint, gbox),  
	OPERATOR	5		>> (tgeompoint, tgeompoint),  
  	-- same
	OPERATOR	6		~= (tgeompoint, timestamptz),  
	OPERATOR	6		~= (tgeompoint, timestampset),  
	OPERATOR	6		~= (tgeompoint, period),
	OPERATOR	6		~= (tgeompoint, periodset),
	OPERATOR	6		~= (tgeompoint, geometry),  
	OPERATOR	6		~= (tgeompoint, gbox),  
	OPERATOR	6		~= (tgeompoint, tgeompoint),  
	-- contains
	OPERATOR	7		@> (tgeompoint, timestamptz),  
	OPERATOR	7		@> (tgeompoint, timestampset),
	OPERATOR	7		@> (tgeompoint, period),
	OPERATOR	7		@> (tgeompoint, periodset),
	OPERATOR	7		@> (tgeompoint, geometry),  
	OPERATOR	7		@> (tgeompoint, gbox),  
	OPERATOR	7		@> (tgeompoint, tgeompoint),  
	-- contained by
	OPERATOR	8		<@ (tgeompoint, timestamptz),  
	OPERATOR	8		<@ (tgeompoint, timestampset),  
	OPERATOR	8		<@ (tgeompoint, period),
	OPERATOR	8		<@ (tgeompoint, periodset),
	OPERATOR	8		<@ (tgeompoint, geometry),  
	OPERATOR	8		<@ (tgeompoint, gbox),  
	OPERATOR	8		<@ (tgeompoint, tgeompoint),  
	-- overlaps or below
	OPERATOR	9		&<| (tgeompoint, geometry),  
	OPERATOR	9		&<| (tgeompoint, gbox),  
	OPERATOR	9		&<| (tgeompoint, tgeompoint),  
	-- strictly below
	OPERATOR	10		<<| (tgeompoint, geometry),  
	OPERATOR	10		<<| (tgeompoint, gbox),  
	OPERATOR	10		<<| (tgeompoint, tgeompoint),  
	-- strictly above
	OPERATOR	11		|>> (tgeompoint, geometry),  
	OPERATOR	11		|>> (tgeompoint, gbox),  
	OPERATOR	11		|>> (tgeompoint, tgeompoint),  
	-- overlaps or above
	OPERATOR	12		|&> (tgeompoint, geometry),  
	OPERATOR	12		|&> (tgeompoint, gbox),  
	OPERATOR	12		|&> (tgeompoint, tgeompoint),  
	-- nearest approach distance
	OPERATOR	25		|=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		|=| (tgeompoint, gbox) FOR ORDER BY pg_catalog.float_ops,
	OPERATOR	25		|=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeompoint, timestamptz),
	OPERATOR	28		&<# (tgeompoint, timestampset),
	OPERATOR	28		&<# (tgeompoint, period),
	OPERATOR	28		&<# (tgeompoint, periodset),
	OPERATOR	28		&<# (tgeompoint, gbox),
	OPERATOR	28		&<# (tgeompoint, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (tgeompoint, timestamptz),
	OPERATOR	29		<<# (tgeompoint, timestampset),
	OPERATOR	29		<<# (tgeompoint, period),
	OPERATOR	29		<<# (tgeompoint, periodset),
	OPERATOR	29		<<# (tgeompoint, gbox),
	OPERATOR	29		<<# (tgeompoint, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (tgeompoint, timestamptz),
	OPERATOR	30		#>> (tgeompoint, timestampset),
	OPERATOR	30		#>> (tgeompoint, period),
	OPERATOR	30		#>> (tgeompoint, periodset),
	OPERATOR	30		#>> (tgeompoint, gbox),
	OPERATOR	30		#>> (tgeompoint, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeompoint, timestamptz),
	OPERATOR	31		#&> (tgeompoint, timestampset),
	OPERATOR	31		#&> (tgeompoint, period),
	OPERATOR	31		#&> (tgeompoint, periodset),
	OPERATOR	31		#&> (tgeompoint, gbox),
	OPERATOR	31		#&> (tgeompoint, tgeompoint),
	-- functions
	FUNCTION	1	gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal),
	FUNCTION	2	gist_tpoint_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_tpoint_penalty(internal, internal, internal),
	FUNCTION	6	gist_tpoint_picksplit(internal, internal),
	FUNCTION	7	gist_tpoint_same(gbox, gbox, internal),
	FUNCTION	8	gist_tgeompoint_distance(internal, tgeompoint, smallint, oid, internal);
	
/******************************************************************************/
