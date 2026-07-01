-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Input/output functions
-------------------------------------------------------------------------------
-- Temporal instant

SELECT th3index '622236723497533439@2012-01-01 08:00:00';
/* Errors */
SELECT th3index 'ABC@2012-01-01 08:00:00';
SELECT th3index '622236723497533439@2012-01-01 08:00:00,';

-------------------------------------------------------------------------------
-- Temporal discrete sequence

SELECT th3index ' { 622236723497533439@2001-01-01 08:00:00 , 622236723497533440@2001-01-01 08:05:00 , 622236723497533441@2001-01-01 08:06:00 } ';
SELECT th3index '{622236723497533439@2001-01-01 08:00:00,622236723497533440@2001-01-01 08:05:00,622236723497533441@2001-01-01 08:06:00}';
/* Errors */
SELECT th3index '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02, 622236723497533441@2001-01-03';
SELECT th3index '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02, 622236723497533441@2001-01-03},';

-------------------------------------------------------------------------------
-- Temporal continuous sequence (step interpolation is inherited from T_INT8)

SELECT th3index ' [ 622236723497533439@2001-01-01 08:00:00 , 622236723497533440@2001-01-01 08:05:00 , 622236723497533441@2001-01-01 08:06:00 ] ';
SELECT th3index '[622236723497533439@2001-01-01 08:00:00,622236723497533440@2001-01-01 08:05:00,622236723497533441@2001-01-01 08:06:00]';
SELECT th3index 'Interp=Step;[622236723497533439@2001-01-01 08:00:00,622236723497533440@2001-01-01 08:05:00,622236723497533441@2001-01-01 08:06:00]';

-------------------------------------------------------------------------------
-- Temporal sequence set

SELECT th3index ' { [ 622236723497533439@2001-01-01 08:00:00 , 622236723497533440@2001-01-01 08:05:00 ] , [ 622236723497533441@2001-01-01 08:10:00 , 622236723497533442@2001-01-01 08:15:00 ] } ';
SELECT th3index '{[622236723497533439@2001-01-01 08:00:00,622236723497533440@2001-01-01 08:05:00],[622236723497533441@2001-01-01 08:10:00,622236723497533442@2001-01-01 08:15:00]}';

-------------------------------------------------------------------------------
-- MF-JSON input/output (h3index cell ids are serialized as their int8 value,
-- mirroring the tbigint sibling)
-------------------------------------------------------------------------------

SELECT asMFJSON(th3index '612544986753269759@2012-01-01 08:00:00');
SELECT asMFJSON(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}');
SELECT asMFJSON(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]');
SELECT asMFJSON(th3index '{[612544986753269759@2001-01-01, 612544986761658367@2001-01-02], [612544986753269759@2001-01-03, 612544986753269759@2001-01-04]}');
-- Bound the geodetic bbox precision so the output is platform-independent
SELECT asMFJSON(th3index '612544986753269759@2012-01-01 08:00:00', 1, 3, 6);

-- Round-trip: th3indexFromMFJSON(asMFJSON(...)) preserves the value
SELECT th3indexFromMFJSON(asMFJSON(th3index '612544986753269759@2012-01-01 08:00:00'));
SELECT th3indexFromMFJSON(asMFJSON(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}'));
SELECT th3indexFromMFJSON(asMFJSON(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]'));
SELECT th3indexFromMFJSON(asMFJSON(th3index '{[612544986753269759@2001-01-01, 612544986761658367@2001-01-02], [612544986753269759@2001-01-03, 612544986753269759@2001-01-04]}'));

-------------------------------------------------------------------------------
-- Typmod
-------------------------------------------------------------------------------

SELECT th3index(Instant) '622236723497533439@2012-01-01 08:00:00';
SELECT th3index(Sequence) '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02}';
SELECT th3index(Sequence) '[622236723497533439@2001-01-01, 622236723497533440@2001-01-02]';
SELECT th3index(SequenceSet) '{[622236723497533439@2001-01-01, 622236723497533440@2001-01-02]}';
/* Errors */
SELECT th3index(Instant) '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02}';
SELECT th3index(Sequence) '622236723497533439@2012-01-01 08:00:00';
SELECT th3index(Garbage) '622236723497533439@2012-01-01 08:00:00';

-------------------------------------------------------------------------------
-- Assignment casts to/from tbigint
--
-- Casts are explicit (AS ASSIGNMENT) and bidirectional. Binary-coercion
-- casts are zero-copy: the on-disk representation is identical.
-------------------------------------------------------------------------------

-- tbigint -> th3index (explicit via ::)
SELECT (tbigint '622236723497533439@2012-01-01 08:00:00')::th3index;
SELECT (tbigint '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02}')::th3index;
SELECT (tbigint '[622236723497533439@2001-01-01, 622236723497533440@2001-01-02]')::th3index;

-- th3index -> tbigint (explicit via ::)
SELECT (th3index '622236723497533439@2012-01-01 08:00:00')::tbigint;
SELECT (th3index '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02}')::tbigint;
SELECT (th3index '[622236723497533439@2001-01-01, 622236723497533440@2001-01-02]')::tbigint;

-- Round-trip preserves value
SELECT ((tbigint '622236723497533439@2012-01-01 08:00:00')::th3index)::tbigint
  = tbigint '622236723497533439@2012-01-01 08:00:00';
SELECT ((th3index '622236723497533439@2012-01-01 08:00:00')::tbigint)::th3index
  = th3index '622236723497533439@2012-01-01 08:00:00';

-- Assignment-context: casts are accepted in INSERT/UPDATE without ::
DROP TABLE IF EXISTS tbl_th3index_assign;
CREATE TABLE tbl_th3index_assign(k int, temp th3index);
INSERT INTO tbl_th3index_assign VALUES (1, tbigint '622236723497533439@2012-01-01 08:00:00');
SELECT * FROM tbl_th3index_assign;
DROP TABLE tbl_th3index_assign;

-- The cast is NOT implicit: comparing a tbigint to a th3index expression
-- without an explicit cast must error (binder reports that no operator
-- exists for the mixed types).
/* Errors */
SELECT tbigint '622236723497533439@2012-01-01 08:00:00' = th3index '622236723497533439@2012-01-01 08:00:00';

-------------------------------------------------------------------------------
-- Send/receive (binary IO round-trip)
-------------------------------------------------------------------------------

DROP TABLE IF EXISTS tbl_th3index_binio;
CREATE TABLE tbl_th3index_binio(k int, temp th3index);
INSERT INTO tbl_th3index_binio VALUES
  (1, th3index '622236723497533439@2012-01-01 08:00:00'),
  (2, th3index '{622236723497533439@2001-01-01, 622236723497533440@2001-01-02}'),
  (3, th3index '[622236723497533439@2001-01-01, 622236723497533440@2001-01-02]');

COPY tbl_th3index_binio TO '/tmp/tbl_th3index_binio' (FORMAT BINARY);

DROP TABLE IF EXISTS tbl_th3index_binio_tmp;
CREATE TABLE tbl_th3index_binio_tmp AS TABLE tbl_th3index_binio WITH NO DATA;
COPY tbl_th3index_binio_tmp FROM '/tmp/tbl_th3index_binio' (FORMAT BINARY);

SELECT COUNT(*) FROM tbl_th3index_binio t1, tbl_th3index_binio_tmp t2
  WHERE t1.k = t2.k AND t1.temp <> t2.temp;

DROP TABLE tbl_th3index_binio;
DROP TABLE tbl_th3index_binio_tmp;

-------------------------------------------------------------------------------
-- Constructors (inherited Temporal<T> surface)
-------------------------------------------------------------------------------
SELECT th3index(h3index '612544986753269759', timestamptz '2001-01-01 08:00');
SELECT th3index(h3index '612544986753269759', tstzset '{2001-01-01, 2001-01-02}');
SELECT th3index(h3index '612544986753269759', tstzspan '[2001-01-01, 2001-01-02]');
SELECT th3index(h3index '612544986753269759', tstzspanset '{[2001-01-01, 2001-01-02]}');
SELECT th3indexSeq(ARRAY[th3index '612544986753269759@2001-01-01', th3index '612544986761658367@2001-01-02']);
SELECT th3indexSeqSet(ARRAY[th3indexSeq(ARRAY[th3index '612544986753269759@2001-01-01'])]);

-------------------------------------------------------------------------------
-- Conversions / transformations
-------------------------------------------------------------------------------
SELECT timeSpan(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]');
SELECT (th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]')::tstzspan;
SELECT th3indexInst(th3index '612544986753269759@2001-01-01');
SELECT th3indexSeq(th3index '612544986753269759@2001-01-01');
SELECT setInterp(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]', 'discrete');
SELECT appendInstant(th3index '[612544986753269759@2001-01-01]', th3index '612544986761658367@2001-01-02');
SELECT merge(th3index '612544986753269759@2001-01-01', th3index '612544986761658367@2001-01-02');
SELECT shiftTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', interval '1 day');
SELECT scaleTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', interval '2 days');

-------------------------------------------------------------------------------
-- Accessors
-------------------------------------------------------------------------------
SELECT tempSubtype(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}');
SELECT interp(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]');
SELECT memSize(th3index '612544986753269759@2001-01-01') > 0;
SELECT getValue(th3index '612544986753269759@2001-01-01');
SELECT getTimestamp(th3index '612544986753269759@2001-01-01');
SELECT getTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]');
SELECT duration(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]');
SELECT numInstants(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}');
SELECT startInstant(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]');
SELECT instantN(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}', 2);
SELECT numTimestamps(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}');
SELECT timestamps(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}');
SELECT numSequences(th3index '{[612544986753269759@2001-01-01], [612544986761658367@2001-01-02]}');
SELECT sequenceN(th3index '{[612544986753269759@2001-01-01], [612544986761658367@2001-01-02]}', 1);
SELECT segments(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02]');
SELECT (rec).value, (rec).time FROM
  (SELECT unnest(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}') AS rec) t;

-------------------------------------------------------------------------------
-- Restrictions
-------------------------------------------------------------------------------
SELECT atValues(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02, 612544986753269759@2001-01-03]', h3index '612544986753269759');
SELECT minusValues(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-02, 612544986753269759@2001-01-03]', h3index '612544986753269759');
SELECT atValues(th3index '{612544986753269759@2001-01-01, 612544986761658367@2001-01-02}', set(ARRAY[h3index '612544986753269759', h3index '612544986761658367']));
SELECT atTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', timestamptz '2001-01-02');
SELECT minusTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', timestamptz '2001-01-02');
SELECT atTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-05]', tstzspan '[2001-01-02, 2001-01-04]');
SELECT beforeTimestamp(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-05]', timestamptz '2001-01-03');
SELECT afterTimestamp(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-05]', timestamptz '2001-01-03');

-------------------------------------------------------------------------------
-- Modifications
-------------------------------------------------------------------------------
SELECT insert(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', th3index '[612544986761658367@2001-01-04, 612544986761658367@2001-01-05]');
SELECT update(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', th3index '612544986761658367@2001-01-02');
SELECT deleteTime(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-03]', timestamptz '2001-01-02');
SELECT (rec).time, (rec).temp FROM
  (SELECT timeSplit(th3index '[612544986753269759@2001-01-01, 612544986761658367@2001-01-04]', interval '2 days', timestamptz '2001-01-01') AS rec) t;

-------------------------------------------------------------------------------
