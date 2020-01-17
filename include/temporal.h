/*****************************************************************************
 *
 * temporal.h
 *	Basic functions for temporal types of any duration.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>

#include "timetypes.h"

#ifndef USE_FLOAT4_BYVAL
#error Postgres needs to be configured with USE_FLOAT4_BYVAL
#endif

#ifndef USE_FLOAT8_BYVAL
#error Postgres needs to be configured with USE_FLOAT8_BYVAL
#endif

#define EPSILON					1.0E-06

#define MOBDB_LIB_VERSION_STR "MobilityDB 1.0alpha1"
#define MOBDB_PGSQL_VERSION 120
#define MOBDB_PGSQL_VERSION_STR "PostgreSQL 12.0"
#define MOBDB_POSTGIS_VERSION 25
#define MOBDB_POSTGIS_VERSION_STR "PostGIS 2.5"

/*****************************************************************************
 * Duration of temporal types
 *****************************************************************************/

#define TEMPORAL			0
#define TEMPORALINST		1
#define TEMPORALI			2
#define TEMPORALSEQ			3
#define TEMPORALS			4

#define TYPMOD_GET_DURATION(typmod) ((typmod == -1) ? (0) : (typmod & 0x0000000F))

/* Structure for the type array */

struct temporal_duration_struct
{
	char *typename;
	int type;
};

#define DURATION_STRUCT_ARRAY_LEN \
	(sizeof temporal_duration_struct_array/sizeof(struct temporal_duration_struct))

/*****************************************************************************
 * Additional operator strategy numbers used in the GiST and SP-GiST temporal
 * opclasses with respect to those defined in the file stratnum.h
 *****************************************************************************/

#define RTOverBeforeStrategyNumber		28		/* for &<# */
#define RTBeforeStrategyNumber			29		/* for <<# */
#define RTAfterStrategyNumber			30		/* for #>> */
#define RTOverAfterStrategyNumber		31		/* for #&> */
#define RTOverFrontStrategyNumber		32		/* for &</ */
#define RTFrontStrategyNumber			33		/* for <</ */
#define RTBackStrategyNumber			34		/* for />> */
#define RTOverBackStrategyNumber		35		/* for /&> */

/*****************************************************************************
 * Macros for manipulating the 'flags' element
 *****************************************************************************/

#define MOBDB_FLAGS_GET_LINEAR(flags) 		((flags) & 0x01)
/* The following flag is only used for TemporalInst */
#define MOBDB_FLAGS_GET_BYVAL(flags) 			(((flags) & 0x02)>>1)
#define MOBDB_FLAGS_GET_X(flags)			 	(((flags) & 0x04)>>2)
#define MOBDB_FLAGS_GET_Z(flags) 				(((flags) & 0x08)>>3)
#define MOBDB_FLAGS_GET_T(flags) 				(((flags) & 0x10)>>4)
#define MOBDB_FLAGS_GET_GEODETIC(flags) 		(((flags) & 0x20)>>5)

#define MOBDB_FLAGS_SET_LINEAR(flags, value) \
	((flags) = (value) ? ((flags) | 0x01) : ((flags) & 0xFE))
/* The following flag is only used for TemporalInst */
#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
	((flags) = (value) ? ((flags) | 0x02) : ((flags) & 0xFD))
#define MOBDB_FLAGS_SET_X(flags, value) \
	((flags) = (value) ? ((flags) | 0x04) : ((flags) & 0xFB))
#define MOBDB_FLAGS_SET_Z(flags, value) \
	((flags) = (value) ? ((flags) | 0x08) : ((flags) & 0xF7))
#define MOBDB_FLAGS_SET_T(flags, value) \
	((flags) = (value) ? ((flags) | 0x10) : ((flags) & 0xEF))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
	((flags) = (value) ? ((flags) | 0x20) : ((flags) & 0xDF))

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/* TBOX */

typedef struct 
{
	double		xmin;			/* minimum numeric value */
	double		xmax;			/* maximum numeric value */
	TimestampTz	tmin;			/* minimum timestamp */
	TimestampTz	tmax;			/* maximum timestamp */
	int16		flags;			/* flags */
} TBOX;

/* STBOX */

typedef struct 
{
	double		xmin;			/* minimum x value */
	double		xmax;			/* maximum x value */
	double		ymin;			/* minimum y value */
	double		ymax;			/* maximum y value */
	double		zmin;			/* minimum z value */
	double		zmax;			/* maximum z value */
	TimestampTz	tmin;			/* minimum timestamp */
	TimestampTz	tmax;			/* maximum timestamp */
	int16		flags;			/* flags */
} STBOX;

/* Temporal */
 
typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		duration;		/* duration */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID (4 bytes) */
	/* variable-length data follows, if any */
} Temporal;

/* Temporal Instant */
 
typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		duration;		/* duration */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID  (4 bytes) */
	TimestampTz t;				/* time span */
	/* variable-length data follows */
} TemporalInst;

/* Temporal Instant Set */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		duration;		/* duration */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID (4 bytes) */
	int32 		count;			/* number of TemporalInst elements */
	size_t		offsets[1];		/* beginning of variable-length data */
} TemporalI;

/* Temporal Sequence */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		duration;		/* duration */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID (4 bytes) */
	int32 		count;			/* number of TemporalInst elements */
	Period 		period;			/* time span (24 bytes) */
	size_t		offsets[1];		/* beginning of variable-length data */
} TemporalSeq;

/* Temporal Sequence Set */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		duration;		/* duration */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID (4 bytes) */
	int32 		count;			/* number of TemporalSeq elements */
	int32 		totalcount;		/* total number of TemporalInst elements in all TemporalSeq elements */
	size_t		offsets[1];		/* beginning of variable-length data */
} TemporalS;

/* bboxunion - Union type for all types of bounding boxes */

union bboxunion 
{
	Period p;
	TBOX b;
	STBOX g;
} bboxunion;

/* Double2 - Internal type for computing aggregates for temporal numeric types */

typedef struct double2
{
	double		a;
	double		b;
} double2;

/* Double3 - Internal type for computing aggregates for 2D temporal point types */

typedef struct double3
{
	double		a;
	double		b;
	double		c;
} double3;

/* Double4 - Internal type for computing aggregates for 3D temporal point types */

typedef struct double4
{
	double		a;
	double		b;
	double		c;
	double		d;
} double4;

typedef int (*qsort_comparator) (const void *a, const void *b);

/*****************************************************************************
 * fmgr macros temporal types
 *****************************************************************************/

/* TBOX */

#define DatumGetTboxP(X)	((TBOX *) DatumGetPointer(X))
#define TboxPGetDatum(X)	PointerGetDatum(X)
#define PG_GETARG_TBOX_P(n) DatumGetTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_TBOX_P(x) return TboxPGetDatum(x)

/* doubleN */

#define DatumGetDouble2P(X)		((double2 *) DatumGetPointer(X))
#define Double2PGetDatum(X)		PointerGetDatum(X)
#define DatumGetDouble3P(X)		((double3 *) DatumGetPointer(X))
#define Double3PGetDatum(X)		PointerGetDatum(X)
#define DatumGetDouble4P(X)		((double4 *) DatumGetPointer(X))
#define Double4PGetDatum(X)		PointerGetDatum(X)

/* Temporal types */

#define DatumGetTemporal(X)			((Temporal *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalInst(X)		((TemporalInst *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalI(X)		((TemporalI *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalSeq(X)		((TemporalSeq *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalS(X)		((TemporalS *) PG_DETOAST_DATUM(X))

#define PG_GETARG_TEMPORAL(i)		((Temporal *) PG_GETARG_VARLENA_P(i))

#define PG_GETARG_ANYDATUM(i) (get_typlen(get_fn_expr_argtype(fcinfo->flinfo, i)) == -1 ? \
	PointerGetDatum(PG_GETARG_VARLENA_P(i)) : PG_GETARG_DATUM(i))

#define FREE_DATUM(value, valuetypid) \
	do { \
		if (get_typlen_fast(valuetypid) == -1) \
			pfree(DatumGetPointer(value)); \
	} while (0)

/*
 * Define POSTGIS_FREE_IF_COPY_P if POSTGIS is not loaded.
 * This macro is based on PG_FREE_IF_COPY, except that it accepts two pointers.
 * See PG_FREE_IF_COPY comment in src/include/fmgr.h in postgres source code
 * for more details. 
 */
#ifndef POSTGIS_FREE_IF_COPY_P
#define POSTGIS_FREE_IF_COPY_P(ptrsrc, ptrori) \
	do { \
		if ((Pointer) (ptrsrc) != (Pointer) (ptrori)) \
			pfree(ptrsrc); \
	} while (0)
#endif

/*****************************************************************************/

/* Internal functions */

extern Temporal *temporal_copy(Temporal *temp);
extern Temporal *pg_getarg_temporal(Temporal *temp);
extern bool intersection_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **inter1, Temporal **inter2);
extern bool synchronize_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **sync1, Temporal **sync2, bool interpoint);
extern bool linear_interpolation(Oid type);

extern const char *temporal_duration_name(uint8_t type);
extern bool temporal_duration_from_string(const char *str, uint8_t *type);

/* Catalog functions */

extern void temporal_typinfo(Oid temptypid, Oid* valuetypid);

/* Oid functions */

extern Oid range_oid_from_base(Oid valuetypid);
extern Oid temporal_oid_from_base(Oid valuetypid);
extern Oid base_oid_from_temporal(Oid temptypid);
extern bool temporal_type_oid(Oid temptypid);

/* Trajectory functions */

extern bool type_has_precomputed_trajectory(Oid valuetypid);

/* Parameter tests */

extern void ensure_valid_duration(int16 type);
extern void ensure_valid_duration_all(int16 type);
extern void ensure_numrange_type(Oid type);
extern void ensure_temporal_base_type(Oid valuetypid);
extern void ensure_temporal_base_type_all(Oid valuetypid);
extern void ensure_linear_interpolation(Oid valuetypid);
extern void ensure_linear_interpolation_all(Oid valuetypid);
extern void ensure_numeric_base_type(Oid type);
extern void ensure_point_base_type(Oid type);

/* Input/output functions */

extern Datum temporal_in(PG_FUNCTION_ARGS); 
extern Datum temporal_out(PG_FUNCTION_ARGS); 
extern Datum temporal_send(PG_FUNCTION_ARGS); 
extern Datum temporal_recv(PG_FUNCTION_ARGS);
extern Temporal* temporal_read(StringInfo buf, Oid valuetypid);
extern void temporal_write(Temporal* temp, StringInfo buf);

/* Constructor functions */

extern Datum temporalinst_constructor(PG_FUNCTION_ARGS);
extern Datum temporali_constructor(PG_FUNCTION_ARGS);
extern Datum tlinearseq_constructor(PG_FUNCTION_ARGS);
extern Datum temporalseq_constructor(PG_FUNCTION_ARGS);
extern Datum temporals_constructor(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tint_to_tfloat(PG_FUNCTION_ARGS);
extern Datum temporal_to_period(PG_FUNCTION_ARGS);

extern Temporal *tint_to_tfloat_internal(Temporal *temp);

/* Accessor functions */

extern Datum temporal_duration(PG_FUNCTION_ARGS);
extern Datum temporal_interpolation(PG_FUNCTION_ARGS);
extern Datum temporal_mem_size(PG_FUNCTION_ARGS);
extern Datum temporal_get_values(PG_FUNCTION_ARGS);
extern Datum temporal_get_time(PG_FUNCTION_ARGS);
extern Datum temporalinst_get_value(PG_FUNCTION_ARGS);
extern Datum tnumber_to_tbox(PG_FUNCTION_ARGS);
extern Datum tnumber_value_range(PG_FUNCTION_ARGS);
extern Datum temporal_start_value(PG_FUNCTION_ARGS);
extern Datum temporal_end_value(PG_FUNCTION_ARGS);
extern Datum temporal_min_value(PG_FUNCTION_ARGS);
extern Datum temporal_max_value(PG_FUNCTION_ARGS);
extern Datum temporal_time(PG_FUNCTION_ARGS);
extern Datum temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum temporal_instants(PG_FUNCTION_ARGS);
extern Datum temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum temporal_shift(PG_FUNCTION_ARGS);

extern Datum temporal_ever_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ne(PG_FUNCTION_ARGS);
extern Datum temporal_ever_lt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_le(PG_FUNCTION_ARGS);
extern Datum temporal_ever_gt(PG_FUNCTION_ARGS);
extern Datum temporal_ever_ge(PG_FUNCTION_ARGS);

extern Datum temporal_always_eq(PG_FUNCTION_ARGS);
extern Datum temporal_always_ne(PG_FUNCTION_ARGS);
extern Datum temporal_always_lt(PG_FUNCTION_ARGS);
extern Datum temporal_always_le(PG_FUNCTION_ARGS);
extern Datum temporal_always_gt(PG_FUNCTION_ARGS);
extern Datum temporal_always_ge(PG_FUNCTION_ARGS);

extern Datum temporal_get_values_internal(Temporal *temp);
extern PeriodSet *temporal_get_time_internal(Temporal *temp);
extern Datum tfloat_ranges(Temporal *temp);
extern Datum temporal_min_value_internal(Temporal *temp);
extern TimestampTz temporal_start_timestamp_internal(Temporal *temp);
extern RangeType *tnumber_value_range_internal(Temporal *temp);
extern bool temporal_ever_eq_internal(Temporal *temp, Datum value);

/* Restriction functions */

extern Datum temporal_at_value(PG_FUNCTION_ARGS);
extern Datum temporal_minus_value(PG_FUNCTION_ARGS);
extern Datum temporal_at_values(PG_FUNCTION_ARGS);
extern Datum tnumber_at_range(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_range(PG_FUNCTION_ARGS);
extern Datum tnumber_at_ranges(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_ranges(PG_FUNCTION_ARGS);
extern Datum temporal_at_min(PG_FUNCTION_ARGS);
extern Datum temporal_minus_min(PG_FUNCTION_ARGS);
extern Datum temporal_at_max(PG_FUNCTION_ARGS);
extern Datum temporal_minus_max(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_value_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_at_period(PG_FUNCTION_ARGS);
extern Datum temporal_minus_period(PG_FUNCTION_ARGS);
extern Datum temporal_at_periodset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_periodset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_period(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_periodset(PG_FUNCTION_ARGS);
 
extern Temporal *temporal_at_min_internal(Temporal *temp);
extern TemporalInst *temporal_at_timestamp_internal(Temporal *temp, TimestampTz t);
extern Temporal *temporal_at_periodset_internal(Temporal *temp, PeriodSet *ps);
extern void temporal_period(Period *p, Temporal *temp);
extern char *temporal_to_string(Temporal *temp, char *(*value_out)(Oid, Datum));
extern void temporal_bbox(void *box, const Temporal *temp);

/* Comparison functions */

extern Datum temporal_lt(PG_FUNCTION_ARGS);
extern Datum temporal_le(PG_FUNCTION_ARGS);
extern Datum temporal_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ge(PG_FUNCTION_ARGS);
extern Datum temporal_gt(PG_FUNCTION_ARGS);
extern Datum temporal_cmp(PG_FUNCTION_ARGS);
extern Datum temporal_hash(PG_FUNCTION_ARGS);

extern uint32 temporal_hash_internal(const Temporal *temp);

/*****************************************************************************/

#endif
