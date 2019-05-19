/*****************************************************************************
 *
 * IndexSPGistTemporal.sql
 *		Quad-tree SP-GiST index for temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_temporal_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_temporal_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_temporal_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************/

CREATE FUNCTION spgist_tnumber_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tbool_ops
	DEFAULT FOR TYPE tbool USING spgist AS
	-- overlaps
	OPERATOR	3		&& (tbool, period),
	OPERATOR	3		&& (tbool, tbool),	
  	-- same
	OPERATOR	6		~= (tbool, period),
	OPERATOR	6		~= (tbool, tbool),
	-- contains
	OPERATOR	7		@> (tbool, period),
	OPERATOR	7		@> (tbool, tbool),
	-- contained by
	OPERATOR	8		<@ (tbool, period),
	OPERATOR	8		<@ (tbool, tbool),
	-- overlaps or before
	OPERATOR	28		&<# (tbool, period),
	OPERATOR	28		&<# (tbool, tbool),
	-- strictly before
	OPERATOR	29		<<# (tbool, period),
	OPERATOR	29		<<# (tbool, tbool),
	-- strictly after
	OPERATOR	30		#>> (tbool, period),
	OPERATOR	30		#>> (tbool, tbool),
	-- overlaps or after
	OPERATOR	31		#&> (tbool, period),
	OPERATOR	31		#&> (tbool, tbool),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_temporal_inner_consistent(internal, internal),
	FUNCTION	5	spgist_temporal_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_temporal_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tint_ops
	DEFAULT FOR TYPE tint USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tint, intrange),
	OPERATOR	1		<< (tint, floatrange),
	OPERATOR	1		<< (tint, box),
	OPERATOR	1		<< (tint, tint),
	OPERATOR	1		<< (tint, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tint, intrange),
	OPERATOR	2		&< (tint, floatrange),
	OPERATOR	2		&< (tint, box),
	OPERATOR	2		&< (tint, tint),
	OPERATOR	2		&< (tint, tfloat),
	-- overlaps
	OPERATOR	3		&& (tint, intrange),
	OPERATOR	3		&& (tint, floatrange),
	OPERATOR	3		&& (tint, box),
	OPERATOR	3		&& (tint, tint),
	OPERATOR	3		&& (tint, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tint, intrange),
	OPERATOR	4		&> (tint, floatrange),
	OPERATOR	4		&> (tint, box),
	OPERATOR	4		&> (tint, tint),
	OPERATOR	4		&> (tint, tfloat),
	-- strictly right
	OPERATOR	5		>> (tint, intrange),
	OPERATOR	5		>> (tint, floatrange),
	OPERATOR	5		>> (tint, box),
	OPERATOR	5		>> (tint, tint),
	OPERATOR	5		>> (tint, tfloat),
  	-- same
	OPERATOR	6		~= (tint, intrange),
	OPERATOR	6		~= (tint, floatrange),
	OPERATOR	6		~= (tint, box),
	OPERATOR	6		~= (tint, tint),
	OPERATOR	6		~= (tint, tfloat),
	-- contains
	OPERATOR	7		@> (tint, intrange),
	OPERATOR	7		@> (tint, floatrange),
	OPERATOR	7		@> (tint, box),
	OPERATOR	7		@> (tint, tint),
	OPERATOR	7		@> (tint, tfloat),
	-- contained by
	OPERATOR	8		<@ (tint, intrange),
	OPERATOR	8		<@ (tint, floatrange),
	OPERATOR	8		<@ (tint, box),
	OPERATOR	8		<@ (tint, tint),
	OPERATOR	8		<@ (tint, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tint, box),
	OPERATOR	28		&<# (tint, tint),
	OPERATOR	28		&<# (tint, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tint, box),
	OPERATOR	29		<<# (tint, tint),
	OPERATOR	29		<<# (tint, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tint, box),
	OPERATOR	30		#>> (tint, tint),
	OPERATOR	30		#>> (tint, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tint, box),
	OPERATOR	31		#&> (tint, tint),
	OPERATOR	31		#&> (tint, tfloat),
	-- functions
	FUNCTION	1	spgist_tnumber_config(internal, internal),
	FUNCTION	2	spgist_tnumber_choose(internal, internal),
	FUNCTION	3	spgist_tnumber_picksplit(internal, internal),
	FUNCTION	4	spgist_tnumber_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tnumber_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tnumber_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tfloat_ops
	DEFAULT FOR TYPE tfloat USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tfloat, intrange),
	OPERATOR	1		<< (tfloat, floatrange),
	OPERATOR	1		<< (tfloat, box),
	OPERATOR	1		<< (tfloat, tint),
	OPERATOR	1		<< (tfloat, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tfloat, intrange),
	OPERATOR	2		&< (tfloat, floatrange),
	OPERATOR	2		&< (tfloat, box),
	OPERATOR	2		&< (tfloat, tint),
	OPERATOR	2		&< (tfloat, tfloat),
	-- overlaps
	OPERATOR	3		&& (tfloat, intrange),
	OPERATOR	3		&& (tfloat, floatrange),
	OPERATOR	3		&& (tfloat, box),
	OPERATOR	3		&& (tfloat, tint),
	OPERATOR	3		&& (tfloat, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tfloat, intrange),
	OPERATOR	4		&> (tfloat, floatrange),
	OPERATOR	4		&> (tfloat, box),
	OPERATOR	4		&> (tfloat, tint),
	OPERATOR	4		&> (tfloat, tfloat),
	-- strictly right
	OPERATOR	5		>> (tfloat, intrange),
	OPERATOR	5		>> (tfloat, floatrange),
	OPERATOR	5		>> (tfloat, box),
	OPERATOR	5		>> (tfloat, tint),
	OPERATOR	5		>> (tfloat, tfloat),
  	-- same
	OPERATOR	6		~= (tfloat, intrange),
	OPERATOR	6		~= (tfloat, floatrange),
	OPERATOR	6		~= (tfloat, box),
	OPERATOR	6		~= (tfloat, tint),
	OPERATOR	6		~= (tfloat, tfloat),
	-- contains
	OPERATOR	7		@> (tfloat, intrange),
	OPERATOR	7		@> (tfloat, floatrange),
	OPERATOR	7		@> (tfloat, box),
	OPERATOR	7		@> (tfloat, tint),
	OPERATOR	7		@> (tfloat, tfloat),
	-- contained by
	OPERATOR	8		<@ (tfloat, intrange),
	OPERATOR	8		<@ (tfloat, floatrange),
	OPERATOR	8		<@ (tfloat, box),
	OPERATOR	8		<@ (tfloat, tint),
	OPERATOR	8		<@ (tfloat, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tfloat, box),
	OPERATOR	28		&<# (tfloat, tint),
	OPERATOR	28		&<# (tfloat, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tfloat, box),
	OPERATOR	29		<<# (tfloat, tint),
	OPERATOR	29		<<# (tfloat, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tfloat, box),
	OPERATOR	30		#>> (tfloat, tint),
	OPERATOR	30		#>> (tfloat, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tfloat, box),
	OPERATOR	31		#&> (tfloat, tint),
	OPERATOR	31		#&> (tfloat, tfloat),
	-- functions
	FUNCTION	1	spgist_tnumber_config(internal, internal),
	FUNCTION	2	spgist_tnumber_choose(internal, internal),
	FUNCTION	3	spgist_tnumber_picksplit(internal, internal),
	FUNCTION	4	spgist_tnumber_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tnumber_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tnumber_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_ttext_ops
	DEFAULT FOR TYPE ttext USING spgist AS
	-- overlaps
	OPERATOR	3		&& (ttext, period),
	OPERATOR	3		&& (ttext, ttext),	
  	-- same
	OPERATOR	6		~= (ttext, period),
	OPERATOR	6		~= (ttext, ttext),
	-- contains
	OPERATOR	7		@> (ttext, period),
	OPERATOR	7		@> (ttext, ttext),
	-- contained by
	OPERATOR	8		<@ (ttext, period),
	OPERATOR	8		<@ (ttext, ttext),
	-- overlaps or before
	OPERATOR	28		&<# (ttext, period),
	OPERATOR	28		&<# (ttext, ttext),
	-- strictly before
	OPERATOR	29		<<# (ttext, period),
	OPERATOR	29		<<# (ttext, ttext),
	-- strictly after
	OPERATOR	30		#>> (ttext, period),
	OPERATOR	30		#>> (ttext, ttext),
	-- overlaps or after
	OPERATOR	31		#&> (ttext, period),
	OPERATOR	31		#&> (ttext, ttext),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_temporal_inner_consistent(internal, internal),
	FUNCTION	5	spgist_temporal_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_temporal_compress(internal);

/******************************************************************************/