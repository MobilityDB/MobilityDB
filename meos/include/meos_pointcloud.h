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
 * @brief API of the Mobility Engine Open Source (MEOS) library — pgpointcloud
 * base types (pcpoint, pcpatch).
 *
 * Phase 8D scope: opaque byte-level helpers. Set types (pcpointset,
 * pcpatchset) arrive in Phase 8E; temporal types (tpcpoint, tpcpatch) in
 * Phase 8G.
 */

#ifndef __MEOS_POINTCLOUD_H__
#define __MEOS_POINTCLOUD_H__

/* C */
#include <stdbool.h>
#include <stdint.h>

/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Type definitions
 *
 * Opaque structures — binary-compatible with pgpointcloud's SERIALIZED_POINT
 * and SERIALIZED_PATCH varlena layouts (see pointcloud-pg/pgsql/pc_pgsql.h).
 * At the MEOS layer they are handled as opaque byte blobs; dimension-level
 * access requires the pgpointcloud schema (looked up by pcid from the
 * pointcloud_formats PG catalog table) and is a later-phase concern.
 *****************************************************************************/

typedef struct Pcpoint Pcpoint;
typedef struct Pcpatch Pcpatch;

/******************************************************************************
 * Functions for pcpoint
 ******************************************************************************/

/* Input and output */

extern Pcpoint *pcpoint_in(const char *str);
extern char *pcpoint_out(const Pcpoint *pt, int maxdd);
extern Pcpoint *pcpoint_from_hexwkb(const char *hexwkb);
extern char *pcpoint_as_hexwkb(const Pcpoint *pt);

/* Constructor */

extern Pcpoint *pcpoint_copy(const Pcpoint *pt);

/* Accessor */

extern uint32_t pcpoint_pcid(const Pcpoint *pt);
extern uint32 pcpoint_hash(const Pcpoint *pt);
extern uint64 pcpoint_hash_extended(const Pcpoint *pt, uint64 seed);

/* Comparison */

extern int pcpoint_cmp(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_eq(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_ne(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_lt(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_le(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_gt(const Pcpoint *pt1, const Pcpoint *pt2);
extern bool pcpoint_ge(const Pcpoint *pt1, const Pcpoint *pt2);

/******************************************************************************
 * Functions for pcpatch
 ******************************************************************************/

/* Input and output */

extern Pcpatch *pcpatch_in(const char *str);
extern char *pcpatch_out(const Pcpatch *pa, int maxdd);
extern Pcpatch *pcpatch_from_hexwkb(const char *hexwkb);
extern char *pcpatch_as_hexwkb(const Pcpatch *pa);

/* Constructor */

extern Pcpatch *pcpatch_copy(const Pcpatch *pa);

/* Accessor */

extern uint32_t pcpatch_pcid(const Pcpatch *pa);
extern uint32_t pcpatch_npoints(const Pcpatch *pa);
extern uint32 pcpatch_hash(const Pcpatch *pa);
extern uint64 pcpatch_hash_extended(const Pcpatch *pa, uint64 seed);

/* Comparison */

extern int pcpatch_cmp(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_eq(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_ne(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_lt(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_le(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_gt(const Pcpatch *pa1, const Pcpatch *pa2);
extern bool pcpatch_ge(const Pcpatch *pa1, const Pcpatch *pa2);

/*****************************************************************************/

#endif /* __MEOS_POINTCLOUD_H__ */
