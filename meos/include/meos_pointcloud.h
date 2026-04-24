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

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

#if MEOS
  #define VALIDATE_PCPOINTSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_PCPOINTSET) ) \
           return (ret); \
    } while (0)
  #define VALIDATE_PCPATCHSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type((set), T_PCPATCHSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_PCPOINTSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_PCPOINTSET); \
    } while (0)
  #define VALIDATE_PCPATCHSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_PCPATCHSET); \
    } while (0)
#endif /* MEOS */

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

/******************************************************************************
 * Functions for pcpoint sets
 ******************************************************************************/

/* Input and output */

extern Set *pcpointset_in(const char *str);
extern char *pcpointset_out(const Set *s, int maxdd);

/* Constructor */

extern Set *pcpointset_make(Pcpoint **values, int count);

/* Conversion */

extern Set *pcpoint_to_set(const Pcpoint *pt);

/* Accessor */

extern Pcpoint *pcpointset_start_value(const Set *s);
extern Pcpoint *pcpointset_end_value(const Set *s);
extern bool pcpointset_value_n(const Set *s, int n, Pcpoint **result);
extern Pcpoint **pcpointset_values(const Set *s);

/* Set operations */

extern bool contains_set_pcpoint(const Set *s, Pcpoint *pt);
extern bool contained_pcpoint_set(const Pcpoint *pt, const Set *s);
extern Set *intersection_pcpoint_set(const Pcpoint *pt, const Set *s);
extern Set *intersection_set_pcpoint(const Set *s, const Pcpoint *pt);
extern Set *minus_pcpoint_set(const Pcpoint *pt, const Set *s);
extern Set *minus_set_pcpoint(const Set *s, const Pcpoint *pt);
extern Set *union_pcpoint_set(const Pcpoint *pt, const Set *s);
extern Set *union_set_pcpoint(const Set *s, const Pcpoint *pt);

/* Aggregate transition */

extern Set *pcpoint_union_transfn(Set *state, const Pcpoint *pt);

/******************************************************************************
 * Functions for pcpatch sets
 ******************************************************************************/

/* Input and output */

extern Set *pcpatchset_in(const char *str);
extern char *pcpatchset_out(const Set *s, int maxdd);

/* Constructor */

extern Set *pcpatchset_make(Pcpatch **values, int count);

/* Conversion */

extern Set *pcpatch_to_set(const Pcpatch *pa);

/* Accessor */

extern Pcpatch *pcpatchset_start_value(const Set *s);
extern Pcpatch *pcpatchset_end_value(const Set *s);
extern bool pcpatchset_value_n(const Set *s, int n, Pcpatch **result);
extern Pcpatch **pcpatchset_values(const Set *s);

/* Set operations */

extern bool contains_set_pcpatch(const Set *s, Pcpatch *pa);
extern bool contained_pcpatch_set(const Pcpatch *pa, const Set *s);
extern Set *intersection_pcpatch_set(const Pcpatch *pa, const Set *s);
extern Set *intersection_set_pcpatch(const Set *s, const Pcpatch *pa);
extern Set *minus_pcpatch_set(const Pcpatch *pa, const Set *s);
extern Set *minus_set_pcpatch(const Set *s, const Pcpatch *pa);
extern Set *union_pcpatch_set(const Pcpatch *pa, const Set *s);
extern Set *union_set_pcpatch(const Set *s, const Pcpatch *pa);

/* Aggregate transition */

extern Set *pcpatch_union_transfn(Set *state, const Pcpatch *pa);

/*****************************************************************************/

#endif /* __MEOS_POINTCLOUD_H__ */
