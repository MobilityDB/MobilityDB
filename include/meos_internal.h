/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file meos_internal.h
 * Internal API of the Mobility Engine Open Source (MEOS) library.
 */

#ifndef __MEOS_INTERNAL_H__
#define __MEOS_INTERNAL_H__

/* JSON-C */
#include <json-c/json.h>
/* PostgreSQL */
// #include "postgres.h"
/* MobilityDB */
#include "general/temporal_catalog.h" /* For mobdbType */

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#define TimestampTzGetDatum(X) Int64GetDatum(X)
#define DatumGetTimestampTz(X) ((TimestampTz) DatumGetInt64(X))

extern int timestamptz_cmp_internal(TimestampTz dt1, TimestampTz dt2);

/*****************************************************************************
 * Functions for span and time types
 *****************************************************************************/

/* Input/output functions for span and time types */

extern Span *span_in(char *str, mobdbType spantype);

/*****************************************************************************/

/* Constructor functions for span and time types */

extern Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, mobdbType basetype);
extern void span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc, mobdbType basetype, Span *s);

/*****************************************************************************/

/* Cast functions for span and time types */

extern Span *elem_to_span(Datum d, mobdbType basetype);

/*****************************************************************************/

/* Accessor functions for span and time types */

extern const Period *periodset_per_n(const PeriodSet *ps, int index);
extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index); // TO ADD

/*****************************************************************************/

/* Transformation functions for span and time types */

/*****************************************************************************
 * Bounding box functions for span and time types
 *****************************************************************************/

/* Topological functions for span and time types */

extern bool contains_span_elem(const Span *s, Datum d, mobdbType basetype);
extern bool contained_elem_span(Datum d, mobdbType basetype, const Span *s);
extern bool adjacent_span_elem(const Span *s, Datum d, mobdbType basetype);

/*****************************************************************************/

/* Position functions for span and time types */

extern bool left_elem_span(Datum d, mobdbType basetype, const Span *s);
extern bool left_span_elem(const Span *s, Datum d, mobdbType basetype);
extern bool right_elem_span(Datum d, mobdbType basetype, const Span *s);
extern bool right_span_elem(const Span *s, Datum d, mobdbType basetype);
extern bool overleft_elem_span(Datum d, mobdbType basetype, const Span *s);
extern bool overleft_span_elem(const Span *s, Datum d, mobdbType basetype);
extern bool overright_elem_span(Datum d, mobdbType basetype, const Span *s);
extern bool overright_span_elem(const Span *s, Datum d, mobdbType basetype);

/*****************************************************************************/

/* Set functions for span and time types */


/*****************************************************************************/

/* Distance functions for span and time types */

extern double distance_elem_elem(Datum l, Datum r, mobdbType typel, mobdbType typer);
extern double distance_span_elem(const Span *s, Datum d, mobdbType basetype);

/*****************************************************************************/

/* Comparison functions for span and time types */


/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */


/*****************************************************************************/

/* Constructor functions for box types */


/*****************************************************************************/

/* Cast functions for box types */

extern void number_set_tbox(Datum value, mobdbType basetype, TBOX *box);
extern void stbox_set_gbox(const STBOX *box, GBOX *gbox);
extern void stbox_set_box3d(const STBOX *box, BOX3D *box3d);

/*****************************************************************************/

/* Accessor functions for box types */


/*****************************************************************************/

/* Transformation functions for box types */



/*****************************************************************************/

/* Topological functions for box types */



/*****************************************************************************/

/* Position functions for box types */


/*****************************************************************************/

/* Set functions for box types */


/*****************************************************************************/

/* Comparison functions for box types */



/*****************************************************************************
 * Functions for temporal types
 *****************************************************************************/

/* Input/output functions for temporal types */

extern Temporal *temporal_in(char *str, mobdbType temptype);
extern TInstant *tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype);
extern TInstantSet *tinstantset_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype);
extern TSequence *tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype, bool linear);
extern TSequenceSet *tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype, bool linear);
extern TInstant *tinstant_in(char *str, mobdbType temptype);
extern TInstantSet *tinstantset_in(char *str, mobdbType temptype);
extern TSequence *tsequence_in(char *str, mobdbType temptype, bool linear);
extern TSequenceSet *tsequenceset_in(char *str, mobdbType temptype, bool linear);
extern Temporal *tpoint_from_text(const char *wkt, mobdbType temptype);
extern Temporal *tpoint_from_ewkt(const char *ewkt, mobdbType temptype);

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *temporal_from_base(Datum value, mobdbType temptype, const Temporal *temp, bool linear);
extern TInstant *tinstant_make(Datum value, mobdbType temptype, TimestampTz t);
extern TInstantSet *tinstantset_make(const TInstant **instants, int count, bool merge);
extern TInstantSet *tinstantset_make_free(TInstant **instants, int count, bool merge);
extern TInstantSet *tinstantset_from_base(Datum value, mobdbType temptype, const TimestampSet *ss);
extern TSequence *tsequence_make(const TInstant **instants, int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TSequence *tsequence_make_free(TInstant **instants, int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TSequence *tsequence_from_base(Datum value, mobdbType temptype, const Period *p, bool linear);
extern TSequenceSet *tsequenceset_make(const TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_make_free(TSequence **sequences, int count, bool normalize);
extern TSequenceSet *tsequenceset_from_base(Datum value, mobdbType temptype, const PeriodSet *ps, bool linear);

/*****************************************************************************/

/* Cast functions for temporal types */


/*****************************************************************************/

/* Accessor functions for temporal types */

extern const TInstant *tinstantset_inst_n(const TInstantSet *is, int index);
extern const TInstant *tsequence_inst_n(const TSequence *seq, int index);
extern const TSequence *tsequenceset_seq_n(const TSequenceSet *ss, int index);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern void tinstantset_set_bbox(const TInstantSet *is, void *box);
extern void tsequence_set_bbox(const TSequence *seq, void *box);
extern void tsequenceset_set_bbox(const TSequenceSet *ss, void *box);

// RENAME
// extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
// extern const TInstant *tsequenceset_start_instant(const TSequenceSet *ss);
// extern const TInstant *tsequenceset_end_instant(const TSequenceSet *ss);

/*****************************************************************************/

/* Transformation functions for temporal types */


/*****************************************************************************/

/* Restriction functions for temporal types */


/*****************************************************************************/

/* Boolean functions for temporal types */


/*****************************************************************************/

/* Mathematical functions for temporal types */

extern Temporal *add_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern Temporal *add_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern Temporal *sub_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern Temporal *sub_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern Temporal *mult_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern Temporal *mult_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern Temporal *div_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern Temporal *div_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);

/*****************************************************************************/

/* Text functions for temporal types */


/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/* Topological functions for temporal types */

extern bool contains_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool contains_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool contained_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool contained_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool overlaps_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool overlaps_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool same_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool same_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool adjacent_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool adjacent_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool left_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool overleft_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool right_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool overright_number_tnumber(Datum number, mobdbType basetype, const Temporal *tnumber);
extern bool left_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool overleft_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool right_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);
extern bool overright_tnumber_number(const Temporal *tnumber, Datum number, mobdbType basetype);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value, mobdbType valuetype, mobdbType restype);
extern Temporal *distance_tnumber_tnumber(const Temporal *temp1, const Temporal *temp2, mobdbType restype);
extern double nad_tnumber_number(const Temporal *temp, Datum value, mobdbType basetype);


/*****************************************************************************/

/* Ever/always functions for temporal types */


/*****************************************************************************/

/* Comparison functions for temporal types */

extern Temporal *teq_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *teq_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);
extern Temporal *tne_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *tne_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);
extern Temporal *tlt_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *tlt_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);
extern Temporal *tle_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *tle_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);
extern Temporal *tgt_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *tgt_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);
extern Temporal *tge_base_temporal(Datum base, mobdbType basetype, const Temporal *temp);
extern Temporal *tge_temporal_base(const Temporal *temp, Datum base, mobdbType basetype);

/*****************************************************************************
  Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */


/*****************************************************************************/

/* Spatial transformation functions for temporal point types */


/*****************************************************************************/

/* Spatial relationship functions for temporal point types */


/*****************************************************************************/

/* Time functions for temporal types */


/*****************************************************************************/

/* Local aggregate functions for temporal types */


/*****************************************************************************/

/* Multidimensional tiling functions for temporal types */


/*****************************************************************************/

/* Similarity functions for temporal types */


/*****************************************************************************/

#endif
