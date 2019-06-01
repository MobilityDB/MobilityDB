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
CREATE FUNCTION gist_tbox_union(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_tbox_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_tbox_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_tbox_same(tbox, tbox, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 


CREATE OPERATOR CLASS gist_tbool_ops
	DEFAULT FOR TYPE tbool USING gist AS
	STORAGE period,
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
	STORAGE tbox,
	-- strictly left
	OPERATOR	1		<< (tint, intrange),
	OPERATOR	1		<< (tint, tbox),
	OPERATOR	1		<< (tint, tint),
	OPERATOR	1		<< (tint, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tint, intrange),
	OPERATOR	2		&< (tint, tbox),
	OPERATOR	2		&< (tint, tint),
	OPERATOR	2		&< (tint, tfloat),
	-- overlaps
	OPERATOR	3		&& (tint, intrange),
	OPERATOR	3		&& (tint, tbox),
	OPERATOR	3		&& (tint, tint),
	OPERATOR	3		&& (tint, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tint, intrange),
	OPERATOR	4		&> (tint, tbox),
	OPERATOR	4		&> (tint, tint),
	OPERATOR	4		&> (tint, tfloat),
	-- strictly right
	OPERATOR	5		>> (tint, intrange),
	OPERATOR	5		>> (tint, tbox),
	OPERATOR	5		>> (tint, tint),
	OPERATOR	5		>> (tint, tfloat),
  	-- same
	OPERATOR	6		~= (tint, intrange),
	OPERATOR	6		~= (tint, tbox),
	OPERATOR	6		~= (tint, tint),
	OPERATOR	6		~= (tint, tfloat),
	-- contains
	OPERATOR	7		@> (tint, intrange),
	OPERATOR	7		@> (tint, tbox),
	OPERATOR	7		@> (tint, tint),
	OPERATOR	7		@> (tint, tfloat),
	-- contained by
	OPERATOR	8		<@ (tint, intrange),
	OPERATOR	8		<@ (tint, tbox),
	OPERATOR	8		<@ (tint, tint),
	OPERATOR	8		<@ (tint, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tint, tbox),
	OPERATOR	28		&<# (tint, tint),
	OPERATOR	28		&<# (tint, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tint, tbox),
	OPERATOR	29		<<# (tint, tint),
	OPERATOR	29		<<# (tint, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tint, tbox),
	OPERATOR	30		#>> (tint, tint),
	OPERATOR	30		#>> (tint, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tint, tbox),
	OPERATOR	31		#&> (tint, tint),
	OPERATOR	31		#&> (tint, tfloat),
	-- functions
	FUNCTION	1	gist_tint_consistent(internal, tint, smallint, oid, internal),
	FUNCTION	2	gist_tbox_union(internal, internal),
	FUNCTION	3	gist_tint_compress(internal),
	FUNCTION	5	gist_tbox_penalty(internal, internal, internal),
	FUNCTION	6	gist_tbox_picksplit(internal, internal),
	FUNCTION	7	gist_tbox_same(tbox, tbox, internal);

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
	STORAGE tbox,
	-- strictly left
	OPERATOR	1		<< (tfloat, floatrange),
	OPERATOR	1		<< (tfloat, tbox),
	OPERATOR	1		<< (tfloat, tint),
	OPERATOR	1		<< (tfloat, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tfloat, floatrange),
	OPERATOR	2		&< (tfloat, tbox),
	OPERATOR	2		&< (tfloat, tint),
	OPERATOR	2		&< (tfloat, tfloat),
	-- overlaps
	OPERATOR	3		&& (tfloat, floatrange),
	OPERATOR	3		&& (tfloat, tbox),
	OPERATOR	3		&& (tfloat, tint),
	OPERATOR	3		&& (tfloat, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tfloat, floatrange),
	OPERATOR	4		&> (tfloat, tbox),
	OPERATOR	4		&> (tfloat, tint),
	OPERATOR	4		&> (tfloat, tfloat),
	-- strictly right
	OPERATOR	5		>> (tfloat, floatrange),
	OPERATOR	5		>> (tfloat, tbox),
	OPERATOR	5		>> (tfloat, tint),
	OPERATOR	5		>> (tfloat, tfloat),
  	-- same
	OPERATOR	6		~= (tfloat, floatrange),
	OPERATOR	6		~= (tfloat, tbox),
	OPERATOR	6		~= (tfloat, tint),
	OPERATOR	6		~= (tfloat, tfloat),
	-- contains
	OPERATOR	7		@> (tfloat, floatrange),
	OPERATOR	7		@> (tfloat, tbox),
	OPERATOR	7		@> (tfloat, tint),
	OPERATOR	7		@> (tfloat, tfloat),
	-- contained by
	OPERATOR	8		<@ (tfloat, floatrange),
	OPERATOR	8		<@ (tfloat, tbox),
	OPERATOR	8		<@ (tfloat, tint),
	OPERATOR	8		<@ (tfloat, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tfloat, tbox),
	OPERATOR	28		&<# (tfloat, tint),
	OPERATOR	28		&<# (tfloat, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tfloat, tbox),
	OPERATOR	29		<<# (tfloat, tint),
	OPERATOR	29		<<# (tfloat, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tfloat, tbox),
	OPERATOR	30		#>> (tfloat, tint),
	OPERATOR	30		#>> (tfloat, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tfloat, tbox),
	OPERATOR	31		#&> (tfloat, tint),
	OPERATOR	31		#&> (tfloat, tfloat),
	-- functions
	FUNCTION	1	gist_tfloat_consistent(internal, tfloat, smallint, oid, internal),
	FUNCTION	2	gist_tbox_union(internal, internal),
	FUNCTION	3	gist_tfloat_compress(internal),
	FUNCTION	5	gist_tbox_penalty(internal, internal, internal),
	FUNCTION	6	gist_tbox_picksplit(internal, internal),
	FUNCTION	7	gist_tbox_same(tbox, tbox, internal);

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
	FUNCTION	1	gist_ttext_consistent(internal, ttext, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_ttext_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/
