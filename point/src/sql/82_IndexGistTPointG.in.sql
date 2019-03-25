/*****************************************************************************
 *
 * IndexGistTemporalPointG.c
 *	  R-tree GiST index for temporal geography points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tpoint_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tgeogpoint_distance(internal, tgeogpoint, smallint, oid, internal)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'gist_tpoint_distance'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING gist AS
	STORAGE gbox,
	-- overlaps
	OPERATOR	3		&& (tgeogpoint, timestamptz),  
	OPERATOR	3		&& (tgeogpoint, timestampset),  
	OPERATOR	3		&& (tgeogpoint, period),
	OPERATOR	3		&& (tgeogpoint, periodset),
	OPERATOR	3		&& (tgeogpoint, geography),  
	OPERATOR	3		&& (tgeogpoint, gbox),  
	OPERATOR	3		&& (tgeogpoint, tgeogpoint),  
  	-- same
	OPERATOR	6		~= (tgeogpoint, timestamptz),  
	OPERATOR	6		~= (tgeogpoint, timestampset),  
	OPERATOR	6		~= (tgeogpoint, period),
	OPERATOR	6		~= (tgeogpoint, periodset),
	OPERATOR	6		~= (tgeogpoint, geography),  
	OPERATOR	6		~= (tgeogpoint, gbox),  
	OPERATOR	6		~= (tgeogpoint, tgeogpoint),  
	-- contains
	OPERATOR	7		@> (tgeogpoint, timestamptz),  
	OPERATOR	7		@> (tgeogpoint, timestampset),
	OPERATOR	7		@> (tgeogpoint, period),
	OPERATOR	7		@> (tgeogpoint, periodset),
	OPERATOR	7		@> (tgeogpoint, geography),  
	OPERATOR	7		@> (tgeogpoint, gbox),  
	OPERATOR	7		@> (tgeogpoint, tgeogpoint),  
	-- contained by
	OPERATOR	8		<@ (tgeogpoint, timestamptz),  
	OPERATOR	8		<@ (tgeogpoint, timestampset),  
	OPERATOR	8		<@ (tgeogpoint, period),
	OPERATOR	8		<@ (tgeogpoint, periodset),
	OPERATOR	8		<@ (tgeogpoint, geography),  
	OPERATOR	8		<@ (tgeogpoint, gbox),  
	OPERATOR	8		<@ (tgeogpoint, tgeogpoint),  
	-- distance
--	OPERATOR	25		<-> (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, gbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeogpoint, timestamptz),
	OPERATOR	28		&<# (tgeogpoint, timestampset),
	OPERATOR	28		&<# (tgeogpoint, period),
	OPERATOR	28		&<# (tgeogpoint, periodset),
	OPERATOR	28		&<# (tgeogpoint, tgeogpoint),
	-- strictly before
	OPERATOR	29		<<# (tgeogpoint, timestamptz),
	OPERATOR	29		<<# (tgeogpoint, timestampset),
	OPERATOR	29		<<# (tgeogpoint, period),
	OPERATOR	29		<<# (tgeogpoint, periodset),
	OPERATOR	29		<<# (tgeogpoint, tgeogpoint),
	-- strictly after
	OPERATOR	30		#>> (tgeogpoint, timestamptz),
	OPERATOR	30		#>> (tgeogpoint, timestampset),
	OPERATOR	30		#>> (tgeogpoint, period),
	OPERATOR	30		#>> (tgeogpoint, periodset),
	OPERATOR	30		#>> (tgeogpoint, tgeogpoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeogpoint, timestamptz),
	OPERATOR	31		#&> (tgeogpoint, timestampset),
	OPERATOR	31		#&> (tgeogpoint, period),
	OPERATOR	31		#&> (tgeogpoint, periodset),
	OPERATOR	31		#&> (tgeogpoint, tgeogpoint),
	-- functions
	FUNCTION	1	gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal),
	FUNCTION	2	gist_tpoint_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_tpoint_penalty(internal, internal, internal),
	FUNCTION	6	gist_tpoint_picksplit(internal, internal),
	FUNCTION	7	gist_tpoint_same(gbox, gbox, internal);
--	FUNCTION	8	gist_tgeogpoint_distance (internal, tgeogpoint, smallint, oid, internal),
--	FUNCTION	9	gist_tpoint_fetch(internal);
	
/******************************************************************************/
