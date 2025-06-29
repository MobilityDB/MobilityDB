/*****************************************************************************
 *
 * This MobilityDB code seq provided under The PostgreSQL License.
 * Copyright(c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License(GPLv2 or later).
 * Copyright(c) 2001-2023, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement seq hereby granted, provided that the above copyright notice and
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
 * @brief Internal API of the Mobility Engine Open Source(MEOS) library.
 */

#ifndef __MEOS_INTERNAL_GEO_H__
#define __MEOS_INTERNAL_GEO_H__

/* C */
#include <stddef.h>
/* JSON-C */
#include <json-c/json.h>
/* PROJ */
#include <proj.h>
/* PostgreSQL */
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>

/*****************************************************************************
 * Direct access to a single point in the GSERIALIZED struct
 *****************************************************************************/

/*
 * Obtain a geometry/geography point from the GSERIALIZED WITHOUT creating
 * the corresponding LWGEOM. These functions constitute a **SERIOUS**
 * break of encapsulation but it is the only way to achieve reasonable
 * performance when manipulating mobility data.
 * The datum_* functions suppose that the GSERIALIZED has been already
 * detoasted. This is typically the case when the datum is within a Temporal*
 * that has been already detoasted with PG_GETARG_TEMPORAL*
 * The first variant (e.g. datum_point2d) is slower than the second (e.g.
 * datum_point2d_p) since the point is passed by value and thus the bytes
 * are copied. The second version is declared const because you aren't allowed
 * to modify the values, only read them.
 */

/**
 * @brief Macro for accessing the GSERIALIZED value of a temporal point.
 * @pre It is assumed that the geometry/geography IS NOT TOASTED
 */
#define DatumGetGserializedP(X)      ((GSERIALIZED *) DatumGetPointer(X))
#define GserializedPGetDatum(X)      PointerGetDatum(X)

/**
 * @brief Definition for the internal aspects of  the GSERIALIZED struct
 */
// #define LWFLAG_EXTFLAGS    0x20
#define LWFLAG_VERSBIT2    0x80

// #define FLAGS_GET_EXTFLAGS(flags)     (((flags) & LWFLAG_EXTFLAGS)>>5)
#define FLAGS_GET_VERSBIT2(flags)     (((flags) & LWFLAG_VERSBIT2)>>7)

#define GS_POINT_PTR(gs)    ( (uint8_t *) ((gs)->data) + 8 + \
  FLAGS_GET_BBOX((gs)->gflags) * FLAGS_NDIMS_BOX((gs)->gflags) * 8 + \
  FLAGS_GET_VERSBIT2((gs)->gflags) * 8 )

/**
 * @brief Return a pointer to a 2D/3DZ point from the datum/GSERIALIZED
 */
#define DATUM_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR(DatumGetGserializedP(gs)) )
#define DATUM_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR(DatumGetGserializedP(gs)) )

#define GSERIALIZED_POINT2D_P(gs)  ( (POINT2D *) GS_POINT_PTR((gs)) )
#define GSERIALIZED_POINT3DZ_P(gs) ( (POINT3DZ *) GS_POINT_PTR((gs)) )

/*****************************************************************************
 * Internal function accessing the PROJ library
 *****************************************************************************/

extern PJ_CONTEXT *proj_get_context(void);

/*****************************************************************************
 * Round functions
 *****************************************************************************/

extern Datum datum_geo_round(Datum value, Datum size);
extern GSERIALIZED *point_round(const GSERIALIZED *gs, int maxdd);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Constructor functions for box types */

extern void stbox_set(bool hasx, bool hasz, bool geodetic, int32 srid, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, const Span *s, STBox *box);

/*****************************************************************************/

/* Conversion functions for box types */

extern STBox *box3d_to_stbox(const BOX3D *box);
extern void gbox_set_stbox(const GBOX *box, int32_t srid, STBox *result);
extern bool geo_set_stbox(const GSERIALIZED *gs, STBox *box);
extern void geoarr_set_stbox(const Datum *values, int count, STBox *box);
extern bool spatial_set_stbox(Datum d, meosType basetype, STBox *box);
extern void spatialset_set_stbox(const Set *set, STBox *box);
extern void stbox_set_box3d(const STBox *box, BOX3D *box3d);
extern void stbox_set_gbox(const STBox *box, GBOX *gbox);
extern void tstzset_set_stbox(const Set *s, STBox *box);
extern void tstzspan_set_stbox(const Span *s, STBox *box);
extern void tstzspanset_set_stbox(const SpanSet *s, STBox *box);

/*****************************************************************************/

/* Transformation functions for box types */

extern void stbox_expand(const STBox *box1, STBox *box2);

/*****************************************************************************/

/* Set functions for box types */

extern bool inter_stbox_stbox(const STBox *box1, const STBox *box2, STBox *result);
extern GSERIALIZED *stbox_geo(const STBox *box);

/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Input and output functions */

extern TInstant *tgeogpointinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeogpointinst_in(const char *str);
extern TSequence *tgeogpointseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeogpointseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeogpointseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeogpointseqset_in(const char *str);
extern TInstant *tgeompointinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeompointinst_in(const char *str);
extern TSequence *tgeompointseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeompointseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeompointseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeompointseqset_in(const char *str);
extern TInstant *tgeographyinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeographyinst_in(const char *str);
extern TSequence *tgeographyseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeographyseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeographyseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeographyseqset_in(const char *str);
extern TInstant *tgeometryinst_from_mfjson(json_object *mfjson, int32_t srid);
extern TInstant *tgeometryinst_in(const char *str);
extern TSequence *tgeometryseq_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequence *tgeometryseq_in(const char *str, interpType interp);
extern TSequenceSet *tgeometryseqset_from_mfjson(json_object *mfjson, int32_t srid, interpType interp);
extern TSequenceSet *tgeometryseqset_in(const char *str);

/*****************************************************************************/

/* Constructor functions */

extern TSequence *tpointseq_make_coords(const double *xcoords, const double *ycoords, const double *zcoords, const TimestampTz *times, int count, int32 srid, bool geodetic, bool lower_inc, bool upper_inc, interpType interp, bool normalize);

/*****************************************************************************/

/* Conversion functions */

/*****************************************************************************/

/* Accessor functions */

/*****************************************************************************/

/* Transformation functions */

/*****************************************************************************/

/* Modification functions */

/*****************************************************************************/

/* Bounding box functions */

extern void tspatial_set_stbox(const Temporal *temp, STBox *box);
extern void tgeoinst_set_stbox(const TInstant *inst, STBox *box);
extern void tspatialseq_set_stbox(const TSequence *seq, STBox *box);
extern void tspatialseqset_set_stbox(const TSequenceSet *ss, STBox *box);

/*****************************************************************************/

/* Restriction functions */

extern Temporal *tgeo_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern Temporal *tgeo_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc, bool atfunc);
extern TInstant *tgeoinst_restrict_geom(const TInstant *inst, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern TInstant *tgeoinst_restrict_stbox(const TInstant *inst, const STBox *box, bool border_inc, bool atfunc);
extern Temporal *tgeoseq_restrict_geom(const TSequence *seq, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern Temporal *tgeoseq_restrict_stbox(const TSequence *seq, const STBox *box, bool border_inc, bool atfunc);
extern TSequenceSet *tgeoseqset_restrict_geom(const TSequenceSet *ss, const GSERIALIZED *gs, const Span *zspan, bool atfunc);
extern TSequenceSet *tgeoseqset_restrict_stbox(const TSequenceSet *ss, const STBox *box, bool border_inc, bool atfunc);

/*****************************************************************************/

/* Traditional comparison functions */

/*****************************************************************************/

/* Ever/always functions */

/*****************************************************************************/

/* Mathematical functions */

/*****************************************************************************/

/* Distance functions */

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern int32_t spatial_srid(Datum d, meosType basetype);
extern bool spatial_set_srid(Datum d, meosType basetype, int32_t srid);
extern int tspatialinst_srid(const TInstant *inst);
extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq, double prevlength);
extern bool tpointseq_is_simple(const TSequence *seq);
extern double tpointseq_length(const TSequence *seq);
extern GSERIALIZED *tpointseq_linear_trajectory(const TSequence *seq, bool unary_union);
extern TSequence *tpointseq_speed(const TSequence *seq);
extern STBox *tgeoseq_stboxes(const TSequence *seq, int *count);
extern STBox *tgeoseq_split_n_stboxes(const TSequence *seq, int max_count, int *count);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ss);
extern bool tpointseqset_is_simple(const TSequenceSet *ss);
extern double tpointseqset_length(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ss);
extern STBox *tgeoseqset_stboxes(const TSequenceSet *ss, int *count);
extern STBox *tgeoseqset_split_n_stboxes(const TSequenceSet *ss, int max_count, int *count);
extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern TInstant *tgeominst_tgeoginst(const TInstant *inst, bool oper);
extern TSequence *tgeomseq_tgeogseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeomseqset_tgeogseqset(const TSequenceSet *ss, bool oper);
extern Temporal *tgeom_tgeog(const Temporal *temp, bool oper);
extern Temporal *tgeo_tpoint(const Temporal *temp, bool oper);
extern void tspatialinst_set_srid(TInstant *inst, int32_t srid);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern void tspatialseq_set_srid(TSequence *seq, int32_t srid);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ss, int *count);
extern void tspatialseqset_set_srid(TSequenceSet *ss, int32_t srid);

/*****************************************************************************/

/* Local aggregate functions */

extern GSERIALIZED *tpointseq_twcentroid(const TSequence *seq);
extern GSERIALIZED *tpointseqset_twcentroid(const TSequenceSet *ss);

/*****************************************************************************/

/* Compact functions for final append aggregate */


/*****************************************************************************/

/* Aggregate functions */


/*****************************************************************************/

/* Tile functions for span and temporal types */


/*****************************************************************************/

/* Similarity functions */


/*****************************************************************************/

#endif /* __MEOS_INTERNAL_GEO_H__ */
