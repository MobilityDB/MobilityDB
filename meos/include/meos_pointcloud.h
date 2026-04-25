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
 * base types (pcpoint, pcpatch), set types (pcpointset, pcpatchset), the
 * TPCBox bounding box, and the lifted temporal types tpcpoint / tpcpatch.
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
/* Forward decl for the parsed pgpointcloud schema; full layout is in
 * libpc.a's @c pc_api.h.  Schema-aware MEOS helpers take this opaque
 * pointer; obtain one via @ref meos_pc_schema. */
typedef struct PCSCHEMA PCSCHEMA;

/**
 * @brief Bounding box for pgpointcloud temporal types.
 *
 * Mirrors STBox (spatiotemporal box) but carries an additional @p pcid
 * field — pgpointcloud schema id — because TPCBoxes from different
 * schemas cannot meaningfully be compared / unioned (the underlying
 * dimensions are schema-specific). Fixed-size struct; no varlena.
 *
 * Flag bits live in @p MEOS_FLAGS_* (see @p meos_internal.h): @p X
 * (bounds present), @p Z (z-dimension present), @p T (time span
 * present), @p GEODETIC (geographic coords). A TPCBox must have at
 * least one of X or T.
 */
typedef struct
{
  Span period;        /**< time span */
  double xmin;        /**< minimum x value */
  double ymin;        /**< minimum y value */
  double zmin;        /**< minimum z value */
  double xmax;        /**< maximum x value */
  double ymax;        /**< maximum y value */
  double zmax;        /**< maximum z value */
  int32_t srid;       /**< SRID */
  uint32_t pcid;      /**< pgpointcloud schema id */
  int16 flags;        /**< flags */
  char padding[6];    /**< explicit pad to 8-byte alignment */
} TPCBox;

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
  #define VALIDATE_TPCBOX(box, ret) \
    do { \
          if (! ensure_not_null((void *) (box)) ) \
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
  #define VALIDATE_TPCBOX(box, ret) \
    do { assert(box); } while (0)
#endif /* MEOS */

/******************************************************************************
 * Functions for pcpoint
 ******************************************************************************/

/* Input and output */

extern Pcpoint *pcpoint_hex_in(const char *str);
extern char *pcpoint_hex_out(const Pcpoint *pt, int maxdd);
extern Pcpoint *pcpoint_from_hexwkb(const char *hexwkb);
extern char *pcpoint_as_hexwkb(const Pcpoint *pt);

/* Constructor */

extern Pcpoint *pcpoint_copy(const Pcpoint *pt);

/* Accessor */

extern uint32_t pcpoint_get_pcid(const Pcpoint *pt);
extern uint32 pcpoint_hash(const Pcpoint *pt);
extern uint64 pcpoint_hash_extended(const Pcpoint *pt, uint64 seed);

/* Schema-aware coordinate accessors.  All write the result through @p out
 * and return @p true on success; @p false (without erroring) when the
 * requested dimension is absent from the schema or could not be read. */

extern bool pcpoint_get_x(const Pcpoint *pt, PCSCHEMA *schema, double *out);
extern bool pcpoint_get_y(const Pcpoint *pt, PCSCHEMA *schema, double *out);
extern bool pcpoint_get_z(const Pcpoint *pt, PCSCHEMA *schema, double *out);
extern bool pcpoint_get_dim(const Pcpoint *pt, PCSCHEMA *schema,
  const char *name, double *out);

/* Schema-aware conversion to TPCBox (degenerate single-point bbox).
 * Returns @p NULL if the schema lacks the required X/Y dimensions. */

extern TPCBox *pcpoint_to_tpcbox(const Pcpoint *pt, PCSCHEMA *schema);

/* Schema lookup — see @c meos_schema_hook.h for the cache + register API
 * used by embedders.  Most callers want this fast path: returns a parsed
 * @c PCSCHEMA from the MEOS-owned cache, falling back to the installed
 * resolution hook (catalog scan in a PG backend) on miss. */

extern PCSCHEMA *meos_pc_schema(uint32_t pcid);

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

extern Pcpatch *pcpatch_hex_in(const char *str);
extern char *pcpatch_hex_out(const Pcpatch *pa, int maxdd);
extern Pcpatch *pcpatch_from_hexwkb(const char *hexwkb);
extern char *pcpatch_as_hexwkb(const Pcpatch *pa);

/* Constructor */

extern Pcpatch *pcpatch_copy(const Pcpatch *pa);

/* Accessor */

extern uint32_t pcpatch_get_pcid(const Pcpatch *pa);
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

/******************************************************************************
 * Functions for the TPCBox bounding box
 ******************************************************************************/

/* Input and output */

extern TPCBox *tpcbox_in(const char *str);
extern char *tpcbox_out(const TPCBox *box, int maxdd);

/* Constructor */

extern TPCBox *tpcbox_make(bool hasx, bool hasz, bool hast, bool geodetic,
  int32_t srid, uint32_t pcid, double xmin, double xmax, double ymin,
  double ymax, double zmin, double zmax, const Span *period);
extern TPCBox *tpcbox_copy(const TPCBox *box);

/* Conversion */

extern TPCBox *pcpatch_to_tpcbox(const Pcpatch *pa, int32_t srid);

/* Accessors */

extern bool tpcbox_hasx(const TPCBox *box);
extern bool tpcbox_hasz(const TPCBox *box);
extern bool tpcbox_hast(const TPCBox *box);
extern bool tpcbox_geodetic(const TPCBox *box);
extern bool tpcbox_xmin(const TPCBox *box, double *result);
extern bool tpcbox_xmax(const TPCBox *box, double *result);
extern bool tpcbox_ymin(const TPCBox *box, double *result);
extern bool tpcbox_ymax(const TPCBox *box, double *result);
extern bool tpcbox_zmin(const TPCBox *box, double *result);
extern bool tpcbox_zmax(const TPCBox *box, double *result);
extern bool tpcbox_tmin(const TPCBox *box, TimestampTz *result);
extern bool tpcbox_tmax(const TPCBox *box, TimestampTz *result);
extern int32_t tpcbox_srid(const TPCBox *box);
extern uint32_t tpcbox_pcid(const TPCBox *box);

/* Transformation */

extern void tpcbox_expand(const TPCBox *box1, TPCBox *box2);

/* Set operations (same-pcid / same-srid required) */

extern TPCBox *union_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2,
  bool strict);
extern bool inter_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2,
  TPCBox *result);
extern TPCBox *intersection_tpcbox_tpcbox(const TPCBox *box1,
  const TPCBox *box2);

/* Topological predicates */

extern bool contains_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool contained_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overlaps_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool same_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool adjacent_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);

/* Comparison */

extern int tpcbox_cmp(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_eq(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_ne(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_lt(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_le(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_gt(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_ge(const TPCBox *box1, const TPCBox *box2);

/* Position predicates — strict and overlap variants across X / Y / Z
 * / time. A predicate evaluates only on dimensions both operands
 * carry; returns false otherwise. */

extern bool left_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overleft_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool right_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overright_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool below_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overbelow_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool above_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overabove_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool front_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overfront_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool back_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overback_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool before_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overbefore_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool after_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);
extern bool overafter_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);

/* Validity helpers */

extern bool ensure_same_pcid_tpcbox(const TPCBox *box1, const TPCBox *box2);

/*****************************************************************************/

#endif /* __MEOS_POINTCLOUD_H__ */
