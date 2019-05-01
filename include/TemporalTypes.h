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

#include <postgres.h>
#include <c.h>
#include <float.h>
#include <fmgr.h>
#include <funcapi.h>
#include <liblwgeom.h>
#include <math.h>
#include <access/gist.h>
#include <access/hash.h>
#include <access/heapam.h>
#include <access/htup_details.h>
#include <access/spgist.h>
#include <access/stratnum.h>
#include <catalog/namespace.h>
#include <catalog/pg_operator.h>
#include <catalog/pg_type.h>
#include <catalog/pg_collation.h>
#include <catalog/pg_statistic.h>
#include <commands/vacuum.h>
#include <lib/stringinfo.h>
#include <libpq/pqformat.h>
#include <nodes/pg_list.h>
#include <parser/parse_oper.h>
#include <utils/array.h>
#include <utils/builtins.h>
#include <utils/date.h>
#include <utils/datetime.h>
#include <utils/datum.h>
#include <utils/fmgroids.h>
#include <utils/geo_decls.h>
#include <utils/lsyscache.h>
#include <utils/memutils.h>
#include <utils/rangetypes.h>
#include <utils/rel.h>
#include <utils/selfuncs.h>
#include "utils/syscache.h"
#include <utils/timestamp.h>
#include <utils/varlena.h>
#include "TimeTypes.h"
#include "OidCache.h"
#include "TemporalSelFuncs.h"

#ifndef USE_FLOAT4_BYVAL
#error Postgres needs to be configured with USE_FLOAT4_BYVAL
#endif

#ifndef USE_FLOAT8_BYVAL
#error Postgres needs to be configured with USE_FLOAT8_BYVAL
#endif

/*****************************************************************************
 * Type of the temporal types
 *****************************************************************************/

#define TEMPORAL			0
#define TEMPORALINST		1
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
 * Struct definitions
 *****************************************************************************/

/* Macros for manipulating the 'flags' element. */

#define MOBDB_FLAGS_GET_CONTINUOUS(flags) 		((flags) & 0x01)
/* Only for TemporalInst */
#define MOBDB_FLAGS_GET_BYVAL(flags) 			(((flags) & 0x02)>>1)
/* Only for TemporalS */
#define MOBDB_FLAGS_GET_TEMPCONTINUOUS(flags) 	(((flags) & 0x04)>>2)
#define MOBDB_FLAGS_GET_Z(flags) 				(((flags) & 0x08)>>3)
#define MOBDB_FLAGS_GET_GEODETIC(flags) 		(((flags) & 0x10)>>4)

#define MOBDB_FLAGS_SET_CONTINUOUS(flags, value) \
	((flags) = (value) ? ((flags) | 0x01) : ((flags) & 0xFE))
/* Only for TemporalInst */
#define MOBDB_FLAGS_SET_BYVAL(flags, value) \
	((flags) = (value) ? ((flags) | 0x02) : ((flags) & 0xFD))
/* Only for TemporalS */
#define MOBDB_FLAGS_SET_TEMPCONTINUOUS(flags, value) \
	((flags) = (value) ? ((flags) | 0x04) : ((flags) & 0xFB))
#define MOBDB_FLAGS_SET_Z(flags, value) \
	((flags) = (value) ? ((flags) | 0x08) : ((flags) & 0xF7))
#define MOBDB_FLAGS_SET_GEODETIC(flags, value) \
	((flags) = (value) ? ((flags) | 0x10) : ((flags) & 0xEF))

/* Temporal */
 
typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		type;			/* type */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID */
	/* variable-length data follows, if any */
} Temporal;

/* Temporal Instant */
 
typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		type;			/* type */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID */
	TimestampTz t;				/* time span */
	/* variable-length data follows */
} TemporalInst;

/* Temporal Set Instant */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		type;			/* type */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID */
	int32 		count;			/* number of TemporalInst elements */
	/* variable-length data follows */
} TemporalI;

/* Temporal Sequence */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		type;			/* type */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID */
	int32 		count;			/* number of TemporalInst elements */
	Period 		period;			/* time span */
	/* variable-length data follows */
} TemporalSeq;

/* Temporal Set Sequence */

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int16		type;			/* type */
	int16		flags;			/* flags */
	Oid 		valuetypid;		/* base type's OID */
	int32 		count;			/* number of TemporalSeq elements */
	int32 		totalcount;		/* total number of TemporalInst elements in all TemporalSeq elements */
	/* variable-length data follows */
} TemporalS;

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
	Temporal 	*values[];
} AggregateState;

typedef struct
{
	Datum		value;			/* a datum value */
	int			tupno;			/* position index for tuple it came from */
} ScalarItem;

/* Extra information used by the default analysis routines */
typedef struct
{
	int			count;			/* # of duplicates */
	int			first;			/* values[] index of first occurrence */
} ScalarMCVItem;

typedef struct
{
	SortSupport ssup;
	int		   *tupnoLink;
} CompareScalarsContext;

typedef int (*qsort_comparator) (const void *a, const void *b);

/*****************************************************************************
 * fmgr macros temporal types
 *****************************************************************************/

/* doubleN */

#define DatumGetDouble2P(X)		((double2 *) DatumGetPointer(X))
#define Double2PGetDatum(X)		PointerGetDatum(X)
#define DatumGetDouble3P(X)		((double3 *) DatumGetPointer(X))
#define Double3PGetDatum(X)		PointerGetDatum(X)
#define DatumGetDouble4P(X)		((double4 *) DatumGetPointer(X))
#define Double4PGetDatum(X)		PointerGetDatum(X)

/* Temporal types */

#define DatumGetTemporal(X)		((Temporal *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalInst(X)	((TemporalInst *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalI(X)	((TemporalI *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalSeq(X)	((TemporalSeq *) PG_DETOAST_DATUM(X))
#define DatumGetTemporalS(X)	((TemporalS *) PG_DETOAST_DATUM(X))

#define PG_GETARG_TEMPORAL(i)	((Temporal *) PG_GETARG_VARLENA_P(i))

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

/*****************************************************************************
 * Oid cache functions defined in OidCache.c
 *****************************************************************************/

extern Oid type_oid(CachedType t);
extern Oid oper_oid(CachedOp op, CachedType lt, CachedType rt);
extern void populate_oidcache();

extern Datum fill_opcache(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Generic functions defined in TemporalTypes.c
 *****************************************************************************/

/* Miscellaneous functions */

extern void _PG_init(void);
extern void debugstr(char *msg);
extern size_t int4_pad(size_t size);
extern size_t double_pad(size_t size);
extern bool type_is_continuous(Oid type);
extern bool type_byval_fast(Oid type);
extern int get_typlen_fast(Oid type);
extern Datum datum_copy(Datum value, Oid type);
extern double datum_double(Datum d, Oid valuetypid);

/* PostgreSQL call helpers */

extern Datum call_input(Oid type, char *str);
extern char *call_output(Oid type, Datum value);
extern bytea *call_send(Oid type, Datum value);
extern Datum call_recv(Oid type, StringInfo buf);
extern Datum call_function1(PGFunction func, Datum op);
extern Datum call_function2(PGFunction func, Datum lop, Datum rop);
extern Datum call_function3(PGFunction func, Datum lop, Datum rop, Datum param);
extern Datum call_function4(PGFunction func, Datum lop, Datum rop, Datum param1, Datum param2);

/* Array functions */

/* Redefinition of the macro in array.h in order to avoid warnings raised 
   by [-Wsign-compare] */
#define MOBDB_ARR_DATA_OFFSET(a) \
		(ARR_HASNULL(a) ? (unsigned int32)((a)->dataoffset) : ARR_OVERHEAD_NONULLS(ARR_NDIM(a)))

extern Datum *datumarr_extract(ArrayType *array, int *count);
extern TimestampTz *timestamparr_extract(ArrayType *array, int *count);
extern Period **periodarr_extract(ArrayType *array, int *count);
extern RangeType **rangearr_extract(ArrayType *array, int *count);
extern Temporal **temporalarr_extract(ArrayType *array, int *count);

extern ArrayType *datumarr_to_array(Datum *values, int count, Oid type);
extern ArrayType *timestamparr_to_array(TimestampTz *times, int count);
extern ArrayType *periodarr_to_array(Period **periods, int count);
extern ArrayType *rangearr_to_array(RangeType **ranges, int count, Oid type);
extern ArrayType *textarr_to_array(text **textarr, int count);
extern ArrayType *temporalarr_to_array(Temporal **temporals, int count);
 
/* Sort functions */

extern void datum_sort(Datum *values, int count, Oid valuetypid);
extern void timestamp_sort(TimestampTz *values, int count);
extern void double2_sort(double2 **doubles, int count);
extern void double3_sort(double3 **triples, int count);
extern void double4_sort(double4 **quadruples, int count);
extern void periodarr_sort(Period **periods, int count);
extern void rangearr_sort(RangeType **ranges, int count);
extern void temporalinstarr_sort(TemporalInst **instants, int count);
extern void temporalseqarr_sort(TemporalSeq **sequences, int count);

/* Remove duplicate functions */

extern int datum_remove_duplicates(Datum *values, int count, Oid valuetypid);
extern int timestamp_remove_duplicates(TimestampTz *values, int count);

/* Text functions */

extern int text_cmp(text *arg1, text *arg2, Oid collid);
extern text *text_copy(text *t);

/* Comparison functions on datums */

extern bool datum_eq(Datum l, Datum r, Oid type);
extern bool datum_ne(Datum l, Datum r, Oid type);
extern bool datum_lt(Datum l, Datum r, Oid type);
extern bool datum_le(Datum l, Datum r, Oid type);
extern bool datum_gt(Datum l, Datum r, Oid type);
extern bool datum_ge(Datum l, Datum r, Oid type);

extern bool datum_eq2(Datum l, Datum r, Oid typel, Oid typer); 
extern bool datum_ne2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_lt2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_le2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_gt2(Datum l, Datum r, Oid typel, Oid typer);
extern bool datum_ge2(Datum l, Datum r, Oid typel, Oid typer);

extern Datum datum2_eq2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_ne2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_lt2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_le2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_gt2(Datum l, Datum r, Oid typel, Oid typer);
extern Datum datum2_ge2(Datum l, Datum r, Oid typel, Oid typer);

/* Oid functions */

extern Oid range_oid_from_base(Oid valuetypid);
extern Oid temporal_oid_from_base(Oid valuetypid);

extern Oid base_oid_from_range(Oid temptypid);
extern Oid base_oid_from_temporal(Oid temptypid);

extern bool temporal_oid(Oid temptypid);

/* Catalog functions */

extern void temporal_typinfo(Oid temptypid, Oid* valuetypid);

/* Trajectory functions */

extern bool type_has_precomputed_trajectory(Oid valuetypid);

/*****************************************************************************
 * Range functions defined in Range.c
 *****************************************************************************/

extern const char *range_to_string(RangeType *range);
extern Datum lower_datum(RangeType *range);
extern Datum upper_datum(RangeType *range);
extern bool lower_inc(RangeType *range);
extern bool upper_inc(RangeType *range);
extern RangeType *range_make(Datum from, Datum to, bool lower_inc, bool upper_inc, Oid subtypid);
extern RangeType **rangearr_normalize(RangeType **ranges, int *count);

extern Datum intrange_canonical(PG_FUNCTION_ARGS);
extern Datum numrange_to_floatrange(PG_FUNCTION_ARGS);

extern RangeType *numrange_to_floatrange_internal(RangeType *range);

extern Datum range_left_elem(PG_FUNCTION_ARGS);
extern Datum range_overleft_elem(PG_FUNCTION_ARGS);
extern Datum range_right_elem(PG_FUNCTION_ARGS);
extern Datum range_overright_elem(PG_FUNCTION_ARGS);
extern Datum range_adjacent_elem(PG_FUNCTION_ARGS);

extern bool range_left_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_overleft_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_right_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_overright_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);
extern bool range_adjacent_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val);

extern Datum elem_left_range(PG_FUNCTION_ARGS);
extern Datum elem_overleft_range(PG_FUNCTION_ARGS);
extern Datum elem_right_range(PG_FUNCTION_ARGS);
extern Datum elem_overright_range(PG_FUNCTION_ARGS);
extern Datum elem_adjacent_range(PG_FUNCTION_ARGS);

extern bool elem_left_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_overleft_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_right_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_overright_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);
extern bool elem_adjacent_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val);

/*****************************************************************************
 * Parsing routines: File Parser.c
 *****************************************************************************/

extern void p_whitespace(char **str);
extern bool p_obrace(char **str);
extern bool p_cbrace(char **str);
extern bool p_obracket(char **str);
extern bool p_cbracket(char **str);
extern bool p_oparen(char **str);
extern bool p_cparen(char **str);
extern bool p_comma(char **str);
extern Datum p_basetype(char **str, Oid basetype);

extern TimestampTz timestamp_parse(char **str);
extern TimestampSet *timestampset_parse(char **str);
extern Period *period_parse(char **str);
extern PeriodSet *periodset_parse(char **str);

extern TemporalInst *temporalinst_parse(char **str, Oid basetype, bool end);
extern TemporalI *temporali_parse(char **str, Oid basetype);
extern Temporal *temporal_parse(char **str, Oid basetype);

/*****************************************************************************
 * GBOX routines: File Gbox.c
 *****************************************************************************/

extern Datum gbox_in(PG_FUNCTION_ARGS);
extern Datum gbox_out(PG_FUNCTION_ARGS);
extern Datum gbox_constructor(PG_FUNCTION_ARGS);
extern Datum gbox_constructor3dm(PG_FUNCTION_ARGS);
extern Datum geodbox_constructor(PG_FUNCTION_ARGS);

extern int gbox_contains(const GBOX *g1, const GBOX *g2);
extern int gbox_cmp_internal(const GBOX *g1, const GBOX *g2);

/*****************************************************************************
 * Temporal routines: File Temporal.c
 *****************************************************************************/

/* Internal functions */

extern char dump_toupper(int in);
extern Temporal *temporal_copy(Temporal *temp);
extern Temporal *pg_getarg_temporal(Temporal *temp);
extern bool intersection_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **inter1, Temporal **inter2);
extern bool synchronize_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **sync1, Temporal **sync2, bool interpoint);
extern RangeType *tnumber_floatrange(Temporal *temp);
extern const char *temporal_type_name(uint8_t type);
extern bool temporal_type_from_string(const char *str, uint8_t *type);


/* Input/output functions */

extern Datum temporal_in(PG_FUNCTION_ARGS); 
extern Datum temporal_out(PG_FUNCTION_ARGS); 
extern Datum temporal_send(PG_FUNCTION_ARGS); 
extern Datum temporal_recv(PG_FUNCTION_ARGS);
extern Temporal* temporal_read(StringInfo buf, Oid valuetypid) ;
extern void temporal_write(Temporal* temp, StringInfo buf);

/* Constructor functions */

extern Datum temporal_make_temporalinst(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporali(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporalseq(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporals(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tint_as_tfloat(PG_FUNCTION_ARGS);

extern Temporal *tint_as_tfloat_internal(Temporal *temp);

/* Accessor functions */

extern Datum temporal_type(PG_FUNCTION_ARGS);
extern Datum temporal_mem_size(PG_FUNCTION_ARGS);
extern Datum tempdisc_get_values(PG_FUNCTION_ARGS);
extern Datum tfloat_ranges(PG_FUNCTION_ARGS);
extern Datum temporal_get_time(PG_FUNCTION_ARGS);
extern Datum temporalinst_get_value(PG_FUNCTION_ARGS);
extern Datum tnumber_box(PG_FUNCTION_ARGS);
extern Datum tnumber_value_range(PG_FUNCTION_ARGS);
extern Datum temporal_start_value(PG_FUNCTION_ARGS);
extern Datum temporal_end_value(PG_FUNCTION_ARGS);
extern Datum temporal_min_value(PG_FUNCTION_ARGS);
extern Datum temporal_max_value(PG_FUNCTION_ARGS);
extern Datum temporal_time(PG_FUNCTION_ARGS);
extern Datum temporal_timespan(PG_FUNCTION_ARGS);
extern Datum temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum temporal_instants(PG_FUNCTION_ARGS);
extern Datum temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum temporal_ever_equals(PG_FUNCTION_ARGS);
extern Datum temporal_always_equals(PG_FUNCTION_ARGS);
extern Datum temporal_shift(PG_FUNCTION_ARGS);

extern Datum tempdisc_get_values_internal(Temporal *temp);
extern Datum temporal_min_value_internal(Temporal *temp);
extern TimestampTz temporal_start_timestamp_internal(Temporal *temp);

/* Restriction Functions */

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
extern Datum temporal_intersects_temporal(PG_FUNCTION_ARGS);
 
extern Temporal *temporal_at_min_internal(Temporal *temp);
extern TemporalInst *temporal_at_timestamp_internal(Temporal *temp, TimestampTz t);
extern void temporal_timespan_internal(Period *p, Temporal *temp);
extern char *temporal_to_string(Temporal *temp, char *(*value_out)(Oid, Datum));
extern void temporal_bbox(void *box, const Temporal *temp);
extern bool temporal_intersects_temporal_internal(Temporal *temp1, Temporal *temp2);
	
extern Datum temporal_lt(PG_FUNCTION_ARGS);
extern Datum temporal_le(PG_FUNCTION_ARGS);
extern Datum temporal_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ge(PG_FUNCTION_ARGS);
extern Datum temporal_gt(PG_FUNCTION_ARGS);
extern Datum temporal_cmp(PG_FUNCTION_ARGS);
extern Datum temporal_hash(PG_FUNCTION_ARGS);

extern uint32 temporal_hash_internal(const Temporal *temp);

/*****************************************************************************
 * TemporalInst routines
 *****************************************************************************/
 
extern TemporalInst *temporalinst_make(Datum value, TimestampTz t, Oid valuetypid);
extern TemporalInst *temporalinst_copy(TemporalInst *inst);
extern Datum* temporalinst_value_ptr(TemporalInst *inst);
extern Datum temporalinst_value(TemporalInst *inst);
extern Datum temporalinst_value_copy(TemporalInst *inst);
extern RangeType *tnumberinst_floatrange(TemporalInst *inst);

/* Input/output functions */

char *temporalinst_to_string(TemporalInst *inst, char *(*value_out)(Oid, Datum));
extern void temporalinst_write(TemporalInst *inst, StringInfo buf);
extern TemporalInst *temporalinst_read(StringInfo buf, Oid valuetypid);

/* Intersection function */

extern bool intersection_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	TemporalInst **inter1, TemporalInst **inter2);
	
/* Cast functions */

extern TemporalInst *tintinst_as_tfloatinst(TemporalInst *inst);
extern TemporalInst *tfloatinst_as_tintinst(TemporalInst *inst);

/* Transformation functions */

extern TemporalInst *temporali_as_temporalinst(TemporalI *ti);
extern TemporalInst *temporalseq_as_temporalinst(TemporalSeq *seq);
extern TemporalInst *temporals_as_temporalinst(TemporalS *ts);

/* Accessor functions */

extern ArrayType *temporalinst_values(TemporalInst *inst);
extern ArrayType *tfloatinst_ranges(TemporalInst *inst);
extern PeriodSet *temporalinst_get_time(TemporalInst *inst);
extern void temporalinst_bbox(void *box, TemporalInst *inst);
extern RangeType *tnumberinst_value_range(TemporalInst *inst);
extern bool temporalinst_ever_equals(TemporalInst *inst, Datum value);
extern bool temporalinst_always_equals(TemporalInst *inst, Datum value);
extern void temporalinst_timespan(Period *p, TemporalInst *inst);
extern ArrayType *temporalinst_timestamps(TemporalInst *inst);
extern ArrayType *temporalinst_instants(TemporalInst *inst);
extern TemporalInst *temporalinst_shift(TemporalInst *inst, Interval *interval);

/* Restriction Functions */

extern TemporalInst *temporalinst_at_value(TemporalInst *inst, Datum val);
extern TemporalInst *temporalinst_minus_value(TemporalInst *inst, Datum val);
extern TemporalInst *temporalinst_at_values(TemporalInst *inst, Datum *values, int count);
extern TemporalInst *temporalinst_minus_values(TemporalInst *inst, Datum *values, int count);
extern TemporalInst *tnumberinst_at_range(TemporalInst *inst, RangeType *range);
extern TemporalInst *tnumberinst_minus_range(TemporalInst *inst, RangeType *range);

extern TemporalInst *temporalinst_at_timestamp(TemporalInst *inst, TimestampTz t);
extern bool temporalinst_value_at_timestamp(TemporalInst *inst, TimestampTz t, Datum *result);
extern TemporalInst *temporalinst_minus_timestamp(TemporalInst *inst, TimestampTz t);
extern TemporalInst *temporalinst_at_timestampset(TemporalInst *inst, TimestampSet *ts);
extern TemporalInst *temporalinst_minus_timestampset(TemporalInst *inst, TimestampSet *ts);
extern TemporalInst *temporalinst_at_period(TemporalInst *inst, Period *p);
extern TemporalInst *temporalinst_minus_period(TemporalInst *inst, Period *p);
extern TemporalInst *temporalinst_at_periodset(TemporalInst *inst, PeriodSet *ps);
extern TemporalInst *temporalinst_minus_periodset(TemporalInst *inst, PeriodSet *ps);

extern TemporalInst *tnumberinst_at_ranges(TemporalInst *inst, RangeType **normranges, int count);
extern TemporalInst *tnumberinst_minus_ranges(TemporalInst *inst, RangeType **normranges, int count);
extern TemporalInst *temporalinst_at_period(TemporalInst *inst, Period *p);
extern TemporalInst *temporalinst_minus_period(TemporalInst *inst, Period *p);

extern bool temporalinst_intersects_timestamp(TemporalInst *inst, TimestampTz t);
extern bool temporalinst_intersects_timestampset(TemporalInst *inst, TimestampSet *ts);
extern bool temporalinst_intersects_period(TemporalInst *inst, Period *p);
extern bool temporalinst_intersects_periodset(TemporalInst *inst, PeriodSet *ps);
extern bool temporalinst_intersects_temporalinst(TemporalInst *inst1, TemporalInst *inst2);

/* Functions for defining B-tree index */

extern int temporalinst_cmp(TemporalInst *inst1, TemporalInst *inst2);
extern bool temporalinst_eq(TemporalInst *inst1, TemporalInst *inst2);
extern bool temporalinst_ne(TemporalInst *inst1, TemporalInst *inst2);

/* Function for defining hash index */

extern uint32 temporalinst_hash(TemporalInst *inst);

/*****************************************************************************
 * TemporalI routines
 *****************************************************************************/

extern TemporalInst *temporali_inst_n(TemporalI *ti, int index);
extern int temporali_find_timestamp(TemporalI *ti, TimestampTz t);
extern int temporalinstarr_find_timestamp(TemporalInst **array, int from, 
	int count, TimestampTz t);
extern TemporalI *temporali_from_temporalinstarr(TemporalInst **instants, 
	int count);
extern TemporalI *temporali_copy(TemporalI *ti);
extern RangeType *tnumberi_floatrange(TemporalI *ti);

/* Intersection functions */

extern bool intersection_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	TemporalI **inter1, TemporalI **inter2);

/* Input/output functions */

extern char *temporali_to_string(TemporalI *ti, char *(*value_out)(Oid, Datum));
extern void temporali_write(TemporalI *ti, StringInfo buf);
extern TemporalI *temporali_read(StringInfo buf, Oid valuetypid);

/* Cast functions */
 
TemporalI *tinti_as_tfloati(TemporalI *ti);
TemporalI *tfloati_as_tinti(TemporalI *ti);
TemporalSeq *temporali_as_temporalseq(TemporalI *ti);

/* Transformation functions */

extern TemporalI *temporalinst_as_temporali(TemporalInst *inst);
extern TemporalI *temporalseq_as_temporali(TemporalSeq *seq);
extern TemporalI *temporals_as_temporali(TemporalS *ts);

/* Accessor functions */

extern Datum *temporali_values1(TemporalI *ti, int *count);
extern ArrayType *temporali_values(TemporalI *ti);
extern ArrayType *tfloati_ranges(TemporalI *ti);
extern PeriodSet *temporali_get_time(TemporalI *ti);
extern void *temporali_bbox_ptr(TemporalI *ti);
extern void temporali_bbox(void *box, TemporalI *ti);
extern RangeType *tnumberi_value_range(TemporalI *ti);
extern Datum temporali_min_value(TemporalI *ti);
extern Datum temporali_max_value(TemporalI *ti);
extern TimestampSet *temporali_time(TemporalI *ti);
extern void temporali_timespan(Period *p, TemporalI *ti);
extern TemporalInst **temporali_instantarr(TemporalI *ti);
extern ArrayType *temporali_instants(TemporalI *ti);
extern Timestamp temporali_start_timestamp(TemporalI *ti);
extern TimestampTz temporali_end_timestamp(TemporalI *ti);
extern ArrayType *temporali_timestamps(TemporalI *ti);
extern bool temporali_ever_equals(TemporalI *ti, Datum value);
extern bool temporali_always_equals(TemporalI *ti, Datum value);
extern TemporalI *temporali_shift(TemporalI *ti, Interval *interval);

/* Restriction Functions */

extern TemporalI *temporali_at_value(TemporalI *ti, Datum value);
extern TemporalI *temporali_minus_value(TemporalI *ti, Datum value);
extern TemporalI *temporali_at_values(TemporalI *ti, Datum *values, int count);
extern TemporalI *temporali_minus_values(TemporalI *ti, Datum *values, int count);
extern TemporalI *tnumberi_at_range(TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_minus_range(TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_at_ranges(TemporalI *ti, RangeType **normranges, int count);
extern TemporalI *tnumberi_minus_ranges(TemporalI *ti, RangeType **normranges, int count);
extern TemporalI *temporali_at_min(TemporalI *ti);
extern TemporalI *temporali_minus_min(TemporalI *ti);
extern TemporalI *temporali_at_max(TemporalI *ti);
extern TemporalI *temporali_minus_max(TemporalI *ti);
extern TemporalInst *temporali_at_timestamp(TemporalI *ti, TimestampTz t);
extern bool temporali_value_at_timestamp(TemporalI *ti, TimestampTz t, Datum *result);
extern TemporalI * temporali_minus_timestamp(TemporalI *ti, TimestampTz t);
extern TemporalI *temporali_at_timestampset(TemporalI *ti, TimestampSet *ts);
extern TemporalI *temporali_minus_timestampset(TemporalI *ti, TimestampSet *ts);
extern TemporalI *temporali_at_period(TemporalI *ti, Period *p);
extern TemporalI *temporali_minus_period(TemporalI *ti, Period *p);
extern TemporalI *temporali_at_periodset(TemporalI *ti, PeriodSet *ps);
extern TemporalI *temporali_minus_periodset(TemporalI *ti, PeriodSet *ps);
extern bool temporali_intersects_timestamp(TemporalI *ti, TimestampTz t);
extern bool temporali_intersects_timestampset(TemporalI *ti, TimestampSet *ts);
extern bool temporali_intersects_period(TemporalI *ti, Period *p);
extern bool temporali_intersects_periodset(TemporalI *ti, PeriodSet *ps);
extern bool temporali_intersects_temporalinst(TemporalI *ti, TemporalInst *inst);
extern bool temporali_intersects_temporali(TemporalI *ti1, TemporalI *ti2);

/* Local aggregate functions */

extern double temporali_twavg(TemporalI *ti);

/* Functions for defining B-tree index */

extern int temporali_cmp(TemporalI *ti1, TemporalI *ti2);
extern bool temporali_eq(TemporalI *ti1, TemporalI *ti2);
extern bool temporali_ne(TemporalI *ti1, TemporalI *ti2);

/* Function for defining hash index */

extern uint32 temporali_hash(TemporalI *ti);

/*****************************************************************************
 * TemporalSeq routines
 *****************************************************************************/

size_t *temporalseq_offsets_ptr(TemporalSeq *seq);
extern char *temporalseq_data_ptr(TemporalSeq *seq);
extern TemporalInst *temporalseq_inst_n(TemporalSeq *seq, int index);
extern TemporalSeq *temporalseq_from_temporalinstarr(TemporalInst **instants, 
	int count, bool lower_inc, bool upper_inc, bool normalize);
extern TemporalSeq *temporalseq_copy(TemporalSeq *seq);
extern int temporalseq_find_timestamp(TemporalSeq *seq, TimestampTz t);
extern Datum temporalseq_value_at_timestamp1(TemporalInst *inst1, 
	TemporalInst *inst2, TimestampTz t);
extern TemporalSeq **temporalseqarr_normalize(TemporalSeq **sequences, int count, 
	int *newcount);

/* Intersection functions */

extern bool intersection_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	TemporalInst **inter1, TemporalInst **inter2);	
extern bool intersection_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	TemporalSeq **inter1, TemporalSeq **inter2);

/* Synchronize functions */

extern bool temporalseq_add_crossing(TemporalInst *inst1, TemporalInst *next1, 
	TemporalInst *inst2, TemporalInst *next2, 
	TemporalInst **cross1, TemporalInst **cross2);
extern bool synchronize_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	TemporalSeq **sync1, TemporalSeq **sync2, bool interpoint);

extern bool tnumberseq_mult_maxmin_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
// To put it in TempDistance.c ?
extern bool tpointseq_min_dist_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
extern bool tpointseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
extern bool temporalseq_intersect_at_timestamp(TemporalInst *start1, 
	TemporalInst *end1, TemporalInst *start2, TemporalInst *end2, TimestampTz *inter);
extern RangeType *tnumberseq_floatrange(TemporalSeq *seq);

/* Input/output functions */

extern char *temporalseq_to_string(TemporalSeq *seq, char *(*value_out)(Oid, Datum));
extern void temporalseq_write(TemporalSeq *seq, StringInfo buf);
extern TemporalSeq *temporalseq_read(StringInfo buf, Oid valuetypid);

/* Cast functions */

extern int tintseq_as_tfloatseq1(TemporalSeq **result, TemporalSeq *seq);
extern TemporalS *tintseq_as_tfloatseq(TemporalSeq *seq);
extern TemporalSeq *tfloatseq_as_tintseq(TemporalSeq *seq);

/* Transformation functions */

extern TemporalSeq *temporalinst_as_temporalseq(TemporalInst *inst);
extern TemporalSeq *temporali_as_temporalseq(TemporalI *ti);
extern TemporalSeq *temporals_as_temporalseq(TemporalS *ts);

/* Accessor functions */

extern Datum *tempdiscseq_values1(TemporalSeq *seq);
extern ArrayType *tempdiscseq_values(TemporalSeq *seq);
extern PeriodSet *temporalseq_get_time(TemporalSeq *seq);
extern void *temporalseq_bbox_ptr(TemporalSeq *seq);
extern void temporalseq_bbox(void *box, TemporalSeq *seq);
extern RangeType *tnumberseq_value_range(TemporalSeq *seq);
extern RangeType *tfloatseq_range(TemporalSeq *seq);
extern ArrayType *tfloatseq_ranges(TemporalSeq *seq);
extern Datum temporalseq_min_value(TemporalSeq *seq);
extern Datum temporalseq_max_value(TemporalSeq *seq);
extern void temporalseq_timespan(Period *p, TemporalSeq *seq);
extern Datum temporalseq_duration(TemporalSeq *seq);
extern TemporalInst **temporalseq_instants(TemporalSeq *seq);
extern ArrayType *temporalseq_instants_array(TemporalSeq *seq);
extern Timestamp temporalseq_start_timestamp(TemporalSeq *seq);
extern Timestamp temporalseq_end_timestamp(TemporalSeq *seq);
extern TimestampTz *temporalseq_timestamps1(TemporalSeq *seq);
extern ArrayType *temporalseq_timestamps(TemporalSeq *seq);
extern bool temporalseq_ever_equals(TemporalSeq *seq, Datum value);
extern bool temporalseq_always_equals(TemporalSeq *seq, Datum value);
extern TemporalSeq *temporalseq_shift(TemporalSeq *seq, 
	Interval *interval);

/* Restriction Functions */

extern bool tempcontseq_timestamp_at_value(TemporalInst *inst1, TemporalInst *inst2, 
	Datum value, Oid valuetypid, TimestampTz *t);
extern int temporalseq_at_value2(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_value(TemporalSeq *seq, Datum value);
extern int temporalseq_minus_value2(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_minus_value(TemporalSeq *seq, Datum value);
extern int temporalseq_at_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, 
	int count);	
extern TemporalS *temporalseq_at_values(TemporalSeq *seq, Datum *values, int count);
extern int temporalseq_minus_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, 
	int count);
extern TemporalS *temporalseq_minus_values(TemporalSeq *seq, Datum *values, int count);
extern TemporalS *temporalseq_minus_values(TemporalSeq *seq, Datum *values, int count);
extern int tnumberseq_at_range2(TemporalSeq **result, TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_at_range(TemporalSeq *seq, RangeType *range);
extern int tnumberseq_minus_range1(TemporalSeq **result, TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_minus_range(TemporalSeq *seq, RangeType *range);
extern int tnumberseq_at_ranges1(TemporalSeq **result, TemporalSeq *seq, 
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_at_ranges(TemporalSeq *seq, 
	RangeType **normranges, int count);
extern int tnumberseq_minus_ranges1(TemporalSeq **result, TemporalSeq *seq, 
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_minus_ranges(TemporalSeq *seq,
	RangeType **normranges, int count);
extern int temporalseq_at_minmax(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_min(TemporalSeq *seq);
extern TemporalS *temporalseq_minus_min(TemporalSeq *seq);
extern TemporalS *temporalseq_at_max(TemporalSeq *seq);
extern TemporalS *temporalseq_minus_max(TemporalSeq *seq);
extern TemporalInst *temporalseq_at_timestamp1(TemporalInst *inst1, 
	TemporalInst *inst2, TimestampTz t);
extern TemporalInst *temporalseq_at_timestamp(TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_value_at_timestamp(TemporalSeq *seq, TimestampTz t, Datum *result);
extern int temporalseq_minus_timestamp1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampTz t);
extern TemporalS *temporalseq_minus_timestamp(TemporalSeq *seq, TimestampTz t);
extern TemporalI *temporalseq_at_timestampset(TemporalSeq *seq, TimestampSet *ts);
extern int temporalseq_minus_timestampset1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampSet *ts);
extern TemporalS *temporalseq_minus_timestampset(TemporalSeq *seq, TimestampSet *ts);
extern TemporalSeq *temporalseq_at_period(TemporalSeq *seq, Period *p);
extern TemporalS *temporalseq_minus_period(TemporalSeq *seq, Period *p);
extern int temporalseq_at_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps);
extern TemporalSeq **temporalseq_at_periodset2(TemporalSeq *seq, PeriodSet *ps, int *count);
extern TemporalS *temporalseq_at_periodset(TemporalSeq *seq, PeriodSet *ps);
extern int temporalseq_minus_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps, 
	int from, int count);
extern TemporalS *temporalseq_minus_periodset(TemporalSeq *seq, PeriodSet *ps);
extern bool temporalseq_intersects_timestamp(TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_intersects_timestampset(TemporalSeq *seq, TimestampSet *t);
extern bool temporalseq_intersects_period(TemporalSeq *seq, Period *p);
extern bool temporalseq_intersects_periodset(TemporalSeq *seq, PeriodSet *ps);
extern bool temporalseq_intersects_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2);
extern bool temporalseq_intersects_temporalinst(TemporalSeq *seq, TemporalInst *inst);
extern bool temporalseq_intersects_temporali(TemporalSeq *seq, TemporalI *ti);

/* Local aggregate functions */

extern double tintseq_integral(TemporalSeq *seq);
extern double tfloatseq_integral(TemporalSeq *seq);
extern double tintseq_twavg(TemporalSeq *seq);
extern double tfloatseq_twavg(TemporalSeq *seq);

/* Functions for defining B-tree index */

extern int temporalseq_cmp(TemporalSeq *seq1, TemporalSeq *seq2);
extern bool temporalseq_eq(TemporalSeq *seq1, TemporalSeq *seq2);
extern bool temporalseq_ne(TemporalSeq *seq1, TemporalSeq *seq2);

/* Function for defining hash index */

extern uint32 temporalseq_hash(TemporalSeq *seq);

/*****************************************************************************
 * TemporalS routines
 *****************************************************************************/

/* General functions */

extern TemporalSeq **temporals_seqs(TemporalS *ts);
extern TemporalSeq *temporals_seq_n(TemporalS *ts, int index);
extern TemporalS *temporals_from_temporalseqarr(TemporalSeq **sequences, 
	int count, bool normalize);
extern TemporalS *temporals_copy(TemporalS *ts);
extern bool temporalseqarr_find_timestamp(TemporalSeq **array, int from, 
	int count, TimestampTz t, int *pos);
extern bool temporals_find_timestamp(TemporalS *ts, TimestampTz t, int *pos);
extern bool temporals_intersects_period(TemporalS *ts, Period *p);
extern double temporals_duration_time(TemporalS *ts);
extern bool temporals_contains_timestamp(TemporalS *ts, TimestampTz t, int *n);
extern RangeType *tnumbers_floatrange(TemporalS *ts);

/* Intersection functions */

extern bool intersection_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporals(TemporalInst *inst, TemporalS *ts,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporals(TemporalI *ti, TemporalS *ts,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts, 
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **inter1, TemporalS **inter2);

/* Synchronize functions */

extern bool synchronize_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	TemporalInst **sync1, TemporalInst **sync2);
extern bool synchronize_temporalinst_temporals(TemporalInst *inst, TemporalS *ts,
	TemporalInst **sync1, TemporalInst **sync2);
extern bool synchronize_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	TemporalI **sync1, TemporalI **sync2);
extern bool synchronize_temporali_temporals(TemporalI *ti, TemporalS *ts,
	TemporalI **sync1, TemporalI **sync2);
extern bool synchronize_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts, 
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **sync1, TemporalS **sync2, bool interpoint);

/* Input/output functions */

extern char *temporals_to_string(TemporalS *ts, char *(*value_out)(Oid, Datum));
extern void temporals_write(TemporalS *ts, StringInfo buf);
extern TemporalS *temporals_read(StringInfo buf, Oid valuetypid);

/* Cast functions */

extern TemporalS *tints_as_tfloats(TemporalS *ts);
extern TemporalS *tfloats_as_tints(TemporalS *ts);

/* Transformation functions */

extern TemporalS *temporalinst_as_temporals(TemporalInst *inst);
extern TemporalS *temporali_as_temporals(TemporalI *ti);
extern TemporalS *temporalseq_as_temporals(TemporalSeq *seq);

/* Accessor functions */

extern ArrayType *tempdiscs_values(TemporalS *ts);
extern ArrayType *tfloats_ranges(TemporalS *ts);
extern void *temporals_bbox_ptr(TemporalS *ts);
extern void temporals_bbox(void *box, TemporalS *ts);
extern RangeType *tnumbers_value_range(TemporalS *ts);
extern Datum temporals_min_value(TemporalS *ts);
extern Datum temporals_max_value(TemporalS *ts);
extern PeriodSet *temporals_get_time(TemporalS *ts);
extern Datum temporals_duration(TemporalS *ts);
extern void temporals_timespan(Period *p, TemporalS *ts);
extern TemporalSeq **temporals_sequencearr(TemporalS *ts);
extern ArrayType *temporals_sequences_internal(TemporalS *ts);
extern int temporals_num_instants(TemporalS *ts);
extern TemporalInst *temporals_instant_n(TemporalS *ts, int n);
extern TemporalInst **temporals_instants1(TemporalS *ts, int *count);
extern ArrayType *temporals_instants(TemporalS *ts);
extern TimestampTz temporals_start_timestamp(TemporalS *ts);
extern TimestampTz temporals_end_timestamp(TemporalS *ts);
extern int temporals_num_timestamps(TemporalS *ts);
extern bool temporals_timestamp_n(TemporalS *ts, int n, TimestampTz *result);
extern TimestampTz *temporals_timestamps1(TemporalS *ts, int *count);
extern ArrayType *temporals_timestamps(TemporalS *ts);
extern bool temporals_ever_equals(TemporalS *ts, Datum value);
extern bool temporals_always_equals(TemporalS *ts, Datum value);
extern TemporalS *temporals_shift(TemporalS *ts, Interval *interval);
extern bool temporals_continuous_value_internal(TemporalS *ts);
extern bool temporals_continuous_time_internal(TemporalS *ts);

/* Restriction Functions */

extern TemporalS *temporals_at_value(TemporalS *ts, Datum value);
extern TemporalS *temporals_minus_value(TemporalS *ts, Datum value);
extern TemporalS *temporals_at_values(TemporalS *ts, Datum *values, int count);
extern TemporalS *temporals_minus_values(TemporalS *ts, Datum *values, int count);
extern TemporalS *tnumbers_at_range(TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_minus_range(TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_at_ranges(TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *tnumbers_minus_ranges(TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *temporals_at_min(TemporalS *ts);
extern TemporalS *temporals_minus_min(TemporalS *ts);
extern TemporalS *temporals_at_max(TemporalS *ts);
extern TemporalS *temporals_minus_max(TemporalS *ts);
extern TemporalInst *temporals_at_timestamp(TemporalS *ts, TimestampTz t);
extern bool temporals_value_at_timestamp(TemporalS *ts, TimestampTz t, Datum *result);
extern TemporalS *temporals_minus_timestamp(TemporalS *ts, TimestampTz t);
extern TemporalI *temporals_at_timestampset(TemporalS *ts, TimestampSet *ts1);
extern TemporalS *temporals_minus_timestampset(TemporalS *ts, TimestampSet *ts1);
extern TemporalS *temporals_at_period(TemporalS *ts, Period *p);
extern TemporalS *temporals_minus_period(TemporalS *ts, Period *p);
extern TemporalS *temporals_at_periodset(TemporalS *ts, PeriodSet *ps);
extern TemporalS *temporals_minus_periodset(TemporalS *ts, PeriodSet *ps);
extern bool temporals_intersects_timestamp(TemporalS *ts, TimestampTz t);
extern bool temporals_intersects_timestampset(TemporalS *ts, TimestampSet *ts1);
extern bool temporals_intersects_period(TemporalS *ts, Period *p);
extern bool temporals_intersects_periodset(TemporalS *ts, PeriodSet *ps);
extern bool temporals_intersects_temporalinst(TemporalS *ts, TemporalInst *inst);
extern bool temporals_intersects_temporali(TemporalS *ts, TemporalI *ti);
extern bool temporals_intersects_temporalseq(TemporalS *ts, TemporalSeq *seq);
extern bool temporals_intersects_temporals(TemporalS *ts1, TemporalS *ts2);

/* Local aggregate functions */

extern double tints_integral(TemporalS *ts);
extern double tfloats_integral(TemporalS *ts);
extern double tints_twavg(TemporalS *ts);
extern double tfloats_twavg(TemporalS *ts);

/* Functions for defining B-tree index */

extern int temporals_cmp(TemporalS *ts1, TemporalS *ts2);
extern bool temporals_eq(TemporalS *ts1, TemporalS *ts2);
extern bool temporals_ne(TemporalS *ts1, TemporalS *ts2);

/* Function for defining hash index */

extern uint32 temporals_hash(TemporalS *ts);

/*****************************************************************************
 * doubleN and tdoubleN functions defined in DoubleN.c
 *****************************************************************************/

extern Datum double2_in(PG_FUNCTION_ARGS);
extern Datum double2_out(PG_FUNCTION_ARGS);

extern double2 *double2_construct(double a, double b);
extern double2 *double2_add(double2 *d1, double2 *d2);
extern bool double2_eq(double2 *d1, double2 *d2);
extern int double2_cmp(double2 *d1, double2 *d2);

extern Datum double3_in(PG_FUNCTION_ARGS);
extern Datum double3_out(PG_FUNCTION_ARGS);

extern double3 *double3_construct(double a, double b, double c);
extern double3 *double3_add(double3 *d1, double3 *d2);
extern bool double3_eq(double3 *d1, double3 *d2);
extern int double3_cmp(double3 *d1, double3 *d2);

extern Datum double4_in(PG_FUNCTION_ARGS);
extern Datum double4_out(PG_FUNCTION_ARGS);

extern double4 *double4_construct(double a, double b, double c, double d);
extern double4 *double4_add(double4 *d1, double4 *d2);
extern bool double4_eq(double4 *d1, double4 *d2);
extern int double4_cmp(double4 *d1, double4 *d2);

extern Datum tdouble2_in(PG_FUNCTION_ARGS);
extern Datum tdouble3_in(PG_FUNCTION_ARGS);
extern Datum tdouble4_in(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal arithmetic operators defined in ArithmeticOps.c
 *****************************************************************************/

extern Datum add_base_temporal(PG_FUNCTION_ARGS);
extern Datum add_temporal_base(PG_FUNCTION_ARGS);
extern Datum add_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum sub_base_temporal(PG_FUNCTION_ARGS);
extern Datum sub_temporal_base(PG_FUNCTION_ARGS);
extern Datum sub_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum mult_base_temporal(PG_FUNCTION_ARGS);
extern Datum mult_temporal_base(PG_FUNCTION_ARGS);
extern Datum mult_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum div_base_temporal(PG_FUNCTION_ARGS);
extern Datum div_temporal_base(PG_FUNCTION_ARGS);
extern Datum div_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal Boolean operators defined in BooleanOps.c
 *****************************************************************************/

extern Datum datum_and(Datum l, Datum r);
extern Datum datum_or(Datum l, Datum r);

extern Datum tand_bool_tbool(PG_FUNCTION_ARGS);
extern Datum tand_tbool_bool(PG_FUNCTION_ARGS);
extern Datum tand_tbool_tbool(PG_FUNCTION_ARGS);

extern Datum tor_bool_tbool(PG_FUNCTION_ARGS);
extern Datum tor_tbool_bool(PG_FUNCTION_ARGS);
extern Datum tor_tbool_tbool(PG_FUNCTION_ARGS);

extern Datum tnot_tbool(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal comparison operators defined in ComparisonOps.c
 *****************************************************************************/

extern Datum teq_base_temporal(PG_FUNCTION_ARGS);
extern Datum teq_temporal_base(PG_FUNCTION_ARGS);
extern Datum teq_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tne_base_temporal(PG_FUNCTION_ARGS);
extern Datum tne_temporal_base(PG_FUNCTION_ARGS);
extern Datum tne_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tlt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tlt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tle_base_temporal(PG_FUNCTION_ARGS);
extern Datum tle_temporal_base(PG_FUNCTION_ARGS);
extern Datum tle_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tgt_base_temporal(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_base(PG_FUNCTION_ARGS);
extern Datum tgt_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum tge_base_temporal(PG_FUNCTION_ARGS);
extern Datum tge_temporal_base(PG_FUNCTION_ARGS);
extern Datum tge_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal aggregate functions defined in AggregateFuncs.c
 *****************************************************************************/

extern Datum datum_min_int32(Datum l, Datum r);
extern Datum datum_max_int32(Datum l, Datum r);
extern Datum datum_sum_int32(Datum l, Datum r);
extern Datum datum_min_float8(Datum l, Datum r);
extern Datum datum_max_float8(Datum l, Datum r);
extern Datum datum_sum_float8(Datum l, Datum r);
extern Datum datum_min_text(Datum l, Datum r);
extern Datum datum_max_text(Datum l, Datum r);
extern Datum datum_sum_double2(Datum l, Datum r);
extern Datum datum_sum_double3(Datum l, Datum r);
extern Datum datum_sum_double4(Datum l, Datum r);

extern AggregateState *aggstate_make(FunctionCallInfo fcinfo, int size, Temporal **values);

extern AggregateState *temporalinst_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalInst *inst, Datum (*operator)(Datum, Datum));
extern AggregateState *temporalinst_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*operator)(Datum, Datum));
extern AggregateState *temporalseq_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	TemporalSeq *seq, Datum (*operator)(Datum, Datum), bool interpoint);
extern AggregateState *temporalseq_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*operator)(Datum, Datum), bool interpoint);
	
extern Datum tbool_tand_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tand_combinefn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_transfn(PG_FUNCTION_ARGS);
extern Datum tbool_tor_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tmax_combinefn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tint_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_tsum_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_tcount_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_combinefn(PG_FUNCTION_ARGS);
extern Datum temporal_tagg_finalfn(PG_FUNCTION_ARGS);
extern Datum temporal_tavg_finalfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmin_combinefn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_transfn(PG_FUNCTION_ARGS);
extern Datum ttext_tmax_combinefn(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Relative position operators defined in RelativePosOps.c
 *****************************************************************************/

/* Internal functions used for the indexes */

extern bool left_box_box_internal(BOX *box1, BOX *box2);
extern bool overleft_box_box_internal(BOX *box1, BOX *box2);
extern bool right_box_box_internal(BOX *box1, BOX *box2);
extern bool overright_box_box_internal(BOX *box1, BOX *box2);
extern bool before_box_box_internal(BOX *box1, BOX *box2);
extern bool overbefore_box_box_internal(BOX *box1, BOX *box2);
extern bool after_box_box_internal(BOX *box1, BOX *box2);
extern bool overafter_box_box_internal(BOX *box1, BOX *box2);

extern Datum left_datum_temporal(PG_FUNCTION_ARGS);
extern Datum overleft_datum_temporal(PG_FUNCTION_ARGS);
extern Datum right_datum_temporal(PG_FUNCTION_ARGS);
extern Datum overright_datum_temporal(PG_FUNCTION_ARGS);

extern Datum left_range_temporal(PG_FUNCTION_ARGS);
extern Datum overleft_range_temporal(PG_FUNCTION_ARGS);
extern Datum right_range_temporal(PG_FUNCTION_ARGS);
extern Datum overright_range_temporal(PG_FUNCTION_ARGS);

extern Datum before_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum after_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_timestamp_temporal(PG_FUNCTION_ARGS);

extern Datum before_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum after_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_timestampset_temporal(PG_FUNCTION_ARGS);

extern Datum before_period_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_period_temporal(PG_FUNCTION_ARGS);
extern Datum after_period_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_period_temporal(PG_FUNCTION_ARGS);

extern Datum before_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum after_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_periodset_temporal(PG_FUNCTION_ARGS);

extern Datum left_box_temporal(PG_FUNCTION_ARGS);
extern Datum overleft_box_temporal(PG_FUNCTION_ARGS);
extern Datum right_box_temporal(PG_FUNCTION_ARGS);
extern Datum overright_box_temporal(PG_FUNCTION_ARGS);
extern Datum before_box_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_box_temporal(PG_FUNCTION_ARGS);
extern Datum after_box_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_box_temporal(PG_FUNCTION_ARGS);

extern Datum left_temporal_datum(PG_FUNCTION_ARGS);
extern Datum overleft_temporal_datum(PG_FUNCTION_ARGS);
extern Datum overright_temporal_datum(PG_FUNCTION_ARGS);
extern Datum right_temporal_datum(PG_FUNCTION_ARGS);

extern Datum left_temporal_range(PG_FUNCTION_ARGS);
extern Datum overleft_temporal_range(PG_FUNCTION_ARGS);
extern Datum overright_temporal_range(PG_FUNCTION_ARGS);
extern Datum right_temporal_range(PG_FUNCTION_ARGS);

extern Datum before_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum after_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_timestamp(PG_FUNCTION_ARGS);

extern Datum before_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum after_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_timestampset(PG_FUNCTION_ARGS);

extern Datum before_temporal_period(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_period(PG_FUNCTION_ARGS);
extern Datum after_temporal_period(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_period(PG_FUNCTION_ARGS);

extern Datum before_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum after_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_periodset(PG_FUNCTION_ARGS);

extern Datum left_temporal_box(PG_FUNCTION_ARGS);
extern Datum overleft_temporal_box(PG_FUNCTION_ARGS);
extern Datum overright_temporal_box(PG_FUNCTION_ARGS);
extern Datum right_temporal_box(PG_FUNCTION_ARGS);
extern Datum before_temporal_box(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_box(PG_FUNCTION_ARGS);
extern Datum after_temporal_box(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_box(PG_FUNCTION_ARGS);

extern Datum left_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overleft_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overright_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum right_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum before_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overbefore_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum after_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum overafter_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Bounding box operators
 * File BoundBoxOps.c
 *****************************************************************************/

extern bool overlaps_box_box_internal(const BOX *box1, const BOX *box2);
extern bool contained_box_box_internal(const BOX *box1, const BOX *box2);
extern bool contains_box_box_internal(const BOX *box1, const BOX *box2);
extern bool same_box_box_internal(const BOX *box1, const BOX *box2);
extern int box_cmp_internal(const BOX *box1, const BOX *box2);
extern size_t temporal_bbox_size(Oid valuetypid);
extern bool temporal_bbox_eq(Oid valuetypid, void *box1, void *box2);
extern bool temporalinst_make_bbox(void *bbox, Datum value, TimestampTz t,  
	Oid valuetypid);
extern bool temporali_make_bbox(void *bbox, TemporalInst **inst, int count);
extern bool temporalseq_make_bbox(void *bbox, TemporalInst** inst, int count, 
	bool lower_inc, bool upper_inc);
extern bool temporals_make_bbox(void *bbox, TemporalSeq **seqs, int count);

extern Period *box_to_period_internal(BOX *box);

extern bool contains_box_datum_internal(BOX *box, Datum d, Oid valuetypid);
extern bool contained_box_datum_internal(BOX *box, Datum d, Oid valuetypid);
extern bool overlaps_box_datum_internal(BOX *box, Datum d, Oid valuetypid);
extern bool same_box_datum_internal(BOX *box, Datum d, Oid valuetypid);

extern bool contains_box_range_internal(BOX *box, RangeType *range, Oid valuetypid);
extern bool contained_box_range_internal(BOX *box, RangeType *range, Oid valuetypid);
extern bool overlaps_box_range_internal(BOX *box, RangeType *range, Oid valuetypid);
extern bool same_box_range_internal(BOX *box, RangeType *range, Oid valuetypid);

extern bool contains_box_timestamp_internal(BOX *box, TimestampTz t);
extern bool contained_box_timestamp_internal(BOX *box, TimestampTz t);
extern bool overlaps_box_timestamp_internal(BOX *box, TimestampTz t);
extern bool same_box_timestamp_internal(BOX *box, TimestampTz t);

extern bool contains_box_period_internal(BOX *box, Period *p);
extern bool contained_box_period_internal(BOX *box, Period *p);
extern bool overlaps_box_period_internal(BOX *box, Period *p);
extern bool same_box_period_internal(BOX *box, Period *p);

extern void base_to_box(BOX *box, Datum value, Oid valuetypid);
extern void range_to_box(BOX *box, RangeType *r);
extern void timestamp_to_box(BOX *box, TimestampTz t);
extern void timestampset_to_box(BOX *box, TimestampSet *ts);
extern void period_to_box(BOX *box, Period *p);
extern void periodset_to_box(BOX *box, PeriodSet *ps);

extern Datum base_timestamp_to_box(PG_FUNCTION_ARGS);
extern Datum base_period_to_box(PG_FUNCTION_ARGS);
extern Datum range_timestamp_to_box(PG_FUNCTION_ARGS);
extern Datum range_period_to_box(PG_FUNCTION_ARGS);

extern BOX *base_timestamp_to_box_internal(Datum value, TimestampTz t, Oid valuetypid);
extern BOX *base_period_to_box_internal(Datum value, Period *p, Oid valuetypid);
extern BOX *range_timestamp_to_box_internal(RangeType *range, TimestampTz t);
extern BOX *range_period_to_box_internal(RangeType *range, Period *p);

extern Datum overlaps_bbox_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum contains_bbox_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum contained_bbox_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum same_bbox_timestamp_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_timestampset_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_periodset_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_timestamp(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_timestampset(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_periodset(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum overlaps_bbox_datum_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_box_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_datum(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_box(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contains_bbox_datum_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_box_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_datum(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_box(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contained_bbox_datum_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_box_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_datum(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_box(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum same_bbox_datum_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_box_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_datum(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_box(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);
 
/*****************************************************************************
 * Generic functions for generalizing temporal operators while syncrhonizing
 * File synchronize.c
 *****************************************************************************/

extern TemporalInst *
sync_tfunc2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc2_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc2_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc2_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc2_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc2_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc2_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalSeq *
sync_tfunc2_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalInst *
sync_tfunc2_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc2_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc2_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc2_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*operator)(Datum, Datum), Datum valuetypid);
extern TemporalS *
sync_tfunc2_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc2_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc2_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc2_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum), Datum valuetypid);

/*****************************************************************************/

extern TemporalInst *
sync_tfunc3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalInst *
sync_tfunc3_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc3_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc3_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc3_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalI *
sync_tfunc3_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);
extern TemporalSeq *
sync_tfunc3_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc3_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc3_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc3_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc3_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid);

/*****************************************************************************/

extern TemporalInst *
sync_tfunc4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalInst *
sync_tfunc4_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalInst *
sync_tfunc4_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalI *
sync_tfunc4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalInst *
sync_tfunc4_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalInst *
sync_tfunc4_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalI *
sync_tfunc4_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalI *
sync_tfunc4_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalSeq *
sync_tfunc4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalInst *
sync_tfunc4_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalInst *
sync_tfunc4_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalI *
sync_tfunc4_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalI *
sync_tfunc4_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
extern TemporalS *
sync_tfunc4_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc4_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc4_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
	
extern Temporal *
sync_tfunc4_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid);
	
/*****************************************************************************
 * Generic functions for generalizing temporal operators
 * Used for arithmetic, Boolean, comparison, and geometric distance 
 * File GenericOps.c
 *****************************************************************************/

extern TemporalInst *tfunc1_temporalinst(TemporalInst *inst, 
    Datum (*func)(Datum), Oid valuetypid, bool mustfree);
extern TemporalSeq *tfunc1_temporalseq(TemporalSeq *seq, 
	Datum (*func)(Datum), Oid valuetypid, bool mustfree);
extern TemporalS *tfunc1_temporals(TemporalS *ts, 
	Datum (*func)(Datum), Oid valuetypid, bool mustfree);
extern Temporal *tfunc1_temporal(Temporal *temp, 
Datum (*operator)(Datum), Oid valuetypid, bool mustfree);

extern TemporalInst *tfunc2_temporalinst(TemporalInst *inst, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid, bool mustfree);
extern Temporal *tfunc2_temporal(Temporal *temp, Datum param,
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool mustfree);

extern TemporalInst *tfunc2_temporalinst_base(TemporalInst *inst, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);
extern TemporalI *tfunc2_temporali_base(TemporalI *ti, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);
extern TemporalSeq *tfunc2_temporalseq_base(TemporalSeq *seq, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);
extern TemporalS *tfunc2_temporals_base(TemporalS *ts, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);

extern TemporalInst *tfunc2_temporalinst_temporalinst(TemporalInst *inst1, 
	TemporalInst *inst2, Datum (*operator)(Datum, Datum), Oid valuetypid);
extern TemporalI *tfunc2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid);
extern TemporalSeq *tfunc2_temporalseq_temporalseq(TemporalSeq *seq1, 
	TemporalSeq *seq2, Datum (*operator)(Datum, Datum),  Oid valuetypid);
extern TemporalS *tfunc2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid);

extern Temporal *tfunc2_temporal_base(Temporal *temp, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert);
extern Temporal *tfunc2_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid);
	
/*****************************************************************************/

extern TemporalInst *tfunc3_temporalinst_base(TemporalInst *inst, Datum value, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert);
extern TemporalI *tfunc3_temporali_base(TemporalI *ti, Datum value, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert);
extern TemporalSeq *tfunc3_temporalseq_base(TemporalSeq *seq, Datum value, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert);
extern TemporalS *tfunc3_temporals_base(TemporalS *ts, Datum value, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert);	
	
extern TemporalInst *tfunc3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum dist, Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);
extern TemporalI *tfunc3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, Datum dist, 
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);
extern TemporalSeq *tfunc3_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);
extern TemporalS *tfunc3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);

/*****************************************************************************/

extern TemporalInst *tfunc4_temporalinst_base(TemporalInst *inst, Datum value,  
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern TemporalI *tfunc4_temporali_base(TemporalI *ti, Datum d, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern TemporalSeq *tfunc4_temporalseq_base(TemporalSeq *seq, Datum d, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern TemporalS *tfunc4_temporals_base(TemporalS *ts, Datum d, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);

extern TemporalInst *tfunc4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);
extern TemporalI *tfunc4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);
extern TemporalSeq *tfunc4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);
extern TemporalS *tfunc4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);

extern Temporal *tfunc4_temporal_base(Temporal *temp, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern Temporal *tfunc4_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);

/*****************************************************************************/

extern TemporalS *tfunc2_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, 
	TemporalSeq *seq2, Datum (*operator)(Datum, Datum), Oid valuetypid);
extern TemporalS *tfunc2_temporals_temporals_crossdisc(TemporalS *ts1, 
	TemporalS *ts2, Datum (*operator)(Datum, Datum), Oid valuetypid);

extern TemporalS *tfunc3_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);
extern TemporalS *tfunc3_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Oid valuetypid);

extern TemporalS *tfunc4_temporalseq_base_crossdisc(TemporalSeq *seq, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern TemporalS *tfunc4_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);

extern TemporalS *tfunc4_temporals_base_crossdisc(TemporalS *ts, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert);
extern TemporalS *tfunc4_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid);

/*****************************************************************************
 * Functions for GiST indexes defined in IndexGistTemporal.c
 *****************************************************************************/

extern Datum gist_temporal_consistent(PG_FUNCTION_ARGS);
extern Datum gist_temporalinst_compress(PG_FUNCTION_ARGS);
extern Datum gist_temporali_compress(PG_FUNCTION_ARGS);
extern Datum gist_temporalseq_compress(PG_FUNCTION_ARGS);
extern Datum gist_temporals_compress(PG_FUNCTION_ARGS);
extern Datum gist_temporal_compress(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Functions for SP-GiST indexes defined in IndexGistTnumber.c
 *****************************************************************************/

extern Datum gist_tintinst_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tfloatinst_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tnumberinst_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumberi_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumberseq_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumbers_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_compress(PG_FUNCTION_ARGS);
extern Datum gist_tintinst_fetch(PG_FUNCTION_ARGS);
extern Datum gist_tfloatinst_fetch(PG_FUNCTION_ARGS);

/* The following functions are also called by IndexSpgistTnumber.c */
extern bool index_leaf_consistent_box(BOX *key, BOX *query, StrategyNumber strategy);

/*****************************************************************************
 * Functions for SP-GiST indexes defined in IndexSpgistTemporal.c
 *****************************************************************************/

extern Datum spgist_temporal_config(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporal_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_leaf_consistent(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Functions for SP-GiST indexes defined in IndexSpgistTnumber.c
 *****************************************************************************/

extern Datum spgist_tnumber_config(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberinst_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberi_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberseq_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tnumbers_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberinst_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberi_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberseq_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tnumbers_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tnumber_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberinst_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberi_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tnumberseq_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tnumbers_leaf_consistent(PG_FUNCTION_ARGS);
 
/*****************************************************************************
 * Moving window temporal aggregate functions defined in WAggregateFuncs.c
 *****************************************************************************/

extern Datum tint_wmin_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wmin_transfn(PG_FUNCTION_ARGS);
extern Datum tint_wmax_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wmax_transfn(PG_FUNCTION_ARGS);
extern Datum tint_wsum_transfn(PG_FUNCTION_ARGS);
extern Datum tfloat_wsum_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_wcount_transfn(PG_FUNCTION_ARGS);
extern Datum temporal_wavg_transfn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
