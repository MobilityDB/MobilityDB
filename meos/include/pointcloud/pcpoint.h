/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Internal declarations for pcpoint — binary-compatible with
 * pgpointcloud's SERIALIZED_POINT varlena
 */

#ifndef __PCPOINT_H__
#define __PCPOINT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <liblwgeom.h>
#include <meos_pointcloud.h>

/*****************************************************************************
 * Struct layout
 *
 * Binary-compatible with pgpointcloud's `SERIALIZED_POINT` (see
 * pointcloud-pg/pgsql/pc_pgsql.h). Treated as opaque byte blob at the
 * MEOS layer: `pcid` is at a fixed offset so same-schema checks work
 * without loading the schema XML from the `pointcloud_formats` table.
 *****************************************************************************/

struct Pcpoint
{
  int32 vl_len_;        /**< Varlena header (do not touch directly!) */
  uint32_t pcid;        /**< Schema id — FK to `pointcloud_formats` */
  uint8_t data[FLEXIBLE_ARRAY_MEMBER]; /**< Packed dimension bytes */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#if MEOS
  #define DatumGetPcpointP(X)       ((Pcpoint *) DatumGetPointer(X))
#else
  #define DatumGetPcpointP(X)       ((Pcpoint *) PG_DETOAST_DATUM(X))
#endif /* MEOS */
#define PcpointPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_PCPOINT_P(X)      DatumGetPcpointP(PG_GETARG_DATUM(X))
#define PG_RETURN_PCPOINT_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************
 * Validity helpers
 *****************************************************************************/

extern bool ensure_same_pcid_pcpoint(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool ensure_valid_pcpointset_pcpoint(const Set *s, const Pcpoint *pt);

/*****************************************************************************
 * Internal byte-level helpers
 *****************************************************************************/

extern Pcpoint *pcpoint_parse(const char **str, bool end);
extern uint8_t pc_wkb_endian(uint8_t variant);
extern void pc_wkb_int32_flip(uint8_t *ptr);
extern uint8_t *pcpoint_to_wkb_bytes(const Pcpoint *pt, uint8_t variant,
  size_t *size_out);
extern size_t pcpoint_meaningful_size(const Pcpoint *pt);

/* The offset of the bytes stored after the pcid, a point and a patch both
 * starting with the varlena header and the pcid */
#define PC_BYTES_OFFSET (VARHDRSZ + sizeof(uint32_t))

extern int pointcloud_bytes_cmp(uint32_t pcid1, const uint8_t *b1, size_t n1,
  uint32_t pcid2, const uint8_t *b2, size_t n2);
extern size_t pointcloud_sortkey_size(const uint8_t *b, size_t n);
extern uint8_t *pointcloud_sortkey_put(uint32_t pcid, const uint8_t *b,
  size_t n, uint8_t *out);
extern size_t pcpoint_sortkey_size(const Pcpoint *pt);
extern uint8_t *pcpoint_sortkey_put(const Pcpoint *pt, uint8_t *out);
extern double *pcpoint_dims(const Pcpoint *pt, int *count);
extern PCSCHEMA *pcpoint_schema(const Pcpoint *pt);

/*****************************************************************************
 * Accessors and conversion reading a point in a given schema
 *****************************************************************************/

extern bool pcpoint_schema_get_x(const Pcpoint *pt, PCSCHEMA *schema,
  double *out);
extern bool pcpoint_schema_get_y(const Pcpoint *pt, PCSCHEMA *schema,
  double *out);
extern bool pcpoint_schema_get_z(const Pcpoint *pt, PCSCHEMA *schema,
  double *out);
extern bool pcpoint_schema_get_dim(const Pcpoint *pt, PCSCHEMA *schema,
  const char *name, double *out);
extern TPCBox *pcpoint_schema_to_tpcbox(const Pcpoint *pt, PCSCHEMA *schema);
extern bool pc_transform_lookup(uint32_t pcid_from, uint32_t pcid_to,
  const PCSCHEMA **schema_to, LWPROJ **pj);
extern Pcpoint *pcpoint_transf_pj(const Pcpoint *pt, uint32_t pcid_to,
  const PCSCHEMA *to, const LWPROJ *pj);

/*****************************************************************************/

#endif /* __PCPOINT_H__ */
