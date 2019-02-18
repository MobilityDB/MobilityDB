/*****************************************************************************
 *
 * RelativePosOpsG.sql
 *	  Relative position operations for 4D (2D/3D spatial value + 1D time)
 *	  temporal geography points
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Timestamp
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestamptz, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * TimestampSet
 *****************************************************************************/

CREATE FUNCTION temporal_before(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = timestampset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * Period
 *****************************************************************************/

CREATE FUNCTION temporal_before(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * PeriodSet
 *****************************************************************************/

CREATE FUNCTION temporal_before(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = periodset, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * gbox
 *****************************************************************************/

CREATE FUNCTION temporal_before(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

/* tgeogpoint op timestamptz */

CREATE FUNCTION temporal_before(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = timestamptz,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op timestampset */

CREATE FUNCTION temporal_before(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = timestampset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op period */

CREATE FUNCTION temporal_before(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op periodset */

CREATE FUNCTION temporal_before(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = periodset,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op gbox */

CREATE FUNCTION temporal_before(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION temporal_before(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = before_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = overbefore_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = after_temporal_sel, JOIN = positionjoinseltemp
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = overafter_temporal_sel, JOIN = positionjoinseltemp
);

/*****************************************************************************/
