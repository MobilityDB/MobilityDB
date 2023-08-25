/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @file
 * @brief Error handling.
 */

/* C */
#include <errno.h>
/* MEOS */
#include <meos.h>

typedef enum
{
  MEOS_SUCCESS                  = 0,  // Successful operation

  MEOS_ERR_INTERNAL_ERROR       = 1,  // Unspecified internal error
  MEOS_ERR_INTERNAL_TYPE_ERROR  = 2,  // Internal type error
  MEOS_ERR_VALUE_OUT_OF_RANGE   = 3,  // Internal out of range error
  MEOS_ERR_MEMORY_ALLOC_ERROR   = 4,  // Internal malloc error
  MEOS_ERR_AGGREGATION_ERROR    = 5,  // Internal aggregation error

  MEOS_ERR_INVALID_ARG          = 10, // Invalid argument
  MEOS_ERR_INVALID_ARG_TYPE     = 11, // Invalid argument type
  MEOS_ERR_INVALID_ARG_VALUE    = 12, // Invalid argument value

  MEOS_ERR_MFJSON_INPUT         = 20, // MFJSON input error
  MEOS_ERR_MFJSON_OUTPUT        = 21, // MFJSON output error
  MEOS_ERR_TEXT_INPUT           = 22, // Text input error
  MEOS_ERR_TEXT_OUTPUT          = 23, // Text output error
  MEOS_ERR_WKB_INPUT            = 24, // WKB input error
  MEOS_ERR_WKB_OUTPUT           = 25, // WKB output error
  MEOS_ERR_GEOJSON_INPUT        = 26, // GEOJSON input error
  MEOS_ERR_GEOJSON_OUTPUT       = 27, // GEOJSON output error

} errorCode;

extern void meos_error(int errlevel, int errcode, char *format, ...); 

/* Set or read error level */

extern int mobdb_errno(void);
extern int mobdb_errno_set(int err);
extern int mobdb_errno_restore(int err);
extern int mobdb_errno_reset(void);

/*****************************************************************************/