/*****************************************************************************
 *
 * tpoint.h
 *	  Functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_H__
#define __TPOINT_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <liblwgeom.h>

#include "temporal.h"

/*****************************************************************************
 * Macros for manipulating the 'typmod' int. An int32_t used as follows:
 * Plus/minus = Top bit.
 * Spare bits = Next 2 bits.
 * SRID = Next 21 bits.
 * TYPE = Next 6 bits.
 * ZM Flags = Bottom 2 bits.
 *****************************************************************************/

/* The following (commented out) definitions are taken from POSTGIS
#define TYPMOD_GET_SRID(typmod) ((((typmod) & 0x0FFFFF00) - ((typmod) & 0x10000000)) >> 8)
#define TYPMOD_SET_SRID(typmod, srid) ((typmod) = (((typmod) & 0xE00000FF) | ((srid & 0x001FFFFF)<<8)))
#define TYPMOD_GET_TYPE(typmod) ((typmod & 0x000000FC)>>2)
#define TYPMOD_SET_TYPE(typmod, type) ((typmod) = (typmod & 0xFFFFFF03) | ((type & 0x0000003F)<<2))
#define TYPMOD_GET_Z(typmod) ((typmod & 0x00000002)>>1)
#define TYPMOD_SET_Z(typmod) ((typmod) = typmod | 0x00000002)
#define TYPMOD_GET_M(typmod) (typmod & 0x00000001)
#define TYPMOD_SET_M(typmod) ((typmod) = typmod | 0x00000001)
#define TYPMOD_GET_NDIMS(typmod) (2+TYPMOD_GET_Z(typmod)+TYPMOD_GET_M(typmod))
*/

/* In order to reuse the above (commented out) macros for manipulating the
   typmod from POSTGIS we need to shift them to take into account that the 
   first 4 bits are taken for the duration type */

#define TYPMOD_DEL_DURATION(typmod) (typmod = typmod >> 4 )
#define TYPMOD_SET_DURATION(typmod, durtype) ((typmod) = typmod << 4 | durtype)

/*****************************************************************************
 * STBOX macros
 *****************************************************************************/

#define DatumGetSTboxP(X)    ((STBOX *) DatumGetPointer(X))
#define STboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_STBOX_P(n) DatumGetSTboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_STBOX_P(x) return STboxPGetDatum(x)

/*****************************************************************************
 * Well-Known Binary (WKB)
 *****************************************************************************/

/* Data type size */
#define WKB_TIMESTAMP_SIZE 8 /* Internal use only */
#define WKB_DOUBLE_SIZE 8 /* Internal use only */
#define WKB_INT_SIZE 4 /* Internal use only */
#define WKB_BYTE_SIZE 1 /* Internal use only */

/* Duration */
#define WKB_TEMPORALINST    1
#define WKB_TEMPORALI       2
#define WKB_TEMPORALSEQ     3
#define WKB_TEMPORALS       4

/* Period bounds */
#define WKB_LOWER_INC   0x01
#define WKB_UPPER_INC   0x02

/* Machine endianness */
#define XDR 0 /* big endian */
#define NDR 1 /* little endian */

/* Variation flags */
#define WKB_ZFLAG           0x10
#define WKB_SRIDFLAG        0x20
#define WKB_LINEAR_INTERP 	0x40
#define WKB_BBOXFLAG        0x80 /* Currently not used */

/*****************************************************************************
 * Miscellaneous functions defined in TemporalPoint.c
 *****************************************************************************/

extern void temporalgeom_init();

/* Input/output functions */

extern Datum tpoint_in(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum tpoint_values(PG_FUNCTION_ARGS);
extern Datum tpoint_stbox(PG_FUNCTION_ARGS);

extern Datum tpoint_ever_eq(PG_FUNCTION_ARGS);
extern Datum tpoint_ever_ne(PG_FUNCTION_ARGS);

extern Datum tpoint_always_eq(PG_FUNCTION_ARGS);
extern Datum tpoint_always_ne(PG_FUNCTION_ARGS);

extern Datum tpoint_values_internal(Temporal *temp);

extern Datum tgeompointi_values(TemporalI *ti);
extern Datum tgeogpointi_values(TemporalI *ti);
extern Datum tpointi_values(TemporalI *ti);

/* Restriction functions */

extern Datum tpoint_at_value(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_value(PG_FUNCTION_ARGS);
extern Datum tpoint_at_values(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_values(PG_FUNCTION_ARGS);
extern Datum tpoints_at_values(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
