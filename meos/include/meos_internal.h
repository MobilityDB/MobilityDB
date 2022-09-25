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
// #include "postgres.h"
/* MobilityDB */
#include "general/temporal_catalog.h" /* For mobdbType */

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#define TimestampTzGetDatum(X) Int64GetDatum(X)
#define DatumGetTimestampTz(X)((TimestampTz) DatumGetInt64(X))

/*****************************************************************************
 * Functions for span and time types
 *****************************************************************************/

/* Input/output functions for span and time types */

extern Span *span_in(const char *str, mobdbType spantype);

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
extern void timestampset_set_period(const TimestampSet *ts, Period *p);
extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index);

/*****************************************************************************/

/* Transformation functions for span and time types */

extern void lower_upper_shift_tscale(const Interval *shift, const Interval *duration, TimestampTz *lower, TimestampTz *upper);

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

extern bool inter_span_span(const Span *s1, const Span *s2, Span *result);

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

extern void int_set_tbox(int i, TBOX *box);
extern void float_set_tbox(double d, TBOX *box);
extern void span_set_tbox(const Span *span, TBOX *box);
extern void timestamp_set_tbox(TimestampTz t, TBOX *box);
extern void timestampset_set_tbox(const TimestampSet *ss, TBOX *box);
extern void period_set_tbox(const Period *p, TBOX *box);
extern void periodset_set_tbox(const PeriodSet *ps, TBOX *box);
extern bool geo_set_stbox(const GSERIALIZED *gs, STBOX *box);
extern void timestamp_set_stbox(TimestampTz t, STBOX *box);
extern void timestampset_set_stbox(const TimestampSet *ts, STBOX *box);
extern void period_set_stbox(const Period *p, STBOX *box);
extern void periodset_set_stbox(const PeriodSet *ps, STBOX *box);

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
extern Temporal *temporal_in(const char *str, mobdbType temptype);
extern char *temporal_out(const Temporal *temp, Datum arg);
extern char **temporalarr_out(const Temporal **temparr, int count, Datum arg);
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
extern TInstant *tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype);
extern TInstant *tinstant_in(const char *str, mobdbType temptype);
extern char *tinstant_out(const TInstant *inst, Datum arg);
extern TSequence *tdiscseq_in(const char *str, mobdbType temptype);
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
extern TSequence *tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype, interpType interp);
extern TSequence *tsequence_in(const char *str, mobdbType temptype, interpType interp);
extern char *tsequence_out(const TSequence *seq, Datum arg);
extern char *tsequenceset_as_mfjson(const TSequenceSet *ss, int precision, bool with_bbox, char *srs);
extern TSequenceSet *tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid, mobdbType temptype, interpType interp);
extern TSequenceSet *tsequenceset_in(const char *str, mobdbType temptype, interpType interp);
extern char *tsequenceset_out(const TSequenceSet *ss, Datum arg);
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

extern Temporal *temporal_from_base(Datum value, mobdbType temptype, const Temporal *temp, interpType interp);
extern TInstant *tinstant_copy(const TInstant *inst);
extern TInstant *tinstant_make(Datum value, mobdbType temptype, TimestampTz t);
extern TSequence *tdiscseq_from_base_time(Datum value, mobdbType temptype, const TimestampSet *ss);
extern TSequence *tsequence_copy(const TSequence *seq);
extern TSequence *tsequence_from_base(Datum value, mobdbType temptype, const TSequence *seq, interpType interp);
extern TSequence *tsequence_from_base_time(Datum value, mobdbType temptype, const Period *p, interpType interp);
extern TSequenceSet *tsequenceset_copy (const TSequenceSet *ss);
extern TSequenceSet *tsequenceset_from_base(Datum value, mobdbType temptype, const TSequenceSet *ss, interpType interp);
extern TSequenceSet *tsequenceset_from_base_time(Datum value, mobdbType temptype, const PeriodSet *ps, interpType interp);

/*****************************************************************************/

/* Cast functions for temporal types */

extern void temporal_set_period(const Temporal *temp, Period *p);
extern TInstant *tfloatinst_to_tintinst(const TInstant *inst);
extern TSequence *tfloatseq_to_tintseq(const TSequence *seq);
extern TSequenceSet *tfloatseqset_to_tintseqset(const TSequenceSet *ss);
extern void tinstant_set_period(const TInstant *inst, Period *p);
extern TInstant *tintinst_to_tfloatinst(const TInstant *inst);
extern TSequence *tintseq_to_tfloatseq(const TSequence *seq);
extern TSequenceSet *tintseqset_to_tfloatseqset(const TSequenceSet *ss);
extern void tsequence_set_period(const TSequence *seq, Period *p);
extern void tsequenceset_set_period(const TSequenceSet *ss, Period *p);

/*****************************************************************************/

/* Accessor functions for temporal types */

extern Datum temporal_end_value(const Temporal *temp);
extern Datum temporal_max_value(const Temporal *temp);
extern Datum temporal_min_value(const Temporal *temp);
extern void temporal_set_bbox(const Temporal *temp, void *box);
extern Datum temporal_start_value(const Temporal *temp);
extern Datum *temporal_values(const Temporal *temp, int *count);
extern Span **tfloatinst_spans(const TInstant *inst, int *count);
extern Span *tfloatseq_span(const TSequence *seq);
extern Span **tfloatseq_spans(const TSequence *seq, int *count);
extern Span *tfloatseqset_span(const TSequenceSet *ss);
extern Span **tfloatseqset_spans(const TSequenceSet *ss, int *count);
extern uint32 tinstant_hash(const TInstant *inst);
extern const TInstant **tinstant_instants(const TInstant *inst, int *count);
extern TSequence **tinstant_sequences(const TInstant *inst, int *count);
extern void tinstant_set_bbox(const TInstant *inst, void *box);
extern PeriodSet *tinstant_time(const TInstant *inst);
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
extern TimestampTz tsequence_start_timestamp(const TSequence *seq);
extern PeriodSet *tsequence_time(const TSequence *seq);
extern TimestampTz *tsequence_timestamps(const TSequence *seq, int *count);
extern bool tsequence_value_at_timestamp(const TSequence *seq, TimestampTz t, bool strict, Datum *result);
extern Datum *tsequence_values(const TSequence *seq, int *count);
extern Interval *tsequenceset_duration(const TSequenceSet *ss);
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
extern PeriodSet *tsequenceset_time(const TSequenceSet *ss);
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
extern Temporal *tsequence_merge(const TSequence *seq1, const TSequence *seq2);
extern Temporal *tsequence_merge_array(const TSequence **sequences, int count);
extern TSequence *tsequence_shift_tscale(const TSequence *seq, const Interval *start, const Interval *duration);
extern TSequenceSet *tsequence_step_to_linear(const TSequence *seq);
extern TInstant *tsequence_to_tinstant(const TSequence *seq);
extern TSequence *tsequence_to_tdiscseq(const TSequence *seq);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tsequenceset_append_tinstant(TSequenceSet *ss, const TInstant *inst, bool expand);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ss1, const TSequenceSet *ss2);
extern TSequenceSet *tsequenceset_merge_array(const TSequenceSet **seqsets, int count);
extern TSequenceSet *tsequenceset_shift_tscale(const TSequenceSet *ss, const Interval *start, const Interval *duration);
extern TSequenceSet *tsequenceset_step_to_linear(const TSequenceSet *ss);
extern TInstant *tsequenceset_to_tinstant(const TSequenceSet *ts);
extern TSequence *tsequenceset_to_tdiscseq(const TSequenceSet *ts);
extern TSequence *tsequenceset_to_tsequence(const TSequenceSet *ss);

/*****************************************************************************/

/* Restriction functions for temporal types */

extern Temporal *temporal_restrict_value(const Temporal *temp, Datum value, bool atfunc);
extern Temporal *temporal_restrict_values(const Temporal *temp, Datum *values, int count, bool atfunc);
extern Temporal *tnumber_restrict_span(const Temporal *temp, const Span *span, bool atfunc);
extern Temporal *tnumber_restrict_spans(const Temporal *temp, Span **spans, int count, bool atfunc);
extern Temporal *temporal_restrict_timestamp(const Temporal *temp, TimestampTz t, bool atfunc);
extern Temporal *temporal_restrict_timestampset(const Temporal *temp, const TimestampSet *ss, bool atfunc);
extern Temporal *temporal_restrict_period(const Temporal *temp, const Period *ps, bool atfunc);
extern Temporal *temporal_restrict_periodset(const Temporal *temp, const PeriodSet *ps, bool atfunc);
extern bool temporal_value_at_timestamp(const Temporal *temp, TimestampTz t, bool strict, Datum *result);
extern TInstant *tinstant_restrict_value(const TInstant *inst, Datum value, bool atfunc);
extern TInstant *tinstant_restrict_values(const TInstant *inst, const Datum *values, int count, bool atfunc);
extern TInstant *tnumberinst_restrict_span(const TInstant *inst, const Span *span, bool atfunc);
extern TInstant *tnumberinst_restrict_spans(const TInstant *inst, Span **normspans, int count, bool atfunc);
extern TInstant *tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool atfunc);
extern TInstant *tinstant_restrict_timestampset(const TInstant *inst, const TimestampSet *ss, bool atfunc);
extern TInstant *tinstant_restrict_period(const TInstant *inst, const Period *period, bool atfunc);
extern TInstant *tinstant_restrict_periodset(const TInstant *inst, const PeriodSet *ps, bool atfunc);
extern TSequence *tdiscseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequence *tdiscseq_restrict_values(const TSequence *seq, const Datum *values, int count, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequence *tnumberdiscseq_restrict_spans(const TSequence *seq, Span **normspans, int count, bool atfunc);
extern TSequence *tdiscseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);
extern TInstant *tsequence_at_timestamp(const TSequence *seq, TimestampTz t);
extern Temporal *tsequence_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tsequence_restrict_timestampset(const TSequence *seq, const TimestampSet *ss, bool atfunc);
extern TSequence *tsequence_restrict_period(const TSequence *seq, const Period *period, bool atfunc);
extern TSequence *tsequence_restrict_periodset(const TSequence *seq, const PeriodSet *ps, bool atfunc);
extern bool tdiscseq_value_at_timestamp(const TSequence *seq, TimestampTz t, Datum *result);
extern TSequenceSet *tcontseq_restrict_value(const TSequence *seq, Datum value, bool atfunc);
extern TSequenceSet *tcontseq_restrict_values(const TSequence *seq, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_span(const TSequence *seq, const Span *span, bool atfunc);
extern TSequenceSet *tnumbercontseq_restrict_spans(const TSequence *seq, Span **normspans, int count, bool atfunc, bool bboxtest);
extern TSequenceSet *tcontseq_restrict_minmax(const TSequence *seq, bool min, bool atfunc);

extern TInstant *tdiscseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tdiscseq_restrict_timestampset(const TSequence *seq, const TimestampSet *ts, bool atfunc);
extern TSequence *tdiscseq_at_period(const TSequence *seq, const Period *period);
extern TSequence *tdiscseq_minus_period(const TSequence *seq, const Period *period);
extern TSequence *tdiscseq_restrict_periodset(const TSequence *seq, const PeriodSet *ps, bool atfunc);

extern TInstant *tcontseq_at_timestamp(const TSequence *seq, TimestampTz t);
extern TSequenceSet *tcontseq_minus_timestamp(const TSequence *seq, TimestampTz t);
extern TSequence *tcontseq_at_timestampset(const TSequence *seq, const TimestampSet *ss);
extern TSequenceSet *tcontseq_minus_timestampset(const TSequence *seq, const TimestampSet *ss);
extern TSequence *tcontseq_at_period(const TSequence *seq, const Period *p);
extern TSequenceSet *tcontseq_minus_period(const TSequence *seq, const Period *p);
extern TSequenceSet *tcontseq_restrict_periodset(const TSequence *seq, const PeriodSet *ps, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ss, Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ss, const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_span(const TSequenceSet *ss, const Span *span, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_spans(const TSequenceSet *ss, Span **normspans, int count, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ss, bool min, bool atfunc);
extern Temporal *tsequenceset_restrict_timestamp(const TSequenceSet *ss, TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_timestampset(const TSequenceSet *ss1, const TimestampSet *ss2, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_period(const TSequenceSet *ss, const Period *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_periodset(const TSequenceSet *ss, const PeriodSet *ps, bool atfunc);
extern TInstant *tpointinst_restrict_geometry(const TInstant *inst, const GSERIALIZED *gs, bool atfunc);
extern TSequence *tpointdiscseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs, bool atfunc);
extern TSequenceSet *tpointseq_restrict_geometry(const TSequence *seq, const GSERIALIZED *gs, bool atfunc);
extern TSequenceSet *tpointseqset_restrict_geometry(const TSequenceSet *ss, const GSERIALIZED *gs, const STBOX *box, bool atfunc);
extern Temporal *tpoint_restrict_geometry(const Temporal *temp, const GSERIALIZED *gs, bool atfunc);
extern Temporal *tpoint_restrict_stbox(const Temporal *temp, const STBOX *box, bool atfunc);

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
extern double nad_tnumber_number(const Temporal *temp, Datum value, mobdbType basetype);

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
extern STBOX *tpointseq_stboxes(const TSequence *seq, int *count);
extern TSequenceSet *tpointseqset_azimuth(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_cumulative_length(const TSequenceSet *ss);
extern bool tpointseqset_is_simple(const TSequenceSet *ss);
extern double tpointseqset_length(const TSequenceSet *ss);
extern TSequenceSet *tpointseqset_speed(const TSequenceSet *ss);
extern int tpointseqset_srid(const TSequenceSet *ss);
extern STBOX *tpointseqset_stboxes(const TSequenceSet *ss, int *count);
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

/* Time functions for temporal types */

extern bool tinstant_intersects_period(const TInstant *inst, const Period *p);
extern bool tinstant_intersects_periodset(const TInstant *inst, const PeriodSet *ps);
extern bool tinstant_intersects_timestamp(const TInstant *inst, TimestampTz t);
extern bool tinstant_intersects_timestampset(const TInstant *inst, const TimestampSet *ss);
extern bool tsequence_intersects_period(const TSequence *seq, const Period *p);
extern bool tsequence_intersects_periodset(const TSequence *seq, const PeriodSet *ps);
extern bool tsequence_intersects_timestamp(const TSequence *seq, TimestampTz t);
extern bool tsequence_intersects_timestampset(const TSequence *seq, const TimestampSet *ss);
extern bool tsequenceset_intersects_period(const TSequenceSet *ss, const Period *p);
extern bool tsequenceset_intersects_periodset(const TSequenceSet *ss, const PeriodSet *ps);
extern bool tsequenceset_intersects_timestamp(const TSequenceSet *ss, TimestampTz t);
extern bool tsequenceset_intersects_timestampset(const TSequenceSet *ss, const TimestampSet *ss1);

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

extern Temporal **tnumber_value_split(const Temporal *temp, Datum start_bucket,
  Datum size, int count, Datum **buckets, int *newcount);

/*****************************************************************************/

/* Similarity functions for temporal types */


/*****************************************************************************/

#endif
