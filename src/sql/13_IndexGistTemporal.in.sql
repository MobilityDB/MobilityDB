/*****************************************************************************
 *
 * IndexGistTemporal.sql
 *		R-tree GiST index for temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tbool_consistent(internal, tbool, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_temporal_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tbool_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_temporal_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tbool_ops
	DEFAULT FOR TYPE tbool USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (tbool, timestamptz),
	OPERATOR	3		&& (tbool, timestampset),
	OPERATOR	3		&& (tbool, period),
	OPERATOR	3		&& (tbool, periodset),
	OPERATOR	3		&& (tbool, tbool),	
  	-- same
	OPERATOR	6		~= (tbool, timestamptz),
	OPERATOR	6		~= (tbool, timestampset),
	OPERATOR	6		~= (tbool, period),
	OPERATOR	6		~= (tbool, periodset),
	OPERATOR	6		~= (tbool, tbool),
	-- contains
	OPERATOR	7		@> (tbool, timestamptz),
	OPERATOR	7		@> (tbool, timestampset),
	OPERATOR	7		@> (tbool, period),
	OPERATOR	7		@> (tbool, periodset),
	OPERATOR	7		@> (tbool, tbool),
	-- contained by
	OPERATOR	8		<@ (tbool, timestamptz),
	OPERATOR	8		<@ (tbool, timestampset),
	OPERATOR	8		<@ (tbool, period),
	OPERATOR	8		<@ (tbool, periodset),
	OPERATOR	8		<@ (tbool, tbool),
	-- overlaps or before
	OPERATOR	28		&<# (tbool, timestamptz),
	OPERATOR	28		&<# (tbool, timestampset),
	OPERATOR	28		&<# (tbool, period),
	OPERATOR	28		&<# (tbool, periodset),
	OPERATOR	28		&<# (tbool, tbool),
	-- strictly before
	OPERATOR	29		<<# (tbool, timestamptz),
	OPERATOR	29		<<# (tbool, timestampset),
	OPERATOR	29		<<# (tbool, period),
	OPERATOR	29		<<# (tbool, periodset),
	OPERATOR	29		<<# (tbool, tbool),
	-- strictly after
	OPERATOR	30		#>> (tbool, timestamptz),
	OPERATOR	30		#>> (tbool, timestampset),
	OPERATOR	30		#>> (tbool, period),
	OPERATOR	30		#>> (tbool, periodset),
	OPERATOR	30		#>> (tbool, tbool),
	-- overlaps or after
	OPERATOR	31		#&> (tbool, timestamptz),
	OPERATOR	31		#&> (tbool, timestampset),
	OPERATOR	31		#&> (tbool, period),
	OPERATOR	31		#&> (tbool, periodset),
	OPERATOR	31		#&> (tbool, tbool),
	-- functions
	FUNCTION	1	gist_tbool_consistent(internal, tbool, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_tbool_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/

CREATE FUNCTION gist_tint_consistent(internal, tint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tnumber_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tnumber_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tint_ops
	DEFAULT FOR TYPE tint USING gist AS
	STORAGE box,
	-- strictly left
	OPERATOR	1		<< (tint, integer),
	OPERATOR	1		<< (tint, intrange),
	OPERATOR	1		<< (tint, float),
	OPERATOR	1		<< (tint, floatrange),
	OPERATOR	1		<< (tint, box),
	OPERATOR	1		<< (tint, tint),
	OPERATOR	1		<< (tint, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tint, integer),
	OPERATOR	2		&< (tint, intrange),
	OPERATOR	2		&< (tint, float),
	OPERATOR	2		&< (tint, floatrange),
	OPERATOR	2		&< (tint, box),
	OPERATOR	2		&< (tint, tint),
	OPERATOR	2		&< (tint, tfloat),
	-- overlaps
	OPERATOR	3		&& (tint, integer),
	OPERATOR	3		&& (tint, intrange),
	OPERATOR	3		&& (tint, float),
	OPERATOR	3		&& (tint, floatrange),
	OPERATOR	3		&& (tint, timestamptz),
	OPERATOR	3		&& (tint, timestampset),
	OPERATOR	3		&& (tint, period),
	OPERATOR	3		&& (tint, periodset),
	OPERATOR	3		&& (tint, box),
	OPERATOR	3		&& (tint, tint),
	OPERATOR	3		&& (tint, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tint, integer),
	OPERATOR	4		&> (tint, intrange),
	OPERATOR	4		&> (tint, float),
	OPERATOR	4		&> (tint, floatrange),
	OPERATOR	4		&> (tint, box),
	OPERATOR	4		&> (tint, tint),
	OPERATOR	4		&> (tint, tfloat),
	-- strictly right
	OPERATOR	5		>> (tint, integer),
	OPERATOR	5		>> (tint, intrange),
	OPERATOR	5		>> (tint, float),
	OPERATOR	5		>> (tint, floatrange),
	OPERATOR	5		>> (tint, box),
	OPERATOR	5		>> (tint, tint),
	OPERATOR	5		>> (tint, tfloat),
  	-- same
	OPERATOR	6		~= (tint, integer),
	OPERATOR	6		~= (tint, intrange),
	OPERATOR	6		~= (tint, float),
	OPERATOR	6		~= (tint, floatrange),
	OPERATOR	6		~= (tint, timestamptz),
	OPERATOR	6		~= (tint, timestampset),
	OPERATOR	6		~= (tint, period),
	OPERATOR	6		~= (tint, periodset),
	OPERATOR	6		~= (tint, box),
	OPERATOR	6		~= (tint, tint),
	OPERATOR	6		~= (tint, tfloat),
	-- contains
	OPERATOR	7		@> (tint, integer),
	OPERATOR	7		@> (tint, intrange),
	OPERATOR	7		@> (tint, float),
	OPERATOR	7		@> (tint, floatrange),
	OPERATOR	7		@> (tint, timestamptz),
	OPERATOR	7		@> (tint, timestampset),
	OPERATOR	7		@> (tint, period),
	OPERATOR	7		@> (tint, periodset),
	OPERATOR	7		@> (tint, box),
	OPERATOR	7		@> (tint, tint),
	OPERATOR	7		@> (tint, tfloat),
	-- contained by
	OPERATOR	8		<@ (tint, integer),
	OPERATOR	8		<@ (tint, intrange),
	OPERATOR	8		<@ (tint, float),
	OPERATOR	8		<@ (tint, floatrange),
	OPERATOR	8		<@ (tint, timestamptz),
	OPERATOR	8		<@ (tint, timestampset),
	OPERATOR	8		<@ (tint, period),
	OPERATOR	8		<@ (tint, periodset),
	OPERATOR	8		<@ (tint, box),
	OPERATOR	8		<@ (tint, tint),
	OPERATOR	8		<@ (tint, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tint, timestamptz),
	OPERATOR	28		&<# (tint, timestampset),
	OPERATOR	28		&<# (tint, period),
	OPERATOR	28		&<# (tint, periodset),
	OPERATOR	28		&<# (tint, box),
	OPERATOR	28		&<# (tint, tint),
	OPERATOR	28		&<# (tint, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tint, timestamptz),
	OPERATOR	29		<<# (tint, timestampset),
	OPERATOR	29		<<# (tint, period),
	OPERATOR	29		<<# (tint, periodset),
	OPERATOR	29		<<# (tint, box),
	OPERATOR	29		<<# (tint, tint),
	OPERATOR	29		<<# (tint, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tint, timestamptz),
	OPERATOR	30		#>> (tint, timestampset),
	OPERATOR	30		#>> (tint, period),
	OPERATOR	30		#>> (tint, periodset),
	OPERATOR	30		#>> (tint, box),
	OPERATOR	30		#>> (tint, tint),
	OPERATOR	30		#>> (tint, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tint, timestamptz),
	OPERATOR	31		#&> (tint, timestampset),
	OPERATOR	31		#&> (tint, period),
	OPERATOR	31		#&> (tint, periodset),
	OPERATOR	31		#&> (tint, box),
	OPERATOR	31		#&> (tint, tint),
	OPERATOR	31		#&> (tint, tfloat),
	-- functions
	FUNCTION	1	gist_tint_consistent(internal, tint, smallint, oid, internal),
	FUNCTION	2	gist_box_union(internal, internal),
	FUNCTION	3	gist_tint_compress(internal),
	FUNCTION	5	gist_box_penalty(internal, internal, internal),
	FUNCTION	6	gist_box_picksplit(internal, internal),
	FUNCTION	7	gist_box_same(box, box, internal);

/******************************************************************************/

CREATE FUNCTION gist_tfloat_consistent(internal, tfloat, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tnumber_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tfloat_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tnumber_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tfloat_ops
	DEFAULT FOR TYPE tfloat USING gist AS
	STORAGE box,
	-- strictly left
	OPERATOR	1		<< (tfloat, integer),
	OPERATOR	1		<< (tfloat, intrange),
	OPERATOR	1		<< (tfloat, float),
	OPERATOR	1		<< (tfloat, floatrange),
	OPERATOR	1		<< (tfloat, box),
	OPERATOR	1		<< (tfloat, tint),
	OPERATOR	1		<< (tfloat, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tfloat, integer),
	OPERATOR	2		&< (tfloat, intrange),
	OPERATOR	2		&< (tfloat, float),
	OPERATOR	2		&< (tfloat, floatrange),
	OPERATOR	2		&< (tfloat, box),
	OPERATOR	2		&< (tfloat, tint),
	OPERATOR	2		&< (tfloat, tfloat),
	-- overlaps
	OPERATOR	3		&& (tfloat, integer),
	OPERATOR	3		&& (tfloat, intrange),
	OPERATOR	3		&& (tfloat, float),
	OPERATOR	3		&& (tfloat, floatrange),
	OPERATOR	3		&& (tfloat, timestamptz),
	OPERATOR	3		&& (tfloat, timestampset),
	OPERATOR	3		&& (tfloat, period),
	OPERATOR	3		&& (tfloat, periodset),
	OPERATOR	3		&& (tfloat, box),
	OPERATOR	3		&& (tfloat, tint),
	OPERATOR	3		&& (tfloat, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tfloat, integer),
	OPERATOR	4		&> (tfloat, intrange),
	OPERATOR	4		&> (tfloat, float),
	OPERATOR	4		&> (tfloat, floatrange),
	OPERATOR	4		&> (tfloat, box),
	OPERATOR	4		&> (tfloat, tint),
	OPERATOR	4		&> (tfloat, tfloat),
	-- strictly right
	OPERATOR	5		>> (tfloat, integer),
	OPERATOR	5		>> (tfloat, intrange),
	OPERATOR	5		>> (tfloat, float),
	OPERATOR	5		>> (tfloat, floatrange),
	OPERATOR	5		>> (tfloat, box),
	OPERATOR	5		>> (tfloat, tint),
	OPERATOR	5		>> (tfloat, tfloat),
  	-- same
	OPERATOR	6		~= (tfloat, integer),
	OPERATOR	6		~= (tfloat, intrange),
	OPERATOR	6		~= (tfloat, float),
	OPERATOR	6		~= (tfloat, floatrange),
	OPERATOR	6		~= (tfloat, timestamptz),
	OPERATOR	6		~= (tfloat, timestampset),
	OPERATOR	6		~= (tfloat, period),
	OPERATOR	6		~= (tfloat, periodset),
	OPERATOR	6		~= (tfloat, box),
	OPERATOR	6		~= (tfloat, tint),
	OPERATOR	6		~= (tfloat, tfloat),
	-- contains
	OPERATOR	7		@> (tfloat, integer),
	OPERATOR	7		@> (tfloat, intrange),
	OPERATOR	7		@> (tfloat, float),
	OPERATOR	7		@> (tfloat, floatrange),
	OPERATOR	7		@> (tfloat, timestamptz),
	OPERATOR	7		@> (tfloat, timestampset),
	OPERATOR	7		@> (tfloat, period),
	OPERATOR	7		@> (tfloat, periodset),
	OPERATOR	7		@> (tfloat, box),
	OPERATOR	7		@> (tfloat, tint),
	OPERATOR	7		@> (tfloat, tfloat),
	-- contained by
	OPERATOR	8		<@ (tfloat, integer),
	OPERATOR	8		<@ (tfloat, intrange),
	OPERATOR	8		<@ (tfloat, float),
	OPERATOR	8		<@ (tfloat, floatrange),
	OPERATOR	8		<@ (tfloat, timestamptz),
	OPERATOR	8		<@ (tfloat, timestampset),
	OPERATOR	8		<@ (tfloat, period),
	OPERATOR	8		<@ (tfloat, periodset),
	OPERATOR	8		<@ (tfloat, box),
	OPERATOR	8		<@ (tfloat, tint),
	OPERATOR	8		<@ (tfloat, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tfloat, timestamptz),
	OPERATOR	28		&<# (tfloat, timestampset),
	OPERATOR	28		&<# (tfloat, period),
	OPERATOR	28		&<# (tfloat, periodset),
	OPERATOR	28		&<# (tfloat, box),
	OPERATOR	28		&<# (tfloat, tint),
	OPERATOR	28		&<# (tfloat, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tfloat, timestamptz),
	OPERATOR	29		<<# (tfloat, timestampset),
	OPERATOR	29		<<# (tfloat, period),
	OPERATOR	29		<<# (tfloat, periodset),
	OPERATOR	29		<<# (tfloat, box),
	OPERATOR	29		<<# (tfloat, tint),
	OPERATOR	29		<<# (tfloat, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tfloat, timestamptz),
	OPERATOR	30		#>> (tfloat, timestampset),
	OPERATOR	30		#>> (tfloat, period),
	OPERATOR	30		#>> (tfloat, periodset),
	OPERATOR	30		#>> (tfloat, box),
	OPERATOR	30		#>> (tfloat, tint),
	OPERATOR	30		#>> (tfloat, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tfloat, timestamptz),
	OPERATOR	31		#&> (tfloat, timestampset),
	OPERATOR	31		#&> (tfloat, period),
	OPERATOR	31		#&> (tfloat, periodset),
	OPERATOR	31		#&> (tfloat, box),
	OPERATOR	31		#&> (tfloat, tint),
	OPERATOR	31		#&> (tfloat, tfloat),
	-- functions
	FUNCTION	1	gist_tfloat_consistent(internal, tfloat, smallint, oid, internal),
	FUNCTION	2	gist_box_union(internal, internal),
	FUNCTION	3	gist_tfloat_compress(internal),
	FUNCTION	5	gist_box_penalty(internal, internal, internal),
	FUNCTION	6	gist_box_picksplit(internal, internal),
	FUNCTION	7	gist_box_same(box, box, internal);

/******************************************************************************/

CREATE FUNCTION gist_ttext_consistent(internal, ttext, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_temporal_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_ttext_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_temporal_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_ttext_ops
	DEFAULT FOR TYPE ttext USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (ttext, timestamptz),
	OPERATOR	3		&& (ttext, timestampset),
	OPERATOR	3		&& (ttext, period),
	OPERATOR	3		&& (ttext, periodset),
	OPERATOR	3		&& (ttext, ttext),	
  	-- same
	OPERATOR	6		~= (ttext, timestamptz),
	OPERATOR	6		~= (ttext, timestampset),
	OPERATOR	6		~= (ttext, period),
	OPERATOR	6		~= (ttext, periodset),
	OPERATOR	6		~= (ttext, ttext),
	-- contains
	OPERATOR	7		@> (ttext, timestamptz),
	OPERATOR	7		@> (ttext, timestampset),
	OPERATOR	7		@> (ttext, period),
	OPERATOR	7		@> (ttext, periodset),
	OPERATOR	7		@> (ttext, ttext),
	-- contained by
	OPERATOR	8		<@ (ttext, timestamptz),
	OPERATOR	8		<@ (ttext, timestampset),
	OPERATOR	8		<@ (ttext, period),
	OPERATOR	8		<@ (ttext, periodset),
	OPERATOR	8		<@ (ttext, ttext),
	-- overlaps or before
	OPERATOR	28		&<# (ttext, timestamptz),
	OPERATOR	28		&<# (ttext, timestampset),
	OPERATOR	28		&<# (ttext, period),
	OPERATOR	28		&<# (ttext, periodset),
	OPERATOR	28		&<# (ttext, ttext),
	-- strictly before
	OPERATOR	29		<<# (ttext, timestamptz),
	OPERATOR	29		<<# (ttext, timestampset),
	OPERATOR	29		<<# (ttext, period),
	OPERATOR	29		<<# (ttext, periodset),
	OPERATOR	29		<<# (ttext, ttext),
	-- strictly after
	OPERATOR	30		#>> (ttext, timestamptz),
	OPERATOR	30		#>> (ttext, timestampset),
	OPERATOR	30		#>> (ttext, period),
	OPERATOR	30		#>> (ttext, periodset),
	OPERATOR	30		#>> (ttext, ttext),
	-- overlaps or after
	OPERATOR	31		#&> (ttext, timestamptz),
	OPERATOR	31		#&> (ttext, timestampset),
	OPERATOR	31		#&> (ttext, period),
	OPERATOR	31		#&> (ttext, periodset),
	OPERATOR	31		#&> (ttext, ttext),
	-- functions
	FUNCTION	1	gist_ttext_consistent(internal, ttext, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_ttext_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/
