/*****************************************************************************
 *
 * IndexGistTNPoint.sql
 *	  R-tree GiST index for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tnpoint_consistent(internal, tnpoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tpoint_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_union(internal, internal)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gist_tpoint_union'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tnpoint_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_penalty'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_picksplit'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_same(gbox, gbox, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_same'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tnpoint_distance(internal, tnpoint, smallint, oid, internal)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'gist_tpoint_distance'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tnpoint_ops
	DEFAULT FOR TYPE tnpoint USING gist AS
	STORAGE gbox,
	-- strictly left
	OPERATOR	1		<< (tnpoint, geometry),  
	OPERATOR	1		<< (tnpoint, gbox),  
	OPERATOR	1		<< (tnpoint, tnpoint),  
	-- overlaps or left
	OPERATOR	2		&< (tnpoint, geometry),  
	OPERATOR	2		&< (tnpoint, gbox),  
	OPERATOR	2		&< (tnpoint, tnpoint),  
	-- overlaps	
	OPERATOR	3		&& (tnpoint, timestamptz),  
	OPERATOR	3		&& (tnpoint, timestampset),  
	OPERATOR	3		&& (tnpoint, period),
	OPERATOR	3		&& (tnpoint, periodset),
	OPERATOR	3		&& (tnpoint, geometry),  
	OPERATOR	3		&& (tnpoint, gbox),  
	OPERATOR	3		&& (tnpoint, tnpoint),  
	-- overlaps or right
	OPERATOR	4		&> (tnpoint, geometry),  
	OPERATOR	4		&> (tnpoint, gbox),  
	OPERATOR	4		&> (tnpoint, tnpoint),  
  	-- strictly right
	OPERATOR	5		>> (tnpoint, geometry),  
	OPERATOR	5		>> (tnpoint, gbox),  
	OPERATOR	5		>> (tnpoint, tnpoint),  
  	-- same
	OPERATOR	6		~= (tnpoint, timestamptz),  
	OPERATOR	6		~= (tnpoint, timestampset),  
	OPERATOR	6		~= (tnpoint, period),
	OPERATOR	6		~= (tnpoint, periodset),
	OPERATOR	6		~= (tnpoint, geometry),  
	OPERATOR	6		~= (tnpoint, gbox),  
	OPERATOR	6		~= (tnpoint, tnpoint),  
	-- contains
	OPERATOR	7		@> (tnpoint, timestamptz),  
	OPERATOR	7		@> (tnpoint, timestampset),
	OPERATOR	7		@> (tnpoint, period),
	OPERATOR	7		@> (tnpoint, periodset),
	OPERATOR	7		@> (tnpoint, geometry),  
	OPERATOR	7		@> (tnpoint, gbox),  
	OPERATOR	7		@> (tnpoint, tnpoint),  
	-- contained by
	OPERATOR	8		<@ (tnpoint, timestamptz),  
	OPERATOR	8		<@ (tnpoint, timestampset),  
	OPERATOR	8		<@ (tnpoint, period),
	OPERATOR	8		<@ (tnpoint, periodset),
	OPERATOR	8		<@ (tnpoint, geometry),  
	OPERATOR	8		<@ (tnpoint, gbox),  
	OPERATOR	8		<@ (tnpoint, tnpoint),  
	-- overlaps or below
	OPERATOR	9		&<| (tnpoint, geometry),  
	OPERATOR	9		&<| (tnpoint, gbox),  
	OPERATOR	9		&<| (tnpoint, tnpoint),  
	-- strictly below
	OPERATOR	10		<<| (tnpoint, geometry),  
	OPERATOR	10		<<| (tnpoint, gbox),  
	OPERATOR	10		<<| (tnpoint, tnpoint),  
	-- strictly above
	OPERATOR	11		|>> (tnpoint, geometry),  
	OPERATOR	11		|>> (tnpoint, gbox),  
	OPERATOR	11		|>> (tnpoint, tnpoint),  
	-- overlaps or above
	OPERATOR	12		|&> (tnpoint, geometry),  
	OPERATOR	12		|&> (tnpoint, gbox),  
	OPERATOR	12		|&> (tnpoint, tnpoint),  
	-- distance
	OPERATOR	25		<-> (tnpoint, geometry) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tnpoint, gbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tnpoint, tnpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or front
	OPERATOR	28		&<# (tnpoint, timestamptz),
	OPERATOR	28		&<# (tnpoint, timestampset),
	OPERATOR	28		&<# (tnpoint, period),
	OPERATOR	28		&<# (tnpoint, periodset),
	OPERATOR	28		&<# (tnpoint, gbox),
	OPERATOR	28		&<# (tnpoint, tnpoint),
	-- strictly front
	OPERATOR	29		<<# (tnpoint, timestamptz),
	OPERATOR	29		<<# (tnpoint, timestampset),
	OPERATOR	29		<<# (tnpoint, period),
	OPERATOR	29		<<# (tnpoint, periodset),
	OPERATOR	29		<<# (tnpoint, gbox),
	OPERATOR	29		<<# (tnpoint, tnpoint),
	-- strictly back
	OPERATOR	30		#>> (tnpoint, timestamptz),
	OPERATOR	30		#>> (tnpoint, timestampset),
	OPERATOR	30		#>> (tnpoint, period),
	OPERATOR	30		#>> (tnpoint, periodset),
	OPERATOR	30		#>> (tnpoint, gbox),
	OPERATOR	30		#>> (tnpoint, tnpoint),
	-- overlaps or back
	OPERATOR	31		#&> (tnpoint, timestamptz),
	OPERATOR	31		#&> (tnpoint, timestampset),
	OPERATOR	31		#&> (tnpoint, period),
	OPERATOR	31		#&> (tnpoint, periodset),
	OPERATOR	31		#&> (tnpoint, gbox),
	OPERATOR	31		#&> (tnpoint, tnpoint),
	-- functions
	FUNCTION	1	gist_tnpoint_consistent(internal, tnpoint, smallint, oid, internal),
	FUNCTION	2	gist_tnpoint_union(internal, internal),
	FUNCTION	3	gist_tnpoint_compress(internal),
	FUNCTION	5	gist_tnpoint_penalty(internal, internal, internal),
	FUNCTION	6	gist_tnpoint_picksplit(internal, internal),
	FUNCTION	7	gist_tnpoint_same(gbox, gbox, internal);
--	FUNCTION	8	gist_tnpoint_distance(internal, tnpoint, smallint, oid, internal),
	
/******************************************************************************/
