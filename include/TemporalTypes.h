/*****************************************************************************
 *
 * TemporalTypes.h
 *	  Functions for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALTYPES_H__
#define __TEMPORALTYPES_H__

#include "TimeTypes.h"

#ifndef USE_FLOAT4_BYVAL
#error Postgres needs to be configured with USE_FLOAT4_BYVAL
#endif

#ifndef USE_FLOAT8_BYVAL
#error Postgres needs to be configured with USE_FLOAT8_BYVAL
#endif

#define EPSILON					1.0E-06

/*****************************************************************************
 * Duration of temporal types
 *****************************************************************************/

#define TEMPORAL			0
#define TEMPORALINST			1
#define TEMPORALI			2
#define TEMPORALSEQ			3
#define TEMPORALS			4

#define TYPMOD_GET_DURATION(typmod) ((typmod == -1)? (0) : (typmod & 0x0000000F))

/* Structure for the type array */

struct temporaltype_struct
{
	char *typename;
	int type;
};

#define TEMPORALTYPE_STRUCT_ARRAY_LEN \
	(sizeof temporaltype_struct_array/sizeof(struct temporaltype_struct))

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

#define MOBDB_FLAGS_GET_CONTINUOUS(flags) 		((flags) & 0x01)
/* Only for TemporalInst */
#define MOBDB_FLAGS_GET_BYVAL(flags) 			(((flags) & 0x02)>>1)
/* Only for TemporalS */
#define MOBDB_FLAGS_GET_TEMPCONTINUOUS(flags) 	(((flags) & 0x04)>>2)
#define MOBDB_FLAGS_GET_X(flags) 				(((flags) & 0x08)>>3)
#define MOBDB_FLAGS_GET_Z(flags) 				(((flags) & 0x10)>>4)
#define MOBDB_FLAGS_GET_GEODETIC(flags) 		(((flags) & 0x20)>>5)
#define MOBDB_FLAGS_GET_T(flags) 				(((flags) & 0x40)>>6)

#define MOBDB_FLAGS_SET_CONTINUOUS(flags, value) \
	((flags) = (value) ? ((flags) | 0x01) : ((flags) & 0xFE))
/* Only for TemporalInst */
#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
	((flags) = (value) ? ((flags) | 0x02) : ((flags) & 0xFD))
/* Only for TemporalS */
#define MOBDB_FLAGS_SET_TEMPCONTINUOUS(flags, value) \
	((flags) = (value) ? ((flags) | 0x04) : ((flags) & 0xFB))
#define MOBDB_FLAGS_SET_X(flags, value) \
	((flags) = (value) ? ((flags) | 0x08) : ((flags) & 0xF7))
#define MOBDB_FLAGS_SET_Z(flags, value) \
	((flags) = (value) ? ((flags) | 0x10) : ((flags) & 0xEF))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
	((flags) = (value) ? ((flags) | 0x20) : ((flags) & 0xDF))
#define MOBDB_FLAGS_SET_T(flags, value) \
	((flags) = (value) ? ((flags) | 0x40) : ((flags) & 0xBF))

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/* TBOX */

typedef struct 
{
	double		xmin;			/* minimum numeric value */
	double		xmax;			/* maximum numeric value */
	double		tmin;			/* minimum timestamp */
	double		tmax;			/* maximum timestamp */
	int32		flags;			/* flags */
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
	double		tmin;			/* minimum timestamp */
	double		tmax;			/* maximum timestamp */
	int32		flags;			/* flags */
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
	/* variable-length data follows */
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
	/* variable-length data follows */
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
	/* variable-length data follows */
} TemporalS;

/* bboxunion - Union type for all types of bounding boxes */

union bboxunion {
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

typedef struct AggregateState
{
	int 		size;
	void		*extra;
	size_t      extrasize;
	Temporal 	*values[];
} AggregateState;

typedef int (*qsort_comparator) (const void *a, const void *b);

/*****************************************************************************
 * fmgr macros temporal types
 *****************************************************************************/

/* TBOX */

#define DatumGetTboxP(X)    ((TBOX *) DatumGetPointer(X))
#define TboxPGetDatum(X)    PointerGetDatum(X)
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

#define DatumGetTemporal(X)		((Temporal *) PG_DETOAST_DATUM(X))
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

#include "Temporal.h"
#include "TemporalInst.h"
#include "TemporalI.h"
#include "TemporalSeq.h"
#include "TemporalS.h"
#include "TemporalUtil.h"
#include "OidCache.h"

/*****************************************************************************/

#endif
