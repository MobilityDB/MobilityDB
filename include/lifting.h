/*****************************************************************************
 *
 * lifting.c
 *	Generic functions for lifting functions and operators on temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __LIFTING_H__
#define __LIFTING_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

TemporalInst *tfunc1_temporalinst(TemporalInst *inst, Datum (*func)(Datum), 
	Oid valuetypid);
TemporalSeq *tfunc1_temporalseq(TemporalSeq *seq, Datum (*func)(Datum), 
	Oid valuetypid);
TemporalS *tfunc1_temporals(TemporalS *ts, Datum (*func)(Datum), 
	Oid valuetypid);
Temporal *tfunc1_temporal(Temporal *temp, Datum (*func)(Datum), 
	Oid valuetypid);

TemporalInst *tfunc2_temporalinst(TemporalInst *inst, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *tfunc2_temporali(TemporalI *ti, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalSeq *tfunc2_temporalseq(TemporalSeq *seq, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *tfunc2_temporals(TemporalS *ts, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
Temporal *tfunc2_temporal(Temporal *temp, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);

TemporalInst *tfunc2_temporalinst_base(TemporalInst *inst, Datum value, 
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalI *tfunc2_temporali_base(TemporalI *ti, Datum value, 
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalSeq *tfunc2_temporalseq_base(TemporalSeq *seq, Datum value, 
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalS *tfunc2_temporals_base(TemporalS *ts, Datum value, 
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
Temporal *tfunc2_temporal_base(Temporal *temp, Datum d, 
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);

TemporalInst *tfunc3_temporalinst_base(TemporalInst *inst, Datum value, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert);
TemporalI *tfunc3_temporali_base(TemporalI *ti, Datum value, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert);

TemporalInst *tfunc4_temporalinst_base(TemporalInst *inst, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalI *tfunc4_temporali_base(TemporalI *ti, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalSeq *tfunc4_temporalseq_base(TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalS *tfunc4_temporals_base(TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
Temporal *tfunc4_temporal_base(Temporal *temp, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

TemporalS *tfunc4_temporalseq_base_cross(TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalS *tfunc4_temporals_base_cross(TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

TemporalInst *sync_tfunc2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalSeq *sync_tfunc2_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum), Oid valuetypid,bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
Temporal *sync_tfunc2_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

TemporalInst *sync_tfunc3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);

TemporalInst *sync_tfunc4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalSeq *sync_tfunc4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));
Temporal *sync_tfunc4_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *));

TemporalS *sync_tfunc2_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum), Oid valuetypid);
Temporal *sync_tfunc2_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
Temporal *sync_tfunc3_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc4_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
Temporal *sync_tfunc4_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);

/*****************************************************************************/

#endif
