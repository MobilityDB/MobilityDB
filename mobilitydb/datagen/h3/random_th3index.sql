/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
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

/*
 * random_th3index.sql
 * Synthetic data generator functions for the th3index temporal type.
 *
 * th3index shares its on-disk representation with tbigint and an
 * ASSIGNMENT cast exists between the two. These generators delegate
 * to their tbigint counterparts and cast the result.
 */

-------------------------------------------------------------------------------
-- th3index instant
-------------------------------------------------------------------------------

/**
 * @brief Generate a random th3index instant
 * @param[in] lowvalue, highvalue Inclusive bounds of the range of values
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 */
DROP FUNCTION IF EXISTS random_th3index_inst;
CREATE FUNCTION random_th3index_inst(lowvalue bigint, highvalue bigint,
  lowtime timestamptz, hightime timestamptz)
  RETURNS th3index AS $$
BEGIN
  RETURN random_tbigint_inst(lowvalue, highvalue, lowtime, hightime)::th3index;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_inst(1, 20, '2001-01-01', '2002-01-01') AS inst
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- th3index discrete sequence
-------------------------------------------------------------------------------

/**
 * @brief Generate a random th3index discrete sequence
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 */
DROP FUNCTION IF EXISTS random_th3index_discseq;
CREATE FUNCTION random_th3index_discseq(lowvalue bigint, highvalue bigint,
  lowtime timestamptz, hightime timestamptz, maxdelta bigint, maxminutes int,
  mincard int, maxcard int)
  RETURNS th3index AS $$
BEGIN
  RETURN random_tbigint_discseq(lowvalue, highvalue, lowtime, hightime,
    maxdelta, maxminutes, mincard, maxcard)::th3index;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_discseq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS ti
FROM generate_series(1,10) k;
*/

-------------------------------------------------------------------------------
-- th3index sequence (step interpolation)
-------------------------------------------------------------------------------

/**
 * @brief Generate a random th3index sequence
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence
 * @param[in] fixstart True if the start timestamp is already fixed, false otherwise
 */
DROP FUNCTION IF EXISTS random_th3index_seq;
CREATE FUNCTION random_th3index_seq(lowvalue bigint, highvalue bigint,
  lowtime timestamptz, hightime timestamptz, maxdelta bigint,
  maxminutes int, mincard int, maxcard int, fixstart bool DEFAULT false)
  RETURNS th3index AS $$
BEGIN
  RETURN random_tbigint_seq(lowvalue, highvalue, lowtime, hightime, maxdelta,
    maxminutes, mincard, maxcard, fixstart)::th3index;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_seq(-100, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10) AS seq
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
-- th3index sequence set
-------------------------------------------------------------------------------

/**
 * @brief Generate a random th3index sequence set
 * @param[in] lowvalue, highvalue Inclusive bounds of the range
 * @param[in] lowtime, hightime Inclusive bounds of the tstzspan
 * @param[in] maxdelta Maximum value difference between consecutive instants
 * @param[in] maxminutes Maximum number of minutes between consecutive instants
 * @param[in] mincardseq, maxcardseq Inclusive bounds of the cardinality of a sequence
 * @param[in] mincard, maxcard Inclusive bounds of the cardinality of the sequence set
 */
DROP FUNCTION IF EXISTS random_th3index_seqset;
CREATE FUNCTION random_th3index_seqset(lowvalue bigint, highvalue bigint,
  lowtime timestamptz, hightime timestamptz, maxdelta bigint, maxminutes int,
  mincardseq int, maxcardseq int, mincard int, maxcard int)
  RETURNS th3index AS $$
BEGIN
  RETURN random_tbigint_seqset(lowvalue, highvalue, lowtime, hightime,
    maxdelta, maxminutes, mincardseq, maxcardseq, mincard, maxcard)::th3index;
END;
$$ LANGUAGE PLPGSQL STRICT;

/*
SELECT k, random_th3index_seqset(1, 100, '2001-01-01', '2002-01-01', 10, 10, 5, 10, 5, 10) AS ts
FROM generate_series(1, 15) AS k;
*/

-------------------------------------------------------------------------------
