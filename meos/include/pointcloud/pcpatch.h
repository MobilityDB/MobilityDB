/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Internal declarations for pcpatch — binary-compatible with
 * pgpointcloud's SERIALIZED_PATCH varlena.
 */

#ifndef __PCPATCH_H__
#define __PCPATCH_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>

/*****************************************************************************
 * Struct layout
 *
 * Binary-compatible with pgpointcloud's `SERIALIZED_PATCH`. `PCBOUNDS` is
 * 4 doubles (xmin,ymin,xmax,ymax); we inline the layout as a plain array
 * to avoid dragging pc_api.h into MEOS internal headers.
 *****************************************************************************/

struct Pcpatch
{
  int32 vl_len_;        /**< Varlena header */
  uint32_t pcid;        /**< Schema id */
  uint32_t compression; /**< Compression scheme (0 = uncompressed) */
  uint32_t npoints;     /**< Number of points in this patch */
  double bounds[4];     /**< Mirrors upstream PCBOUNDS:
                             [0]=xmin, [1]=xmax, [2]=ymin, [3]=ymax */
  uint8_t data[FLEXIBLE_ARRAY_MEMBER]; /**< Compressed point data */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#if MEOS
  #define DatumGetPcpatchP(X)       ((Pcpatch *) DatumGetPointer(X))
#else
  #define DatumGetPcpatchP(X)       ((Pcpatch *) PG_DETOAST_DATUM(X))
#endif /* MEOS */
#define PcpatchPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_PCPATCH_P(X)      DatumGetPcpatchP(PG_GETARG_DATUM(X))
#define PG_RETURN_PCPATCH_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************
 * Validity helpers
 *****************************************************************************/

extern bool ensure_same_pcid_pcpatch(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool ensure_valid_pcpatchset_pcpatch(const Set *s, const Pcpatch *pa);

/*****************************************************************************
 * Internal byte-level helpers
 *****************************************************************************/

extern Pcpatch *pcpatch_parse(const char **str, bool end);

/*****************************************************************************/

#endif /* __PCPATCH_H__ */
