/*****************************************************************************
 *
 * IndexSPGistTime.sql
 *		Quad-tree SP-GiST index for time types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_time_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_timestampset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_periodset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_timestampset_ops
	DEFAULT FOR TYPE timestampset USING spgist AS
--	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (timestampset, timestampset),
	OPERATOR	3		&& (timestampset, period),
	OPERATOR	3		&& (timestampset, periodset),
	-- contains
	OPERATOR	7		@> (timestampset, timestamptz),
	OPERATOR	7		@> (timestampset, timestampset),
	-- contained by
	OPERATOR	8		<@ (timestampset, timestampset),
	OPERATOR	8		<@ (timestampset, period),
	OPERATOR	8		<@ (timestampset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (timestampset, timestamptz),
	OPERATOR	28		&<# (timestampset, timestampset),
	OPERATOR	28		&<# (timestampset, period),
	OPERATOR	28		&<# (timestampset, periodset),
	-- strictly before
	OPERATOR	29		<<# (timestampset, timestamptz),
	OPERATOR	29		<<# (timestampset, timestampset),
	OPERATOR	29		<<# (timestampset, period),
	OPERATOR	29		<<# (timestampset, periodset),
	-- strictly after
	OPERATOR	30		#>> (timestampset, timestamptz),
	OPERATOR	30		#>> (timestampset, timestampset),
	OPERATOR	30		#>> (timestampset, period),
	OPERATOR	30		#>> (timestampset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (timestampset, timestamptz),
	OPERATOR	31		#&> (timestampset, timestampset),
	OPERATOR	31		#&> (timestampset, period),
	OPERATOR	31		#&> (timestampset, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_timestampset_compress(internal);
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_period_ops
	DEFAULT FOR TYPE period USING spgist AS
	-- overlaps
	OPERATOR	3		&& (period, timestampset),
	OPERATOR	3		&& (period, period),
	OPERATOR	3		&& (period, periodset),
	-- contains
	OPERATOR	7		@> (period, timestamptz),
	OPERATOR	7		@> (period, timestampset),
	OPERATOR	7		@> (period, period),
	OPERATOR	7		@> (period, periodset),
	-- contained by
	OPERATOR	8		<@ (period, period),
	OPERATOR	8		<@ (period, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (period, timestamptz),
	OPERATOR	28		&<# (period, timestampset),
	OPERATOR	28		&<# (period, period),
	OPERATOR	28		&<# (period, periodset),
	-- strictly before
	OPERATOR	29		<<# (period, timestamptz),
	OPERATOR	29		<<# (period, timestampset),
	OPERATOR	29		<<# (period, period),
	OPERATOR	29		<<# (period, periodset),
	-- strictly after
	OPERATOR	30		#>> (period, timestamptz),
	OPERATOR	30		#>> (period, timestampset),
	OPERATOR	30		#>> (period, period),
	OPERATOR	30		#>> (period, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (period, timestamptz),
	OPERATOR	31		#&> (period, timestampset),
	OPERATOR	31		#&> (period, period),
	OPERATOR	31		#&> (period, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal);
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_periodset_ops
	DEFAULT FOR TYPE periodset USING spgist AS
--	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (periodset, timestampset),
	OPERATOR	3		&& (periodset, period),
	OPERATOR	3		&& (periodset, periodset),
	-- contains
	OPERATOR	7		@> (periodset, timestamptz),
	OPERATOR	7		@> (periodset, timestampset),
	OPERATOR	7		@> (periodset, period),
	OPERATOR	7		@> (periodset, periodset),
	-- contained by
	OPERATOR	8		<@ (periodset, period),
	OPERATOR	8		<@ (periodset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (periodset, timestamptz),
	OPERATOR	28		&<# (periodset, timestampset),
	OPERATOR	28		&<# (periodset, period),
	OPERATOR	28		&<# (periodset, periodset),
	-- strictly before
	OPERATOR	29		<<# (periodset, timestamptz),
	OPERATOR	29		<<# (periodset, timestampset),
	OPERATOR	29		<<# (periodset, period),
	OPERATOR	29		<<# (periodset, periodset),
	-- strictly after
	OPERATOR	30		#>> (periodset, timestamptz),
	OPERATOR	30		#>> (periodset, timestampset),
	OPERATOR	30		#>> (periodset, period),
	OPERATOR	30		#>> (periodset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (periodset, timestamptz),
	OPERATOR	31		#&> (periodset, timestampset),
	OPERATOR	31		#&> (periodset, period),
	OPERATOR	31		#&> (periodset, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_periodset_compress(internal);

/******************************************************************************/
