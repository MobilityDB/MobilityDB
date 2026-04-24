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
  double bounds[4];     /**< xmin, ymin, xmax, ymax */
  uint8_t data[FLEXIBLE_ARRAY_MEMBER]; /**< Compressed point data */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetPcpatchP(X)         ((Pcpatch *) PG_DETOAST_DATUM(X))
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
