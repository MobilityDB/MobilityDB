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

TemporalInst *tfunc1_temporalinst(const TemporalInst *inst, Datum (*func)(Datum),
	Oid valuetypid);
TemporalI *tfunc1_temporali(const TemporalI *ti, Datum (*func)(Datum),
	Oid valuetypid);
TemporalSeq *tfunc1_temporalseq(const TemporalSeq *seq, Datum (*func)(Datum),
	Oid valuetypid);
TemporalS *tfunc1_temporals(const TemporalS *ts, Datum (*func)(Datum),
	Oid valuetypid);
Temporal *tfunc1_temporal(const Temporal *temp, Datum (*func)(Datum),
	Oid valuetypid);

TemporalInst *tfunc2_temporalinst(const TemporalInst *inst, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *tfunc2_temporali(const TemporalI *ti, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalSeq *tfunc2_temporalseq(const TemporalSeq *seq, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *tfunc2_temporals(const TemporalS *ts, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);
Temporal *tfunc2_temporal(const Temporal *temp, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid);

TemporalInst *tfunc2_temporalinst_base(const TemporalInst *inst, Datum value,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalI *tfunc2_temporali_base(const TemporalI *ti, Datum value,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalSeq *tfunc2_temporalseq_base(const TemporalSeq *seq, Datum value,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
TemporalS *tfunc2_temporals_base(const TemporalS *ts, Datum value,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);
Temporal *tfunc2_temporal_base(const Temporal *temp, Datum d,
	Datum (*func)(Datum, Datum), Oid restypid, bool invert);

TemporalInst *tfunc3_temporalinst_base(const TemporalInst *inst, Datum value, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert);
TemporalI *tfunc3_temporali_base(const TemporalI *ti, Datum value, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid restypid, bool invert);

TemporalInst *tfunc4_temporalinst_base(const TemporalInst *inst, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalI *tfunc4_temporali_base(const TemporalI *ti, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalSeq *tfunc4_temporalseq_base(const TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalS *tfunc4_temporals_base(const TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
Temporal *tfunc4_temporal_base(const Temporal *temp, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

TemporalS *tfunc4_temporalseq_base_cross(const TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
TemporalS *tfunc4_temporals_base_cross(const TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

TemporalInst *sync_tfunc2_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc2_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc2_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalSeq *sync_tfunc2_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum (*func)(Datum, Datum), Oid valuetypid,bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc2_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
Temporal *sync_tfunc2_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));

TemporalInst *sync_tfunc3_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalInst *sync_tfunc3_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalI *sync_tfunc3_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalSeq *sync_tfunc3_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid,bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc3_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc3_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc3_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
Temporal *sync_tfunc3_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));

TemporalInst *sync_tfunc4_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalInst *sync_tfunc4_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalI *sync_tfunc4_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalSeq *sync_tfunc4_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
TemporalS *sync_tfunc4_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
Temporal *sync_tfunc4_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));

TemporalS *sync_tfunc2_temporalseq_temporalseq_cross(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporals_temporalseq_cross(const TemporalS *ts, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporalseq_temporals_cross(const TemporalSeq *seq, const TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc2_temporals_temporals_cross(const TemporalS *ts1, const TemporalS *ts2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
Temporal *sync_tfunc2_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporalseq_temporalseq_cross(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporals_temporalseq_cross(const TemporalS *ts, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporalseq_temporals_cross(const TemporalSeq *seq, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc3_temporals_temporals_cross(const TemporalS *ts1, const TemporalS *ts2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
Temporal *sync_tfunc3_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid);
TemporalS *sync_tfunc4_temporalseq_temporalseq_cross(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporals_temporalseq_cross(const TemporalS *ts, const TemporalSeq *seq,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporalseq_temporals_cross(const TemporalSeq *seq, const TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
TemporalS *sync_tfunc4_temporals_temporals_cross(const TemporalS *ts1, const TemporalS *ts2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);
Temporal *sync_tfunc4_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid);

/*****************************************************************************/

#endif
