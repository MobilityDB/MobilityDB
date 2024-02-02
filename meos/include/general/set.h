/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief Basic functions for set of (distinct) timestamps.
 */

#ifndef __SET_H__
#define __SET_H__

/*****************************************************************************/

/*
 * fmgr macros for ordered set types
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include "general/meos_catalog.h"

#if MEOS
  #define DatumGetSetP(X)      ((Set *) DatumGetPointer(X))
#else
  #define DatumGetSetP(X)      ((Set *) PG_DETOAST_DATUM(X))
#endif /* MEOS */
#define SetPGetDatum(X)        PointerGetDatum(X)
#define PG_GETARG_SET_P(X)     ((Set *) PG_GETARG_VARLENA_P(X))
#define PG_RETURN_SET_P(X)     PG_RETURN_POINTER(X)

#define MINIDX 0
#define MAXIDX count - 1

/*****************************************************************************
 * Struct definition for the unnest operation
 *****************************************************************************/

/**
 * Structure to represent information about an entry that can be placed
 * to either group without affecting overlap over selected axis ("common entry").
 */
typedef struct
{
  bool done;
  int i;
  int count;
  Set *set;  /* Set to unnest */
  Datum *values;   /* Values obtained by getValues(temp) */
} SetUnnestState;

/*****************************************************************************/

/* General functions */

extern bool ensure_set_isof_type(const Set *s, meosType settype);
extern bool ensure_set_isof_basetype(const Set *s, meosType basetype);
extern bool ensure_same_set_type(const Set *s1, const Set *s2);
extern bool set_find_value(const Set *s, Datum, int *loc);

extern SetUnnestState *set_unnest_state_make(const Set *set);
extern void set_unnest_state_next(SetUnnestState *state);

/*****************************************************************************/

#endif /* __SET_H__ */
