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

/* Definition of a variadic function type for temporal lifting */
typedef Datum (*varfunc)	(Datum, ...);

extern TemporalI *
tfunc_temporali(const TemporalI *ti, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TemporalS *
tfunc_temporals(const TemporalS *ts, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern Temporal *
tfunc_temporal(const Temporal *temp, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);

extern TemporalInst *
tfunc_temporalinst_base(const TemporalInst *inst, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TemporalI *
tfunc_temporali_base(const TemporalI *ti, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TemporalSeq *
tfunc_temporalseq_base(const TemporalSeq *seq, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TemporalS *
tfunc_temporals_base(const TemporalS *ts, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);

extern TemporalS *
tfunc4_temporalseq_base_cross(const TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
extern TemporalS *
tfunc4_temporals_base_cross(const TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

extern TemporalInst *
sync_tfunc_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TemporalI *
sync_tfunc_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TemporalSeq *
sync_tfunc_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
extern TemporalS *
sync_tfunc_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));
extern Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *));

extern Temporal *
sync_tfunc_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);

/*****************************************************************************/

#endif
