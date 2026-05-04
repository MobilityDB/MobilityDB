/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 *****************************************************************************/

/**
 * @brief Internal declarations for pcpoint — binary-compatible with
 * pgpointcloud's SERIALIZED_POINT varlena.
 */

#ifndef __PCPOINT_H__
#define __PCPOINT_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
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

#define DatumGetPcpointP(X)         ((Pcpoint *) PG_DETOAST_DATUM(X))
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

/*****************************************************************************/

#endif /* __PCPOINT_H__ */
