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

extern TInstant *tfunc_tinstant(const TInstant *inst, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TInstantSet *tfunc_tinstantset(const TInstantSet *ti, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TSequence *tfunc_tsequence(const TSequence *seq, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TSequenceSet *tfunc_tsequenceset(const TSequenceSet *ts, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern Temporal *tfunc_temporal(const Temporal *temp, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid);

extern TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TInstantSet *
tfunc_tinstantset_base(const TInstantSet *ti, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TSequence *
tfunc_tsequence_base(const TSequence *seq, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);
extern Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert);

extern TSequenceSet *
tfunc4_tsequence_base_cross(const TSequence *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);
extern TSequenceSet *
tfunc4_tsequenceset_base_cross(const TSequenceSet *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert);

extern TInstant *
sync_tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TInstantSet *
sync_tfunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);
extern TSequence *
sync_tfunc_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *));
extern TSequenceSet *
sync_tfunc_tsequenceset_tsequenceset(const TSequenceSet *ts1, const TSequenceSet *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *));
extern Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool cross, bool (*turnpoint)(const TInstant *, const TInstant *, 
		const TInstant *, const TInstant *, TimestampTz *));

extern Temporal *
sync_tfunc_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid);

/*****************************************************************************/

#endif
