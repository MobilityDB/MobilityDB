/*****************************************************************************
 *
 * time_gist.sql
 *		R-tree GiST index for time types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_timestampset_consistent(internal, timestampset, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_period_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_period_union(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_timestampset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_timestampset_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_period_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_period_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_period_same(period, period, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_period_fetch(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR CLASS gist_timestampset_ops
	DEFAULT FOR TYPE timestampset USING gist AS
	STORAGE period,
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
	FUNCTION	1	gist_timestampset_consistent(internal, timestampset, smallint, oid, internal),
	FUNCTION	2	gist_period_union(internal, internal),
	FUNCTION	3	gist_timestampset_compress(internal),
	FUNCTION	5	gist_period_penalty(internal, internal, internal),
	FUNCTION	6	gist_period_picksplit(internal, internal),
	FUNCTION	7	gist_period_same(period, period, internal);
	
/******************************************************************************/

CREATE FUNCTION gist_period_consistent(internal, period, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_period_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_period_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_period_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_period_ops
	DEFAULT FOR TYPE period USING gist AS
	STORAGE period,
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
	FUNCTION	1	gist_period_consistent(internal, period, smallint, oid, internal),
	FUNCTION	2	gist_period_union(internal, internal),
	FUNCTION	3	gist_period_compress(internal),
	FUNCTION	5	gist_period_penalty(internal, internal, internal),
	FUNCTION	6	gist_period_picksplit(internal, internal),
	FUNCTION	7	gist_period_same(period, period, internal),
	FUNCTION	9	gist_period_fetch(internal);
	
/******************************************************************************/

CREATE FUNCTION gist_periodset_consistent(internal, periodset, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_period_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_periodset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_periodset_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_periodset_ops
	DEFAULT FOR TYPE periodset USING gist AS
	STORAGE period,
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
	FUNCTION	1	gist_periodset_consistent(internal, periodset, smallint, oid, internal),
	FUNCTION	2	gist_period_union(internal, internal),
	FUNCTION	3	gist_periodset_compress(internal),
	FUNCTION	5	gist_period_penalty(internal, internal, internal),
	FUNCTION	6	gist_period_picksplit(internal, internal),
	FUNCTION	7	gist_period_same(period, period, internal);

/******************************************************************************/
