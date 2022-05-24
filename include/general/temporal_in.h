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
 * @file temporal_in.h
 * Input of temporal types in WKB (Well-Known Binary) format
 */

#ifndef __TEMPORAL_IN_H__
#define __TEMPORAL_IN_H__

/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************
 * Input in EWKB format
 * Please refer to the files temporal_out.c and tpoint_out.c where the binary
 * format is explained
 *****************************************************************************/

/**
 * Structure used for passing the parse state between the parsing functions.
 */
typedef struct
{
  const uint8_t *wkb;  /**< Points to start of WKB */
  size_t wkb_size;     /**< Expected size of WKB */
  bool swap_bytes;     /**< Do an endian flip? */
  uint8_t subtype;     /**< Current subtype we are handling */
  int32_t srid;        /**< Current SRID we are handling */
  bool hasz;           /**< Z? */
  bool geodetic;       /**< Geodetic? */
  bool has_srid;       /**< SRID? */
  bool linear;         /**< Linear interpolation? */
  const uint8_t *pos;  /**< Current parse position */
} wkb_parse_state;

/*****************************************************************************/

extern char byte_from_wkb_state(wkb_parse_state *s);
extern uint32_t int32_from_wkb_state(wkb_parse_state *s);
extern uint64_t int64_from_wkb_state(wkb_parse_state *s);
extern double double_from_wkb_state(wkb_parse_state *s);
extern TimestampTz timestamp_from_wkb_state(wkb_parse_state *s);
extern void temporal_bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc,
  bool *upper_inc);

/*****************************************************************************/

#endif /* __TEMPORAL_IN_H__ */
