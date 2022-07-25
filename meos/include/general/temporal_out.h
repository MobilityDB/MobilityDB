/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Basic routines for spans (a.k.a. ranges) composed of two `Datum`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#ifndef __TEMPORAL_OUT_H__
#define __TEMPORAL_OUT_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
// #include <meos.h>
#include "general/temporal_catalog.h"

/*
 * Export functions
 */

/*****************************************************************************
 * Definitions taken from the file liblwgeom_internal.h 
 *****************************************************************************/

/* Any (absolute) values outside this range will be printed in scientific
 * notation */
#define OUT_MIN_DOUBLE 1E-8
#define OUT_MAX_DOUBLE 1E15
#define OUT_DEFAULT_DECIMAL_DIGITS 15

/* 17 digits are sufficient for round-tripping
 * Then we might add up to 8 (from OUT_MIN_DOUBLE) max leading zeroes (or
 * 2 digits for "e+") */
#define OUT_MAX_DIGITS 17 + 8

/* Limit for the max amount of characters that a double can use, including dot
 * and sign */
#define OUT_MAX_BYTES_DOUBLE (1 /* Sign */ + 2 /* 0.x */ + OUT_MAX_DIGITS)
#define OUT_DOUBLE_BUFFER_SIZE OUT_MAX_BYTES_DOUBLE + 1 /* +1 including NULL */

/*****************************************************************************/

extern uint8_t *datum_as_wkb(Datum value, mobdbType type, uint8_t variant,
  size_t *size_out);
extern char *datum_as_hexwkb(Datum value, mobdbType type, uint8_t variant,
  size_t *size);

/*****************************************************************************/

#endif /* __TEMPORAL_OUT_H__ */
