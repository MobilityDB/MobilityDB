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
 * AND FITNESS FOR A PARTICULAR PURRGEO. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 */

#ifndef __MEOS_RGEO_H__
#define __MEOS_RGEO_H__

/* C */
#include <stdbool.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal rigid geometry
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TRGEOMETRY(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TRGEOMETRY) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TRGEOMETRY(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TRGEOMETRY); \
    } while (0)
#endif

/*===========================================================================*
 * Functions for temporal rigid geometries
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern char *trgeometry_out(const Temporal *temp);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern TInstant *trgeoinst_make(const GSERIALIZED *geom, const Pose *pose, TimestampTz t);
extern Temporal *geo_tpose_to_trgeometry(const GSERIALIZED *gs, const Temporal *temp);

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

extern Temporal *trgeometry_to_tpose(const Temporal *temp);
extern Temporal *trgeometry_to_tpoint(const Temporal *temp);
extern Temporal *trgeometry_to_tgeometry(const Temporal *temp);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

extern TInstant *trgeometry_end_instant(const Temporal *temp);
extern TSequence *trgeometry_end_sequence(const Temporal *temp);
extern GSERIALIZED *trgeometry_end_value(const Temporal *temp);
extern GSERIALIZED *trgeometry_geom(const Temporal *temp);
extern TInstant *trgeometry_instant_n(const Temporal *temp, int n);
extern TInstant **trgeometry_instants(const Temporal *temp, int *count);
extern Set *trgeometry_points(const Temporal *temp);
extern Temporal *trgeometry_rotation(const Temporal *temp);
extern TSequence **trgeometry_segments(const Temporal *temp, int *count);
extern TSequence *trgeometry_sequence_n(const Temporal *temp, int i);
extern TSequence **trgeometry_sequences(const Temporal *temp, int *count);
extern TInstant *trgeometry_start_instant(const Temporal *temp);
extern TSequence *trgeometry_start_sequence(const Temporal *temp);
extern GSERIALIZED *trgeometry_start_value(const Temporal *temp);
extern bool trgeometry_value_n(const Temporal *temp, int n, GSERIALIZED **result);
extern GSERIALIZED *trgeometry_traversed_area(const Temporal *temp, bool unary_union);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

extern Temporal *trgeometry_append_tinstant(Temporal *temp, const TInstant *inst, interpType interp, double maxdist, const Interval *maxt, bool expand);
extern Temporal *trgeometry_append_tsequence(Temporal *temp, const TSequence *seq, bool expand);
extern Temporal *trgeometry_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect);
extern Temporal *trgeometry_delete_tstzset(const Temporal *temp, const Set *s, bool connect);
extern Temporal *trgeometry_delete_tstzspan(const Temporal *temp, const Span *s, bool connect);
extern Temporal *trgeometry_delete_tstzspanset(const Temporal *temp, const SpanSet *ss, bool connect);
extern Temporal *trgeometry_round(const Temporal *temp, int maxdd);
extern Temporal *trgeometry_set_interp(const Temporal *temp, interpType interp);
extern TInstant *trgeometry_to_tinstant(const Temporal *temp);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

extern Temporal *trgeometry_after_timestamptz(const Temporal *temp, TimestampTz t, bool strict);
extern Temporal *trgeometry_before_timestamptz(const Temporal *temp, TimestampTz t, bool strict);

extern Temporal *trgeometry_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *trgeometry_restrict_values(const Temporal *temp, const Set *s, bool atfunc);

extern Temporal *trgeometry_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *trgeometry_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc);
extern Temporal *trgeometry_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc);
extern Temporal *trgeometry_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss, bool atfunc);

extern Temporal *trgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *trgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *trgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *trgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);
// extern Temporal *trgeo_at_geo(const Temporal *temp, const GSERIALIZED *gs);
// extern Temporal *trgeo_at_elevation(const Temporal *temp, const Span *s);
// extern Temporal *trgeo_minus_geo(const Temporal *temp, const GSERIALIZED *gs);
// extern Temporal *trgeo_minus_elevation(const Temporal *temp, const Span *s);

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

extern Temporal *tdistance_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tdistance_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *tdistance_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_trgeometry(const STBox *box, const Temporal *temp);
extern double nad_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_trgeometry_stbox(const Temporal *temp, const STBox *box);
extern double nad_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

/* Ever/always and temporal comparison functions */

extern int always_eq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern int always_eq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_eq_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern int always_ne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_ne_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_eq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_eq_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_ne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_ne_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs);

/*****************************************************************************/

/* Ever and always spatial relationship functions */

extern int econtains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int acontains_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int econtains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int acontains_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int econtains_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int acontains_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int ecovers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int acovers_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int ecovers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int acovers_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ecovers_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int acovers_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int edisjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int adisjoint_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int edisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int adisjoint_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int eintersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int aintersects_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int eintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int aintersects_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int etouches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int atouches_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int etouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int atouches_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int etouches_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int atouches_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int edwithin_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  double dist);
extern int adwithin_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp,
  double dist);
extern int edwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist);
extern int adwithin_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist);
extern int edisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int adisjoint_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int eintersects_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);
extern int aintersects_trgeo_trgeo(const Temporal *temp1,
  const Temporal *temp2);
extern int edwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  double dist);
extern int adwithin_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2,
  double dist);

/*****************************************************************************/

/* Spatiotemporal relationship functions */


/*****************************************************************************/

#endif /* __MEOS_RGEO_H__ */
