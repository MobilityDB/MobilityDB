/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_GEO_H__
#define __MEOS_GEO_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* MEOS */
#include <meos.h>
// #include "general/temporal.h"

/*****************************************************************************
 * Type definitions
 *****************************************************************************/

/**
 * @brief Enumeration that defines the spatial relationships for which a call
 * to GEOS is made.
 */
typedef enum
{
  INTERSECTS =     0,
  CONTAINS =       1,
  TOUCHES =        2,
  COVERS =         3,
} spatialRel;

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro ensuring that a set is a geometry set
 */
#if MEOS
  #define VALIDATE_GEOMSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type(set, T_GEOMSET) ) \
            return (ret); \
    } while (0)
#else
  #define VALIDATE_GEOMSET(set, ret) \
    do { \
      assert(temp); \
      assert((set)->settype == T_GEOMSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a set is a geography set
 */
#if MEOS
  #define VALIDATE_GEOGSET(set, ret) ( \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_set_isof_type(set, T_GEOGSET) ) \
            return (ret); \
    } while (0)
#else
  #define VALIDATE_GEOGSET(set, ret) \
    do { \
      assert(temp); \
      assert((set)->settype == T_GEOGSET); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a set is of a geometry or geography set
 */
#if MEOS
  #define VALIDATE_GEOSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_geoset_type((set)->settype) ) \
            return (ret); \
    } while (0)
#else
  #define VALIDATE_GEOSET(set, ret) \
    do { \
      assert(temp); \
      assert(geoset_type((set)->settype); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a set is of a spatial set
 */
#if MEOS
  #define VALIDATE_SPATIALSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) (set)) || \
              ! ensure_spatialset_type((set)->settype) ) \
            return (ret); \
    } while (0)
#else
  #define VALIDATE_SPATIALSET(set, ret) \
    do { \
      assert(set); \
      assert(spatialset_type((set)->settype)); \
    } while (0)
#endif /* MEOS */

/*****************************************************************************/

/**
 * @brief Macro ensuring that a temporal value is a temporal geometry
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOMETRY(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_temporal_isof_type((Temporal *) (temp), T_TGEOMETRY) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOMETRY(temp, ret) \
    do { \
      assert(temp); \
      assert((temp)->temptype == T_TGEOMETRY); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is a temporal geography
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOGRAPHY(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_temporal_isof_type((Temporal *) (temp), T_TGEOGRAPHY) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOGRAPHY(temp, ret) \
    do { \
      assert(temp); \
      assert((temp)->temptype == T_TGEOGRAPHY); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is a temporal geometry point
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOMPOINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_temporal_isof_type((Temporal *) (temp), T_TGEOMPOINT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOMPOINT(temp, ret) \
    do { \
      assert(temp); \
      assert((temp)->temptype == T_TGEOMPOINT); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is a temporal geography point
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOGPOINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_temporal_isof_type((Temporal *) (temp), T_TGEOGPOINT) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOGPOINT(temp, ret) \
    do { \
      assert(temp); \
      assert((temp)->temptype == T_TGEOGPOINT); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is a temporal geometry or
 * geography
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEO(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_tgeo_type_all(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEO(temp, ret) \
    do { \
      assert(temp); \
      assert(tgeo_type_all(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is a temporal geometry
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOM(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_tgeometry_type(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOM(temp, ret) \
    do { \
      assert(temp); \
      assert(tgeometry_type(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is of a temporal geography
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TGEOG(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_tgeodetic_type(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TGEOG(temp, ret) \
    do { \
      assert(temp); \
      assert(tgeodetic_type(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is of a temporal
 * geometry/geography point
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TPOINT(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_tpoint_type(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TPOINT(temp, ret) \
    do { \
      assert(temp); \
      assert(tpoint_type(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/**
 * @brief Macro ensuring that a temporal value is of a temporal spatial type
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TSPATIAL(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ! ensure_tspatial_type(((Temporal *) (temp))->temptype) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TSPATIAL(temp, ret) \
    do { \
      assert(temp); \
      assert(tspatial_type(((Temporal *) (temp))->temptype)); \
    } while (0)
#endif /* MEOS */

/*===========================================================================*
 * Functions for PostGIS types
 *===========================================================================*/

extern uint8_t *geo_as_ewkb(const GSERIALIZED *gs, const char *endian, size_t *size);
extern char *geo_as_ewkt(const GSERIALIZED *gs, int precision);
extern char *geo_as_geojson(const GSERIALIZED *gs, int option, int precision, const char *srs);
extern char *geo_as_hexewkb(const GSERIALIZED *gs, const char *endian);
extern char *geo_as_text(const GSERIALIZED *gs, int precision);
extern GSERIALIZED *geo_collect_garray(GSERIALIZED **gsarr, int count);
extern GSERIALIZED *geo_copy(const GSERIALIZED *g);
extern int geo_equals(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *geo_from_ewkb(const uint8_t *wkb, size_t wkb_size, int32 srid);
extern GSERIALIZED *geo_from_geojson(const char *geojson);
extern GSERIALIZED *geo_from_text(const char *wkt, int32_t srid);
extern bool geo_is_empty(const GSERIALIZED *g);
extern GSERIALIZED *geo_makeline_garray(GSERIALIZED **gsarr, int count);
extern char *geo_out(const GSERIALIZED *gs);
extern GSERIALIZED *geo_reverse(const GSERIALIZED *gs);
extern GSERIALIZED *geo_round(const GSERIALIZED *gs, int maxdd);
extern bool geo_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED * geo_set_srid(const GSERIALIZED *gs, int32_t srid);
extern int32_t geo_srid(const GSERIALIZED *gs);
extern GSERIALIZED *geo_transform(GSERIALIZED *geom, int32_t srid_to);
extern GSERIALIZED *geo_transform_pipeline(const GSERIALIZED *gs, char *pipeline, int32_t srid_to, bool is_forward);

extern double geog_area(const GSERIALIZED *g, bool use_spheroid);
extern GSERIALIZED *geog_centroid(const GSERIALIZED *g, bool use_spheroid);
extern double geog_distance(const GSERIALIZED *g1, const GSERIALIZED *g2);
extern bool geog_dwithin(const GSERIALIZED *g1, const GSERIALIZED *g2, double tolerance, bool use_spheroid);
extern GSERIALIZED *geog_from_binary(const char *wkb_bytea);
extern GSERIALIZED *geog_from_geom(const GSERIALIZED *geom);
extern GSERIALIZED *geog_from_hexewkb(const char *wkt);
extern GSERIALIZED *geog_in(const char *str, int32 typmod);
extern bool geog_intersects(const GSERIALIZED *gs1, const GSERIALIZED *gs2, bool use_spheroid);
extern double geog_length(const GSERIALIZED *g, bool use_spheroid);
extern double geog_perimeter(const GSERIALIZED *g, bool use_spheroid);

extern GSERIALIZED *geom_array_union(GSERIALIZED **gsarr, int count);
extern bool geom_azimuth(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result);
extern GSERIALIZED *geom_boundary(const GSERIALIZED *gs);
extern GSERIALIZED *geom_buffer(const GSERIALIZED *gs, double size, char *params);
extern GSERIALIZED *geom_centroid(const GSERIALIZED *gs);
extern bool geom_contains(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *geom_convex_hull(const GSERIALIZED *gs);
extern bool geom_covers(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *geom_difference2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_disjoint2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_distance2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_distance3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_dwithin2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance);
extern bool geom_dwithin3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double tolerance);
extern GSERIALIZED *geom_from_geog(const GSERIALIZED *geog);
extern GSERIALIZED *geom_from_hexewkb(const char *wkt);
extern GSERIALIZED *geom_from_text(const char *wkt, int32_t srid);
extern GSERIALIZED *geom_in(const char *str, int32 typmod);
extern GSERIALIZED *geom_intersection2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geom_intersects3d(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern double geom_length(const GSERIALIZED *gs);
extern double geom_perimeter(const GSERIALIZED *gs);
extern bool geom_relate_pattern(const GSERIALIZED *gs1, const GSERIALIZED *gs2, char *patt);
extern GSERIALIZED *geom_shortestline2d(const GSERIALIZED *gs1, const GSERIALIZED *s2);
extern GSERIALIZED *geom_shortestline3d(const GSERIALIZED *gs1, const GSERIALIZED *s2);
extern bool geom_touches(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *geom_unary_union(GSERIALIZED *gs, double prec);

extern GSERIALIZED *line_interpolate_point(GSERIALIZED *gs, double distance_fraction, bool repeat);
extern int line_numpoints(const GSERIALIZED *gs);
extern double line_locate_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern GSERIALIZED *line_point_n(const GSERIALIZED *geom, int n);
extern GSERIALIZED *line_substring(const GSERIALIZED *gs, double from, double to);

extern STBox *geo_stboxes(const GSERIALIZED *gs, int *count);
extern STBox *geo_split_n_stboxes(const GSERIALIZED *gs, int box_count, int *count);

/*****************************************************************************
 * Functions for geo and spatial sets
 *****************************************************************************/

extern bool contained_geo_set(const GSERIALIZED *gs, const Set *s);
extern bool contains_set_geo(const Set *s, GSERIALIZED *gs);
extern Set *geo_set(const GSERIALIZED *gs);
extern GSERIALIZED *geoset_end_value(const Set *s);
extern Set *geoset_make(const GSERIALIZED **values, int count);
extern GSERIALIZED *geoset_start_value(const Set *s);
extern bool geoset_value_n(const Set *s, int n, GSERIALIZED **result);
extern GSERIALIZED **geoset_values(const Set *s);
extern Set *intersection_geo_set(const GSERIALIZED *gs, const Set *s);
extern Set *intersection_set_geo(const Set *s, const GSERIALIZED *gs);
extern Set *minus_geo_set(const GSERIALIZED *gs, const Set *s);
extern Set *minus_set_geo(const Set *s, const GSERIALIZED *gs);
extern int32_t spatialset_srid(const Set *s);
extern Set *spatialset_set_srid(const Set *s, int32_t srid);
extern Set *spatialset_transform(const Set *s, int32_t srid);
extern Set *spatialset_transform_pipeline(const Set *s, const char *pipelinestr, int32_t srid, bool is_forward);
extern Set *union_geo_set(const GSERIALIZED *gs, const Set *s);
extern Set *union_set_geo(const Set *s, const GSERIALIZED *gs);

/*****************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output */

extern STBox *stbox_from_wkb(const uint8_t *wkb, size_t size);
extern STBox *stbox_from_hexwkb(const char *hexwkb);
extern uint8_t *stbox_as_wkb(const STBox *box, uint8_t variant, size_t *size_out);
extern char *stbox_as_hexwkb(const STBox *box, uint8_t variant, size_t *size);
extern STBox *stbox_in(const char *str);
extern char *stbox_out(const STBox *box, int maxdd);

/* Constructors */

extern STBox *geo_tstzspan_to_stbox(const GSERIALIZED *gs, const Span *s);
extern STBox *geo_timestamptz_to_stbox(const GSERIALIZED *gs, TimestampTz t);
extern STBox *stbox_copy(const STBox *box);
extern STBox *stbox_make(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *s);

/* Conversions */

extern STBox *geo_stbox(const GSERIALIZED *gs);
extern GSERIALIZED *stbox_geo(const STBox *box);
extern STBox *spatialset_stbox(const Set *s);
extern GBOX *stbox_gbox(const STBox *box);
extern BOX3D *stbox_box3d(const STBox *box);
extern Span *stbox_tstzspan(const STBox *box);
extern STBox *timestamptz_stbox(TimestampTz t);
extern STBox *tstzset_stbox(const Set *s);
extern STBox *tstzspan_stbox(const Span *s);
extern STBox *tstzspanset_stbox(const SpanSet *ss);
extern STBox *tspatial_stbox(const Temporal *temp);

/* Accessors */

extern double stbox_area(const STBox *box, bool spheroid);
extern bool stbox_hast(const STBox *box);
extern bool stbox_hasx(const STBox *box);
extern bool stbox_hasz(const STBox *box);
extern bool stbox_isgeodetic(const STBox *box);
extern double stbox_perimeter(const STBox *box, bool spheroid);
extern int32_t stbox_srid(const STBox *box);
extern bool stbox_tmax(const STBox *box, TimestampTz *result);
extern bool stbox_tmax_inc(const STBox *box, bool *result);
extern bool stbox_tmin(const STBox *box, TimestampTz *result);
extern bool stbox_tmin_inc(const STBox *box, bool *result);
extern double stbox_volume(const STBox *box);
extern bool stbox_xmax(const STBox *box, double *result);
extern bool stbox_xmin(const STBox *box, double *result);
extern bool stbox_ymax(const STBox *box, double *result);
extern bool stbox_ymin(const STBox *box, double *result);
extern bool stbox_zmax(const STBox *box, double *result);
extern bool stbox_zmin(const STBox *box, double *result);

/* Transformations */

extern STBox *stbox_expand_space(const STBox *box, double d);
extern STBox *stbox_expand_time(const STBox *box, const Interval *interv);
extern STBox *stbox_get_space(const STBox *box);
extern STBox *stbox_quad_split(const STBox *box, int *count);
extern STBox *stbox_round(const STBox *box, int maxdd);
extern STBox *stbox_set_srid(const STBox *box, int32_t srid);
extern STBox *stbox_shift_scale_time(const STBox *box, const Interval *shift, const Interval *duration);
extern STBox *stbox_transform(const STBox *box, int32_t srid);
extern STBox *stbox_transform_pipeline(const STBox *box, const char *pipelinestr, int32_t srid, bool is_forward);
extern STBox *stboxarr_round(const STBox *boxarr, int count, int maxdd);

/* Set functions */

extern STBox *union_stbox_stbox(const STBox *box1, const STBox *box2, bool strict);
extern STBox *intersection_stbox_stbox(const STBox *box1, const STBox *box2);

/* Topological functions */

extern bool adjacent_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool contained_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool contains_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overlaps_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool same_stbox_stbox(const STBox *box1, const STBox *box2);

/* Position functions */

extern bool left_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overleft_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool right_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overright_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool below_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overbelow_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool above_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overabove_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool front_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overfront_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool back_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overback_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool before_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overbefore_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool after_stbox_stbox(const STBox *box1, const STBox *box2);
extern bool overafter_stbox_stbox(const STBox *box1, const STBox *box2);

/* Comparisons */

extern bool stbox_eq(const STBox *box1, const STBox *box2);
extern bool stbox_ne(const STBox *box1, const STBox *box2);
extern int stbox_cmp(const STBox *box1, const STBox *box2);
extern bool stbox_lt(const STBox *box1, const STBox *box2);
extern bool stbox_le(const STBox *box1, const STBox *box2);
extern bool stbox_ge(const STBox *box1, const STBox *box2);
extern bool stbox_gt(const STBox *box1, const STBox *box2);

/* RTree functions */

extern RTree *rtree_create_stbox();
extern void rtree_insert(RTree *rtree, STBox *box, int64 id);
extern int *rtree_search(const RTree *rtree,const STBox *query, int *count);
extern void rtree_free(RTree *rtree);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Input and output */

extern char *tgeo_out(const Temporal *temp, int maxdd);
extern char *tspatial_as_text(const Temporal *temp, int maxdd);
extern char *tspatial_as_ewkt(const Temporal *temp, int maxdd);
extern Temporal *tgeogpoint_in(const char *str);
extern Temporal *tgeogpoint_from_mfjson(const char *str);
extern Temporal *tgeography_in(const char *str);
extern Temporal *tgeometry_in(const char *str);
extern Temporal *tgeompoint_in(const char *str);
extern Temporal *tgeompoint_from_mfjson(const char *str);

extern Set *geogset_in(const char *str);
extern Set *geomset_in(const char *str);
extern char *spatialset_as_text(const Set *set, int maxdd);
extern char *spatialset_as_ewkt(const Set *set, int maxdd);

/* Constructors */

extern Temporal *tgeo_from_base_temp(const GSERIALIZED *gs, const Temporal *temp);
extern TInstant *tgeoinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tgeoseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s);
extern TSequenceSet *tgeoseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss, interpType interp);
extern Temporal *tpoint_from_base_temp(const GSERIALIZED *gs, const Temporal *temp);
extern TInstant *tpointinst_make(const GSERIALIZED *gs, TimestampTz t);
extern TSequence *tpointseq_from_base_tstzspan(const GSERIALIZED *gs, const Span *s, interpType interp);
extern TSequence *tpointseq_from_base_tstzset(const GSERIALIZED *gs, const Set *s);
extern TSequenceSet *tpointseqset_from_base_tstzspanset(const GSERIALIZED *gs, const SpanSet *ss, interpType interp);

/* Accessors */

extern GSERIALIZED *tgeo_end_value(const Temporal *temp);
extern GSERIALIZED *tgeo_start_value(const Temporal *temp);
extern bool tgeo_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, GSERIALIZED **value);
extern bool tgeo_value_n(const Temporal *temp, int n, GSERIALIZED **result);
extern GSERIALIZED **tgeo_values(const Temporal *temp, int *count);
extern GSERIALIZED *tgeo_traversed_area(const Temporal *temp);
extern Temporal *tgeo_centroid(const Temporal *temp);

/* Transformations */

int32_t tspatial_srid(const Temporal *temp);
extern Temporal *tspatial_set_srid(const Temporal *temp, int32_t srid);
extern Temporal *tspatial_transform(const Temporal *temp, int32 srid);
extern Temporal *tspatial_transform_pipeline(const Temporal *temp, const char *pipelinestr, int32 srid, bool is_forward);

/* Restrictions */

extern Temporal *tgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpoint_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tpoint_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpoint_at_value(const Temporal *temp, GSERIALIZED *gs);
extern Temporal *tpoint_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
extern Temporal *tpoint_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpoint_minus_value(const Temporal *temp, GSERIALIZED *gs);

/* Ever and always comparisons */

extern int always_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int always_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int always_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_eq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_eq_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_ne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_ne_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);

/* Temporal comparisons */

extern Temporal *teq_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);

/* Bounding box functions */

extern STBox *geo_split_each_n_stboxes(const GSERIALIZED *gs, int elem_count, int *count);
extern STBox *geo_split_n_stboxes(const GSERIALIZED *gs, int box_count, int *count);
extern STBox *geo_split_each_n_stboxes(const GSERIALIZED *gs, int box_count, int *count);
extern STBox *tgeo_stboxes(const Temporal *temp, int *count);
extern STBox *tgeo_space_boxes(const Temporal *temp, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc, int *count);
extern STBox *tgeo_space_time_boxes(const Temporal *temp, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool bitmatrix, bool border_inc, int *count);
extern STBox *tgeo_split_each_n_stboxes(const Temporal *temp, int elem_count, int *count);
extern STBox *tgeo_split_n_stboxes(const Temporal *temp, int box_count, int *count);

/* Topological functions */

extern bool adjacent_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool adjacent_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool adjacent_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool contained_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool contained_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool contained_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool contains_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool contains_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool contains_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overlaps_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overlaps_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overlaps_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool same_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool same_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool same_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);

/* Position functions */

extern bool above_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool above_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool above_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool after_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool after_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool after_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool back_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool back_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool back_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool before_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool before_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool before_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool below_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool below_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool below_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool front_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool front_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool front_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool left_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool left_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool left_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overabove_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overabove_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overabove_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overafter_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overafter_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overafter_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overback_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overback_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overback_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overbefore_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overbefore_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overbefore_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overbelow_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overbelow_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overbelow_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overfront_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overfront_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overfront_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overleft_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overleft_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overleft_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool overright_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool overright_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool overright_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);
extern bool right_stbox_tspatial(const STBox *box, const Temporal *temp);
extern bool right_tspatial_stbox(const Temporal *temp, const STBox *box);
extern bool right_tspatial_tspatial(const Temporal *temp1, const Temporal *temp2);

/* Spatial accessor functions for temporal points */

extern bool bearing_point_point(const GSERIALIZED *gs1, const GSERIALIZED *gs2, double *result);
extern Temporal *bearing_tpoint_point(const Temporal *temp, const GSERIALIZED *gs, bool invert);
extern Temporal *bearing_tpoint_tpoint(const Temporal *temp1, const Temporal *temp2);
extern GBOX *geo_gboxes(const GSERIALIZED *gs, int *count);
extern Temporal *tpoint_angular_difference(const Temporal *temp);
extern Temporal *tpoint_azimuth(const Temporal *temp);
extern GSERIALIZED *tgeo_convex_hull(const Temporal *temp);
extern Temporal *tpoint_cumulative_length(const Temporal *temp);
extern bool tpoint_direction(const Temporal *temp, double *result);
extern Temporal *tpoint_get_x(const Temporal *temp);
extern Temporal *tpoint_get_y(const Temporal *temp);
extern Temporal *tpoint_get_z(const Temporal *temp);
extern bool tpoint_is_simple(const Temporal *temp);
extern double tpoint_length(const Temporal *temp);
extern Temporal *tpoint_speed(const Temporal *temp);
extern GSERIALIZED *tpoint_trajectory(const Temporal *temp);
extern GSERIALIZED *tpoint_twcentroid(const Temporal *temp);
extern int32_t tspatial_srid(const Temporal *temp);

/* Spatial transformation functions */

extern STBox *geo_expand_space(const GSERIALIZED *gs, double d);
extern Temporal *geomeas_tpoint(const GSERIALIZED *gs);
extern Temporal *tgeogpoint_tgeography(const Temporal *temp);
extern Temporal *tgeography_tgeometry(const Temporal *temp);
extern Temporal *tgeography_tgeogpoint(const Temporal *temp);
extern Temporal *tgeometry_tgeography(const Temporal *temp);
extern Temporal *tgeometry_tgeompoint(const Temporal *temp);
extern Temporal *tgeompoint_tgeometry(const Temporal *temp);
extern Temporal *tgeo_affine(const Temporal *temp, const AFFINE *a);
extern bool tpoint_AsMVTGeom(const Temporal *temp, const STBox *bounds, int32_t extent, int32_t buffer, bool clip_geom, GSERIALIZED **gsarr, int64 **timesarr, int *count);
extern STBox *tspatial_expand_space(const Temporal *temp, double d);
extern Temporal **tpoint_make_simple(const Temporal *temp, int *count);
extern Temporal *tgeo_scale(const Temporal *temp, const GSERIALIZED *scale, const GSERIALIZED *sorigin);
extern Temporal *tspatial_set_srid(const Temporal *temp, int32_t srid);
extern bool tpoint_tfloat_to_geomeas(const Temporal *tpoint, const Temporal *measure, bool segmentize, GSERIALIZED **result);

/* Ever and always spatial relationships */

extern int acontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int adisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int adisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int adwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int adwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist);
extern int aintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int aintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int atouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int econtains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int edisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int edisjoint_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist);
extern int edwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist);
extern int eintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern int etouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);

/* Temporal spatial relationships */

extern Temporal *tcontains_geo_tgeo(const GSERIALIZED *gs, const Temporal *temp, bool restr, bool atvalue);
extern Temporal *tdisjoint_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *tdisjoint_tgeo_tgeo (const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue);
extern Temporal *tdwithin_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, double dist, bool restr, bool atvalue);
extern Temporal *tdwithin_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2, double dist, bool restr, bool atvalue);
extern Temporal *tintersects_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);
extern Temporal *tintersects_tgeo_tgeo (const Temporal *temp1, const Temporal *temp2, bool restr, bool atvalue);
extern Temporal *ttouches_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs, bool restr, bool atvalue);

/* Distance */

extern double nad_stbox_geo(const STBox *box, const GSERIALIZED *gs);
extern double nad_stbox_stbox(const STBox *box1, const STBox *box2);
extern double nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tgeo_stbox(const Temporal *temp, const STBox *box);
extern double nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);

extern Temporal *distance_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern double nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tgeo_stbox(const Temporal *temp, const STBox *box);
extern double nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2);

/* Aggregates */

extern Temporal *tpoint_tcentroid_finalfn(SkipList *state);
extern SkipList *tpoint_tcentroid_transfn(SkipList *state, Temporal *temp);
extern STBox *tspatial_extent_transfn(STBox *box, const Temporal *temp);

/* Tile functions */

extern STBox *stbox_get_space_tile(const GSERIALIZED *point, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin);
extern STBox *stbox_get_space_time_tile(const GSERIALIZED *point, TimestampTz t, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin);
extern STBox *stbox_get_time_tile(TimestampTz t, const Interval *duration, TimestampTz torigin);
extern STBox *stbox_space_tiles(const STBox *bounds, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool border_inc, int *count);
extern STBox *stbox_space_time_tiles(const STBox *bounds, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool border_inc, int *count);
extern STBox *stbox_time_tiles(const STBox *bounds, const Interval *duration, TimestampTz torigin, bool border_inc, int *count);
extern Temporal **tpoint_space_split(const Temporal *temp, double xsize, double ysize, double zsize, const GSERIALIZED *sorigin, bool bitmatrix, bool border_inc, GSERIALIZED ***space_bins, int *count);
extern Temporal **tgeo_space_time_split(const Temporal *temp, double xsize, double ysize, double zsize, const Interval *duration, const GSERIALIZED *sorigin, TimestampTz torigin, bool bitmatrix, bool border_inc, GSERIALIZED ***space_bins, TimestampTz **time_bins, int *count);
extern Temporal **tgeo_time_split(const Temporal *temp, const Interval *duration, TimestampTz torigin, bool border_inc, TimestampTz **time_bins, int *count);


/*****************************************************************************/

#endif
