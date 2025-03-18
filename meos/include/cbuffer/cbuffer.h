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
 * @brief Functions for temporal buffers.
 */

#ifndef __CBUFFER_H__
#define __CBUFFER_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/* Structure to represent circular buffers */

struct Cbuffer
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  double radius;        /**< radius */
  Datum point;          /**< First 8 bytes of the point which is passed by 
                             reference. The extra bytes needed are added upon 
                             creation. */
  /* variable-length data follows */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

/* Cbuffer */
#define DatumGetCbufferP(X)         ((Cbuffer *) DatumGetPointer(X))
#define CbufferPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_CBUFFER_P(X)      DatumGetCbufferP(PG_GETARG_DATUM(X))
#define PG_RETURN_CBUFFER_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************/

/* Collinear functions */

extern bool cbuffer_collinear(Cbuffer *cbuf1, Cbuffer *cbuf2, Cbuffer *cbuf3,
  double ratio);

/* Input/output functions */

extern char *cbuffer_wkt_out(Datum value, int maxdd, bool extended);

extern Datum datum_cbuffer_round(Datum buffer, Datum size);
extern Cbuffer *cbuffer_round(const Cbuffer *cbuf, int maxdd);
extern Cbuffer **cbufferarr_round(const Cbuffer **cbufarr, int count, int maxdd);
extern Set *cbufferset_round(const Set *s, int maxdd);
extern Temporal *tcbuffer_round(const Temporal *temp, int maxdd);
  
/*****************************************************************************/

#endif /* __CBUFFER_H__ */
