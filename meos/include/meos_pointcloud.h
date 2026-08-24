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
#include <assert.h>      /* for static_assert on STBox/TPCBox layout */
#include <stdbool.h>
#include <stddef.h>      /* for offsetof in the static asserts below */
#include <stdint.h>

/* MEOS */
#include <meos.h>
#include <meos_geo.h>   /* GSERIALIZED — needed by tpcpoint × geometry bridge */

/*****************************************************************************
 * Type definitions
 *
 * Opaque structures — binary-compatible with pgpointcloud's SERIALIZED_POINT
 * and SERIALIZED_PATCH varlena layouts (see pointcloud-pg/pgsql/pc_pgsql.h).
 * At the MEOS layer they are handled as opaque byte blobs; dimension-level
 * access requires the pgpointcloud schema (looked up by pcid from the
 * pointcloud_formats PG catalog table) and is not exposed here.
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
  int16 flags;        /**< flags */
  /* Every field above is byte-identical to STBox, so a TPCBox is read
   * through an @p STBox pointer by the shared spatiotemporal code. The
   * pgpointcloud schema id follows that common prefix. */
  char padding[2];    /**< explicit pad, kept zero: send/recv copy the struct */
  uint32_t pcid;      /**< pgpointcloud schema id */
} TPCBox;

/* A TPCBox begins with a whole STBox: every STBox field sits at the same
 * offset, so the shared spatiotemporal code reads a TPCBox through an
 * @p STBox pointer, and a TPCBox is projected to an STBox by dropping the
 * trailing pcid. The layout-compatibility cast (CREATE CAST tpcbox AS
 * stbox) relies on this, as does the SRID accessor.
 *
 * ⛔ The flags offset is the one that matters most: nearly every box
 * operation reads the X / Z / T / GEODETIC bits, so a divergence there
 * silently misreads every box rather than failing loudly. */
static_assert(offsetof(TPCBox, period) == offsetof(STBox, period),
               "TPCBox and STBox must share the period offset");
static_assert(offsetof(TPCBox, xmin) == offsetof(STBox, xmin),
               "TPCBox and STBox must share the xmin offset");
static_assert(offsetof(TPCBox, ymin) == offsetof(STBox, ymin),
               "TPCBox and STBox must share the ymin offset");
static_assert(offsetof(TPCBox, zmin) == offsetof(STBox, zmin),
               "TPCBox and STBox must share the zmin offset");
static_assert(offsetof(TPCBox, xmax) == offsetof(STBox, xmax),
               "TPCBox and STBox must share the xmax offset");
static_assert(offsetof(TPCBox, ymax) == offsetof(STBox, ymax),
               "TPCBox and STBox must share the ymax offset");
static_assert(offsetof(TPCBox, zmax) == offsetof(STBox, zmax),
               "TPCBox and STBox must share the zmax offset");
static_assert(offsetof(TPCBox, srid) == offsetof(STBox, srid),
               "TPCBox and STBox must share the srid offset");
static_assert(offsetof(TPCBox, flags) == offsetof(STBox, flags),
               "TPCBox and STBox must share the flags offset");
static_assert(offsetof(TPCBox, pcid) >= sizeof(STBox),
               "the pcid of a TPCBox must follow the whole STBox prefix");

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

/* Schema cache — process-global lookup table of parsed PCSCHEMA values
 * keyed by pcid.  In a PG backend the cache is populated lazily via the
 * hook installed by @c mobilitydb_init (catalog scan of
 * @c pointcloud_formats).  In standalone-MEOS programs, the application
 * registers schemas explicitly before calling schema-aware helpers. */

/**
 * @brief One dimension of a point cloud schema, as it is stated
 *
 * The fields are the ones a schema states; @p size and @p byteoffset are
 * absent because the engine computes them from @p interpretation and from
 * the dimensions before this one.
 */
typedef struct
{
  const char *name;           /**< Dimension name, unique within the schema */
  const char *description;    /**< Dimension description, may be @p NULL */
  int32_t position;           /**< Position within the schema, from 1 */
  const char *interpretation; /**< Name of the numeric type stored */
  double scale;               /**< Factor a stored value is multiplied by */
  double offset;              /**< Value added to a scaled stored value */
  bool active;                /**< True when the dimension holds values */
} PCDimensionSpec;

extern PCSCHEMA *meos_pc_schema(uint32_t pcid);
extern void meos_pc_schema_register(uint32_t pcid, PCSCHEMA *schema);
extern PCSCHEMA *meos_pc_schema_from_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims);
extern bool meos_pc_schema_register_dims(uint32_t pcid, int32_t srid,
  const char *compression, const PCDimensionSpec *dims, int ndims);
extern void meos_pc_schema_register_xml(uint32_t pcid, PCSCHEMA *schema,
  const char *xml_text);
extern const char *meos_pc_schema_xml(uint32_t pcid);
extern void meos_pc_schema_clear(void);

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

/* Conversion */

extern GSERIALIZED *pcpatch_to_geom(const Pcpatch *pa);

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
extern Pcpoint **pcpointset_values(const Set *s, int *count);

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
extern Pcpatch **pcpatchset_values(const Set *s, int *count);

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
extern bool tpcbox_tmin_inc(const TPCBox *box, bool *result);
extern bool tpcbox_tmax(const TPCBox *box, TimestampTz *result);
extern bool tpcbox_tmax_inc(const TPCBox *box, bool *result);
extern int32_t tpcbox_srid(const TPCBox *box);
extern uint32_t tpcbox_pcid(const TPCBox *box);
extern STBox *tpcbox_to_stbox(const TPCBox *box);

/* Transformation */

extern TPCBox *tpcbox_round(const TPCBox *box, int maxdd);
extern TPCBox *tpcbox_set_srid(const TPCBox *box, int32_t srid);

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

/*****************************************************************************
 * Temporal pgpointcloud types (tpcpoint / tpcpatch) — value surface
 *****************************************************************************/

#if MEOS
  #define VALIDATE_TPCPOINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TPCPOINT) ) \
           return (ret); \
    } while (0)
  #define VALIDATE_TPCPATCH(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TPCPATCH) ) \
           return (ret); \
    } while (0)
  #define VALIDATE_TPOINTCLOUD(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_tpointcloud_temptype(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TPCPOINT(temp, ret) \
    do { assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TPCPOINT); } while (0)
  #define VALIDATE_TPCPATCH(temp, ret) \
    do { assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TPCPATCH); } while (0)
  #define VALIDATE_TPOINTCLOUD(temp, ret) \
    do { assert(temp); \
      assert(tpointcloud_temptype(((Temporal *) (temp))->temptype)); } while (0)
#endif

/* Conversion */

extern Temporal *tpointcloud_to_tgeompoint(const Temporal *temp);
extern Temporal *tpcpatch_to_tgeometry(const Temporal *temp);

extern TInstant *tpcpointinst_make(const Pcpoint *pt, TimestampTz t);
extern TSequence *tpcpointseq_from_base_tstzset(const Pcpoint *pt, const Set *s);
extern TSequence *tpcpointseq_from_base_tstzspan(const Pcpoint *pt, const Span *sp);
extern TSequenceSet *tpcpointseqset_from_base_tstzspanset(const Pcpoint *pt, const SpanSet *ss);
extern Temporal *tpcpoint_from_base_temp(const Pcpoint *pt, const Temporal *temp);
extern Pcpoint *tpcpoint_start_value(const Temporal *temp);
extern Pcpoint *tpcpoint_end_value(const Temporal *temp);
extern bool tpcpoint_value_n(const Temporal *temp, int n, Pcpoint **result);
extern Pcpoint **tpcpoint_values(const Temporal *temp, int *count);
extern bool tpcpoint_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, Pcpoint **value);
extern Temporal *tpcpoint_at_value(const Temporal *temp, const Pcpoint *pt);
extern Temporal *tpcpoint_minus_value(const Temporal *temp, const Pcpoint *pt);

extern TInstant *tpcpatchinst_make(const Pcpatch *pa, TimestampTz t);
extern TSequence *tpcpatchseq_from_base_tstzset(const Pcpatch *pa, const Set *s);
extern TSequence *tpcpatchseq_from_base_tstzspan(const Pcpatch *pa, const Span *sp);
extern TSequenceSet *tpcpatchseqset_from_base_tstzspanset(const Pcpatch *pa, const SpanSet *ss);
extern Temporal *tpcpatch_from_base_temp(const Pcpatch *pa, const Temporal *temp);
extern Pcpatch *tpcpatch_start_value(const Temporal *temp);
extern Pcpatch *tpcpatch_end_value(const Temporal *temp);
extern bool tpcpatch_value_n(const Temporal *temp, int n, Pcpatch **result);
extern Pcpatch **tpcpatch_values(const Temporal *temp, int *count);
extern bool tpcpatch_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, Pcpatch **value);
extern Temporal *tpcpatch_at_value(const Temporal *temp, const Pcpatch *pa);
extern Temporal *tpcpatch_minus_value(const Temporal *temp, const Pcpatch *pa);

/*****************************************************************************
 * Ever/always and temporal comparisons
 *****************************************************************************/

/* Ever/always comparison functions */

extern int ever_eq_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp);
extern int ever_eq_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt);
extern int ever_eq_tpcpoint_tpcpoint(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp);
extern int ever_ne_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt);
extern int ever_ne_tpcpoint_tpcpoint(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp);
extern int always_eq_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt);
extern int always_eq_tpcpoint_tpcpoint(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_pcpoint_tpcpoint(const Pcpoint *pt, const Temporal *temp);
extern int always_ne_tpcpoint_pcpoint(const Temporal *temp, const Pcpoint *pt);
extern int always_ne_tpcpoint_tpcpoint(const Temporal *temp1,
  const Temporal *temp2);

extern int ever_eq_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp);
extern int ever_eq_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa);
extern int ever_eq_tpcpatch_tpcpatch(const Temporal *temp1,
  const Temporal *temp2);
extern int ever_ne_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp);
extern int ever_ne_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa);
extern int ever_ne_tpcpatch_tpcpatch(const Temporal *temp1,
  const Temporal *temp2);
extern int always_eq_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp);
extern int always_eq_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa);
extern int always_eq_tpcpatch_tpcpatch(const Temporal *temp1,
  const Temporal *temp2);
extern int always_ne_pcpatch_tpcpatch(const Pcpatch *pa, const Temporal *temp);
extern int always_ne_tpcpatch_pcpatch(const Temporal *temp, const Pcpatch *pa);
extern int always_ne_tpcpatch_tpcpatch(const Temporal *temp1,
  const Temporal *temp2);

/* Temporal comparison functions */

extern Temporal *teq_pcpoint_tpcpoint(const Pcpoint *pt,
  const Temporal *temp);
extern Temporal *teq_tpcpoint_pcpoint(const Temporal *temp,
  const Pcpoint *pt);
extern Temporal *tne_pcpoint_tpcpoint(const Pcpoint *pt,
  const Temporal *temp);
extern Temporal *tne_tpcpoint_pcpoint(const Temporal *temp,
  const Pcpoint *pt);

extern Temporal *teq_pcpatch_tpcpatch(const Pcpatch *pa,
  const Temporal *temp);
extern Temporal *teq_tpcpatch_pcpatch(const Temporal *temp,
  const Pcpatch *pa);
extern Temporal *tne_pcpatch_tpcpatch(const Pcpatch *pa,
  const Temporal *temp);
extern Temporal *tne_tpcpatch_pcpatch(const Temporal *temp,
  const Pcpatch *pa);

/* Bounding box topological functions */

extern bool adjacent_tpcbox_tpointcloud(const TPCBox *box,
  const Temporal *temp);
extern bool adjacent_tpointcloud_tpcbox(const Temporal *temp,
  const TPCBox *box);
extern bool adjacent_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);

extern bool contained_tpcbox_tpointcloud(const TPCBox *box,
  const Temporal *temp);
extern bool contained_tpointcloud_tpcbox(const Temporal *temp,
  const TPCBox *box);
extern bool contained_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);

extern bool contains_tpcbox_tpointcloud(const TPCBox *box,
  const Temporal *temp);
extern bool contains_tpointcloud_tpcbox(const Temporal *temp,
  const TPCBox *box);
extern bool contains_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);

extern bool overlaps_tpcbox_tpointcloud(const TPCBox *box,
  const Temporal *temp);
extern bool overlaps_tpointcloud_tpcbox(const Temporal *temp,
  const TPCBox *box);
extern bool overlaps_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);

extern bool same_tpcbox_tpointcloud(const TPCBox *box,
  const Temporal *temp);
extern bool same_tpointcloud_tpcbox(const Temporal *temp,
  const TPCBox *box);
extern bool same_tpointcloud_tpointcloud(const Temporal *temp1,
  const Temporal *temp2);
/*****************************************************************************
 * tpcpoint spatial predicates
 *****************************************************************************/

extern bool eintersects_tpcpoint_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern double nad_tpcpoint_geo(const Temporal *temp, const GSERIALIZED *gs);

/*****************************************************************************/

#endif /* __MEOS_POINTCLOUD_H__ */
