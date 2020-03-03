/*****************************************************************************
 *
 * tpoint_gist.c
 *	  R-tree GiST index for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_stbox_consistent(internal, stbox, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_stbox_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_stbox_union(internal, internal)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'gist_stbox_union'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_stbox_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_stbox_penalty'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_stbox_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_stbox_picksplit'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_stbox_same(stbox, stbox, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_stbox_same'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_stbox_ops
	DEFAULT FOR TYPE stbox USING gist AS
	STORAGE stbox,
	-- strictly left
	OPERATOR	1		<< (stbox, stbox),
	OPERATOR	1		<< (stbox, tgeompoint),
	-- overlaps or left
	OPERATOR	2		&< (stbox, stbox),
	OPERATOR	2		&< (stbox, tgeompoint),
	-- overlaps
	OPERATOR	3		&& (stbox, stbox),
	OPERATOR	3		&& (stbox, tgeompoint),
	-- overlaps or right
	OPERATOR	4		&> (stbox, stbox),
	OPERATOR	4		&> (stbox, tgeompoint),
  	-- strictly right
	OPERATOR	5		>> (stbox, stbox),
	OPERATOR	5		>> (stbox, tgeompoint),
  	-- same
	OPERATOR	6		~= (stbox, stbox),
	OPERATOR	6		~= (stbox, tgeompoint),
	-- contains
	OPERATOR	7		@> (stbox, stbox),
	OPERATOR	7		@> (stbox, tgeompoint),
	-- contained by
	OPERATOR	8		<@ (stbox, stbox),
	OPERATOR	8		<@ (stbox, tgeompoint),
	-- overlaps or below
	OPERATOR	9		&<| (stbox, stbox),
	OPERATOR	9		&<| (stbox, tgeompoint),
	-- strictly below
	OPERATOR	10		<<| (stbox, stbox),
	OPERATOR	10		<<| (stbox, tgeompoint),
	-- strictly above
	OPERATOR	11		|>> (stbox, stbox),
	OPERATOR	11		|>> (stbox, tgeompoint),
	-- overlaps or above
	OPERATOR	12		|&> (stbox, stbox),
	OPERATOR	12		|&> (stbox, tgeompoint),
	-- nearest approach distance
--	OPERATOR	25		|=| (stbox, stbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		|=| (stbox, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (stbox, stbox),
	OPERATOR	28		&<# (stbox, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (stbox, stbox),
	OPERATOR	29		<<# (stbox, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (stbox, stbox),
	OPERATOR	30		#>> (stbox, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (stbox, stbox),
	OPERATOR	31		#&> (stbox, tgeompoint),
	-- overlaps or front
	OPERATOR	32		&</ (stbox, stbox),
	OPERATOR	32		&</ (stbox, tgeompoint),
	-- strictly front
	OPERATOR	33		<</ (stbox, stbox),
	OPERATOR	33		<</ (stbox, tgeompoint),
	-- strictly back
	OPERATOR	34		/>> (stbox, stbox),
	OPERATOR	34		/>> (stbox, tgeompoint),
	-- overlaps or back
	OPERATOR	35		/&> (stbox, stbox),
	OPERATOR	35		/&> (stbox, tgeompoint),
	-- functions
	FUNCTION	1	gist_stbox_consistent(internal, stbox, smallint, oid, internal),
	FUNCTION	2	gist_stbox_union(internal, internal),
	FUNCTION	5	gist_stbox_penalty(internal, internal, internal),
	FUNCTION	6	gist_stbox_picksplit(internal, internal),
	FUNCTION	7	gist_stbox_same(stbox, stbox, internal);

/******************************************************************************/

CREATE FUNCTION gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_stbox_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_stbox_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING gist AS
	STORAGE stbox,
	-- strictly left
	OPERATOR	1		<< (tgeompoint, geometry),  
	OPERATOR	1		<< (tgeompoint, stbox),  
	OPERATOR	1		<< (tgeompoint, tgeompoint),  
	-- overlaps or left
	OPERATOR	2		&< (tgeompoint, geometry),  
	OPERATOR	2		&< (tgeompoint, stbox),  
	OPERATOR	2		&< (tgeompoint, tgeompoint),  
	-- overlaps	
	OPERATOR	3		&& (tgeompoint, geometry),  
	OPERATOR	3		&& (tgeompoint, stbox),  
	OPERATOR	3		&& (tgeompoint, tgeompoint),  
	-- overlaps or right
	OPERATOR	4		&> (tgeompoint, geometry),  
	OPERATOR	4		&> (tgeompoint, stbox),  
	OPERATOR	4		&> (tgeompoint, tgeompoint),  
  	-- strictly right
	OPERATOR	5		>> (tgeompoint, geometry),  
	OPERATOR	5		>> (tgeompoint, stbox),  
	OPERATOR	5		>> (tgeompoint, tgeompoint),  
  	-- same
	OPERATOR	6		~= (tgeompoint, geometry),  
	OPERATOR	6		~= (tgeompoint, stbox),  
	OPERATOR	6		~= (tgeompoint, tgeompoint),  
	-- contains
	OPERATOR	7		@> (tgeompoint, geometry),  
	OPERATOR	7		@> (tgeompoint, stbox),  
	OPERATOR	7		@> (tgeompoint, tgeompoint),  
	-- contained by
	OPERATOR	8		<@ (tgeompoint, geometry),  
	OPERATOR	8		<@ (tgeompoint, stbox),  
	OPERATOR	8		<@ (tgeompoint, tgeompoint),  
	-- overlaps or below
	OPERATOR	9		&<| (tgeompoint, geometry),  
	OPERATOR	9		&<| (tgeompoint, stbox),  
	OPERATOR	9		&<| (tgeompoint, tgeompoint),  
	-- strictly below
	OPERATOR	10		<<| (tgeompoint, geometry),  
	OPERATOR	10		<<| (tgeompoint, stbox),  
	OPERATOR	10		<<| (tgeompoint, tgeompoint),  
	-- strictly above
	OPERATOR	11		|>> (tgeompoint, geometry),  
	OPERATOR	11		|>> (tgeompoint, stbox),  
	OPERATOR	11		|>> (tgeompoint, tgeompoint),  
	-- overlaps or above
	OPERATOR	12		|&> (tgeompoint, geometry),  
	OPERATOR	12		|&> (tgeompoint, stbox),  
	OPERATOR	12		|&> (tgeompoint, tgeompoint),  
	-- nearest approach distance
	OPERATOR	25		|=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		|=| (tgeompoint, stbox) FOR ORDER BY pg_catalog.float_ops,
	OPERATOR	25		|=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeompoint, stbox),
	OPERATOR	28		&<# (tgeompoint, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (tgeompoint, stbox),
	OPERATOR	29		<<# (tgeompoint, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (tgeompoint, stbox),
	OPERATOR	30		#>> (tgeompoint, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeompoint, stbox),
	OPERATOR	31		#&> (tgeompoint, tgeompoint),
	-- overlaps or front
	OPERATOR	32		&</ (tgeompoint, stbox),
	OPERATOR	32		&</ (tgeompoint, tgeompoint),
	-- strictly front
	OPERATOR	33		<</ (tgeompoint, stbox),
	OPERATOR	33		<</ (tgeompoint, tgeompoint),
	-- strictly back
	OPERATOR	34		/>> (tgeompoint, stbox),
	OPERATOR	34		/>> (tgeompoint, tgeompoint),
	-- overlaps or back
	OPERATOR	35		/&> (tgeompoint, stbox),
	OPERATOR	35		/&> (tgeompoint, tgeompoint),
	-- functions
	FUNCTION	1	gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal),
	FUNCTION	2	gist_stbox_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_stbox_penalty(internal, internal, internal),
	FUNCTION	6	gist_stbox_picksplit(internal, internal),
	FUNCTION	7	gist_stbox_same(stbox, stbox, internal);
	
CREATE OPERATOR CLASS gist_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING gist AS
	STORAGE stbox,
	-- overlaps
	OPERATOR	3		&& (tgeogpoint, geography),  
	OPERATOR	3		&& (tgeogpoint, stbox),  
	OPERATOR	3		&& (tgeogpoint, tgeogpoint),  
  	-- same
	OPERATOR	6		~= (tgeogpoint, geography),  
	OPERATOR	6		~= (tgeogpoint, stbox),  
	OPERATOR	6		~= (tgeogpoint, tgeogpoint),  
	-- contains
	OPERATOR	7		@> (tgeogpoint, geography),  
	OPERATOR	7		@> (tgeogpoint, stbox),  
	OPERATOR	7		@> (tgeogpoint, tgeogpoint),  
	-- contained by
	OPERATOR	8		<@ (tgeogpoint, geography),  
	OPERATOR	8		<@ (tgeogpoint, stbox),  
	OPERATOR	8		<@ (tgeogpoint, tgeogpoint),  
	-- distance
--	OPERATOR	25		<-> (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeogpoint, stbox),
	OPERATOR	28		&<# (tgeogpoint, tgeogpoint),
	-- strictly before
	OPERATOR	29		<<# (tgeogpoint, stbox),
	OPERATOR	29		<<# (tgeogpoint, tgeogpoint),
	-- strictly after
	OPERATOR	30		#>> (tgeogpoint, stbox),
	OPERATOR	30		#>> (tgeogpoint, tgeogpoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeogpoint, stbox),
	OPERATOR	31		#&> (tgeogpoint, tgeogpoint),
	-- functions
	FUNCTION	1	gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal),
	FUNCTION	2	gist_stbox_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_stbox_penalty(internal, internal, internal),
	FUNCTION	6	gist_stbox_picksplit(internal, internal),
	FUNCTION	7	gist_stbox_same(stbox, stbox, internal);
	
/******************************************************************************/
