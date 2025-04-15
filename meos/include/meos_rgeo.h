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
#include <stdint.h>
#include <stdio.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include "geo/stbox.h"

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal rigid geometry
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#ifdef MEOS
  #define VALIDATE_TRGEOMETRY(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
            ensure_temporal_isof_type((Temporal *) (temp), T_TRGEOMETRY) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TRGEOMETRY(temp, ret) \
    do { \
      assert(temp); \
      assert(temporal_isof_type((Temporal *) (temp), T_TRGEOMETRY)); \
    } while (0)
#endif

/*===========================================================================*
 * Functions for temporal rigid geometries
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern char *trgeo_out(const Temporal *temp);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern TInstant *trgeoinst_make(const GSERIALIZED *geom, const Pose *pose, TimestampTz t);
extern Temporal *geo_tpose_to_trgeo(const GSERIALIZED *gs, const Temporal *temp);

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

extern Temporal *trgeo_tpose(const Temporal *temp);
extern Temporal *trgeo_tpoint(const Temporal *temp);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

extern TInstant *trgeo_end_instant(const Temporal *temp);
extern TSequence *trgeo_end_sequence(const Temporal *temp);
extern GSERIALIZED *trgeo_end_value(const Temporal *temp);
extern GSERIALIZED *trgeo_geom(const Temporal *temp);
extern TInstant *trgeo_instant_n(const Temporal *temp, int n);
extern TInstant **trgeo_instants(const Temporal *temp, int *count);
extern TSequence **trgeo_segments(const Temporal *temp, int *count);
extern TSequence *trgeo_sequence_n(const Temporal *temp, int i);
extern TSequence **trgeo_sequences(const Temporal *temp, int *count);
extern TInstant *trgeo_start_instant(const Temporal *temp);
extern TSequence *trgeo_start_sequence(const Temporal *temp);
extern GSERIALIZED *trgeo_start_value(const Temporal *temp);
extern bool trgeo_value_n(const Temporal *temp, int n, GSERIALIZED **result);
extern GSERIALIZED *trgeo_traversed_area(const Temporal *temp);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

extern Temporal *trgeo_append_tinstant(Temporal *temp, const TInstant *inst, interpType interp, double maxdist, const Interval *maxt, bool expand);
extern Temporal *trgeo_append_tsequence(Temporal *temp, const TSequence *seq, bool expand);
extern Temporal *trgeo_delete_timestamptz(const Temporal *temp, TimestampTz t, bool connect);
extern Temporal *trgeo_delete_tstzset(const Temporal *temp, const Set *s, bool connect);
extern Temporal *trgeo_delete_tstzspan(const Temporal *temp, const Span *s, bool connect);
extern Temporal *trgeo_delete_tstzspanset(const Temporal *temp, const SpanSet *ss, bool connect);
extern Temporal *trgeo_round(const Temporal *temp, int maxdd);
extern Temporal *trgeo_set_interp(const Temporal *temp, const char *interp_str);
extern TInstant *trgeo_to_tinstant(const Temporal *temp);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

extern Temporal *trgeo_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *trgeo_restrict_values(const Temporal *temp, const Set *s, bool atfunc);

extern Temporal *trgeo_restrict_timestamptz(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *trgeo_restrict_tstzset(const Temporal *temp, const Set *s, bool atfunc);
extern Temporal *trgeo_restrict_tstzspan(const Temporal *temp, const Span *s, bool atfunc);
extern Temporal *trgeo_restrict_tstzspanset(const Temporal *temp, const SpanSet *ss, bool atfunc);

// extern Temporal *trgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *trgeo_at_geo(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *trgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
// extern Temporal *trgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *trgeo_minus_geo(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *trgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

extern Temporal *distance_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_trgeo_tpoint(const Temporal *temp1, const Temporal *temp2);
extern Temporal *distance_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_trgeo(const STBox *box, const Temporal *temp);
extern double nad_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_trgeo_stbox(const Temporal *temp, const STBox *box);
extern double nad_trgeo_tpoint(const Temporal *temp1, const Temporal *temp2);
extern double nad_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_trgeo_tpoint(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_trgeo_tpoint(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions for temporal points
 *****************************************************************************/

/* Spatial accessor functions for temporal points */

extern Set *trgeo_points(const Temporal *temp);
extern Temporal *trgeo_rotation(const Temporal *temp);
// extern GSERIALIZED *trgeo_traversed_area(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions */

extern Temporal *trgeo_tpoint(const Temporal *temp);
// extern TInstant *tposeinst_tgeompointinst(const TInstant *inst);

/*****************************************************************************/

/* Distance functions */



/*****************************************************************************/

/* Ever/always and temporal comparison functions */

extern int always_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int always_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int always_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int always_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_eq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_eq_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern int ever_ne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern int ever_ne_trgeo_trgeo(const Temporal *temp1, const Temporal *temp2);
extern Temporal *teq_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *teq_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tne_geo_trgeo(const GSERIALIZED *gs, const Temporal *temp);
extern Temporal *tne_trgeo_geo(const Temporal *temp, const GSERIALIZED *gs);

/*****************************************************************************/

/* Ever and always spatial relationship functions */


/*****************************************************************************/

/* Temporal spatial relationship functions */


/*****************************************************************************/

#endif /* __MEOS_RGEO_H__ */
