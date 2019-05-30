/*****************************************************************************
 *
 * IndexSpistTemporalPoint.c
 *	  Oct-tree SP-GiST index for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_tpoint_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tgeompoint, geometry),  
	OPERATOR	1		<< (tgeompoint, gbox),  
	OPERATOR	1		<< (tgeompoint, tgeompoint),  
	-- overlaps or left
	OPERATOR	2		&< (tgeompoint, geometry),  
	OPERATOR	2		&< (tgeompoint, gbox),  
	OPERATOR	2		&< (tgeompoint, tgeompoint),  
	-- overlaps	
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
	OPERATOR	6		~= (tgeompoint, geometry),  
	OPERATOR	6		~= (tgeompoint, gbox),  
	OPERATOR	6		~= (tgeompoint, tgeompoint),  
	-- contains
	OPERATOR	7		@> (tgeompoint, geometry),  
	OPERATOR	7		@> (tgeompoint, gbox),  
	OPERATOR	7		@> (tgeompoint, tgeompoint),  
	-- contained by
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
	-- overlaps or before
	OPERATOR	28		&<# (tgeompoint, gbox),
	OPERATOR	28		&<# (tgeompoint, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (tgeompoint, gbox),
	OPERATOR	29		<<# (tgeompoint, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (tgeompoint, gbox),
	OPERATOR	30		#>> (tgeompoint, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeompoint, gbox),
	OPERATOR	31		#&> (tgeompoint, tgeompoint),
	-- overlaps or front
	OPERATOR	32		&</ (tgeompoint, gbox),
	OPERATOR	32		&</ (tgeompoint, tgeompoint),
	-- strictly front
	OPERATOR	33		<</ (tgeompoint, gbox),
	OPERATOR	33		<</ (tgeompoint, tgeompoint),
	-- strictly back
	OPERATOR	34		/>> (tgeompoint, gbox),
	OPERATOR	34		/>> (tgeompoint, tgeompoint),
	-- overlaps or back
	OPERATOR	35		/&> (tgeompoint, gbox),
	OPERATOR	35		/&> (tgeompoint, tgeompoint),
	-- functions
	FUNCTION	1	spgist_tpoint_config(internal, internal),
	FUNCTION	2	spgist_tpoint_choose(internal, internal),
	FUNCTION	3	spgist_tpoint_picksplit(internal, internal),
	FUNCTION	4	spgist_tpoint_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tpoint_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tpoint_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING spgist AS
	-- overlaps
	OPERATOR	3		&& (tgeogpoint, geography),  
	OPERATOR	3		&& (tgeogpoint, gbox),  
	OPERATOR	3		&& (tgeogpoint, tgeogpoint),  
  	-- same
	OPERATOR	6		~= (tgeogpoint, geography),  
	OPERATOR	6		~= (tgeogpoint, gbox),  
	OPERATOR	6		~= (tgeogpoint, tgeogpoint),  
	-- contains
	OPERATOR	7		@> (tgeogpoint, geography),  
	OPERATOR	7		@> (tgeogpoint, gbox),  
	OPERATOR	7		@> (tgeogpoint, tgeogpoint),  
	-- contained by
	OPERATOR	8		<@ (tgeogpoint, geography),  
	OPERATOR	8		<@ (tgeogpoint, gbox),  
	OPERATOR	8		<@ (tgeogpoint, tgeogpoint),  
	-- distance
--	OPERATOR	25		<-> (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, gbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeogpoint, gbox),
	OPERATOR	28		&<# (tgeogpoint, tgeogpoint),
	-- strictly before
	OPERATOR	29		<<# (tgeogpoint, gbox),
	OPERATOR	29		<<# (tgeogpoint, tgeogpoint),
	-- strictly after
	OPERATOR	30		#>> (tgeogpoint, gbox),
	OPERATOR	30		#>> (tgeogpoint, tgeogpoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeogpoint, gbox),
	OPERATOR	31		#&> (tgeogpoint, tgeogpoint),
	-- functions
	FUNCTION	1	spgist_tpoint_config(internal, internal),
	FUNCTION	2	spgist_tpoint_choose(internal, internal),
	FUNCTION	3	spgist_tpoint_picksplit(internal, internal),
	FUNCTION	4	spgist_tpoint_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tpoint_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tpoint_compress(internal);
	
/******************************************************************************/
