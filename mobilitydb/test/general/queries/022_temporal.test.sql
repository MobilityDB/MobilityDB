-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2022, PostGIS contributors
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
-- Utility functions
-------------------------------------------------------------------------------

SELECT mobilitydb_version() LIKE 'MobilityDB%';
SELECT left(mobilitydb_full_version(), 10) = 'MobilityDB';

-------------------------------------------------------------------------------
-- Input/output functions
-------------------------------------------------------------------------------
-- Temporal instant

SELECT tbool 'TRUE@2012-01-01 08:00:00';
SELECT tbool 'FALSE@2012-01-01 08:00:00';
SELECT tint '1@2012-01-01 08:00:00';
SELECT tint '2@2012-01-01 08:00:00';
SELECT tfloat '1@2012-01-01 08:00:00';
SELECT tfloat '2@2012-01-01 08:00:00';
SELECT ttext 'AAA@2012-01-01 08:00:00';
SELECT ttext 'BBB@2012-01-01 08:00:00';
/* Errors */
SELECT tbool '2@2012-01-01 08:00:00';
SELECT tint 'TRUE@2012-01-01 08:00:00';
SELECT tfloat 'ABC@2012-01-01 08:00:00';
SELECT tfloat '25';
SELECT tfloat '2@2012-01-01 08:00:00,';

-------------------------------------------------------------------------------
-- Temporal discrete sequence

SELECT tbool ' { true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 } ';
SELECT tbool '{true@2001-01-01 08:00:00,false@2001-01-01 08:05:00,true@2001-01-01 08:06:00}';
SELECT tint ' { 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 } ';
SELECT tint '{1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00}';
SELECT tfloat ' { 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 } ';
SELECT tfloat '{1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00}';
SELECT ttext ' { A@2001-01-01 08:00:00 , B@2001-01-01 08:05:00 , C@2001-01-01 08:06:00 } ';
SELECT ttext '{A@2001-01-01 08:00:00,B@2001-01-01 08:05:00,C@2001-01-01 08:06:00}';
/* Errors */
SELECT tbool_discseq(tbool '{true@2000-01-01, true@2000-01-03, false@2000-01-02, false@2000-01-04}');
SELECT tint_discseq(tint '{1@2000-01-01, 2@2000-01-03, 1@2000-01-02, 2@2000-01-04}');
SELECT tfloat_discseq(tfloat '{1@2000-01-01, 2@2000-01-03, 1@2000-01-02, 2@2000-01-04}');
SELECT ttext_discseq(ttext '{AA@2000-01-01, BB@2000-01-03, AA@2000-01-02, BB@2000-01-04}');
SELECT tint '{1@2001-01-01, 2@2001-01-02, 3@2001-01-03';
SELECT tint '{1@2001-01-01, 2@2001-01-02, 3@2001-01-03},';

-------------------------------------------------------------------------------
-- Temporal sequence

SELECT tbool ' [ true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 ] ';
SELECT tbool '[true@2001-01-01 08:00:00,false@2001-01-01 08:05:00,true@2001-01-01 08:06:00]';
SELECT tint ' [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ] ';
SELECT tint '[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT tint 'Interp=Stepwise;[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT tfloat ' [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ] ';
SELECT tfloat '[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT tfloat 'Interp=Stepwise;[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT ttext ' [ A@2001-01-01 08:00:00 , B@2001-01-01 08:05:00 , C@2001-01-01 08:06:00 ] ';
SELECT ttext '[A@2001-01-01 08:00:00,B@2001-01-01 08:05:00,C@2001-01-01 08:06:00]';
/* Errors */
SELECT tbool '[true@2001-01-01 08:00:00)';
SELECT tbool '[true@2001-01-01 08:00:00, true@2001-01-01 08:00:00)';
SELECT tbool '[true@2001-01-01 08:00:00, false@2001-01-01 08:05:00, true@2001-01-01 08:06:00)';
SELECT tbool '[true@2001-01-01, true@2001-01-02';
SELECT tbool '[true@2001-01-01, true@2001-01-02],';

-------------------------------------------------------------------------------
-- Temporal sequence set

SELECT tbool '  { [ true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 ],
 [ true@2001-01-01 09:00:00 , false@2001-01-01 09:05:00 , true@2001-01-01 09:06:00 ] } ';
SELECT tbool '{[true@2001-01-01 08:00:00,false@2001-01-01 08:05:00,true@2001-01-01 08:06:00],
 [true@2001-01-01 09:00:00,false@2001-01-01 09:05:00,true@2001-01-01 09:06:00]}';

SELECT tint '  { [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ],
 [ 1@2001-01-01 09:00:00 , 2@2001-01-01 09:05:00 , 1@2001-01-01 09:06:00 ] } ';
SELECT tint '{[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00],
 [1@2001-01-01 09:00:00,2@2001-01-01 09:05:00,1@2001-01-01 09:06:00]}';
SELECT tint 'Interp=Stepwise;{[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00],
 [1@2001-01-01 09:00:00,2@2001-01-01 09:05:00,1@2001-01-01 09:06:00]}';

SELECT tfloat '  { [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ],
 [ 1@2001-01-01 09:00:00 , 2@2001-01-01 09:05:00 , 1@2001-01-01 09:06:00 ] } ';
SELECT tfloat '{[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00],
 [1@2001-01-01 09:00:00,2@2001-01-01 09:05:00,1@2001-01-01 09:06:00]}';
SELECT tfloat 'Interp=Stepwise;{[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00],
 [1@2001-01-01 09:00:00,2@2001-01-01 09:05:00,1@2001-01-01 09:06:00]}';

SELECT ttext '  { [ AAA@2001-01-01 08:00:00 , BBB@2001-01-01 08:05:00 , CCC@2001-01-01 08:06:00 ],
 [ AAA@2001-01-01 09:00:00 , BBB@2001-01-01 09:05:00 , CCC@2001-01-01 09:06:00 ] } ';
SELECT ttext '{[AAA@2001-01-01 08:00:00,BBB@2001-01-01 08:05:00,CCC@2001-01-01 08:06:00],
 [AAA@2001-01-01 09:00:00,BBB@2001-01-01 09:05:00,CCC@2001-01-01 09:06:00]}';

/* Errors */
SELECT tbool_seqset(tbool '{[true@2000-01-01, true@2000-01-03], [false@2000-01-02, false@2000-01-04]}');
SELECT tint_seqset(tint '{[1@2000-01-01, 1@2000-01-03], [2@2000-01-02, 2@2000-01-04]}');
SELECT tfloat_seqset(tfloat '{[1@2000-01-01, 2@2000-01-03], [2@2000-01-02, 1@2000-01-04]}');
SELECT ttext_seqset(ttext '{[AA@2000-01-01, AA@2000-01-03], [AA@2000-01-02, AA@2000-01-04]}');
SELECT tfloat_seqset(tfloat '{[1@2000-01-01, 2@2000-01-03], [2@2000-01-02, 1@2000-01-04]');
SELECT tfloat_seqset(tfloat '{[1@2000-01-01, 2@2000-01-03], [2@2000-01-02, 1@2000-01-04]},');

-------------------------------------------------------------------------------
-- typmod
-------------------------------------------------------------------------------

SELECT format_type(oid, -1) FROM (SELECT oid FROM pg_type WHERE typname = 'tfloat') t;
SELECT format_type(oid, temporal_typmod_in(ARRAY[cstring 'Instant']))
FROM (SELECT oid FROM pg_type WHERE typname = 'tfloat') t;
/* Errors */
SELECT temporal_typmod_in(ARRAY[[cstring 'Instant']]);
SELECT temporal_typmod_in(ARRAY[cstring 'Instant', NULL]);
SELECT tfloat('') '1@2000-01-01';
SELECT tfloat(Instant, Sequence) '1@2000-01-01';

SELECT tbool 'true@2000-01-01';
SELECT tbool '{true@2000-01-01, false@2000-01-02}';
SELECT tbool '[true@2000-01-01, false@2000-01-02]';
SELECT tbool '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(Instant) 'true@2000-01-01';
SELECT tbool(Sequence) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(Sequence) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(SequenceSet) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';

-- Coverage: Input of Boolean values
SELECT tbool ' t @2000-01-01';
SELECT tbool 'y@2000-01-01';
SELECT tbool 'n@2000-01-01';
SELECT tbool 'on@2000-01-01';
SELECT tbool 'off@2000-01-01';
SELECT tbool '1@2000-01-01';
SELECT tbool '0@2000-01-01';

/* Errors */
SELECT tbool(Instan) 'true@2000-01-01';
SELECT tbool(Sequence) 'true@2000-01-01';
SELECT tbool(Sequence) 'true@2000-01-01';
SELECT tbool(SequenceSet) 'true@2000-01-01';
SELECT tbool(Instant) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(SequenceSet) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(Instant) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(SequenceSet) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(Instant) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(Sequence) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(Sequence) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';

SELECT tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02}';
SELECT tint '[1@2000-01-01, 2@2000-01-02]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(Instant) '1@2000-01-01';
SELECT tint(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT tint(Sequence) '1@2000-01-01';
SELECT tint(Sequence) '1@2000-01-01';
SELECT tint(SequenceSet) '1@2000-01-01';
SELECT tint(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';

SELECT tfloat '1@2000-01-01';
SELECT tfloat '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(Instant) '1@2000-01-01';
SELECT tfloat(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT tfloat(Sequence) '1@2000-01-01';
SELECT tfloat(Sequence) '1@2000-01-01';
SELECT tfloat(SequenceSet) '1@2000-01-01';
SELECT tfloat(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';

SELECT ttext '1@2000-01-01';
SELECT ttext '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(Instant) '1@2000-01-01';
SELECT ttext(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT ttext(Sequence) '1@2000-01-01';
SELECT ttext(Sequence) '1@2000-01-01';
SELECT ttext(SequenceSet) '1@2000-01-01';
SELECT ttext(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';

-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

SELECT tbool_inst(TRUE, timestamptz '2012-01-01 08:00:00');
SELECT tint_inst(1, timestamptz '2012-01-01 08:00:00');
SELECT tfloat_inst(1, timestamptz '2012-01-01 08:00:00');
SELECT ttext_inst('AAA', timestamptz '2001-01-01 08:00:00');
-- NULL
SELECT tbool_inst(NULL, timestamptz '2012-01-01 08:00:00');
SELECT tint_inst(NULL, timestamptz '2012-01-01 08:00:00');
SELECT tfloat_inst(NULL, timestamptz '2012-01-01 08:00:00');

SELECT tbool_discseq(true, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT tint_discseq(1, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT tfloat_discseq(1.5, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT ttext_discseq('AAA', timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
-- NULL
SELECT tbool_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT tint_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT tfloat_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');
SELECT ttext_discseq(NULL, timestampset '{2012-01-01, 2012-01-02, 2012-01-03}');

SELECT tbool_seq(true, period '[2012-01-01, 2012-01-03]');
SELECT tint_seq(1, period '[2012-01-01, 2012-01-03]');
SELECT tfloat_seq(1.5, period '[2012-01-01, 2012-01-01]');
SELECT tfloat_seq(1.5, period '[2012-01-01, 2012-01-03]');
SELECT tfloat_seq(1.5, period '[2012-01-01, 2012-01-03]', false);
SELECT ttext_seq('AAA', period '[2012-01-01, 2012-01-03]');
-- NULL
SELECT tbool_seq(NULL, period '[2012-01-01, 2012-01-03]');
SELECT tint_seq(NULL, period '[2012-01-01, 2012-01-03]');
SELECT tfloat_seq(NULL, period '[2012-01-01, 2012-01-03]');
SELECT ttext_seq(NULL, period '[2012-01-01, 2012-01-03]');

SELECT tbool_seqset(true, periodset '{[2012-01-01, 2012-01-03]}');
SELECT tint_seqset(1, periodset '{[2012-01-01, 2012-01-03]}');
SELECT tfloat_seqset(1.5, periodset '{[2012-01-01, 2012-01-03]}');
SELECT tfloat_seqset(1.5, periodset '{[2012-01-01, 2012-01-03]}', false);
SELECT ttext_seqset('AAA', periodset '{[2012-01-01, 2012-01-03]}');
-- NULL
SELECT tbool_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}');
SELECT tint_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}');
SELECT tfloat_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}');
SELECT ttext_seqset(NULL, periodset '{[2012-01-01, 2012-01-03]}');

-------------------------------------------------------------------------------

SELECT tbool_discseq(ARRAY[
tbool_inst(true, timestamptz '2012-01-01 08:00:00'),
tbool_inst(true, timestamptz '2012-01-01 08:10:00'),
tbool_inst(true, timestamptz '2012-01-01 08:20:00')
]);
SELECT tint_discseq(ARRAY[
tint_inst(1, timestamptz '2012-01-01 08:00:00'),
tint_inst(2, timestamptz '2012-01-01 08:10:00'),
tint_inst(3, timestamptz '2012-01-01 08:20:00')
]);
SELECT tfloat_discseq(ARRAY[
tfloat_inst(1, timestamptz '2012-01-01 08:00:00'),
tfloat_inst(2, timestamptz '2012-01-01 08:10:00'),
tfloat_inst(3, timestamptz '2012-01-01 08:20:00')
]);
SELECT ttext_discseq(ARRAY[
ttext_inst('A', timestamptz '2012-01-01 08:00:00'),
ttext_inst('B', timestamptz '2012-01-01 08:10:00'),
ttext_inst('C', timestamptz '2012-01-01 08:20:00')
]);

/* Errors */
SELECT tbool_discseq('{}'::tbool[]);
SELECT tint_discseq('{}'::tint[]);
SELECT tfloat_discseq('{}'::tfloat[]);
SELECT ttext_discseq('{}'::ttext[]);
SELECT tbool_discseq(ARRAY[tbool '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tint_discseq(ARRAY[tint '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tfloat_discseq(ARRAY[tfloat '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT ttext_discseq(ARRAY[ttext 'AA@2000-01-01', '[BB@2000-01-02,BB@2000-01-03]']);

-------------------------------------------------------------------------------

SELECT tbool_seq(ARRAY[
tbool_inst(true, timestamptz '2012-01-01 08:00:00'),
tbool_inst(true, timestamptz '2012-01-01 08:10:00'),
tbool_inst(true, timestamptz '2012-01-01 08:20:00')
]);
SELECT tint_seq(ARRAY[
tint_inst(1, timestamptz '2012-01-01 08:00:00'),
tint_inst(2, timestamptz '2012-01-01 08:10:00'),
tint_inst(3, timestamptz '2012-01-01 08:20:00')
]);
SELECT tfloat_seq(ARRAY[
tfloat_inst(1, timestamptz '2012-01-01 08:00:00'),
tfloat_inst(2, timestamptz '2012-01-01 08:10:00'),
tfloat_inst(3, timestamptz '2012-01-01 08:20:00')
]);
SELECT ttext_seq(ARRAY[
ttext_inst('A', timestamptz '2012-01-01 08:00:00'),
ttext_inst('B', timestamptz '2012-01-01 08:10:00'),
ttext_inst('C', timestamptz '2012-01-01 08:20:00')
]);

SELECT tbool_seq(ARRAY[tbool 'true@2000-01-01', 'false@2000-01-02', 'false@2000-01-03'], false, true);
SELECT tint_seq(ARRAY[tint '1@2000-01-01', '1@2000-01-02', '3@2000-01-03'], false, true);
SELECT tfloat_seq(ARRAY[tfloat '1@2000-01-01', '2@2000-01-02', '3@2000-01-03'], false, true, true);
SELECT tfloat_seq(ARRAY[tfloat '1@2000-01-01', '2@2000-01-02', '3@2000-01-03'], false, true, false);
SELECT ttext_seq(ARRAY[ttext 'AA@2000-01-01', 'AA@2000-01-02', 'BB@2000-01-03'], false, true);
/* Errors */
SELECT tbool_seq('{}'::tbool[]);
SELECT tint_seq('{}'::tint[]);
SELECT tfloat_seq('{}'::tfloat[]);
SELECT ttext_seq('{}'::ttext[]);
SELECT tbool_seq(ARRAY[tbool '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tint_seq(ARRAY[tint '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tfloat_seq(ARRAY[tfloat '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT ttext_seq(ARRAY[ttext 'AA@2000-01-01', '[BB@2000-01-02,BB@2000-01-03]']);

-------------------------------------------------------------------------------

SELECT tbool_seqset(ARRAY[
tbool_seq(ARRAY[
tbool_inst(true,  timestamptz '2012-01-01 08:00:00'),
tbool_inst(false, timestamptz '2012-01-01 08:10:00'),
tbool_inst(true,  timestamptz '2012-01-01 08:20:00')
]),
tbool_seq(ARRAY[
tbool_inst(true,  timestamptz '2012-01-01 09:00:00'),
tbool_inst(false, timestamptz  '2012-01-01 09:10:00'),
tbool_inst(true,  timestamptz '2012-01-01 09:20:00')
])]);
SELECT tint_seqset(ARRAY[
tint_seq(ARRAY[
tint_inst(1, timestamptz '2012-01-01 08:00:00'),
tint_inst(2, timestamptz '2012-01-01 08:10:00'),
tint_inst(3, timestamptz '2012-01-01 08:20:00')
]),
tint_seq(ARRAY[
tint_inst(1, timestamptz '2012-01-01 09:00:00'),
tint_inst(2, timestamptz '2012-01-01 09:10:00'),
tint_inst(1, timestamptz '2012-01-01 09:20:00')
])]);
SELECT tfloat_seqset(ARRAY[
tfloat_seq(ARRAY[
tfloat_inst(1, timestamptz '2012-01-01 08:00:00'),
tfloat_inst(2, timestamptz '2012-01-01 08:10:00'),
tfloat_inst(3, timestamptz '2012-01-01 08:20:00')
]),
tfloat_seq(ARRAY[
tfloat_inst(1, timestamptz '2012-01-01 09:00:00'),
tfloat_inst(2, timestamptz '2012-01-01 09:10:00'),
tfloat_inst(1, timestamptz '2012-01-01 09:20:00')
])]);
SELECT ttext_seqset(ARRAY[
ttext_seq(ARRAY[
ttext_inst('A', timestamptz '2012-01-01 08:00:00'),
ttext_inst('B', timestamptz '2012-01-01 08:10:00'),
ttext_inst('C', timestamptz '2012-01-01 08:20:00')
]),
ttext_seq(ARRAY[
ttext_inst('A', timestamptz '2012-01-01 09:00:00'),
ttext_inst('B', timestamptz '2012-01-01 09:10:00'),
ttext_inst('C', timestamptz '2012-01-01 09:20:00')
])]);

SELECT tbool_seqset(ARRAY[tbool '[true@2000-01-01, true@2000-01-02]', '[false@2000-01-03, false@2000-01-04]']);
SELECT tint_seqset(ARRAY[tint '[1@2000-01-01, 1@2000-01-02]', '[2@2000-01-03, 2@2000-01-04]']);
SELECT tfloat_seqset(ARRAY[tfloat '[1@2000-01-01, 2@2000-01-02]', '[2@2000-01-03, 1@2000-01-04]']);
SELECT tfloat_seqset(ARRAY[tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]', 'Interp=Stepwise;[2@2000-01-03, 1@2000-01-04]']);
SELECT ttext_seqset(ARRAY[ttext '[AA@2000-01-01, AA@2000-01-02]', '[AA@2000-01-03, AA@2000-01-04]']);
/* Errors */
SELECT tbool_seqset('{}'::tbool[]);
SELECT tint_seqset('{}'::tint[]);
SELECT tfloat_seqset('{}'::tfloat[]);
SELECT ttext_seqset('{}'::ttext[]);
SELECT tbool_seqset(ARRAY[tbool '[true@2000-01-01, true@2000-01-03]', '[false@2000-01-02, false@2000-01-04]']);
SELECT tint_seqset(ARRAY[tint '[1@2000-01-01, 1@2000-01-03]', '[2@2000-01-02, 2@2000-01-04]']);
SELECT tfloat_seqset(ARRAY[tfloat '[1@2000-01-01, 2@2000-01-03]', '[2@2000-01-02, 1@2000-01-04]']);
SELECT ttext_seqset(ARRAY[ttext '[AA@2000-01-01, AA@2000-01-03]', '[AA@2000-01-02, AA@2000-01-04]']);
SELECT tbool_seqset(ARRAY[tbool '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tint_seqset(ARRAY[tint '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tfloat_seqset(ARRAY[tfloat '1@2000-01-01', '[1@2000-01-02,1@2000-01-03]']);
SELECT tfloat_seqset(ARRAY[tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]', '[2@2000-01-03, 1@2000-01-04]']);
SELECT ttext_seqset(ARRAY[ttext 'AA@2000-01-01', '[BB@2000-01-02,BB@2000-01-03]']);

-------------------------------------------------------------------------------

SELECT tint_seqset_gaps(ARRAY[tint '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05']);
SELECT tint_seqset_gaps(ARRAY[tint '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], 1, '0 min');
SELECT tint_seqset_gaps(ARRAY[tint '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], 0, '1 day');
SELECT tint_seqset_gaps(ARRAY[tint '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], 1, '1 day');

SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05']);
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], true);
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], false);
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], true, 1.0);
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], false, 1.0);
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], true, 0, '1 day');
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], false, 0, '1 day');
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], true, 1, '1 day');
SELECT tfloat_seqset_gaps(ARRAY[tfloat '1@2000-01-01', '3@2000-01-02', '4@2000-01-03', '5@2000-01-05'], false, 1, '1 day');

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01'::intspan;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}'::intspan;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]'::intspan;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}'::intspan;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}'::intspan;
SELECT tfloat '1.5@2000-01-01'::floatspan;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}'::floatspan;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]'::floatspan;
SELECT tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]'::floatspan;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}'::floatspan;
SELECT tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}'::floatspan;

SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}'::floatspan;
SELECT tfloat '{[1@2000-01-01, 2@2000-01-02],[2@2000-01-03, 3@2000-01-04]}'::floatspan;

SELECT tfloat(tint '1@2001-01-01');
SELECT tfloat(tint '{1@2001-01-01, 2@2001-01-02}');
SELECT tfloat(tint '[1@2001-01-01, 1@2001-01-02]');
SELECT tfloat(tint '[1@2001-01-01, 2@2001-01-02, 2@2001-01-03]');
SELECT tfloat(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT tfloat(tint '{[1@2001-01-01, 1@2001-01-02], [2@2001-01-03, 2@2001-01-04]}');

SELECT tint(tfloat '1.5@2001-01-01');
SELECT tint(tfloat '{1.5@2001-01-01, 2.5@2001-01-02}');
SELECT tint(tfloat 'Interp=Stepwise;[1.5@2001-01-01, 2.5@2001-01-02, 2.5@2001-01-03]');
SELECT tint(tfloat 'Interp=Stepwise;{[1.5@2001-01-01, 1.5@2001-01-02], [2.5@2001-01-03, 2.5@2001-01-04]}');
/* Errors */
SELECT tint(tfloat '[1@2001-01-01, 2@2001-01-02, 2@2001-01-03]');
SELECT tint(tfloat '{[1@2001-01-01, 1@2001-01-02], [2@2001-01-03, 2@2001-01-04]}');

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT tbool_inst(tbool 't@2000-01-01');
SELECT tbool_discseq(tbool 't@2000-01-01');
SELECT tbool_discseq(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_seq(tbool 't@2000-01-01');
SELECT tbool_seq(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_seq(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_seqset(tbool 't@2000-01-01');
SELECT tbool_seqset(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_seqset(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_seqset(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
/* Errors */
SELECT tbool_inst(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_inst(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_inst(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbool_discseq(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_discseq(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbool_seq(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');

SELECT tint_inst(tint '1@2000-01-01');
SELECT tint_discseq(tint '1@2000-01-01');
SELECT tint_discseq(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tint_seq(tint '1@2000-01-01');
SELECT tint_seq(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tint_seq(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tint_seqset(tint '1@2000-01-01');
SELECT tint_seqset(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tint_seqset(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tint_seqset(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
/* Errors */
SELECT tint_inst(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tint_inst(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tint_inst(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tint_discseq(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tint_discseq(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tint_seq(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT tfloat_inst(tfloat '1.5@2000-01-01');
SELECT tfloat_inst(tfloat '{1.5@2000-01-01}');
SELECT tfloat_inst(tfloat '[1.5@2000-01-01]');
SELECT tfloat_inst(tfloat '{[1.5@2000-01-01]}');
SELECT tfloat_discseq(tfloat '1.5@2000-01-01');
SELECT tfloat_discseq(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloat_discseq(tfloat '[1.5@2000-01-01]');
SELECT tfloat_discseq(tfloat '{[1.5@2000-01-01], [2.5@2000-01-02], [1.5@2000-01-03]}');
SELECT tfloat_seq(tfloat '1.5@2000-01-01');
SELECT tfloat_seq(tfloat '{1.5@2000-01-01}');
SELECT tfloat_seq(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloat_seq(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_seq(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}');
SELECT tfloat_seq(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}');
SELECT tfloat_seqset(tfloat '1.5@2000-01-01');
SELECT tfloat_seqset(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloat_seqset(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_seqset(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_seqset(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
/* Errors */
SELECT tfloat_inst(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloat_inst(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_inst(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tfloat_discseq(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_discseq(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tfloat_seq(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT ttext_inst(ttext 'AAA@2000-01-01');
SELECT ttext_discseq(ttext 'AAA@2000-01-01');
SELECT ttext_discseq(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttext_seq(ttext 'AAA@2000-01-01');
SELECT ttext_seq(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttext_seq(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttext_seqset(ttext 'AAA@2000-01-01');
SELECT ttext_seqset(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttext_seqset(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttext_seqset(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT ttext_inst(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttext_inst(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttext_inst(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT ttext_discseq(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttext_discseq(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT ttext_seq(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

-------------------------------------------------------------------------------

SELECT toLinear(tfloat '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04]');
SELECT toLinear(tfloat 'Interp=Stepwise;[1@2000-01-01]');
SELECT toLinear(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04]');
SELECT toLinear(tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04], [3@2000-01-05, 4@2000-01-06]}');
SELECT toLinear(tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04]}');

-------------------------------------------------------------------------------

SELECT appendInstant(tbool 't@2000-01-01', tbool 't@2000-01-02');
SELECT appendInstant(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool 't@2000-01-04');
SELECT appendInstant(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool 't@2000-01-04');
SELECT appendInstant(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool 't@2000-01-06');
SELECT appendInstant(tint '1@2000-01-01', tint '1@2000-01-02');
SELECT appendInstant(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '1@2000-01-04');
SELECT appendInstant(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '1@2000-01-04');
SELECT appendInstant(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tint '1@2000-01-06');
SELECT appendInstant(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-02');
SELECT appendInstant(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '1.5@2000-01-04');
SELECT appendInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-04');
SELECT appendInstant(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '1.5@2000-01-04');
SELECT appendInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-06');
SELECT appendInstant(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-06');
SELECT appendInstant(ttext 'AAA@2000-01-01', ttext 'AAA@2000-01-02');
SELECT appendInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ttext 'AAA@2000-01-04');
SELECT appendInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ttext 'AAA@2000-01-04');
SELECT appendInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ttext 'AAA@2000-01-06');

SELECT appendInstant(tbool 't@2000-01-01', tbool 't@2000-01-01');
SELECT appendInstant(tfloat '{1@2000-01-01, 2@2000-01-02}', tfloat '2@2000-01-02');
SELECT appendInstant(tfloat '[1@2000-01-01, 1@2000-01-02]', '1@2000-01-02');
SELECT appendInstant(tfloat '[1@2000-01-01, 1@2000-01-02)', '1@2000-01-02');
SELECT appendInstant(tfloat '[1@2000-01-01, 1@2000-01-02)', '2@2000-01-02');
SELECT appendInstant(tfloat 'Interp=Stepwise;[1@2000-01-01, 1@2000-01-02)', '2@2000-01-02');
SELECT appendInstant(tfloat '{[1@2000-01-01, 1@2000-01-02]}', '1@2000-01-02');
SELECT appendInstant(tfloat '{[1@2000-01-01, 1@2000-01-02)}', '1@2000-01-02');
SELECT appendInstant(tfloat '{[1@2000-01-01, 1@2000-01-02)}', '2@2000-01-02');
SELECT appendInstant(tfloat 'Interp=Stepwise;{[1@2000-01-01, 1@2000-01-02)}', '2@2000-01-02');

/* Errors */
SELECT appendInstant(tfloat '{1@2000-01-01, 2@2000-01-02}', tfloat '2@2000-01-01');
SELECT appendInstant(tfloat '[1@2000-01-01, 1@2000-01-02]', '2@2000-01-02');
SELECT appendInstant(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-04, 1@2000-01-05]');
SELECT appendInstant(tfloat '{[1@2000-01-01, 1@2000-01-02]}', '2@2000-01-02');
SELECT appendInstant(tfloat '[1@2000-01-01, 2@2000-01-03]', '1@2000-01-02');

-------------------------------------------------------------------------------

SELECT merge(tbool 't@2000-01-01', tbool 't@2000-01-02');
SELECT merge(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool '{t@2000-01-03, f@2000-01-04, t@2000-01-05}');
SELECT merge(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool '{f@2000-01-04, t@2000-01-05}');
SELECT merge(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool '[t@2000-01-03, f@2000-01-04, t@2000-01-05]');
SELECT merge(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool '[f@2000-01-04, t@2000-01-05]');
SELECT merge(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool '{[t@2000-01-05, f@2000-01-06, t@2000-01-07],[t@2000-01-08, t@2000-01-09]}');
SELECT merge(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool '{[f@2000-01-06, t@2000-01-07],[t@2000-01-08, t@2000-01-09]}');

SELECT merge(tint '1@2000-01-01', tint '1@2000-01-02');
SELECT merge(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{1@2000-01-03, 2@2000-01-04, 1@2000-01-05}');
SELECT merge(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{2@2000-01-04, 1@2000-01-05}');
SELECT merge(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[1@2000-01-03, 2@2000-01-04, 1@2000-01-05]');
SELECT merge(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[2@2000-01-04, 1@2000-01-05]');
SELECT merge(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', tint '{[1@2000-01-05, 2@2000-01-06, 1@2000-01-07],[1@2000-01-08, 1@2000-01-09]}');
SELECT merge(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', tint '{[2@2000-01-06, 1@2000-01-07],[1@2000-01-08, 1@2000-01-09]}');

SELECT merge(tfloat '1.5@2000-01-01', tfloat '1.5@2000-01-02');
SELECT merge(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{1.5@2000-01-03, 2.5@2000-01-04, 1.5@2000-01-05}');
SELECT merge(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tfloat '{2.5@2000-01-04, 1.5@2000-01-05}');
SELECT merge(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[1.5@2000-01-03, 2.5@2000-01-04, 1.5@2000-01-05]');
SELECT merge(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tfloat '[2.5@2000-01-04, 1.5@2000-01-05]');
SELECT merge(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[1.5@2000-01-04, 1.5@2000-01-05]}', tfloat '{[1.5@2000-01-05, 2.5@2000-01-06, 1.5@2000-01-07],[1.5@2000-01-08, 1.5@2000-01-09]}');
SELECT merge(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[1.5@2000-01-04, 1.5@2000-01-05]}', tfloat '{[2.5@2000-01-06, 1.5@2000-01-07],[1.5@2000-01-08, 1.5@2000-01-09]}');

SELECT merge(ttext 'AAA@2000-01-01', ttext 'AAA@2000-01-02');
SELECT merge(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ttext '{AAA@2000-01-03, BBB@2000-01-04, AAA@2000-01-05}');
SELECT merge(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ttext '{BBB@2000-01-04, AAA@2000-01-05}');
SELECT merge(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ttext '[AAA@2000-01-03, BBB@2000-01-04, AAA@2000-01-05]');
SELECT merge(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ttext '[BBB@2000-01-04, AAA@2000-01-05]');
SELECT merge(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[AAA@2000-01-04, AAA@2000-01-05]}', ttext '{[AAA@2000-01-05, BBB@2000-01-06, AAA@2000-01-07],[AAA@2000-01-08, AAA@2000-01-09]}');
SELECT merge(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[AAA@2000-01-04, AAA@2000-01-05]}', ttext '{[BBB@2000-01-06, AAA@2000-01-07],[AAA@2000-01-08, AAA@2000-01-09]}');

SELECT merge(tint '1@2000-01-01', tint '{1@2000-01-03, 2@2000-01-04}');
SELECT merge(tint '{1@2000-01-03, 2@2000-01-04}', tint '1@2000-01-01');
SELECT merge(tint '1@2000-01-01', tint '1@2000-01-01');
SELECT merge(tint '1@2000-01-01', tint '1@2000-01-02');
SELECT merge(tint '1@2000-01-01', tint '2@2000-01-02');
SELECT merge(tint '2@2000-01-02', tint '1@2000-01-01');

SELECT merge(ARRAY[tint '1@2000-01-01', '[2@2000-01-02, 3@2000-01-03]']);
SELECT merge(ARRAY[tint '1@2000-01-01', '{[2@2000-01-02, 3@2000-01-03]}']);
SELECT merge(ARRAY[tint '{1@2000-01-01}', '[2@2000-01-02, 3@2000-01-03]']);
SELECT merge(ARRAY[tint '{1@2000-01-01}', '{[2@2000-01-02, 3@2000-01-03]}']);
SELECT merge(ARRAY[tint '[1@2000-01-01]', '{[2@2000-01-02, 3@2000-01-03]}']);
SELECT merge(ARRAY[tint '[1@2000-01-01]', '[1@2000-01-01, 1@2000-01-02, 2@2000-01-03]', '[2@2000-01-04]']);

SELECT merge(tint '{1@2000-01-03, 2@2000-01-04}', tint '1@2000-01-01');
SELECT merge(tint '[1@2000-01-03, 2@2000-01-04]', tint '1@2000-01-01');
SELECT merge(tint '{[2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}', tint '1@2000-01-01');
SELECT merge(tint '[1@2000-01-03, 2@2000-01-04]', tint '{1@2000-01-01}');
SELECT merge(tint '[1@2000-01-03, 2@2000-01-04]', tint '{1@2000-01-01, 2@2000-01-02}');
SELECT merge(tint '{[2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}', tint '{1@2000-01-01}');
SELECT merge(tint '{[2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}', tint '{1@2000-01-01, 2@2000-01-02}');
SELECT merge(tint '{[2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}', tint '[1@2000-01-01, 2@2000-01-02]');

/* Errors */
SELECT merge(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{2@2000-01-03, 2@2000-01-04, 1@2000-01-05}');
SELECT merge(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[2@2000-01-03, 2@2000-01-04, 1@2000-01-05]');
SELECT merge(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', tint '{[2@2000-01-05, 2@2000-01-06, 1@2000-01-07],[1@2000-01-08, 1@2000-01-09]}');
SELECT merge(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tint '{2@2000-01-02, 2@2000-01-03, 1@2000-01-04}');
SELECT merge(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tint '[2@2000-01-02, 2@2000-01-03, 1@2000-01-04]');
SELECT merge(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', tint '{[2@2000-01-04, 2@2000-01-05, 1@2000-01-06],[1@2000-01-08, 1@2000-01-09]}');

-------------------------------------------------------------------------------

SELECT merge(ARRAY[tint '1@2000-01-01', '1@2000-01-02']);
SELECT merge(ARRAY[tint '{1@2000-01-01, 2@2000-01-02}', '{2@2000-01-02, 1@2000-01-03}']);
SELECT merge(ARRAY[tint '{1@2000-01-01, 2@2000-01-02}', '{3@2000-01-03, 4@2000-01-04}']);
SELECT merge(ARRAY[tint '[1@2000-01-01, 2@2000-01-02]', '[2@2000-01-02, 1@2000-01-03]']);
SELECT merge(ARRAY[tint '[1@2000-01-01, 2@2000-01-02]', '[3@2000-01-03, 4@2000-01-04]']);
SELECT merge(ARRAY[tint '[2@2000-01-01, 1@2000-01-02]', '[1@2000-01-02, 1@2000-01-03)']);
SELECT merge(ARRAY[tint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}', '{[4@2000-01-04, 5@2000-01-05], [6@2000-01-06, 7@2000-01-07]}']);
SELECT merge(ARRAY[tint '{[1@2000-01-01, 2@2000-01-02]}', '{[2@2000-01-02, 1@2000-01-03]}']);
SELECT merge(ARRAY[tint '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}', '{[5@2000-01-05, 6@2000-01-06], [7@2000-01-07, 8@2000-01-08], [9@2000-01-09, 8@2000-01-10]}']);

SELECT merge(ARRAY[tint '1@2000-01-01', '1@2000-01-01']);
SELECT merge(ARRAY[tint '1@2000-01-01', '{1@2000-01-03, 2@2000-01-04, 1@2000-01-05}']);

/* Errors */
SELECT merge(ARRAY[]::tint[]);
SELECT merge(ARRAY[tfloat '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 'Interp=Stepwise;[2@2000-01-02, 2@2000-01-03, 1@2000-01-04]']);
SELECT merge(ARRAY[tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '{2@2000-01-03, 2@2000-01-04, 1@2000-01-05}']);
SELECT merge(ARRAY[tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '[2@2000-01-03, 2@2000-01-04, 1@2000-01-05]']);
SELECT merge(ARRAY[tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', '{[2@2000-01-05, 2@2000-01-06, 1@2000-01-07],[1@2000-01-08, 1@2000-01-09]}']);
SELECT merge(ARRAY[tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '{2@2000-01-02, 2@2000-01-03, 1@2000-01-04}']);
SELECT merge(ARRAY[tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '[2@2000-01-02, 2@2000-01-03, 1@2000-01-04]']);
SELECT merge(ARRAY[tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', '{[2@2000-01-04, 2@2000-01-05, 1@2000-01-06],[1@2000-01-08, 1@2000-01-09]}']);

-------------------------------------------------------------------------------

SELECT merge(tfloat '1@2000-01-01', tfloat '1@2000-01-01');

SELECT merge(tfloat '{1@2000-01-01, 2@2000-01-02}', tfloat '{1@2000-01-01, 2@2000-01-02}');
SELECT merge(tfloat '{1@2000-01-01, 3@2000-01-03}', tfloat '{2@2000-01-02, 4@2000-01-04}');
SELECT merge(tfloat '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}', tfloat '{2@2000-01-02, 3@2000-01-03, 4@2000-01-04}');

SELECT merge(tfloat '[1@2000-01-01, 2@2000-01-02]', tfloat '[2@2000-01-02, 4@2000-01-04]');
SELECT merge(tfloat '[2@2000-01-02, 4@2000-01-04]', tfloat '[1@2000-01-01, 2@2000-01-02]');

SELECT merge(tfloat '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}', tfloat '{[2@2000-01-02, 3@2000-01-03], [4@2000-01-04, 5@2000-01-05]}');

-- NULL
SELECT merge(NULL::tfloat, NULL::tfloat);
SELECT merge(tfloat '1@2000-01-01', NULL);
SELECT merge(NULL, tfloat '1@2000-01-01');/* Errors */
SELECT merge(tfloat '1@2000-01-01', tfloat '2@2000-01-01');
SELECT merge(tfloat '{1@2000-01-01, 2@2000-01-02, 3@2000-01-03}', tfloat '{3@2000-01-02, 3@2000-01-03, 4@2000-01-04}');
SELECT merge(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02, 3@2000-01-03]', tfloat 'Interp=Stepwise;[3@2000-01-02, 3@2000-01-03, 4@2000-01-04]');
SELECT merge(tfloat '[1@2000-01-01, 2@2000-01-02]', tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]');
SELECT merge(tfloat '(1@2000-01-01, 2@2000-01-02]', tfloat '[1@2000-01-01, 2@2000-01-02]');
SELECT merge(tfloat '[1@2000-01-01, 2@2000-01-02)', tfloat '[1@2000-01-01, 2@2000-01-02]');
SELECT merge(tfloat '[1@2000-01-01, 3@2000-01-03]', tfloat '[2@2000-01-02, 4@2000-01-04]');
SELECT merge(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]', tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]');
SELECT merge(tfloat '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}', tfloat '{[1@2000-01-01, 2@2000-01-02], [3@2000-01-03, 4@2000-01-04]}');

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT tempSubtype(tbool 't@2000-01-01');
SELECT tempSubtype(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tempSubtype(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tempSubtype(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tempSubtype(tint '1@2000-01-01');
SELECT tempSubtype(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tempSubtype(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tempSubtype(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tempSubtype(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tempSubtype(tfloat '1.5@2000-01-01');
SELECT tempSubtype(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tempSubtype(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tempSubtype(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tempSubtype(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tempSubtype(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tempSubtype(ttext 'AAA@2000-01-01');
SELECT tempSubtype(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT tempSubtype(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT tempSubtype(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT interpolation(tbool 't@2000-01-01');
SELECT interpolation(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT interpolation(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT interpolation(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT interpolation(tint '1@2000-01-01');
SELECT interpolation(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT interpolation(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT interpolation(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT interpolation(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT interpolation(tfloat '1.5@2000-01-01');
SELECT interpolation(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT interpolation(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT interpolation(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT interpolation(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT interpolation(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT interpolation(ttext 'AAA@2000-01-01');
SELECT interpolation(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT interpolation(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT interpolation(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT memSize(tbool 't@2000-01-01');
SELECT memSize(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT memSize(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT memSize(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT memSize(tint '1@2000-01-01');
SELECT memSize(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT memSize(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT memSize(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT memSize(tfloat '1.5@2000-01-01');
SELECT memSize(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT memSize(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT memSize(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT memSize(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT memSize(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT memSize(ttext 'AAA@2000-01-01');
SELECT memSize(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT memSize(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT memSize(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

/*
SELECT tbox(tint '1@2000-01-01');
SELECT tbox(tfloat '1.5@2000-01-01');
*/

SELECT getValue(tbool 't@2000-01-01');
SELECT getValue(tint '1@2000-01-01');
SELECT getValue(tfloat '1.5@2000-01-01');
SELECT getValue(ttext 'AAA@2000-01-01');
/* Errors */
SELECT getValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT getValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT getValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT getValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT getValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT getValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT getValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT getValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT getValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT getValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT getValues(tbool 't@2000-01-01');
SELECT getValues(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT getValues(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT getValues(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT getValues(tint '1@2000-01-01');
SELECT getValues(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT getValues(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT getValues(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT getValues(tfloat '1.5@2000-01-01');
SELECT getValues(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT getValues(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getValues(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getValues(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getValues(ttext 'AAA@2000-01-01');
SELECT getValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT getValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT getValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT getValues(tfloat '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04)');

SELECT startValue(tbool 't@2000-01-01');
SELECT startValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT startValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT startValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT startValue(tint '1@2000-01-01');
SELECT startValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT startValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT startValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT startValue(tfloat '1.5@2000-01-01');
SELECT startValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT startValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startValue(ttext 'AAA@2000-01-01');
SELECT startValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT startValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT startValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT endValue(tbool 't@2000-01-01');
SELECT endValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT endValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT endValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT endValue(tint '1@2000-01-01');
SELECT endValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT endValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT endValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT endValue(tfloat '1.5@2000-01-01');
SELECT endValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT endValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endValue(ttext 'AAA@2000-01-01');
SELECT endValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT endValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT endValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT minValue(tint '1@2000-01-01');
SELECT minValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minValue(tfloat '1.5@2000-01-01');
SELECT minValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minValue(ttext 'AAA@2000-01-01');
SELECT minValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT maxValue(tint '1@2000-01-01');
SELECT maxValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT maxValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxValue(tfloat '1.5@2000-01-01');
SELECT maxValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT maxValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxValue(ttext 'AAA@2000-01-01');
SELECT maxValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT maxValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT maxValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT minInstant(tint '1@2000-01-01');
SELECT minInstant(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minInstant(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minInstant(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minInstant(tfloat '1.5@2000-01-01');
SELECT minInstant(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minInstant(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minInstant(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minInstant(ttext 'AAA@2000-01-01');
SELECT minInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT maxInstant(tint '1@2000-01-01');
SELECT maxInstant(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT maxInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxInstant(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxInstant(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxInstant(tfloat '1.5@2000-01-01');
SELECT maxInstant(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT maxInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxInstant(ttext 'AAA@2000-01-01');
SELECT maxInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT maxInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT maxInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT getTimestamp(tbool 't@2000-01-01');
SELECT getTimestamp(tint '1@2000-01-01');
SELECT getTimestamp(tfloat '1.5@2000-01-01');
SELECT getTimestamp(ttext 'AAA@2000-01-01');
/* Errors */
SELECT getTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT getTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT getTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT getTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT getTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT getTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT getTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT getTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT getTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT getTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT getTime(tbool 't@2000-01-01');
SELECT getTime(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT getTime(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT getTime(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT getTime(tint '1@2000-01-01');
SELECT getTime(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT getTime(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT getTime(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT getTime(tfloat '1.5@2000-01-01');
SELECT getTime(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT getTime(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getTime(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT getTime(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getTime(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getTime(ttext 'AAA@2000-01-01');
SELECT getTime(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT getTime(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT getTime(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT timespan(tbool 't@2000-01-01');
SELECT timespan(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT timespan(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT timespan(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT timespan(tint '1@2000-01-01');
SELECT timespan(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT timespan(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT timespan(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT timespan(tfloat '1.5@2000-01-01');
SELECT timespan(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT timespan(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT timespan(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT timespan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timespan(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timespan(ttext 'AAA@2000-01-01');
SELECT timespan(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT timespan(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT timespan(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT duration(tbool 't@2000-01-01');
SELECT duration(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT duration(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT duration(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT duration(tint '1@2000-01-01');
SELECT duration(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT duration(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT duration(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT duration(tfloat '1.5@2000-01-01');
SELECT duration(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT duration(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT duration(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT duration(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT duration(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT duration(ttext 'AAA@2000-01-01');
SELECT duration(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT duration(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT duration(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT period(tbool 't@2000-01-01');
SELECT period(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT period(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT period(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT period(tint '1@2000-01-01');
SELECT period(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT period(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT period(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT period(tfloat '1.5@2000-01-01');
SELECT period(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT period(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT period(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT period(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT period(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT period(ttext 'AAA@2000-01-01');
SELECT period(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT period(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT period(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numSequences(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT numSequences(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT numSequences(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT numSequences(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT numSequences(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numSequences(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numSequences(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numSequences(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numSequences(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT numSequences(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
/* Errors */
SELECT numSequences(tbool 't@2000-01-01');
SELECT numSequences(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT numSequences(tint '1@2000-01-01');
SELECT numSequences(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT numSequences(tfloat '1.5@2000-01-01');
SELECT numSequences(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT numSequences(ttext 'AAA@2000-01-01');
SELECT numSequences(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');

SELECT startSequence(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT startSequence(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT startSequence(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT startSequence(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT startSequence(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startSequence(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startSequence(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startSequence(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startSequence(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT startSequence(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
/* Errors */
SELECT startSequence(tbool 't@2000-01-01');
SELECT startSequence(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT startSequence(tint '1@2000-01-01');
SELECT startSequence(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT startSequence(tfloat '1.5@2000-01-01');
SELECT startSequence(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT startSequence(ttext 'AAA@2000-01-01');
SELECT startSequence(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');

SELECT endSequence(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT endSequence(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT endSequence(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT endSequence(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT endSequence(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endSequence(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endSequence(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endSequence(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endSequence(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT endSequence(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT endSequence(tbool 't@2000-01-01');
SELECT endSequence(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT endSequence(tint '1@2000-01-01');
SELECT endSequence(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT endSequence(tfloat '1.5@2000-01-01');
SELECT endSequence(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT endSequence(ttext 'AAA@2000-01-01');
SELECT endSequence(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');

SELECT sequenceN(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', 1);
SELECT sequenceN(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', 1);
SELECT sequenceN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT sequenceN(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT sequenceN(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT sequenceN(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT sequenceN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT sequenceN(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT sequenceN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT sequenceN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);
-- NULL
SELECT sequenceN(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', 1);
SELECT sequenceN(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', 3);
SELECT sequenceN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT sequenceN(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 3);
SELECT sequenceN(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT sequenceN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 3);
SELECT sequenceN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT sequenceN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 3);
/* Errors */
SELECT sequenceN(tbool 't@2000-01-01', 1);
SELECT sequenceN(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', 1);
SELECT sequenceN(tint '1@2000-01-01', 1);
SELECT sequenceN(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT sequenceN(tfloat '1.5@2000-01-01', 1);

SELECT sequenceN(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1);
SELECT sequenceN(ttext 'AAA@2000-01-01', 1);
SELECT sequenceN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);

SELECT sequences(tbool 't@2000-01-01');
SELECT sequences(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT sequences(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT sequences(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT sequences(tint '1@2000-01-01');
SELECT sequences(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT sequences(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT sequences(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT sequences(tfloat '1.5@2000-01-01');
SELECT sequences(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT sequences(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT sequences(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT sequences(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT sequences(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT sequences(ttext 'AAA@2000-01-01');
SELECT sequences(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT sequences(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT sequences(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT segments(tbool 't@2000-01-01');
SELECT segments(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT segments(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT segments(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT segments(tint '1@2000-01-01');
SELECT segments(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT segments(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT segments(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT segments(tfloat '1.5@2000-01-01');
SELECT segments(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT segments(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT segments(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT segments(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT segments(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT segments(ttext 'AAA@2000-01-01');
SELECT segments(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT segments(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT segments(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numInstants(tbool 't@2000-01-01');
SELECT numInstants(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT numInstants(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT numInstants(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT numInstants(tint '1@2000-01-01');
SELECT numInstants(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT numInstants(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT numInstants(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT numInstants(tfloat '1.5@2000-01-01');
SELECT numInstants(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT numInstants(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numInstants(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numInstants(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numInstants(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numInstants(ttext 'AAA@2000-01-01');
SELECT numInstants(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT numInstants(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT numInstants(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numInstants(tfloat '{[1@2000-01-01, 2@2000-01-02),(2@2000-01-02, 3@2000-01-03]}');

SELECT startInstant(tbool 't@2000-01-01');
SELECT startInstant(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT startInstant(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT startInstant(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT startInstant(tint '1@2000-01-01');
SELECT startInstant(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT startInstant(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT startInstant(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT startInstant(tfloat '1.5@2000-01-01');
SELECT startInstant(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT startInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startInstant(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startInstant(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startInstant(ttext 'AAA@2000-01-01');
SELECT startInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT startInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT startInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT endInstant(tbool 't@2000-01-01');
SELECT endInstant(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT endInstant(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT endInstant(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT endInstant(tint '1@2000-01-01');
SELECT endInstant(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT endInstant(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT endInstant(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT endInstant(tfloat '1.5@2000-01-01');
SELECT endInstant(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT endInstant(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endInstant(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endInstant(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endInstant(ttext 'AAA@2000-01-01');
SELECT endInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT endInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT endInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT instantN(tbool 't@2000-01-01', 1);
SELECT instantN(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', 1);
SELECT instantN(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', 1);
SELECT instantN(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', 1);
SELECT instantN(tint '1@2000-01-01', 1);
SELECT instantN(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT instantN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT instantN(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT instantN(tfloat '1.5@2000-01-01', 1);
SELECT instantN(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1);
SELECT instantN(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT instantN(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT instantN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT instantN(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT instantN(ttext 'AAA@2000-01-01', 1);
SELECT instantN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);
SELECT instantN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT instantN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);

SELECT instantN(tfloat '{[1@2000-01-01, 2@2000-01-02),(2@2000-01-02, 3@2000-01-03]}', 3);
SELECT instantN(tfloat '{[1@2000-01-01, 2@2000-01-02),(2@2000-01-02, 3@2000-01-03]}', 4);
-- NULL
SELECT instantN(tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]', 4);
SELECT instantN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 4);
SELECT instantN(tfloat '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 4);
SELECT instantN(ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]', 4);

SELECT instants(tbool 't@2000-01-01');
SELECT instants(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT instants(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT instants(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT instants(tint '1@2000-01-01');
SELECT instants(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT instants(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT instants(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT instants(tfloat '1.5@2000-01-01');
SELECT instants(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT instants(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT instants(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT instants(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT instants(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT instants(ttext 'AAA@2000-01-01');
SELECT instants(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT instants(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT instants(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numTimestamps(tbool 't@2000-01-01');
SELECT numTimestamps(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT numTimestamps(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT numTimestamps(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT numTimestamps(tint '1@2000-01-01');
SELECT numTimestamps(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT numTimestamps(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT numTimestamps(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT numTimestamps(tfloat '1.5@2000-01-01');
SELECT numTimestamps(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT numTimestamps(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numTimestamps(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numTimestamps(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numTimestamps(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numTimestamps(ttext 'AAA@2000-01-01');
SELECT numTimestamps(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT numTimestamps(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT numTimestamps(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numTimestamps(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03),[3.5@2000-01-03, 3.5@2000-01-05]}');

SELECT startTimestamp(tbool 't@2000-01-01');
SELECT startTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT startTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT startTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT startTimestamp(tint '1@2000-01-01');
SELECT startTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT startTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT startTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT startTimestamp(tfloat '1.5@2000-01-01');
SELECT startTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT startTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startTimestamp(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startTimestamp(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startTimestamp(ttext 'AAA@2000-01-01');
SELECT startTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT startTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT startTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT endTimestamp(tbool 't@2000-01-01');
SELECT endTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT endTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT endTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT endTimestamp(tint '1@2000-01-01');
SELECT endTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT endTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT endTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT endTimestamp(tfloat '1.5@2000-01-01');
SELECT endTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT endTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endTimestamp(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endTimestamp(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endTimestamp(ttext 'AAA@2000-01-01');
SELECT endTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT endTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT endTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT timestampN(tbool 't@2000-01-01', 1);
SELECT timestampN(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', 1);
SELECT timestampN(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', 1);
SELECT timestampN(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', 1);
SELECT timestampN(tint '1@2000-01-01', 1);
SELECT timestampN(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT timestampN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT timestampN(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT timestampN(tfloat '1.5@2000-01-01', 1);
SELECT timestampN(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1);
SELECT timestampN(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT timestampN(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT timestampN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT timestampN(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT timestampN(ttext 'AAA@2000-01-01', 1);
SELECT timestampN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);
SELECT timestampN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT timestampN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);
-- NULL
SELECT timestampN(tbool '[true@2000-01-01, false@2000-01-02, true@2000-01-03]', 4);
SELECT timestampN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 4);
SELECT timestampN(tfloat '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 4);
SELECT timestampN(ttext '[AA@2000-01-01, BB@2000-01-02, AA@2000-01-03]', 4);
SELECT timestampN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03),[3.5@2000-01-03, 3.5@2000-01-05]}',0);
SELECT timestampN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03),[3.5@2000-01-03, 3.5@2000-01-05]}',10);

SELECT timestamps(tbool 't@2000-01-01');
SELECT timestamps(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT timestamps(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT timestamps(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT timestamps(tint '1@2000-01-01');
SELECT timestamps(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT timestamps(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT timestamps(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT timestamps(tfloat '1.5@2000-01-01');
SELECT timestamps(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT timestamps(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT timestamps(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT timestamps(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timestamps(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timestamps(ttext 'AAA@2000-01-01');
SELECT timestamps(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT timestamps(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT timestamps(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

-------------------------------------------------------------------------------
-- Shift and tscale functions
-------------------------------------------------------------------------------

SELECT shift(tbool 't@2000-01-01', '5 min');
SELECT shift(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', '5 min');
SELECT shift(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', '5 min');
SELECT shift(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', '5 min');
SELECT shift(tint '1@2000-01-01', '5 min');
SELECT shift(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '5 min');
SELECT shift(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '5 min');
SELECT shift(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', '5 min');
SELECT shift(tfloat '1.5@2000-01-01', '5 min');
SELECT shift(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', '5 min');
SELECT shift(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '5 min');
SELECT shift(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '5 min');
SELECT shift(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '5 min');
SELECT shift(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '5 min');
SELECT shift(ttext 'AAA@2000-01-01', '5 min');
SELECT shift(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '5 min');
SELECT shift(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '5 min');
SELECT shift(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '5 min');

SELECT tscale(tbool 't@2000-01-01', '1 day');
SELECT tscale(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', '1 day');
SELECT tscale(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', '1 day');
SELECT tscale(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', '1 day');
SELECT tscale(tint '1@2000-01-01', '1 day');
SELECT tscale(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '1 day');
SELECT tscale(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '1 day');
SELECT tscale(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', '1 day');
SELECT tscale(tfloat '1.5@2000-01-01', '1 day');
SELECT tscale(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', '1 day');
SELECT tscale(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 day');
SELECT tscale(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 day');
SELECT tscale(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 day');
SELECT tscale(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 day');
SELECT tscale(ttext 'AAA@2000-01-01', '1 day');
SELECT tscale(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '1 day');
SELECT tscale(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '1 day');
SELECT tscale(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '1 day');

SELECT shiftTscale(tbool 't@2000-01-01', '1 day', '1 day');
SELECT shiftTscale(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', '1 day', '1 day');
SELECT shiftTscale(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', '1 day', '1 day');
SELECT shiftTscale(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', '1 day', '1 day');
SELECT shiftTscale(tint '1@2000-01-01', '1 day', '1 day');
SELECT shiftTscale(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '1 day', '1 day');
SELECT shiftTscale(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '1 day', '1 day');
SELECT shiftTscale(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', '1 day', '1 day');
SELECT shiftTscale(tfloat '1.5@2000-01-01', '1 day', '1 day');
SELECT shiftTscale(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', '1 day', '1 day');
SELECT shiftTscale(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 day', '1 day');
SELECT shiftTscale(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '1 day', '1 day');
SELECT shiftTscale(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 day', '1 day');
SELECT shiftTscale(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '1 day', '1 day');
SELECT shiftTscale(ttext 'AAA@2000-01-01', '1 day', '1 day');
SELECT shiftTscale(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '1 day', '1 day');
SELECT shiftTscale(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '1 day', '1 day');
SELECT shiftTscale(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '1 day', '1 day');

/* Errors */
SELECT tscale(tfloat '1@2000-01-01', '0');
SELECT tscale(tfloat '1@2000-01-01', '-1 day');


-------------------------------------------------------------------------------
-- Ever/always comparison functions
-------------------------------------------------------------------------------

SELECT tbool 't@2000-01-01' ?= true;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' ?= true;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' ?= true;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' ?= true;
SELECT tbool 't@2000-01-01' ?= false;
SELECT tbool '{t@2000-01-01, t@2000-01-02}' ?= false;
SELECT tbool '[t@2000-01-01, t@2000-01-02]' ?= false;
SELECT tbool '{[t@2000-01-01, t@2000-01-02],[t@2000-01-03, t@2000-01-04]}' ?= false;
SELECT tint '1@2000-01-01' ?= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?= 1;
SELECT tint '1@2000-01-01' ?= 4;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?= 4;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?= 4;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?= 4;
SELECT tfloat '1.5@2000-01-01' ?= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?= 1.5;
SELECT tfloat '1.5@2000-01-01' ?= 4.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?= 4.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?= 4.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?= 4.5;
SELECT ttext 'AAA@2000-01-01' ?= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?= 'AAA';
SELECT ttext 'AAA@2000-01-01' ?= 'DDD';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?= 'DDD';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?= 'DDD';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?= 'DDD';

SELECT tfloat '[1@2000-01-01, 1@2000-01-03]' ?= 1;
SELECT tfloat '[1@2000-01-01, 2@2000-01-03]' ?= 2;

SELECT tbool 't@2000-01-01' %= true;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' %= true;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' %= true;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' %= true;
SELECT tint '1@2000-01-01' %= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %= 1;
SELECT tfloat '1.5@2000-01-01' %= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %= 1.5;
SELECT ttext 'AAA@2000-01-01' %= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %= 'AAA';

SELECT tint '{[1@2000-01-01, 1@2000-01-02]}' %= 1;
SELECT tfloat '{[1@2000-01-01, 1@2000-01-02]}' %= 1;
SELECT tfloat '{[1@2000-01-01, 1@2000-01-02]}' %= 2;
SELECT ttext '{[AAA@2000-01-01, AAA@2000-01-02]}' %= 'AAA';

-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01' ?<> 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?<> 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?<> 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?<> 1;
SELECT tfloat '1.5@2000-01-01' ?<> 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?<> 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?<> 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?<> 1.5;
SELECT ttext 'AAA@2000-01-01' ?<> 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?<> 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?<> 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?<> 'AAA';

SELECT tint '1@2000-01-01' %<> 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %<> 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %<> 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %<> 1;
SELECT tfloat '1.5@2000-01-01' %<> 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %<> 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %<> 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %<> 1.5;
SELECT ttext 'AAA@2000-01-01' %<> 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %<> 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %<> 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %<> 'AAA';

-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01' ?< 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?< 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?< 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?< 1;
SELECT tfloat '1.5@2000-01-01' ?< 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?< 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?< 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?< 1.5;
SELECT ttext 'AAA@2000-01-01' ?< 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?< 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?< 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?< 'AAA';

SELECT tint '1@2000-01-01' %< 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %< 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %< 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %< 1;
SELECT tfloat '1.5@2000-01-01' %< 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %< 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %< 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %< 1.5;
SELECT ttext 'AAA@2000-01-01' %< 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %< 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %< 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %< 'AAA';

SELECT tfloat '[1.5@2000-01-01, 1.5@2000-01-02]' %< 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02]' %< 2.5;

-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01' ?> 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?> 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?> 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?> 1;
SELECT tfloat '1.5@2000-01-01' ?> 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?> 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?> 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?> 1.5;
SELECT ttext 'AAA@2000-01-01' ?> 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?> 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?> 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?> 'AAA';

SELECT tint '1@2000-01-01' %> 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %> 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %> 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %> 1;
SELECT tfloat '1.5@2000-01-01' %> 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %> 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %> 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %> 1.5;
SELECT ttext 'AAA@2000-01-01' %> 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %> 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %> 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %> 'AAA';

-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01' ?<= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?<= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?<= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?<= 1;
SELECT tfloat '1.5@2000-01-01' ?<= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?<= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?<= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?<= 1.5;
SELECT ttext 'AAA@2000-01-01' ?<= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?<= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?<= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?<= 'AAA';

SELECT tfloat '[1.5@2000-01-01, 1.5@2000-01-02]' ?<= 1.5;

SELECT tint '1@2000-01-01' %<= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %<= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %<= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %<= 1;
SELECT tfloat '1.5@2000-01-01' %<= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %<= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %<= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %<= 1.5;
SELECT ttext 'AAA@2000-01-01' %<= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %<= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %<= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %<= 'AAA';

-------------------------------------------------------------------------------

SELECT tint '1@2000-01-01' ?>= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' ?>= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' ?>= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' ?>= 1;
SELECT tfloat '1.5@2000-01-01' ?>= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' ?>= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' ?>= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' ?>= 1.5;
SELECT ttext 'AAA@2000-01-01' ?>= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' ?>= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' ?>= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' ?>= 'AAA';

SELECT tint '1@2000-01-01' %>= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' %>= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' %>= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' %>= 1;
SELECT tfloat '1.5@2000-01-01' %>= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' %>= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' %>= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' %>= 1.5;
SELECT ttext 'AAA@2000-01-01' %>= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' %>= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' %>= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' %>= 'AAA';

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT atValue(tbool 't@2000-01-01', true);
SELECT atValue(tbool '{t@2000-01-01}', true);
SELECT atValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', true);
SELECT atValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', true);
SELECT atValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', true);
SELECT atValue(tint '1@2000-01-01', 1);
SELECT atValue(tint '{1@2000-01-01}', 1);
SELECT atValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT atValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT atValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT atValue(tfloat '1.5@2000-01-01', 1.5);
SELECT atValue(tfloat '{1.5@2000-01-01}', 1.5);
SELECT atValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1.5);
SELECT atValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 2);
SELECT atValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 2);
SELECT atValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 2);
SELECT atValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 2);
SELECT atValue(ttext 'AAA@2000-01-01', 'AAA');
SELECT atValue(ttext '{AAA@2000-01-01}', 'AAA');
SELECT atValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 'AAA');
SELECT atValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 'AAA');
SELECT atValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 'AAA');

/* Roundoff errors */
SELECT atValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 - 1e-16);
SELECT atValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 - 1e-17);
SELECT atValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 + 1e-16);
SELECT atValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 2 - 1e-16);
SELECT atValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 2 + 1e-16);

SELECT minusValue(tbool 't@2000-01-01', true);
SELECT minusValue(tbool '{t@2000-01-01}', true);
SELECT minusValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', true);
SELECT minusValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', true);
SELECT minusValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', true);
SELECT minusValue(tint '1@2000-01-01', 1);
SELECT minusValue(tint '{1@2000-01-01}', 1);
SELECT minusValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT minusValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT minusValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT minusValue(tfloat '1.5@2000-01-01', 1.5);
SELECT minusValue(tfloat '{1.5@2000-01-01}', 1.5);
SELECT minusValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1.5);
SELECT minusValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1.5);
SELECT minusValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 2);
SELECT minusValue(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 2);
SELECT minusValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1.5);
SELECT minusValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 2);
SELECT minusValue(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 2);
SELECT minusValue(ttext 'AAA@2000-01-01', 'AAA');
SELECT minusValue(ttext '{AAA@2000-01-01}', 'AAA');
SELECT minusValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 'AAA');
SELECT minusValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 'AAA');
SELECT minusValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 'AAA');

SELECT minusValue(tbool '{[t@2000-01-01, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', true);
SELECT minusValue(tint '{[1@2000-01-01, 1@2000-01-03],[1@2000-01-04, 1@2000-01-05]}', 1);
SELECT minusValue(tfloat '[1@2000-01-01, 1@2000-01-02, 3@2000-01-03]', 2);
SELECT minusValue(tfloat '{[1.5@2000-01-01, 1.5@2000-01-03],[1.5@2000-01-04, 1.5@2000-01-05]}', 1.5);
SELECT minusValue(ttext '{[AA@2000-01-01, AA@2000-01-03],[AA@2000-01-04, AA@2000-01-05]}', text 'AA');

/* Roundoff errors */
SELECT minusValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 - 1e-16);
SELECT minusValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 - 1e-17);
SELECT minusValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 1 + 1e-12);
SELECT minusValue(tfloat '(1@2000-01-01, 2@2000-01-02]', 1 + 1e-12);
SELECT round(minusValue(tfloat '[1@2000-01-01, 2@2000-01-02)', 2 - 1e-15), 12);
SELECT minusValue(tfloat '[1@2000-01-01, 2@2000-01-02]', 2 + 1e-16);
WITH values(v) AS (SELECT unnest(ARRAY[1 - 1e-17, 1 + 1e-12, 2 - 1e-15, 2 + 1e-16])),
  temp(t) AS (SELECT tfloat '[1@2000-01-01, 2@2000-01-02]')
SELECT DISTINCT t = merge(atValue(t,v), minusValue(t,v)) FROM temp, values;

SELECT atValues(tbool 't@2000-01-01', ARRAY[true]);
SELECT atValues(tbool '{t@2000-01-01}', ARRAY[true]);
SELECT atValues(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', ARRAY[true]);
SELECT atValues(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', ARRAY[true]);
SELECT atValues(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', ARRAY[true]);
SELECT atValues(tint '1@2000-01-01', ARRAY[1]);
SELECT atValues(tint '{1@2000-01-01}', ARRAY[1]);
SELECT atValues(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[1]);
SELECT atValues(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[1]);
SELECT atValues(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[1]);
SELECT atValues(tfloat '1.5@2000-01-01', ARRAY[1.5,2]);
SELECT atValues(tfloat '{1.5@2000-01-01}', ARRAY[1.5,2]);
SELECT atValues(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[1.5,2]);
SELECT atValues(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5,2]);
SELECT atValues(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5,2]);
SELECT atValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5,2]);
SELECT atValues(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5,2]);
SELECT atValues(ttext 'AAA@2000-01-01', ARRAY[text 'AAA']);
SELECT atValues(ttext '{AAA@2000-01-01}', ARRAY[text 'AAA']);
SELECT atValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ARRAY[text 'AAA']);
SELECT atValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ARRAY[text 'AAA']);
SELECT atValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ARRAY[text 'AAA']);

SELECT atValues(tbool '{t@2000-01-01}', '{}'::bool[]);
SELECT atValues(tint '{1@2000-01-01}', '{}'::int[]);
SELECT atValues(tfloat '{1@2000-01-01}', '{}'::float[]);
SELECT atValues(ttext '{1@2000-01-01}', '{}'::text[]);

SELECT minusValues(tbool 't@2000-01-01', ARRAY[true]);
SELECT minusValues(tbool '{t@2000-01-01}', ARRAY[true]);
SELECT minusValues(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', ARRAY[true]);
SELECT minusValues(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', ARRAY[true]);
SELECT minusValues(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', ARRAY[true]);
SELECT minusValues(tint '1@2000-01-01', ARRAY[1]);
SELECT minusValues(tint '{1@2000-01-01}', ARRAY[1]);
SELECT minusValues(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[1]);
SELECT minusValues(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[1]);
SELECT minusValues(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[1]);
SELECT minusValues(tfloat '1.5@2000-01-01', ARRAY[1.5]);
SELECT minusValues(tfloat '{1.5@2000-01-01}', ARRAY[1.5]);
SELECT minusValues(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[1.5,2]);
SELECT minusValues(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5,2]);
SELECT minusValues(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5,2]);
SELECT minusValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5,2]);
SELECT minusValues(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5,2]);
SELECT minusValues(ttext 'AAA@2000-01-01', ARRAY[text 'AAA']);
SELECT minusValues(ttext '{AAA@2000-01-01}', ARRAY[text 'AAA']);
SELECT minusValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ARRAY[text 'AAA']);
SELECT minusValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ARRAY[text 'AAA']);
SELECT minusValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ARRAY[text 'AAA']);

SELECT minusValues(tbool '{t@2000-01-01}', '{}'::bool[]);
SELECT minusValues(tint '{1@2000-01-01}', '{}'::int[]);
SELECT minusValues(tfloat '{1@2000-01-01}', '{}'::float[]);
SELECT minusValues(ttext '{1@2000-01-01}', '{}'::text[]);
SELECT minusValues(tbool '{[t@2000-01-01, t@2000-01-03],[f@2000-01-04, f@2000-01-05]}', ARRAY[true, false]);
SELECT minusValues(tint '{[1@2000-01-01, 1@2000-01-03],[2@2000-01-04, 2@2000-01-05]}', ARRAY[1, 2]);
SELECT minusValues(tfloat '{[1.5@2000-01-01, 1.5@2000-01-03],[2.5@2000-01-04, 2.5@2000-01-05]}', ARRAY[1.5, 2.5]);
SELECT minusValues(ttext '{[AA@2000-01-01, AA@2000-01-03],[BB@2000-01-04, BB@2000-01-05]}', ARRAY[text 'AA', 'BB']);

SELECT atSpan(tint '1@2000-01-01', intspan '[1,3]');
SELECT atSpan(tint '{1@2000-01-01}', intspan '[1,3]');
SELECT atSpan(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', intspan '[1,3]');
SELECT atSpan(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', intspan '[1,3]');
SELECT atSpan(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', intspan '[1,3]');
SELECT atSpan(tfloat '1.5@2000-01-01', floatspan '[1,3]');
SELECT atSpan(tfloat '{1.5@2000-01-01}', floatspan '[1,3]');
SELECT atSpan(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', floatspan '[1,3]');
SELECT atSpan(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', floatspan '[2,3]');
SELECT atSpan(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[1,3]');
SELECT atSpan(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[2,3]');
SELECT atSpan(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[1,3]');
SELECT atSpan(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[2,3]');
SELECT atSpan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[1,3]');
SELECT atSpan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[2,3]');
SELECT atSpan(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[1,3]');
SELECT atSpan(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[2,3]');

SELECT atSpan(tfloat '[1@2000-01-01, 2@2000-01-02]', floatspan '[2, 3]');

SELECT minusSpan(tint '1@2000-01-01', intspan '[1,3]');
SELECT minusSpan(tint '{1@2000-01-01}', intspan '[1,3]');
SELECT minusSpan(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', intspan '[1,3]');
SELECT minusSpan(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', intspan '[1,3]');
SELECT minusSpan(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', intspan '[1,3]');
SELECT minusSpan(tfloat '1.5@2000-01-01', floatspan '[1,3]');
SELECT minusSpan(tfloat '{1.5@2000-01-01}', floatspan '[1,3]');
SELECT minusSpan(tfloat '[1@2000-01-01,2@2000-01-02]','[2,3]');
SELECT minusSpan(tfloat '[1@2000-01-01,2@2000-01-02]','[0,1]');
SELECT minusSpan(tfloat '[1@2000-01-01,2@2000-01-02]','(1,2)');
SELECT minusSpan(tfloat '[1@2000-01-01,3@2000-01-02]','(2,3)');
SELECT minusSpan(tfloat '[1@2000-01-01,3@2000-01-02]','(1,2)');
SELECT minusSpan(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', floatspan '[1,3]');
SELECT minusSpan(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[1,3]');
SELECT minusSpan(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[2,3]');
SELECT minusSpan(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[1,3]');
SELECT minusSpan(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatspan '[2,3]');
SELECT minusSpan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[1,3]');
SELECT minusSpan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[2,3]');
SELECT minusSpan(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[1,3]');
SELECT minusSpan(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatspan '[2,3]');

SELECT atSpans(tint '1@2000-01-01', ARRAY[intspan '[1,3]']);
SELECT atSpans(tint '{1@2000-01-01}', ARRAY[intspan '[1,3]']);
SELECT atSpans(tint '{1@2000-01-01}', ARRAY[intspan '[2,3]']);
SELECT atSpans(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intspan '[1,3]']);
SELECT atSpans(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intspan '[1,3]']);
SELECT atSpans(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intspan '[1,3]']);
SELECT atSpans(tfloat '1.5@2000-01-01', ARRAY[floatspan '[1,3]']);
SELECT atSpans(tfloat '{1.5@2000-01-01}', ARRAY[floatspan '[1,3]']);
SELECT atSpans(tfloat '{1.5@2000-01-01}', ARRAY[floatspan '[2,3]']);
SELECT atSpans(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatspan '[1,3]']);
SELECT atSpans(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatspan '[1,3]']);
SELECT atSpans(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatspan '[1,3]']);

SELECT atSpans(tfloat '1@2000-01-01', ARRAY[floatspan '[2,3]', '[4,5]']);
SELECT atSpans(tfloat '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[floatspan '[1,1]','[2,3]']);
SELECT atSpans(tfloat '[1@2000-01-01]', ARRAY[floatspan '(1, 3]']);
SELECT atSpans(tfloat '{[1@2000-01-01, 2@2000-01-02), (2@2000-01-02, 3@2000-01-03)}', ARRAY[floatspan '[1,3]','[2,4]','[5,6]']);
SELECT atSpans(tfloat '[1@2000-01-01,10@2000-01-02]', ARRAY[floatspan '[1,2)', '[2,3]', '[4,5]']);
SELECT atSpans(tfloat '{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 6@2000-01-04]}', ARRAY[floatspan '(2,3)','(4,5)']);
SELECT atSpans(tfloat '{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 6@2000-01-04]}', ARRAY[floatspan '[3,4]','[7,8]']);
SELECT atSpans(tfloat '{[1@2000-01-01, 3@2000-01-03],[4@2000-01-04]}', ARRAY[floatspan '[1,2]', '[3,4]']);

SELECT atSpans(tint '{1@2000-01-01}', '{}'::intspan[]);
SELECT atSpans(tfloat '{1@2000-01-01}', '{}'::floatspan[]);
SELECT atSpans(tint '1@2000-01-01', ARRAY[intspan '[5,6]']);
SELECT atSpans(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intspan '[5,6]']);
SELECT atSpans(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intspan '[5,6]']);
SELECT atSpans(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intspan '[5,6]']);
SELECT atSpans(tfloat '1.5@2000-01-01', ARRAY[floatspan '[5,6]']);
SELECT atSpans(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatspan '[5,6]']);
SELECT atSpans(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatspan '[5,6]']);
SELECT atSpans(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatspan '[5,6]']);

SELECT atSpans(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', ARRAY[floatspan '[1,2]', '[2.5,3]']);
SELECT atSpans(tfloat'{[1@2000-01-01, 2@2000_01-02],[7@2000-01-03, 8@2000_01-04]}',ARRAY[floatspan '[3,4]','[5,6]']);

SELECT minusSpans(tint '1@2000-01-01', ARRAY[intspan '[1,3]']);
SELECT minusSpans(tint '{1@2000-01-01}', ARRAY[intspan '[1,3]']);
SELECT minusSpans(tint '{1@2000-01-01}', ARRAY[intspan '[2,3]']);
SELECT minusSpans(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intspan '[1,3]']);
SELECT minusSpans(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intspan '[1,3]']);
SELECT minusSpans(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intspan '[1,3]']);
SELECT minusSpans(tfloat '1.5@2000-01-01', ARRAY[floatspan '[1,3]']);
SELECT minusSpans(tfloat '{1.5@2000-01-01}', ARRAY[floatspan '[1,3]']);
SELECT minusSpans(tfloat '{1.5@2000-01-01}', ARRAY[floatspan '[2,3]']);
SELECT minusSpans(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatspan '[1,3]']);
SELECT minusSpans(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatspan '[1,3]']);
SELECT minusSpans(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatspan '[1,3]']);
SELECT minusSpans(tfloat '{[1@2000-01-01, 2@2000-01-02], [5@2000-01-03, 6@2000-01-04]}', ARRAY[floatspan '(2,3)','(4,5)']);

SELECT minusSpans(tint '{1@2000-01-01}', '{}'::intspan[]);
SELECT minusSpans(tfloat '{1@2000-01-01}', '{}'::floatspan[]);
SELECT minusSpans(tint '1@2000-01-01', ARRAY[intspan '[5,6]']);
SELECT minusSpans(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intspan '[5,6]']);
SELECT minusSpans(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intspan '[5,6]']);
SELECT minusSpans(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intspan '[5,6]']);
SELECT minusSpans(tfloat '1.5@2000-01-01', ARRAY[floatspan '[5,6]']);
SELECT minusSpans(tfloat '1@2000-01-01', ARRAY[floatspan '[2,3]', '[4,5]']);
SELECT minusSpans(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatspan '[5,6]']);
SELECT minusSpans(tfloat '[1@2000-01-01]', ARRAY[floatspan '(1, 3]']);
SELECT minusSpans(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatspan '[5,6]']);
SELECT minusSpans(tfloat '{[1@2000-01-01, 4@2000-01-02],[2@2000-01-03, 3@2000-01-04]}', ARRAY[floatspan '[1,1]', '[4,4]']);
SELECT minusSpans(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatspan '[5,6]']);
SELECT minusSpans(tfloat'[1@2000-01-01, 2@2000_01-03]',ARRAY[floatspan '[1,1.1]','[1.5,2]']);
SELECT minusSpans(tfloat '[1@2000-01-01, 2@2000-01-02]', ARRAY[floatspan '[3,4]', '[5,6]']);
SELECT minusSpans(tfloat '1@2000-01-01', ARRAY[floatspan '[0,1)', '(1,2]']);

SELECT atMin(tint '1@2000-01-01');
SELECT atMin(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT atMin(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT atMin(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT atMin(tfloat '1.5@2000-01-01');
SELECT atMin(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT atMin(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT atMin(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT atMin(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMin(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMin(ttext 'AAA@2000-01-01');
SELECT atMin(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT atMin(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT atMin(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT atMin(tfloat '(1@2000-01-01, 2@2000-01-02)');
SELECT atMin(tfloat '(1@2000-01-01, 2@2000-01-02, 1@2000-01-03)');
SELECT atMin(tfloat '{(1@2000-01-01, 2@2000-01-02)}');
SELECT atMin(tfloat '{(1@2000-01-01, 2@2000-01-02, 1@2000-01-03)}');
SELECT atMin(tfloat '{[2@2012-01-01, 1@2012-01-03), (1@2012-01-03, 1@2012-01-05)}');
SELECT atMin(tfloat '{[2@2012-01-01, 1@2012-01-03), (1@2012-01-03, 2@2012-01-05)}');

SELECT minusMin(tint '1@2000-01-01');
SELECT minusMin(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minusMin(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minusMin(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minusMin(tfloat '1.5@2000-01-01');
SELECT minusMin(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minusMin(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMin(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMin(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minusMin(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minusMin(ttext 'AAA@2000-01-01');
SELECT minusMin(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minusMin(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minusMin(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT atMax(tint '1@2000-01-01');
SELECT atMax(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT atMax(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT atMax(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT atMax(tfloat '1.5@2000-01-01');
SELECT atMax(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT atMax(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT atMax(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT atMax(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMax(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMax(ttext 'AAA@2000-01-01');
SELECT atMax(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT atMax(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT atMax(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT atMax(tfloat '(1@2000-01-01, 2@2000-01-02)');
SELECT atMax(tfloat '(2@2000-01-01, 1@2000-01-02, 2@2000-01-03)');
SELECT atMax(tfloat '{(1@2000-01-01, 2@2000-01-02)}');
SELECT atMax(tfloat '{(2@2000-01-01, 1@2000-01-02, 2@2000-01-03)}');
SELECT atMax(tfloat '{[1@2012-01-01, 3@2012-01-03), (3@2012-01-03, 1@2012-01-05)}');

SELECT minusMax(tint '1@2000-01-01');
SELECT minusMax(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minusMax(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minusMax(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minusMax(tfloat '1.5@2000-01-01');
SELECT minusMax(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minusMax(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMax(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMax(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minusMax(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minusMax(ttext 'AAA@2000-01-01');
SELECT minusMax(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minusMax(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minusMax(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT atTimestamp(tbool 't@2000-01-01', timestamptz '2000-01-01');
SELECT atTimestamp(tbool 't@2000-01-02', timestamptz '2000-01-01');
SELECT atTimestamp(tbool 't@2000-01-01', timestamptz '2000-01-02');
SELECT atTimestamp(tbool '{t@2000-01-01}', timestamptz '2000-01-01');
SELECT atTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamptz '2000-01-01');
SELECT atTimestamp(tbool '{t@2000-01-01, t@2000-01-03}', timestamptz '2000-01-02');
SELECT atTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamptz '2000-01-01');
SELECT atTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamptz '2000-01-01');
SELECT atTimestamp(tint '1@2000-01-01', timestamptz '2000-01-01');
SELECT atTimestamp(tint '{1@2000-01-01}', timestamptz '2000-01-01');
SELECT atTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamptz '2000-01-01');
SELECT atTimestamp(tint '{1@2000-01-01, 1@2000-01-03}', timestamptz '2000-01-02');
SELECT atTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamptz '2000-01-01');
SELECT atTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '1.5@2000-01-01', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '{1.5@2000-01-01}', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '{1.5@2000-01-01, 1.5@2000-01-03}', timestamptz '2000-01-02');
SELECT atTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamptz '2000-01-01');
SELECT atTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT atTimestamp(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT atTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT atTimestamp(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT atTimestamp(ttext 'AAA@2000-01-01', timestamptz '2000-01-01');
SELECT atTimestamp(ttext '{AAA@2000-01-01}', timestamptz '2000-01-01');
SELECT atTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamptz '2000-01-01');
SELECT atTimestamp(ttext '{AAA@2000-01-01, AAA@2000-01-03}', timestamptz '2000-01-02');
SELECT atTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamptz '2000-01-01');
SELECT atTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamptz '2000-01-01');

SELECT valueAtTimestamp(tbool 't@2000-01-01', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tint '1@2000-01-01', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tfloat '1.5@2000-01-01', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT valueAtTimestamp(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT valueAtTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT valueAtTimestamp(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT valueAtTimestamp(ttext 'AAA@2000-01-01', timestamptz '2000-01-01');
SELECT valueAtTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamptz '2000-01-01');
SELECT valueAtTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamptz '2000-01-01');
SELECT valueAtTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamptz '2000-01-01');

SELECT minusTimestamp(tbool 't@2000-01-01', timestamptz '2000-01-01');
SELECT minusTimestamp(tbool '{t@2000-01-01}', timestamptz '2000-01-01');
SELECT minusTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '1@2000-01-01', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '{1@2000-01-01}', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '1.5@2000-01-01', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '{1.5@2000-01-01}', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamptz '2000-01-01');
SELECT minusTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT minusTimestamp(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05]', timestamptz '2000-01-02');
SELECT minusTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT minusTimestamp(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-03, 1.5@2000-01-05],[3.5@2000-01-06, 3.5@2000-01-07]}', timestamptz '2000-01-02');
SELECT minusTimestamp(ttext 'AAA@2000-01-01', timestamptz '2000-01-01');
SELECT minusTimestamp(ttext '{AAA@2000-01-01}', timestamptz '2000-01-01');
SELECT minusTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamptz '2000-01-01');
SELECT minusTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamptz '2000-01-01');
SELECT minusTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', timestamptz '2000-01-01');
SELECT minusTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamptz '2000-01-01');

SELECT minusTimestamp(tfloat '[1@2000-01-01]', timestamptz '2000-01-01');

SELECT atTimestampSet(tbool 't@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{t@2000-01-01}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '1@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{1@2000-01-01}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '1.5@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{1.5@2000-01-01}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext 'AAA@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{AAA@2000-01-01}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestampset '{2000-01-01}');

SELECT atTimestampSet(tfloat '{1@2000-01-02}', '{2000-01-01, 2000-01-03}');
SELECT atTimestampSet(tfloat '{1@2000-01-02, 1@2000-01-03}', '{2000-01-01, 2000-01-04}');
SELECT atTimestampSet(tfloat '[1@2000-01-01]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '[1@2000-01-01]', timestampset '{2000-01-01, 2000-01-02}');
SELECT atTimestampSet(tfloat '[1@2000-01-02]', '{2000-01-01, 2000-01-03}');
SELECT atTimestampSet(tfloat '[1@2000-01-02, 1@2000-01-03]', '{2000-01-01, 2000-01-04}');
SELECT atTimestampSet(tfloat '[1@2000-01-02, 2@2000-01-04, 1@2000-01-05]', timestampset '{2000-01-01, 2000-01-02, 2000-01-03, 2000-01-04, 2000-01-05, 2000-01-06}');
SELECT atTimestampSet(tfloat '(1@2000-01-02, 2@2000-01-04, 1@2000-01-05)', timestampset '{2000-01-01, 2000-01-02, 2000-01-03, 2000-01-04, 2000-01-05, 2000-01-06}');
SELECT atTimestampSet(tfloat '{[1@2000-01-03, 1@2000-01-04]}', timestampset '{2000-01-01, 2000-01-02}');
SELECT atTimestampSet(tfloat '{[1@2000-01-02, 1@2000-01-03],[1@2000-01-05, 1@2000-01-06]}', timestampset '{2000-01-01, 2000-01-04}');
SELECT atTimestampSet(tfloat '{[1@2000-01-01, 2@2000-01-02]}', timestampset '{2000-01-01, 2000-01-02}');
SELECT atTimestampSet(tfloat '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04, 1@2000-01-05}', '{2000-01-02, 2000-01-04}');

SELECT minusTimestampSet(tbool 't@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{t@2000-01-01}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '1@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{1@2000-01-01}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{1.5@2000-01-01}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '1.5@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext 'AAA@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{AAA@2000-01-01}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestampset '{2000-01-01}');

SELECT minusTimestampSet(tfloat '{1@2000-01-02}', '{2000-01-01, 2000-01-04}');
SELECT minusTimestampSet(tfloat '{1@2000-01-02, 1@2000-01-03}', '{2000-01-01, 2000-01-04}');
SELECT minusTimestampSet(tfloat '[1@2000-01-01]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '[1@2000-01-02]', '{2000-01-01, 2000-01-03}');
SELECT minusTimestampSet(tfloat '{[1@2000-01-01], [1@2000-01-02]}', timestampset '{2000-01-01, 2000-01-02}');
SELECT minusTimestamp(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamptz '2000-01-02');
SELECT minusTimestampset(tfloat '{[1@2000-01-01, 2@2000-01-02]}', timestampset '{2000-01-01, 2000-01-02}');
SELECT minusTimestampset(tfloat '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03, 2@2000-01-04, 1@2000-01-05}', '{2000-01-02, 2000-01-04}');

SELECT atPeriod(tbool 't@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{t@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '1@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{1@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '1.5@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{1.5@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext 'AAA@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{AAA@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT minusPeriod(tbool 't@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{t@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '1@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{1@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '1.5@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{1.5@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext 'AAA@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{AAA@2000-01-01}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT minusPeriod(tfloat '[1@2000-01-01]', period '[2000-01-01, 2000-01-02]');

SELECT atPeriodSet(tbool 't@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{t@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '1@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{1@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '1.5@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{1.5@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext 'AAA@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{AAA@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT atPeriodSet(tfloat '{1@2000-01-02}', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT atPeriodSet(tfloat '{1@2000-01-03}', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT atPeriodSet(tfloat '[1@2000-01-02]', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT atPeriodSet(tfloat '[1@2000-01-03]', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT atPeriodSet(tfloat '[1@2000-01-01,1@2000-01-02]', '{[2000-01-01,2000-01-03],[2000-01-04,2000-01-05]}');
SELECT atPeriodSet(tfloat '{[1@2000-01-01, 1@2000-01-02]}', periodset '{[2000-01-03, 2000-01-04]}');
SELECT atPeriodSet(tfloat '{[1@2000-01-02, 1@2000-01-03),[1@2000-01-04, 1@2000-01-05]}', periodset '{[2000-01-01, 2000-01-02),[2000-01-03, 2000-01-04)}');
SELECT atPeriodSet(tfloat '{[1@2000-01-01, 2@2000-01-02]}', '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-06]}');

SELECT minusPeriodSet(tbool 't@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{t@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '1@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{1@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '1.5@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{1.5@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext 'AAA@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{AAA@2000-01-01}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT minusPeriodSet(tfloat '{1@2000-01-02}', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT minusPeriodSet(tfloat '{1@2000-01-03}', '{[2000-01-01,2000-01-02],[2000-01-04,2000-01-05]}');
SELECT minusPeriodSet(tfloat '[1@2000-01-01]', periodset '{[2000-01-01, 2000-01-02]}');
SELECT minusPeriodSet(tfloat '[1@2000-01-03]', periodset '{[2000-01-01, 2000-01-02],[2000-01-04, 2000-01-05]}');
SELECT minusPeriodSet(tfloat '[1@2000-01-01,1@2000-01-03]', periodset '{[2000-01-02, 2000-01-03],[2000-01-04, 2000-01-05]}');
SELECT minusPeriodSet(tfloat '{[1@2000-01-01, 1@2000-01-02]}', periodset '{[2000-01-01, 2000-01-02]}');
SELECT minusPeriodSet(tfloat '{[1@2000-01-01, 1@2000-01-02],[1@2000-01-03, 1@2000-01-04]}', periodset '{[2000-01-01, 2000-01-02],[2000-01-03, 2000-01-04]}');

SELECT atTBox(tint '1@2000-01-01', tbox 'TBOX X([1,2])');
SELECT atTBox(tint '1@2000-01-01', tbox 'TBOX T([2000-01-01,2000-01-02])');
SELECT atTBox(tint '1@2000-01-01', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tint '{1@2000-01-01}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '1.5@2000-01-01', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '{1.5@2000-01-01}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT atTBox(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
-- NULL
SELECT atTbox(tfloat '[1@2000-01-01, 2@2000-01-02)', tbox 'TBOX XT([2,2],[2000-01-02,2000-01-02])');

SELECT minusTBox(tint '1@2000-01-01', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tint '{1@2000-01-01}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '1.5@2000-01-01', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '{1.5@2000-01-01}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');
SELECT minusTBox(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tbox 'TBOX XT([1,2],[2000-01-01,2000-01-02])');

-------------------------------------------------------------------------------
-- Intesection functions
-------------------------------------------------------------------------------

SELECT intersectsTimestamp(tbool 't@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tint '1@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tfloat '1.5@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(ttext 'AAA@2000-01-01', timestamptz '2000-01-01');
SELECT intersectsTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamptz '2000-01-01');
SELECT intersectsTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamptz '2000-01-01');
SELECT intersectsTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamptz '2000-01-01');

SELECT intersectsTimestampSet(tbool 't@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tint '1@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tfloat '1.5@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(ttext 'AAA@2000-01-01', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestampset '{2000-01-01}');
SELECT intersectsTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestampset '{2000-01-01}');

SELECT intersectsPeriod(tbool 't@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tint '1@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tfloat '1.5@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(ttext 'AAA@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT intersectsPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT intersectsPeriod(tfloat '{[1@2000-01-02, 1@2000-01-03]}', period '[2000-01-01, 2000-01-04]');

SELECT intersectsPeriodSet(tbool 't@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tint '1@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tfloat '1.5@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(ttext 'AAA@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT intersectsPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT integral(tint '1@2000-01-01');
SELECT integral(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT integral(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT integral(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT integral(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT integral(tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT integral(tfloat '1.5@2000-01-01');
SELECT integral(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT integral(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT integral(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT integral(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT integral(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT round(twAvg(tint '1@2000-01-01')::numeric, 6);
SELECT round(twAvg(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}')::numeric, 6);
SELECT round(twAvg(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]')::numeric, 6);
SELECT round(twAvg(tfloat 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]')::numeric, 6);
SELECT round(twAvg(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}')::numeric, 6);
SELECT round(twAvg(tfloat 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}')::numeric, 6);

SELECT round(twAvg(tint '{[1@2000-01-01], [2@2000-01-02], [1@2000-01-03]}')::numeric, 6);

SELECT round(twAvg(tfloat '1.5@2000-01-01')::numeric, 6);
SELECT round(twAvg(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}')::numeric, 6);
SELECT round(twAvg(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]')::numeric, 6);
SELECT round(twAvg(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]')::numeric, 6);
SELECT round(twAvg(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}')::numeric, 6);
SELECT round(twAvg(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}')::numeric, 6);

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
-------------------------------------------------------------------------------

SELECT tbool_cmp(tbool 't@2000-01-01', tbool 't@2000-01-01');
SELECT tbool_cmp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool 't@2000-01-01');
SELECT tbool_cmp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool 't@2000-01-01');
SELECT tbool_cmp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool 't@2000-01-01');

SELECT tbool_cmp(tbool 't@2000-01-01', tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_cmp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_cmp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_cmp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');

SELECT tbool_cmp(tbool 't@2000-01-01', tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_cmp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_cmp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_cmp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');

SELECT tbool_cmp(tbool 't@2000-01-01', tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbool_cmp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbool_cmp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbool_cmp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');

SELECT tfloat_cmp('1@2000-01-01', '{1@2000-01-01}');
SELECT tfloat_cmp('[1@2000-01-01, 2@2000-01-02]', '(1@2000-01-01, 2@2000-01-02]');
SELECT tfloat_cmp('(1@2000-01-01, 2@2000-01-02]', '[1@2000-01-01, 2@2000-01-02]');
SELECT tfloat_cmp('Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]', '[1@2000-01-01, 2@2000-01-02]');
SELECT tfloat_cmp('[1@2000-01-01, 2@2000-01-02]', 'Interp=Stepwise;[1@2000-01-01, 2@2000-01-02]');
SELECT tfloat_cmp('[1@2000-01-01, 2@2000-01-02, 4@2000-01-05]', '[1@2000-01-01, 3@2000-01-03, 4@2000-01-05]');
SELECT tfloat_cmp('[1@2000-01-01, 3@2000-01-03, 4@2000-01-05]', '[1@2000-01-01, 2@2000-01-02, 4@2000-01-05]');

SELECT tfloat_cmp('{[1@2000-01-01, 2@2000-01-02]}', '{(1@2000-01-01, 2@2000-01-02]}');
SELECT tfloat_cmp('{(1@2000-01-01, 2@2000-01-02]}', '{[1@2000-01-01, 2@2000-01-02]}');
SELECT tfloat_cmp('Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02]}', '{[1@2000-01-01, 2@2000-01-02]}');
SELECT tfloat_cmp('{[1@2000-01-01, 2@2000-01-02]}', 'Interp=Stepwise;{[1@2000-01-01, 2@2000-01-02]}');

-------------------------------------------------------------------------------

SELECT tbool 't@2000-01-01' = tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' = tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' = tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' = tbool 't@2000-01-01';

SELECT tbool 't@2000-01-01' = tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' = tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' = tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' = tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';

SELECT tbool 't@2000-01-01' = tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' = tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' = tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' = tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';

SELECT tbool 't@2000-01-01' = tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' = tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' = tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' = tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' <> tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <> tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <> tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <> tbool 't@2000-01-01';

SELECT tbool 't@2000-01-01' <> tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <> tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <> tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <> tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';

SELECT tbool 't@2000-01-01' <> tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <> tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <> tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <> tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';

SELECT tbool 't@2000-01-01' <> tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <> tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <> tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <> tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' < tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' < tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' < tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' < tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' < tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' < tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' < tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' < tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' < tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' < tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' < tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' < tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' < tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' < tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' < tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' < tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' <= tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <= tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <= tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <= tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' <= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' <= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' <= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' <= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' <= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' <= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' > tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' > tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' > tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' > tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' > tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' > tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' > tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' > tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' > tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' > tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' > tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' > tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' > tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' > tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' > tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' > tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tbool 't@2000-01-01' >= tbool 't@2000-01-01';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' >= tbool 't@2000-01-01';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' >= tbool 't@2000-01-01';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' >= tbool 't@2000-01-01';
SELECT tbool 't@2000-01-01' >= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' >= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' >= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' >= tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}';
SELECT tbool 't@2000-01-01' >= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' >= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' >= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' >= tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]';
SELECT tbool 't@2000-01-01' >= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' >= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' >= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' >= tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}';

SELECT tint '1@2000-01-01' = tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' = tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' = tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' = tint '1@2000-01-01';
SELECT tint '1@2000-01-01' = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' = tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' = tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' = tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT tint '1@2000-01-01' <> tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <> tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <> tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <> tint '1@2000-01-01';
SELECT tint '1@2000-01-01' <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <> tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <> tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <> tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT tint '1@2000-01-01' < tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '1@2000-01-01';
SELECT tint '1@2000-01-01' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02}' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '{1@2000-01-01, 2@2000-01-02}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02]' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '[1@2000-01-01, 2@2000-01-02]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02]}' < tint '{[1@2000-01-01, 2@2000-01-02],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02],[3@2000-01-04, 3@2000-01-05]}' < tint '{[1@2000-01-01, 2@2000-01-02]}';

SELECT tint '1@2000-01-01' <= tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <= tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <= tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <= tint '1@2000-01-01';
SELECT tint '1@2000-01-01' <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' <= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT tint '1@2000-01-01' > tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' > tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' > tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' > tint '1@2000-01-01';
SELECT tint '1@2000-01-01' > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' > tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' > tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' > tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT tint '1@2000-01-01' >= tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' >= tint '1@2000-01-01';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' >= tint '1@2000-01-01';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' >= tint '1@2000-01-01';
SELECT tint '1@2000-01-01' >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' >= tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' >= tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' >= tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

SELECT tfloat '1.5@2000-01-01' = tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' = tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' = tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' = tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' = tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' = tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' = tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT tfloat '1.5@2000-01-01' <> tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <> tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <> tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <> tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <> tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <> tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <> tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT tfloat '1.5@2000-01-01' < tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' < tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' < tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' < tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' < tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' < tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' < tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT tfloat '[1@2000-01-01, 2@2000-01-03]' < tfloat '{[1@2000-01-01, 3@2000-01-03 ]}';

SELECT tfloat '1.5@2000-01-01' <= tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <= tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <= tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <= tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' <= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT tfloat '1.5@2000-01-01' > tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' > tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' > tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' > tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' > tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' > tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' > tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT tfloat '1.5@2000-01-01' >= tfloat '1.5@2000-01-01';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' >= tfloat '1.5@2000-01-01';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' >= tfloat '1.5@2000-01-01';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' >= tfloat '1.5@2000-01-01';
SELECT tfloat '1.5@2000-01-01' >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' >= tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}';
SELECT tfloat '1.5@2000-01-01' >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' >= tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]';
SELECT tfloat '1.5@2000-01-01' >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' >= tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}';

SELECT ttext 'AAA@2000-01-01' = ttext 'AAA@2000-01-01';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' = ttext 'AAA@2000-01-01';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' = ttext 'AAA@2000-01-01';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' = ttext 'AAA@2000-01-01';

SELECT ttext 'AAA@2000-01-01' = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' = ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';

SELECT ttext 'AAA@2000-01-01' = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' = ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';

SELECT ttext 'AAA@2000-01-01' = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' = ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

SELECT ttext 'AAA@2000-01-01' <> ttext 'AAA@2000-01-01';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <> ttext 'AAA@2000-01-01';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' <> ttext 'AAA@2000-01-01';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' <> ttext 'AAA@2000-01-01';

SELECT ttext 'AAA@2000-01-01' <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' <> ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}';

SELECT ttext 'AAA@2000-01-01' <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' <> ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]';

SELECT ttext 'AAA@2000-01-01' <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' <> ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}';

SELECT tfloat '{1@2000-01-01, 1@2000-01-02}' = tfloat '{[1@2000-01-01], [1@2000-01-02]}';

-------------------------------------------------------------------------------

SELECT tbool_hash(tbool 't@2000-01-01');
SELECT tbool_hash(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbool_hash(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbool_hash(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tint_hash(tint '1@2000-01-01');
SELECT tint_hash(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tint_hash(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tint_hash(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tfloat_hash(tfloat '1.5@2000-01-01');
SELECT tfloat_hash(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloat_hash(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloat_hash(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT ttext_hash(ttext 'AAA@2000-01-01');
SELECT ttext_hash(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttext_hash(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttext_hash(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

------------------------------------------------------------------------------
