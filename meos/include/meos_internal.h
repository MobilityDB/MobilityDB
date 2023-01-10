/*****************************************************************************
 *
 * This MobilityDB code seq provided under The PostgreSQL License.
 * Copyright(c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License(GPLv2 or later).
 * Copyright(c) 2001-2022, PostGIS contributors
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

#ifndef __MEOS_INTERNAL_H__
#define __MEOS_INTERNAL_H__

/* JSON-C */
#include <json-c/json.h>
/* PostgreSQL */
/* MEOS */
#include "general/meos_catalog.h" /* For meosType */

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#define TimestampTzGetDatum(X) Int64GetDatum(X)
#define DatumGetTimestampTz(X)((TimestampTz) DatumGetInt64(X))

/*****************************************************************************
 * Functions for set and span types
 *****************************************************************************/

/* Input/output functions for set and span types */

extern Set *set_in(const char *str, meosType basetype);
extern Span *span_in(const char *str, meosType spantype);
extern SpanSet *spanset_in(const char *str, meosType spantype);

/*****************************************************************************/

/* Constructor functions for set and span types */

extern size_t *set_offsets_ptr(const Set *s);
extern Set *set_make(const Datum *values, int count, meosType basetype, bool ordered);
extern Set *set_make_free(Datum *values, int count, meosType basetype, bool ordered);
extern Set *set_copy(const Set *s);
extern Span *span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype);
extern void span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc, meosType basetype, Span *s);

/*****************************************************************************/

/* Cast functions for set and span types */

extern Set *value_to_set(Datum d, meosType basetype);
extern Span *value_to_span(Datum d, meosType basetype);
extern SpanSet *value_to_spanset(Datum d, meosType basetype);

/*****************************************************************************/

/* Accessor functions for set and span types */

extern uint32 datum_hash(Datum d, meosType basetype);
extern uint64 datum_hash_extended(Datum d, meosType basetype, uint64 seed);
extern Datum set_val_n(const Set *ts, int index);
extern Datum set_start_value(const Set *s);
extern Datum set_end_value(const Set *s);
extern bool set_value_n(const Set *s, int n, Datum *result);
extern Datum *set_values(const Set *s);
extern const Span *spanset_sp_n(const SpanSet *ss, int index);
extern void tstzset_set_period(const Set *ts, Span *p);

/*****************************************************************************/

/* Transformation functions for set and span types */

extern void span_shift(Span *s, Datum value);
extern void spanset_shift(SpanSet *s, Datum value);
extern void lower_upper_shift_tscale(TimestampTz *lower, TimestampTz *upper, const Interval *shift, const Interval *duration);

/*****************************************************************************/

/* Aggregate functions for set and span types */

extern Set *set_agg_transfn(Set *s, Datum d, meosType basetype);

/*****************************************************************************
 * Bounding box functions for set and span types
 *****************************************************************************/

/* Topological functions for set and span types */

extern bool adjacent_span_value(const Span *s, Datum d, meosType basetype);
extern bool adjacent_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool contains_span_value(const Span *s, Datum d, meosType basetype);
extern bool contains_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool contains_set_value(const Set *s, Datum d, meosType basetype);
extern bool contains_set_set(const Set *s1, const Set *s2);
extern bool contained_value_span(Datum d, meosType basetype, const Span *s);
extern bool contained_value_set(Datum d, meosType basetype, const Set *s);
extern bool contained_set_set(const Set *s1, const Set *s2);
extern bool contained_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overlaps_value_span(Datum d, meosType basetype, const Span *s);
extern bool overlaps_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overlaps_span_value(const Span *s, Datum d, meosType basetype);
extern bool overlaps_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overlaps_set_set(const Set *s1, const Set *s2);

/*****************************************************************************/

/* Position functions for set and span types */

extern bool left_set_set(const Set *s1, const Set *s2);
extern bool left_set_value(const Set *s, Datum d, meosType basetype);
extern bool left_span_value(const Span *s, Datum d, meosType basetype);
extern bool left_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool left_value_set(Datum d, meosType basetype, const Set *s);
extern bool left_value_span(Datum d, meosType basetype, const Span *s);
extern bool left_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool right_value_set(Datum d, meosType basetype, const Set *s);
extern bool right_set_value(const Set *s, Datum d, meosType basetype);
extern bool right_set_set(const Set *s1, const Set *s2);
extern bool right_value_span(Datum d, meosType basetype, const Span *s);
extern bool right_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool right_span_value(const Span *s, Datum d, meosType basetype);
extern bool right_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overleft_value_set(Datum d, meosType basetype, const Set *s);
extern bool overleft_set_value(const Set *s, Datum d, meosType basetype);
extern bool overleft_set_set(const Set *s1, const Set *s2);
extern bool overleft_value_span(Datum d, meosType basetype, const Span *s);
extern bool overleft_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overleft_span_value(const Span *s, Datum d, meosType basetype);
extern bool overleft_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool overright_value_set(Datum d, meosType basetype, const Set *s);
extern bool overright_set_value(const Set *s, Datum d, meosType basetype);
extern bool overright_set_set(const Set *s1, const Set *s2);
extern bool overright_value_span(Datum d, meosType basetype, const Span *s);
extern bool overright_value_spanset(Datum d, meosType basetype, const SpanSet *ss);
extern bool overright_span_value(const Span *s, Datum d, meosType basetype);
extern bool overright_spanset_value(const SpanSet *ss, Datum d, meosType basetype);

/*****************************************************************************/

/* Set functions for set and span types */

extern bool inter_span_span(const Span *s1, const Span *s2, Span *result);
extern bool intersection_set_value(const Set *s, Datum d, meosType basetype, Datum *result);
extern bool intersection_span_value(const Span *s, Datum d, meosType basetype, Datum *result);
extern bool intersection_spanset_value(const SpanSet *ss, Datum d, meosType basetype, Datum *result);
extern bool intersection_value_value(Datum d1, Datum d2, meosType basetype, Datum *result);

extern Set *minus_set_value(const Set *s, Datum d, meosType basetype);
extern int minus_span_span1(const Span *s1, const Span *s2, Span **result);
extern int minus_span_value1(const Span *s, Datum d, meosType basetype, Span **result);
extern SpanSet *minus_span_value(const Span *s, Datum d, meosType basetype);
extern SpanSet *minus_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern bool minus_value_set(Datum d, meosType basetype, const Set *s, Datum *result);
extern bool minus_value_span(Datum d, meosType basetype, const Span *s, Datum *result);
extern bool minus_value_spanset(Datum d, meosType basetype, const SpanSet *ss, Datum *result);
extern bool minus_value_value(Datum d1, Datum d2, meosType basetype, Datum *result);

extern Set *union_set_value(const Set *s, const Datum d, meosType basetype);
extern SpanSet *union_span_value(const Span *s, Datum v, meosType basetype);
extern SpanSet *union_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern Set *union_value_value(Datum d1, Datum d2, meosType basetype);

/*****************************************************************************/

/* Distance functions for set and span types */

extern double distance_value_value(Datum l, Datum r, meosType typel, meosType typer);
extern double distance_span_value(const Span *s, Datum d, meosType basetype);
extern double distance_spanset_value(const SpanSet *ss, Datum d, meosType basetype);
extern double distance_value_set(Datum d, meosType basetype, const Set *s);
extern double distance_set_value(const Set *s, Datum d, meosType basetype);
extern double distance_set_set(const Set *s1, const Set *s2);

/*****************************************************************************/

/* Hash functions for set and span types */

extern uint32 datum_hash(Datum d, meosType basetype);
extern uint64 datum_hash_extended(Datum d, meosType basetype, uint64 seed);

/******************************************************************************
 * Functions for box types
 *****************************************************************************/

/* Input/output functions for box types */


/*****************************************************************************/

/* Constructor functions for box types */


/*****************************************************************************/

/* Cast functions for box types */

extern void number_set_tbox(Datum d, meosType basetype, TBox *box);
extern void int_set_tbox(int i, TBox *box);
extern void float_set_tbox(double d, TBox *box);
extern void timestamp_set_tbox(TimestampTz t, TBox *box);
extern void numset_set_tbox(const Set *s, TBox *box);
extern void tstzset_set_tbox(const Set *ts, TBox *box);
extern void numspan_set_tbox(const Span *span, TBox *box);
extern void numspanset_set_tbox(const SpanSet *ss, TBox *box);
extern void period_set_tbox(const Span *p, TBox *box);
extern void periodset_set_tbox(const SpanSet *ps, TBox *box);
extern TBox *number_timestamp_to_tbox(Datum d, meosType basetype, TimestampTz t);
extern TBox *number_period_to_tbox(Datum d, meosType basetype, const Span *p);

extern bool geo_set_stbox(const GSERIALIZED *gs, STBox *box);
extern void geoarr_set_stbox(const Datum *values, int count, STBox *box);
extern void timestamp_set_stbox(TimestampTz t, STBox *box);
extern void tstzset_set_stbox(const Set *ts, STBox *box);
extern void period_set_stbox(const Span *p, STBox *box);
extern void periodset_set_stbox(const SpanSet *ps, STBox *box);

extern void number_set_tbox(Datum value, meosType basetype, TBox *box);
extern void stbox_set_gbox(const STBox *box, GBOX *gbox);
extern void stbox_set_box3d(const STBox *box, BOX3D *box3d);

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

extern char **geoarr_as_text(const Datum *geoarr, int count, int maxdd, bool extended);
extern char *tboolinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *tboolinst_from_mfjson(json_object *mfjson);
extern TInstant *tboolinst_in(const char *str);
extern char *tboolseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *tboolseq_from_mfjson(json_object *mfjson);
extern TSequence *tboolseq_in(const char *str, interpType interp);
extern char *tboolseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *tboolseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tboolseqset_in(const char *str);
extern Temporal *temporal_in(const char *str, meosType temptype);
extern char *temporal_out(const Temporal *temp, int maxdd);
extern char **temporalarr_out(const Temporal **temparr, int count, int maxdd);
extern char *tfloatinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision);
extern TInstant *tfloatinst_from_mfjson(json_object *mfjson);
extern TInstant *tfloatinst_in(const char *str);
extern char *tfloatseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision);
extern TSequence *tfloatseq_from_mfjson(json_object *mfjson, interpType interp);
extern TSequence *tfloatseq_in(const char *str, interpType interp);
extern char *tfloatseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision);
extern TSequenceSet *tfloatseqset_from_mfjson(json_object *mfjson, interpType interp);
extern TSequenceSet *tfloatseqset_in(const char *str);
extern char *tgeogpointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern TInstant *tgeogpointinst_from_mfjson(json_object *mfjson, int srid);
extern TInstant *tgeogpointinst_in(const char *str);
extern char *tgeogpointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern TSequence *tgeogpointseq_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequence *tgeogpointseq_in(const char *str, interpType interp);
extern char *tgeogpointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TSequenceSet *tgeogpointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequenceSet *tgeogpointseqset_in(const char *str);
extern char *tgeompointinst_as_mfjson(const TInstant *inst, bool with_bbox, int precision, char *srs);
extern TInstant *tgeompointinst_from_mfjson(json_object *mfjson, int srid);
extern TInstant *tgeompointinst_in(const char *str);
extern char *tgeompointseq_as_mfjson(const TSequence *seq, bool with_bbox, int precision, char *srs);
extern TSequence *tgeompointseq_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequence *tgeompointseq_in(const char *str, interpType interp);
extern char *tgeompointseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox, int precision, char *srs);
extern TSequenceSet *tgeompointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp);
extern TSequenceSet *tgeompointseqset_in(const char *str);
extern char *tinstant_as_mfjson(const TInstant *inst, int precision, bool with_bbox, char *srs);
extern TInstant *tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype);
extern TInstant *tinstant_in(const char *str, meosType temptype);
extern char *tinstant_out(const TInstant *inst, int maxdd);
extern TSequence *tdiscseq_in(const char *str, meosType temptype);
extern char *tintinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *tintinst_from_mfjson(json_object *mfjson);
extern TInstant *tintinst_in(const char *str);
extern char *tintseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *tintseq_from_mfjson(json_object *mfjson);
extern TSequence *tintseq_in(const char *str, interpType interp);
extern char *tintseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *tintseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *tintseqset_in(const char *str);
extern char **tpointarr_as_text(const Temporal **temparr, int count, int maxdd, bool extended);
extern char *tsequence_as_mfjson(const TSequence *seq, int precision, bool with_bbox, char *srs);
extern TSequence *tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype, interpType interp);
extern TSequence *tsequence_in(const char *str, meosType temptype, interpType interp);
extern char *tsequence_out(const TSequence *seq, int maxdd);
extern char *tsequenceset_as_mfjson(const TSequenceSet *ss, int precision, bool with_bbox, char *srs);
extern TSequenceSet *tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid, meosType temptype, interpType interp);
extern TSequenceSet *tsequenceset_in(const char *str, meosType temptype, interpType interp);
extern char *tsequenceset_out(const TSequenceSet *ss, int maxdd);
extern char *ttextinst_as_mfjson(const TInstant *inst, bool with_bbox);
extern TInstant *ttextinst_from_mfjson(json_object *mfjson);
extern TInstant *ttextinst_in(const char *str);
extern char *ttextseq_as_mfjson(const TSequence *seq, bool with_bbox);
extern TSequence *ttextseq_from_mfjson(json_object *mfjson);
extern TSequence *ttextseq_in(const char *str, interpType interp);
extern char *ttextseqset_as_mfjson(const TSequenceSet *ss, bool with_bbox);
extern TSequenceSet *ttextseqset_from_mfjson(json_object *mfjson);
extern TSequenceSet *ttextseqset_in(const char *str);

/*****************************************************************************/

/* Constructor functions for temporal types */

extern Temporal *temporal_from_base(Datum value, meosType temptype, const Temporal *temp, interpType interp);
extern TInstant *tinstant_copy(const TInstant *inst);
extern TInstant *tinstant_make(Datum value, meosType temptype, TimestampTz t);
extern TSequence *tdiscseq_from_base_time(Datum value, meosType temptype, const Set *ss);
extern TSequence *tsequence_compact(const TSequence *seq);
extern TSequence *tsequence_copy(const TSequence *seq);
extern TSequence *tsequence_from_base(Datum value, meosType temptype, const TSequence *seq, interpType interp);
extern TSequence *tsequence_from_base_time(Datum value, meosType temptype, const Span *p, interpType interp);
extern TSequenceSet *tsequenceset_compact(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_copy(const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_from_base(Datum value, meosType temptype, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tsequenceset_from_base_time(Datum value, meosType temptype, const SpanSet *ps, interpType interp);

/*****************************************************************************/

/* Cast functions for temporal types */

extern void temporal_set_period(const Temporal *temp, Span *p);
extern TInstant *tfloatinst_to_tintinst(const TInstant *inst);
extern TSequence *tfloatseq_to_tintseq(const TSequence *seq);
extern TSequenceSet *tfloatseqset_to_tintseqset(const TSequenceSet *ss);
extern void tinstant_set_period(const TInstant *inst, Span *p);
extern TInstant *tintinst_to_tfloatinst(const TInstant *inst);
extern TSequence *tintseq_to_tfloatseq(const TSequence *seq);
extern TSequenceSet *tintseqset_to_tfloatseqset(const TSequenceSet *ss);
extern void tsequence_set_period(const TSequence *seq, Span *p);
extern void tsequenceset_set_period(const TSequenceSet *ss, Span *p);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern Datum temporal_end_value(const Temporal *temp);
extern Datum temporal_max_value(const Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern Datum temporal_start_value(const Temporal *temp);
extern Datum *temporal_values(const Temporal *temp, int *count);
extern SpanSet *tfloatinst_spanset(const TInstant *inst);
extern Span *tfloatseq_span(const TSequence *seq);
extern SpanSet *tfloatseq_spanset(const TSequence *seq);
extern Span *tfloatseqset_span(const TSequenceSet *ss);
extern SpanSet *tfloatseqset_spanset(const TSequenceSet *ss);
extern uint32 tinstant_hash(const TInstant *inst);
extern const TInstant **tinstant_instants(const TInstant *inst, int *count);
extern TSequence **tinstant_sequences(const TInstant *inst, int *count);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern SpanSet *tinstant_time(const TInstant *inst);
extern TimestampTz *tinstant_timestamps(const TInstant *inst, int *count);
extern Datum tinstant_value(const TInstant *inst);
extern bool tinstant_value_at_timestamp(const TInstant *inst, TimestampTz t, Datum *result);
extern Datum tinstant_value_copy(const TInstant *inst);
extern Datum *tinstant_values(const TInstant *inst, int *count);
extern uint32 tdiscseq_hash(const TSequence *seq);
extern const TInstant *tsequence_inst_n(const TSequence *seq, int index);
extern bool tdiscseq_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result);
extern Interval *tsequence_duration(const TSequence *seq);
extern TimestampTz tsequence_end_timestamp(const TSequence *seq);
extern uint32 tsequence_hash(const TSequence *seq);
extern const TInstant *tsequence_inst_n(const TSequence *seq, int index);
extern const TInstant **tsequence_instants(const TSequence *seq, int *count);
extern const TInstant *tsequence_max_instant(const TSequence *seq);
extern Datum tsequence_max_value(const TSequence *seq);
extern const TInstant *tsequence_min_instant(const TSequence *seq);
extern Datum tsequence_min_value(const TSequence *seq);
extern size_t *tsequence_offsets_ptr(const TSequence *seq);
extern TSequence **tsequence_segments(const TSequence *seq, int *count);
extern const TSequence *tsequenceset_seq_n(const TSequenceSet *ss, int index);
extern TSequence **tsequence_sequences(const TSequence *seq, int *count);
extern void tsequence_set_bbox(const TSequence *seq, void *box);
extern void tsequence_expand_bbox(TSequence *seq, const TInstant *inst);
extern void tsequenceset_expand_bbox(TSequenceSet *ss, const TSequence *seq);
extern TimestampTz tsequence_start_timestamp(const TSequence *seq);
extern SpanSet *tsequence_time(const TSequence *seq);
extern TimestampTz *tsequence_timestamps(const TSequence *seq, int *count);
extern bool tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequence_values(const TSequence *seq, int *count);
extern Interval *tsequenceset_duration(const TSequenceSet *ss, bool boundspan);
extern TimestampTz tsequenceset_end_timestamp(const TSequenceSet *ss);
extern uint32 tsequenceset_hash(const TSequenceSet *ss);
extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
extern const TInstant **tsequenceset_instants(const TSequenceSet *ss, int *count);
extern const TInstant *tsequenceset_max_instant(const TSequenceSet *ss);
extern Datum tsequenceset_max_value(const TSequenceSet *ss);
extern const TInstant *tsequenceset_min_instant(const TSequenceSet *ss);
extern Datum tsequenceset_min_value(const TSequenceSet *ss);
extern int tsequenceset_num_instants(const TSequenceSet *ss);
extern int tsequenceset_num_timestamps(const TSequenceSet *ss);
extern TSequence **tsequenceset_segments(const TSequenceSet *ss, int *count);
extern TSequence **tsequenceset_sequences(const TSequenceSet *ss, int *count);
extern const TSequence **tsequenceset_sequences_p(const TSequenceSet *ss);
extern void tsequenceset_set_bbox(const TSequenceSet *ss, void *box);
extern TimestampTz tsequenceset_start_timestamp(const TSequenceSet *ss);
extern SpanSet *tsequenceset_time(const TSequenceSet *ss);
extern Interval *tsequenceset_timespan(const TSequenceSet *ss);
extern bool tsequenceset_timestamp_n(const TSequenceSet *ss, int n, TimestampTz *result);
extern TimestampTz *tsequenceset_timestamps(const TSequenceSet *ss, int *count);
extern bool tsequenceset_value_at_timestamp(const TSequenceSet *ss, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequenceset_values(const TSequenceSet *ss, int *count);

// RENAME
// extern const TInstant *tsequenceset_inst_n(const TSequenceSet *ss, int n);
// extern const TInstant *tsequenceset_start_instant(const TSequenceSet *ss);
// extern const TInstant *tsequenceset_end_instant(const TSequenceSet *ss);

/*****************************************************************************/

/* Transformation functions for temporal types */

extern Temporal *tinstant_merge(const TInstant *inst1, const TInstant *inst2);
extern Temporal *tinstant_merge_array(const TInstant **instants, int count);
extern TInstant *tinstant_shift(const TInstant *inst, const Interval *interval);
extern TSequence *tinstant_to_tsequence(const TInstant *inst, interpType interp);
extern TSequenceSet *tinstant_to_tsequenceset(const TInstant *inst, interpType interp);
extern Temporal *tdiscseq_merge(const TSequence *is1, const TSequence *is2);
extern Temporal *tdiscseq_merge_array(const TSequence **sequences, int count);
extern TSequence *tdiscseq_to_tsequence(const TSequence *seq, interpType interp);
extern TSequenceSet *tdiscseq_to_tsequenceset(const TSequence *seq, interpType interp);
extern Temporal *tsequence_append_tinstant(TSequence *seq, const TInstant *inst, bool expand);
extern Temporal *tsequence_append_tsequence(TSequence *seq1, const TSequence *seq2, bool expand);
extern Temporal *tsequence_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tsequence_merge_array(const TSequence **sequences, int count);
extern TSequence *tsequence_shift_tscale(const TSequence *seq, const Interval *start, const Interval *duration);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TSequence *tsequence_to_tdiscseq(const TSequence *seq);
extern TSequence *tsequence_to_tcontseq(const TSequence *seq);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tsequenceset_append_tinstant(TSequenceSet *ss, const TInstant *inst, bool expand);
extern TSequenceSet *tsequenceset_append_tsequence(TSequenceSet *ss, const TSequence *seq, bool expand);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern TSequenceSet *tsequenceset_merge_array(const TSequenceSet **seqsets, int count);
extern TSequenceSet *tsequenceset_shift_tscale(const TSequenceSet *ss, const Interval *start, const Interval *duration);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ts);
extern TSequence *tsequenceset_to_tdiscseq(const TSequenceSet *ts);
extern TSequence *tsequenceset_to_tsequence(const TSequenceSet *ss);
extern TSequenceSet *tstepseq_to_linear(const TSequence *seq);
extern int tstepseq_to_linear1(const TSequence *seq, TSequence **result);
extern TSequenceSet *tstepseqset_to_linear(const TSequenceSet *ss);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, Datum *values, int count, bool atfunc);
extern Temporal *tnumber_restrict_span(const Temporal *temp, const Span *span, bool atfunc);
extern Temporal *tnumber_restrict_spanset(const Temporal *temp, const SpanSet *ss, bool atfunc);
extern Temporal *temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_tstzset(const Temporal *temp, const Set *ss, bool atfunc);
extern Temporal *temporal_restrict_period(const Temporal *temp, const Span *ps, bool atfunc);
extern Temporal *temporal_restrict_periodset(const Temporal *temp, const SpanSet *ps, bool atfunc);
extern bool temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, Datum *result);
extern TInstant *tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc);
extern TInstant *tinstant_restrict_values(const TInstant *inst, const Datum *values, int count, bool atfunc);
extern TInstant *tnumberinst_restrict_span(const TInstant *inst, const Span *span, bool atfunc);
extern TInstant *tnumberinst_restrict_spanset(const TInstant *inst, const SpanSet *ss, bool atfunc);
extern TInstant *tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc);
extern TInstant *tinstant_restrict_tstzset(const TInstant *inst, const Set *ss, bool atfunc);
extern TInstant *tinstant_restrict_period(const TInstant *inst, const Span *period, bool atfunc);
extern TInstant *tinstant_restrict_periodset(const TInstant *inst, const SpanSet *ps, bool atfunc);
extern TSequence *tdiscseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequence *tdiscseq_restrict_values(const TSequence *seq, const Datum *values, int count, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_spanset(const TSequence *seq, const SpanSet *ss, bool atfunc);
extern TSequence *tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TInstant *tsequence_at_timestamp(const TSequence *seq, TimestampTz t);
extern bool tdiscseq_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result);
extern TSequenceSet *tcontseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequenceSet *tcontseq_restrict_values(const TSequence *seq, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_spanset(const TSequence *seq, const SpanSet *ss, bool atfunc);
extern TSequenceSet *tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);

extern TInstant *tdiscseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_restrict_tstzset(const TSequence *seq, const Set *ts, bool atfunc);
extern TSequence *tdiscseq_at_period(const TSequence *seq, const Span *period);
extern TSequence *tdiscseq_minus_period(const TSequence *seq, const Span *period);
extern TSequence *tdiscseq_restrict_periodset(const TSequence *seq, const SpanSet *ps, bool atfunc);

extern TInstant *tcontseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequenceSet *tcontseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tcontseq_at_tstzset(const TSequence *seq, const Set *ss);
extern TSequenceSet *tcontseq_minus_tstzset(const TSequence *seq, const Set *ss);
extern TSequence *tcontseq_at_period(const TSequence *seq, const Span *p);
extern TSequenceSet *tcontseq_minus_period(const TSequence *seq, const Span *p);
extern TSequenceSet *tcontseq_restrict_periodset(const TSequence *seq, const SpanSet *ps, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ss, Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ss, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_span(const TSequenceSet *ss, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_spanset(const TSequenceSet *ss, const SpanSet *spanset, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ss, bool min, bool atfunc);
extern Temporal *tsequenceset_restrict_timestamp(const TSequenceSet *ss, TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_tstzset(const TSequenceSet *ss1, const Set *ss2, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_period(const TSequenceSet *ss, const Span *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_periodset(const TSequenceSet *ss, const SpanSet *ps, bool atfunc);
extern TInstant *tpointinst_restrict_geometry(const TInstant *inst, const GSERIALIZED *gs, bool atfunc);
extern TSequence *tpointdiscseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs, bool atfunc);
extern TSequenceSet *tpointseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_geometry(const TSequenceSet *ss, const GSERIALIZED *gs, const STBox *box, bool atfunc);
extern Temporal *tpoint_restrict_geometry(const Temporal *temp, const GSERIALIZED *gs, bool atfunc);
extern Temporal *tpoint_restrict_stbox(const Temporal *temp, const STBox *box, bool atfunc);

/*****************************************************************************/

/* Boolean functions for temporal types */


/*****************************************************************************/

/* Mathematical functions for temporal types */

extern TSequence *tnumberseq_derivative(const TSequence *seq);
extern TSequenceSet *tnumberseqset_derivative(const TSequenceSet *ts);

/*****************************************************************************/

/* Text functions for temporal types */

/*****************************************************************************
 * Bounding box functions for temporal types
 *****************************************************************************/

/* Topological functions for temporal types */

extern bool contains_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool contains_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool contained_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool contained_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool overlaps_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool overlaps_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool same_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool same_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool adjacent_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool adjacent_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);

/*****************************************************************************/

/* Position functions for temporal types */

extern bool left_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool overleft_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool right_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool overright_number_tnumber(Datum number, meosType basetype, const Temporal *tnumber);
extern bool left_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool overleft_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool right_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);
extern bool overright_tnumber_number(const Temporal *tnumber, Datum number, meosType basetype);

/*****************************************************************************/

/* Distance functions for temporal types */

extern Temporal *distance_tnumber_number(const Temporal *temp, Datum value, meosType valuetype, meosType restype);
extern double nad_tnumber_number(const Temporal *temp, Datum value, meosType basetype);

/*****************************************************************************/

/* Ever/always functions for temporal types */

extern bool temporal_always_eq(const Temporal *temp, Datum value);
extern bool temporal_always_le(const Temporal *temp, Datum value);
extern bool temporal_always_lt(const Temporal *temp, Datum value);
extern bool temporal_ever_eq(const Temporal *temp, Datum value);
extern bool temporal_ever_le(const Temporal *temp, Datum value);
extern bool temporal_ever_lt(const Temporal *temp, Datum value);
extern bool tinstant_always_eq(const TInstant *inst, Datum value);
extern bool tinstant_always_le(const TInstant *inst, Datum value);
extern bool tinstant_always_lt(const TInstant *inst, Datum value);
extern bool tinstant_ever_eq(const TInstant *inst, Datum value);
extern bool tinstant_ever_le(const TInstant *inst, Datum value);
extern bool tinstant_ever_lt(const TInstant *inst, Datum value);
extern bool tpoint_always_eq(const Temporal *temp, Datum value);
extern bool tpoint_ever_eq(const Temporal *temp, Datum value);
extern bool tpointinst_always_eq(const TInstant *inst, Datum value);
extern bool tpointinst_ever_eq(const TInstant *inst, Datum value);
extern bool tpointseq_always_eq(const TSequence *seq, Datum value);
extern bool tpointseq_ever_eq(const TSequence *seq, Datum value);
extern bool tpointseqset_always_eq(const TSequenceSet *ss, Datum value);
extern bool tpointseqset_ever_eq(const TSequenceSet *ss, Datum value);
extern bool tsequence_always_eq(const TSequence *seq, Datum value);
extern bool tsequence_always_le(const TSequence *seq, Datum value);
extern bool tsequence_always_lt(const TSequence *seq, Datum value);
extern bool tsequence_ever_eq(const TSequence *seq, Datum value);
extern bool tsequence_ever_le(const TSequence *seq, Datum value);
extern bool tsequence_ever_lt(const TSequence *seq, Datum value);
extern bool tsequenceset_always_eq(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_always_le(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_always_lt(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_eq(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_le(const TSequenceSet *ss, Datum value);
extern bool tsequenceset_ever_lt(const TSequenceSet *ss, Datum value);

/*****************************************************************************/

/* Comparison functions for temporal types */

extern Temporal *teq_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *teq_temporal_base(const Temporal *temp, Datum base, meosType basetype);
extern Temporal *tne_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *tne_temporal_base(const Temporal *temp, Datum base, meosType basetype);
extern Temporal *tlt_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *tlt_temporal_base(const Temporal *temp, Datum base, meosType basetype);
extern Temporal *tle_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *tle_temporal_base(const Temporal *temp, Datum base, meosType basetype);
extern Temporal *tgt_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *tgt_temporal_base(const Temporal *temp, Datum base, meosType basetype);
extern Temporal *tge_base_temporal(Datum base, meosType basetype, const Temporal *temp);
extern Temporal *tge_temporal_base(const Temporal *temp, Datum base, meosType basetype);

extern int tinstant_cmp(const TInstant *inst1, const TInstant *inst2);
extern bool tinstant_eq(const TInstant *inst1, const TInstant *inst2);
extern int tdiscseq_cmp(const TSequence *is1, const TSequence *is2);
extern bool tdiscseq_eq(const TSequence *is1, const TSequence *is2);
extern int tsequence_cmp(const TSequence *seq1, const TSequence *seq2);
extern bool tsequence_eq(const TSequence *seq1, const TSequence *seq2);
extern int tsequenceset_cmp(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern bool tsequenceset_eq(const TSequenceSet *ss1, const TSequenceSet *ss2);

/*****************************************************************************
 * Spatial functions for temporal point types
 *****************************************************************************/

/* Spatial accessor functions for temporal point types */

extern int tpointinst_srid(const TInstant *inst);
extern bool tpointdiscseq_is_simple(const TSequence *seq);
extern GSERIALIZED *tpointcontseq_trajectory(const TSequence *seq);
extern GSERIALIZED *tpointdiscseq_trajectory(const TSequence *seq);
extern TSequenceSet *tpointseq_azimuth(const TSequence *seq);
extern TSequence *tpointseq_cumulative_length(const TSequence *seq, double prevlength);
extern bool tpointseq_is_simple(const TSequence *seq);
extern double tpointseq_length(const TSequence *seq);
extern TSequence *tpointseq_speed(const TSequence *seq);
extern int tpointseq_srid(const TSequence *seq);
extern STBox *tpointseq_stboxes(const TSequence *seq, int *count);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ss);
extern bool tpointseqset_is_simple(const TSequenceSet *ss);
extern double tpointseqset_length(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ss);
extern int tpointseqset_srid(const TSequenceSet *ss);
extern STBox *tpointseqset_stboxes(const TSequenceSet *ss, int *count);
extern GSERIALIZED *tpointseqset_trajectory(const TSequenceSet *ss);

/*****************************************************************************/

/* Spatial transformation functions for temporal point types */

extern TInstant *tgeompointinst_tgeogpointinst(const TInstant *inst, bool oper);
extern TSequence *tgeompointseq_tgeogpointseq(const TSequence *seq, bool oper);
extern TSequenceSet *tgeompointseqset_tgeogpointseqset(const TSequenceSet *ss, bool oper);
extern TInstant *tpointinst_set_srid(const TInstant *inst, int32 srid);
extern TSequence **tpointseq_make_simple(const TSequence *seq, int *count);
extern TSequence *tpointseq_set_srid(const TSequence *seq, int32 srid);
extern TSequence **tpointseqset_make_simple(const TSequenceSet *ss, int *count);
extern TSequenceSet *tpointseqset_set_srid(const TSequenceSet *ss, int32 srid);

/*****************************************************************************/

/* Spatial relationship functions for temporal point types */


/*****************************************************************************/

/* Modification functions for temporal types */

extern Temporal *tcontseq_insert(const TSequence *seq1, const TSequence *seq2);
// extern Temporal *tcontseq_update(const TSequence *seq1, const TSequence *seq2);
extern TSequenceSet *tsequenceset_insert(const TSequenceSet *ss1, const TSequenceSet *ss2);
// extern TSequenceSet *tsequenceset_update(const TSequenceSet *ss1, const TSequenceSet *ss2);

extern TSequence *tcontseq_delete_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tcontseq_delete_tstzset(const TSequence *seq, const Set *ts);
extern TSequence *tcontseq_delete_period(const TSequence *seq, const Span *p);
extern TSequence *tcontseq_delete_periodset(const TSequence *seq, const SpanSet *ps);
extern TSequenceSet *tsequenceset_delete_timestamp(const TSequenceSet *ss, TimestampTz t);
extern TSequenceSet *tsequenceset_delete_tstzset(const TSequenceSet *ss, const Set *ts);
extern TSequenceSet *tsequenceset_delete_period(const TSequenceSet *ss, const Span *p);
extern TSequenceSet *tsequenceset_delete_periodset(const TSequenceSet *ss, const SpanSet *ps);

/*****************************************************************************/

/* Overlaps functions for temporal types */

extern bool tinstant_overlaps_period(const TInstant *inst, const Span *p);
extern bool tinstant_overlaps_periodset(const TInstant *inst, const SpanSet *ps);
extern bool tinstant_overlaps_timestamp(const TInstant *inst, TimestampTz t);
extern bool tinstant_overlaps_tstzset(const TInstant *inst, const Set *ss);
extern bool tsequence_overlaps_period(const TSequence *seq, const Span *p);
extern bool tsequence_overlaps_periodset(const TSequence *seq, const SpanSet *ps);
extern bool tsequence_overlaps_timestamp(const TSequence *seq, TimestampTz t);
extern bool tsequence_overlaps_tstzset(const TSequence *seq, const Set *ss);
extern bool tsequenceset_overlaps_period(const TSequenceSet *ss, const Span *p);
extern bool tsequenceset_overlaps_periodset(const TSequenceSet *ss, const SpanSet *ps);
extern bool tsequenceset_overlaps_timestamp(const TSequenceSet *ss, TimestampTz t);
extern bool tsequenceset_overlaps_tstzset(const TSequenceSet *ss, const Set *ss1);

/*****************************************************************************/

/* Local aggregate functions for temporal types */

extern double tnumberseq_integral(const TSequence *seq);
extern double tnumbercontseq_twavg(const TSequence *seq);
extern double tnumberdiscseq_twavg(const TSequence *seq);
extern double tnumberseqset_integral(const TSequenceSet *ss);
extern double tnumberseqset_twavg(const TSequenceSet *ss);
extern GSERIALIZED *tpointseq_twcentroid(const TSequence *seq);
extern GSERIALIZED *tpointseqset_twcentroid(const TSequenceSet *ss);

/*****************************************************************************/

/* Multidimensional tiling functions for temporal types */

extern Temporal **temporal_time_split1(const Temporal *temp, TimestampTz start,
  TimestampTz end, int64 tunits, TimestampTz torigin, int count,
  TimestampTz **buckets, int *newcount);
extern Temporal **tnumber_value_split1(const Temporal *temp, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount);

/*****************************************************************************/

/* Similarity functions for temporal types */


/*****************************************************************************/

#endif
