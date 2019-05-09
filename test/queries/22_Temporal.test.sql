﻿-------------------------------------------------------------------------------
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

-------------------------------------------------------------------------------

-- Temporal instant set

SELECT tbool ' { true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 } ';
SELECT tbool '{true@2001-01-01 08:00:00,false@2001-01-01 08:05:00,true@2001-01-01 08:06:00}';
SELECT tint ' { 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 } ';
SELECT tint '{1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00}';
SELECT tfloat ' { 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 } ';
SELECT tfloat '{1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00}';
SELECT ttext ' { A@2001-01-01 08:00:00 , B@2001-01-01 08:05:00 , C@2001-01-01 08:06:00 } ';
SELECT ttext '{A@2001-01-01 08:00:00,B@2001-01-01 08:05:00,C@2001-01-01 08:06:00}';

-------------------------------------------------------------------------------

-- Temporal sequence

SELECT tbool ' [ true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 ] ';
SELECT tbool '[true@2001-01-01 08:00:00,false@2001-01-01 08:05:00,true@2001-01-01 08:06:00]';
SELECT tint ' [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ] ';
SELECT tint '[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT tfloat ' [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ] ';
SELECT tfloat '[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00]';
SELECT ttext ' [ A@2001-01-01 08:00:00 , B@2001-01-01 08:05:00 , C@2001-01-01 08:06:00 ] ';
SELECT ttext '[A@2001-01-01 08:00:00,B@2001-01-01 08:05:00,C@2001-01-01 08:06:00]';
/* Error */
SELECT tbool ' [ true@2001-01-01 08:00:00 , false@2001-01-01 08:05:00 , true@2001-01-01 08:06:00 ) ';

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

SELECT tfloat '  { [ 1@2001-01-01 08:00:00 , 2@2001-01-01 08:05:00 , 3@2001-01-01 08:06:00 ],
 [ 1@2001-01-01 09:00:00 , 2@2001-01-01 09:05:00 , 1@2001-01-01 09:06:00 ] } ';
SELECT tfloat '{[1@2001-01-01 08:00:00,2@2001-01-01 08:05:00,3@2001-01-01 08:06:00],
 [1@2001-01-01 09:00:00,2@2001-01-01 09:05:00,1@2001-01-01 09:06:00]}';

SELECT ttext '  { [ AAA@2001-01-01 08:00:00 , BBB@2001-01-01 08:05:00 , CCC@2001-01-01 08:06:00 ],
 [ AAA@2001-01-01 09:00:00 , BBB@2001-01-01 09:05:00 , CCC@2001-01-01 09:06:00 ] } ';
SELECT ttext '{[AAA@2001-01-01 08:00:00,BBB@2001-01-01 08:05:00,CCC@2001-01-01 08:06:00],
 [AAA@2001-01-01 09:00:00,BBB@2001-01-01 09:05:00,CCC@2001-01-01 09:06:00]}';

-------------------------------------------------------------------------------
-- typmod
-------------------------------------------------------------------------------

SELECT tbool 'true@2000-01-01';
SELECT tbool '{true@2000-01-01, false@2000-01-02}';
SELECT tbool '[true@2000-01-01, false@2000-01-02]';
SELECT tbool '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(Instant) 'true@2000-01-01';
SELECT tbool(InstantSet) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(Sequence) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(SequenceSet) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
/* Errors */
SELECT tbool(InstantSet) 'true@2000-01-01';
SELECT tbool(Sequence) 'true@2000-01-01';
SELECT tbool(SequenceSet) 'true@2000-01-01';
SELECT tbool(Instant) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(Sequence) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(SequenceSet) '{true@2000-01-01, false@2000-01-02}';
SELECT tbool(Instant) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(InstantSet) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(SequenceSet) '[true@2000-01-01, false@2000-01-02]';
SELECT tbool(Instant) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(InstantSet) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';
SELECT tbool(Sequence) '{[true@2000-01-01, false@2000-01-02], [true@2000-01-03, false@2000-01-04]}';

SELECT tint '1@2000-01-01';
SELECT tint '{1@2000-01-01, 2@2000-01-02}';
SELECT tint '[1@2000-01-01, 2@2000-01-02]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(Instant) '1@2000-01-01';
SELECT tint(InstantSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT tint(InstantSet) '1@2000-01-01';
SELECT tint(Sequence) '1@2000-01-01';
SELECT tint(SequenceSet) '1@2000-01-01';
SELECT tint(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tint(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(InstantSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tint(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(InstantSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tint(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';

SELECT tfloat '1@2000-01-01';
SELECT tfloat '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(Instant) '1@2000-01-01';
SELECT tfloat(InstantSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT tfloat(InstantSet) '1@2000-01-01';
SELECT tfloat(Sequence) '1@2000-01-01';
SELECT tfloat(SequenceSet) '1@2000-01-01';
SELECT tfloat(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT tfloat(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(InstantSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT tfloat(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(InstantSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT tfloat(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';

SELECT ttext '1@2000-01-01';
SELECT ttext '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(Instant) '1@2000-01-01';
SELECT ttext(InstantSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(Sequence) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(SequenceSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
/* Errors */
SELECT ttext(InstantSet) '1@2000-01-01';
SELECT ttext(Sequence) '1@2000-01-01';
SELECT ttext(SequenceSet) '1@2000-01-01';
SELECT ttext(Instant) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(Sequence) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(SequenceSet) '{1@2000-01-01, 2@2000-01-02}';
SELECT ttext(Instant) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(InstantSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(SequenceSet) '[1@2000-01-01, 2@2000-01-02]';
SELECT ttext(Instant) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(InstantSet) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
SELECT ttext(Sequence) '{[1@2000-01-01, 2@2000-01-02], [1@2000-01-03, 2@2000-01-04]}';
-------------------------------------------------------------------------------
-- Constructor functions
-------------------------------------------------------------------------------

SELECT tboolinst(TRUE, '2012-01-01 08:00:00');
SELECT tboolinst(NULL, '2012-01-01 08:00:00');
SELECT tintinst(1, '2012-01-01 08:00:00');
SELECT tintinst(NULL, '2012-01-01 08:00:00');
SELECT tfloatinst(1, '2012-01-01 08:00:00');
SELECT tfloatinst(NULL, '2012-01-01 08:00:00');
SELECT ttextinst('AAA', '2001-01-01 08:00:00');

-------------------------------------------------------------------------------

SELECT tbooli(ARRAY[
tboolinst(true, '2012-01-01 08:00:00'),
tboolinst(true, '2012-01-01 08:10:00'),
tboolinst(true, '2012-01-01 08:20:00')
]);
SELECT tinti(ARRAY[
tintinst(1, '2012-01-01 08:00:00'),
tintinst(2, '2012-01-01 08:10:00'),
tintinst(3, '2012-01-01 08:20:00')
]);
SELECT tfloati(ARRAY[
tfloatinst(1, '2012-01-01 08:00:00'),
tfloatinst(2, '2012-01-01 08:10:00'),
tfloatinst(3, '2012-01-01 08:20:00')
]);
SELECT ttexti(ARRAY[
ttextinst('A', '2012-01-01 08:00:00'),
ttextinst('B', '2012-01-01 08:10:00'),
ttextinst('C', '2012-01-01 08:20:00')
]);

-------------------------------------------------------------------------------

SELECT tboolseq(ARRAY[
tboolinst(true, '2012-01-01 08:00:00'),
tboolinst(true, '2012-01-01 08:10:00'),
tboolinst(true, '2012-01-01 08:20:00')
]);
SELECT tintseq(ARRAY[
tintinst(1, '2012-01-01 08:00:00'),
tintinst(2, '2012-01-01 08:10:00'),
tintinst(3, '2012-01-01 08:20:00')
]);
SELECT tfloatseq(ARRAY[
tfloatinst(1, '2012-01-01 08:00:00'),
tfloatinst(2, '2012-01-01 08:10:00'),
tfloatinst(3, '2012-01-01 08:20:00')
]);
SELECT ttextseq(ARRAY[
ttextinst('A', '2012-01-01 08:00:00'),
ttextinst('B', '2012-01-01 08:10:00'),
ttextinst('C', '2012-01-01 08:20:00')
]);

-------------------------------------------------------------------------------

SELECT tbools(ARRAY[
tboolseq(ARRAY[
tboolinst(true, '2012-01-01 08:00:00'),
tboolinst(false, '2012-01-01 08:10:00'),
tboolinst(true, '2012-01-01 08:20:00')
]),
tboolseq(ARRAY[
tboolinst(true, '2012-01-01 09:00:00'),
tboolinst(false, '2012-01-01 09:10:00'),
tboolinst(true, '2012-01-01 09:20:00')
])]);
SELECT tints(ARRAY[
tintseq(ARRAY[
tintinst(1, '2012-01-01 08:00:00'),
tintinst(2, '2012-01-01 08:10:00'),
tintinst(3, '2012-01-01 08:20:00')
]),
tintseq(ARRAY[
tintinst(1, '2012-01-01 09:00:00'),
tintinst(2, '2012-01-01 09:10:00'),
tintinst(1, '2012-01-01 09:20:00')
])]);
SELECT tfloats(ARRAY[
tfloatseq(ARRAY[
tfloatinst(1, '2012-01-01 08:00:00'),
tfloatinst(2, '2012-01-01 08:10:00'),
tfloatinst(3, '2012-01-01 08:20:00')
]),
tfloatseq(ARRAY[
tfloatinst(1, '2012-01-01 09:00:00'),
tfloatinst(2, '2012-01-01 09:10:00'),
tfloatinst(1, '2012-01-01 09:20:00')
])]);
SELECT ttexts(ARRAY[
ttextseq(ARRAY[
ttextinst('A', '2012-01-01 08:00:00'),
ttextinst('B', '2012-01-01 08:10:00'),
ttextinst('C', '2012-01-01 08:20:00')
]),
ttextseq(ARRAY[
ttextinst('A', '2012-01-01 09:00:00'),
ttextinst('B', '2012-01-01 09:10:00'),
ttextinst('C', '2012-01-01 09:20:00')
])]);

-------------------------------------------------------------------------------
-- Cast functions
-------------------------------------------------------------------------------

SELECT tfloat(tint '1@2001-01-01');
SELECT tfloat(tint '{1@2001-01-01, 2@2001-01-02}');
SELECT tfloat(tint '[1@2001-01-01, 1@2001-01-02]');
select tfloat(tint '[1@2001-01-01, 2@2001-01-02, 2@2001-01-03]');
select tfloat(tint '[1@2001-01-01, 2@2001-01-02, 1@2001-01-03]');
SELECT tfloat(tint '{[1@2001-01-01, 1@2001-01-02], [2@2001-01-03, 2@2001-01-04]}');

-------------------------------------------------------------------------------
-- Transformation functions
-------------------------------------------------------------------------------

SELECT tboolinst(tbool 't@2000-01-01');
SELECT tbooli(tbool 't@2000-01-01');
SELECT tbooli(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tboolseq(tbool 't@2000-01-01');
SELECT tboolseq(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbools(tbool 't@2000-01-01');
SELECT tbools(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tbools(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbools(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
/* Errors */
SELECT tboolinst(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tboolinst(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tboolinst(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tbooli(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT tbooli(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT tboolseq(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT tboolseq(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');

SELECT tintinst(tint '1@2000-01-01');
SELECT tinti(tint '1@2000-01-01');
SELECT tinti(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tintseq(tint '1@2000-01-01');
SELECT tintseq(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tints(tint '1@2000-01-01');
SELECT tints(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tints(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tints(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
/* Errors */
SELECT tintinst(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tintinst(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tintinst(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tinti(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT tinti(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT tintseq(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT tintseq(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT tfloatinst(tfloat '1.5@2000-01-01');
SELECT tfloati(tfloat '1.5@2000-01-01');
SELECT tfloati(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloatseq(tfloat '1.5@2000-01-01');
SELECT tfloatseq(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloats(tfloat '1.5@2000-01-01');
SELECT tfloats(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloats(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloats(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
/* Errors */
SELECT tfloatinst(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloatinst(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloatinst(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tfloati(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT tfloati(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT tfloatseq(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT tfloatseq(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT ttextinst(ttext 'AAA@2000-01-01');
SELECT ttexti(ttext 'AAA@2000-01-01');
SELECT ttexti(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttextseq(ttext 'AAA@2000-01-01');
SELECT ttextseq(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttexts(ttext 'AAA@2000-01-01');
SELECT ttexts(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttexts(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttexts(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT ttextinst(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttextinst(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttextinst(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT ttexti(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT ttexti(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
SELECT ttextseq(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT ttextseq(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

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
SELECT appendInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', tfloat '1.5@2000-01-06');
SELECT appendInstant(ttext 'AAA@2000-01-01', ttext 'AAA@2000-01-02');
SELECT appendInstant(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ttext 'AAA@2000-01-04');
SELECT appendInstant(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ttext 'AAA@2000-01-04');
SELECT appendInstant(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ttext 'AAA@2000-01-06');

-------------------------------------------------------------------------------
-- Accessor functions
-------------------------------------------------------------------------------

SELECT temporalType(tbool 't@2000-01-01');
SELECT temporalType(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT temporalType(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT temporalType(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT temporalType(tint '1@2000-01-01');
SELECT temporalType(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT temporalType(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT temporalType(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT temporalType(tfloat '1.5@2000-01-01');
SELECT temporalType(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT temporalType(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT temporalType(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT temporalType(ttext 'AAA@2000-01-01');
SELECT temporalType(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT temporalType(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT temporalType(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

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
SELECT memSize(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT memSize(ttext 'AAA@2000-01-01');
SELECT memSize(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT memSize(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT memSize(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

/*
SELECT period(tbool 't@2000-01-01');
SELECT box(tint '1@2000-01-01');
SELECT box(tfloat '1.5@2000-01-01');
SELECT period(ttext 'AAA@2000-01-01');
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
SELECT getValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT getValues(ttext 'AAA@2000-01-01');
SELECT getValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT getValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT getValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT valueRange(tint '1@2000-01-01');
SELECT valueRange(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT valueRange(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT valueRange(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT valueRange(tfloat '1.5@2000-01-01');
SELECT valueRange(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT valueRange(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT valueRange(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

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
SELECT startValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT endValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT minValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minValue(ttext 'AAA@2000-01-01');
SELECT minValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT maxValue(tint '1@2000-01-01');
SELECT maxValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT maxValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT maxValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT maxValue(tfloat '1.5@2000-01-01');
SELECT maxValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT maxValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT maxValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT maxValue(ttext 'AAA@2000-01-01');
SELECT maxValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT maxValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT maxValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

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
SELECT getTime(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT timespan(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timespan(ttext 'AAA@2000-01-01');
SELECT timespan(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT timespan(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT timespan(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT numSequences(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT numSequences(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT numSequences(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numSequences(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT numSequences(tbool 't@2000-01-01');
SELECT numSequences(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT numSequences(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT numSequences(tint '1@2000-01-01');
SELECT numSequences(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT numSequences(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT numSequences(tfloat '1.5@2000-01-01');
SELECT numSequences(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT numSequences(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT numSequences(ttext 'AAA@2000-01-01');
SELECT numSequences(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT numSequences(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');

SELECT startSequence(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT startSequence(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT startSequence(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT startSequence(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT startSequence(tbool 't@2000-01-01');
SELECT startSequence(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT startSequence(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT startSequence(tint '1@2000-01-01');
SELECT startSequence(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT startSequence(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT startSequence(tfloat '1.5@2000-01-01');
SELECT startSequence(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT startSequence(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT startSequence(ttext 'AAA@2000-01-01');
SELECT startSequence(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT startSequence(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');

SELECT endSequence(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT endSequence(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT endSequence(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT endSequence(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT endSequence(tbool 't@2000-01-01');
SELECT endSequence(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT endSequence(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT endSequence(tint '1@2000-01-01');
SELECT endSequence(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT endSequence(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT endSequence(tfloat '1.5@2000-01-01');
SELECT endSequence(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT endSequence(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT endSequence(ttext 'AAA@2000-01-01');
SELECT endSequence(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT endSequence(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');

SELECT sequenceN(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', 1);
SELECT sequenceN(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT sequenceN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT sequenceN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);
/* Errors */
SELECT sequenceN(tbool 't@2000-01-01', 1);
SELECT sequenceN(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', 1);
SELECT sequenceN(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', 1);
SELECT sequenceN(tint '1@2000-01-01', 1);
SELECT sequenceN(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT sequenceN(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT sequenceN(tfloat '1.5@2000-01-01', 1);
SELECT sequenceN(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1);
SELECT sequenceN(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1);
SELECT sequenceN(ttext 'AAA@2000-01-01', 1);
SELECT sequenceN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);
SELECT sequenceN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);

SELECT sequences(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}');
SELECT sequences(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT sequences(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT sequences(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');
/* Errors */
SELECT sequences(tbool 't@2000-01-01');
SELECT sequences(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}');
SELECT sequences(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]');
SELECT sequences(tint '1@2000-01-01');
SELECT sequences(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT sequences(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT sequences(tfloat '1.5@2000-01-01');
SELECT sequences(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT sequences(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT sequences(ttext 'AAA@2000-01-01');
SELECT sequences(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT sequences(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');

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
SELECT numInstants(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numInstants(ttext 'AAA@2000-01-01');
SELECT numInstants(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT numInstants(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT numInstants(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

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
SELECT startInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT endInstant(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT instantN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT instantN(ttext 'AAA@2000-01-01', 1);
SELECT instantN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);
SELECT instantN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT instantN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);

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
SELECT instants(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT numTimestamps(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT numTimestamps(ttext 'AAA@2000-01-01');
SELECT numTimestamps(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT numTimestamps(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT numTimestamps(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

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
SELECT startTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT endTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT timestampN(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1);
SELECT timestampN(ttext 'AAA@2000-01-01', 1);
SELECT timestampN(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 1);
SELECT timestampN(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 1);
SELECT timestampN(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 1);

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
SELECT timestamps(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT timestamps(ttext 'AAA@2000-01-01');
SELECT timestamps(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT timestamps(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT timestamps(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT tbool 't@2000-01-01' &= true;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' &= true;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' &= true;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' &= true;
SELECT tint '1@2000-01-01' &= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' &= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' &= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' &= 1;
SELECT tfloat '1.5@2000-01-01' &= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' &= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' &= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' &= 1.5;
SELECT ttext 'AAA@2000-01-01' &= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' &= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' &= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' &= 'AAA';

SELECT tbool 't@2000-01-01' @= true;
SELECT tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}' @= true;
SELECT tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]' @= true;
SELECT tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}' @= true;
SELECT tint '1@2000-01-01' @= 1;
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' @= 1;
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' @= 1;
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' @= 1;
SELECT tfloat '1.5@2000-01-01' @= 1.5;
SELECT tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}' @= 1.5;
SELECT tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]' @= 1.5;
SELECT tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}' @= 1.5;
SELECT ttext 'AAA@2000-01-01' @= 'AAA';
SELECT ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}' @= 'AAA';
SELECT ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]' @= 'AAA';
SELECT ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}' @= 'AAA';
 
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
SELECT shift(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '5 min');
SELECT shift(ttext 'AAA@2000-01-01', '5 min');
SELECT shift(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '5 min');
SELECT shift(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '5 min');
SELECT shift(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '5 min');

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

SELECT atValue(tbool 't@2000-01-01', true);
SELECT atValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', true);
SELECT atValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', true);
SELECT atValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', true);
SELECT atValue(tint '1@2000-01-01', 1);
SELECT atValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT atValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT atValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT atValue(tfloat '1.5@2000-01-01', 1.5);
SELECT atValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1.5);
SELECT atValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1.5);
SELECT atValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1.5);
SELECT atValue(ttext 'AAA@2000-01-01', 'AAA');
SELECT atValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 'AAA');
SELECT atValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 'AAA');
SELECT atValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 'AAA');

SELECT minusValue(tbool 't@2000-01-01', true);
SELECT minusValue(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', true);
SELECT minusValue(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', true);
SELECT minusValue(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', true);
SELECT minusValue(tint '1@2000-01-01', 1);
SELECT minusValue(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 1);
SELECT minusValue(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 1);
SELECT minusValue(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 1);
SELECT minusValue(tfloat '1.5@2000-01-01', 1.5);
SELECT minusValue(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 1.5);
SELECT minusValue(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 1.5);
SELECT minusValue(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 1.5);
SELECT minusValue(ttext 'AAA@2000-01-01', 'AAA');
SELECT minusValue(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', 'AAA');
SELECT minusValue(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', 'AAA');
SELECT minusValue(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', 'AAA');

SELECT atValues(tbool 't@2000-01-01', ARRAY[true]);
SELECT atValues(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', ARRAY[true]);
SELECT atValues(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', ARRAY[true]);
SELECT atValues(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', ARRAY[true]);
SELECT atValues(tint '1@2000-01-01', ARRAY[1]);
SELECT atValues(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[1]);
SELECT atValues(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[1]);
SELECT atValues(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[1]);
SELECT atValues(tfloat '1.5@2000-01-01', ARRAY[1.5]);
SELECT atValues(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[1.5]);
SELECT atValues(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5]);
SELECT atValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5]);
SELECT atValues(ttext 'AAA@2000-01-01', ARRAY[text 'AAA']);
SELECT atValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ARRAY[text 'AAA']);
SELECT atValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ARRAY[text 'AAA']);
SELECT atValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ARRAY[text 'AAA']);

SELECT minusValues(tbool 't@2000-01-01', ARRAY[true]);
SELECT minusValues(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', ARRAY[true]);
SELECT minusValues(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', ARRAY[true]);
SELECT minusValues(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', ARRAY[true]);
SELECT minusValues(tint '1@2000-01-01', ARRAY[1]);
SELECT minusValues(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[1]);
SELECT minusValues(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[1]);
SELECT minusValues(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[1]);
SELECT minusValues(tfloat '1.5@2000-01-01', ARRAY[1.5]);
SELECT minusValues(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[1.5]);
SELECT minusValues(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[1.5]);
SELECT minusValues(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[1.5]);
SELECT minusValues(ttext 'AAA@2000-01-01', ARRAY[text 'AAA']);
SELECT minusValues(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', ARRAY[text 'AAA']);
SELECT minusValues(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', ARRAY[text 'AAA']);
SELECT minusValues(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', ARRAY[text 'AAA']);

SELECT atRange(tint '1@2000-01-01', intrange '[1,3]');
SELECT atRange(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', intrange '[1,3]');
SELECT atRange(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', intrange '[1,3]');
SELECT atRange(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', intrange '[1,3]');
SELECT atRange(tfloat '1.5@2000-01-01', floatrange '[1,3]');
SELECT atRange(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', floatrange '[1,3]');
SELECT atRange(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatrange '[1,3]');
SELECT atRange(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatrange '[1,3]');

SELECT minusRange(tint '1@2000-01-01', intrange '[1,3]');
SELECT minusRange(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', intrange '[1,3]');
SELECT minusRange(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', intrange '[1,3]');
SELECT minusRange(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', intrange '[1,3]');
SELECT minusRange(tfloat '1.5@2000-01-01', floatrange '[1,3]');
SELECT minusRange(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', floatrange '[1,3]');
SELECT minusRange(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', floatrange '[1,3]');
SELECT minusRange(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', floatrange '[1,3]');

SELECT atRanges(tint '1@2000-01-01', ARRAY[intrange '[1,3]']);
SELECT atRanges(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intrange '[1,3]']);
SELECT atRanges(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intrange '[1,3]']);
SELECT atRanges(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intrange '[1,3]']);
SELECT atRanges(tfloat '1.5@2000-01-01', ARRAY[floatrange '[1,3]']);
SELECT atRanges(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatrange '[1,3]']);
SELECT atRanges(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatrange '[1,3]']);
SELECT atRanges(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatrange '[1,3]']);

SELECT minusRanges(tint '1@2000-01-01', ARRAY[intrange '[1,3]']);
SELECT minusRanges(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', ARRAY[intrange '[1,3]']);
SELECT minusRanges(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', ARRAY[intrange '[1,3]']);
SELECT minusRanges(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', ARRAY[intrange '[1,3]']);
SELECT minusRanges(tfloat '1.5@2000-01-01', ARRAY[floatrange '[1,3]']);
SELECT minusRanges(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', ARRAY[floatrange '[1,3]']);
SELECT minusRanges(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', ARRAY[floatrange '[1,3]']);
SELECT minusRanges(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', ARRAY[floatrange '[1,3]']);

SELECT atMin(tint '1@2000-01-01');
SELECT atMin(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT atMin(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT atMin(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT atMin(tfloat '1.5@2000-01-01');
SELECT atMin(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT atMin(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT atMin(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMin(ttext 'AAA@2000-01-01');
SELECT atMin(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT atMin(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT atMin(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

select atMin(tfloat '(1@2000-01-01, 2@2000-01-02)');
select atMin(tfloat '(1@2000-01-01, 2@2000-01-02, 1@2000-01-03)');
select atMin(tfloat '{(1@2000-01-01, 2@2000-01-02)}');
select atMin(tfloat '{(1@2000-01-01, 2@2000-01-02, 1@2000-01-03)}');
SELECT atMin(tfloat '{[2@2012-01-01, 1@2012-01-03), (1@2012-01-03, 1@2012-01-05)}');
SELECT atMin(tfloat '{[2@2012-01-01, 1@2012-01-03), (1@2012-01-03, 2@2012-01-05)}');

SELECT minusMin(tint '1@2000-01-01');
SELECT minusMin(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minusMin(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minusMin(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minusMin(tfloat '1.5@2000-01-01');
SELECT minusMin(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minusMin(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMin(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
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
SELECT atMax(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT atMax(ttext 'AAA@2000-01-01');
SELECT atMax(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT atMax(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT atMax(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

select atMax(tfloat '(1@2000-01-01, 2@2000-01-02)');
select atMax(tfloat '(2@2000-01-01, 1@2000-01-02, 2@2000-01-03)');
select atMax(tfloat '{(1@2000-01-01, 2@2000-01-02)}');
select atMax(tfloat '{(2@2000-01-01, 1@2000-01-02, 2@2000-01-03)}');
SELECT atMax(tfloat '{[1@2012-01-01, 3@2012-01-03), (3@2012-01-03, 1@2012-01-05)}');

SELECT minusMax(tint '1@2000-01-01');
SELECT minusMax(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT minusMax(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT minusMax(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');
SELECT minusMax(tfloat '1.5@2000-01-01');
SELECT minusMax(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT minusMax(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT minusMax(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');
SELECT minusMax(ttext 'AAA@2000-01-01');
SELECT minusMax(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}');
SELECT minusMax(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]');
SELECT minusMax(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}');

SELECT atTimestamp(tbool 't@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamp '2000-01-01');
SELECT atTimestamp(tint '1@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamp '2000-01-01');
SELECT atTimestamp(tfloat '1.5@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamp '2000-01-01');
SELECT atTimestamp(ttext 'AAA@2000-01-01', timestamp '2000-01-01');
SELECT atTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamp '2000-01-01');
SELECT atTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamp '2000-01-01');
SELECT atTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamp '2000-01-01');

SELECT valueAtTimestamp(tbool 't@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tint '1@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tfloat '1.5@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamp '2000-01-01');
SELECT valueAtTimestamp(ttext 'AAA@2000-01-01', timestamp '2000-01-01');
SELECT valueAtTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamp '2000-01-01');
SELECT valueAtTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamp '2000-01-01');
SELECT valueAtTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamp '2000-01-01');

SELECT minusTimestamp(tbool 't@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamp '2000-01-01');
SELECT minusTimestamp(tint '1@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamp '2000-01-01');
SELECT minusTimestamp(tfloat '1.5@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamp '2000-01-01');
SELECT minusTimestamp(ttext 'AAA@2000-01-01', timestamp '2000-01-01');
SELECT minusTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamp '2000-01-01');
SELECT minusTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamp '2000-01-01');
SELECT minusTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamp '2000-01-01');

SELECT atTimestampSet(tbool 't@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '1@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '1.5@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext 'AAA@2000-01-01', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestampset '{2000-01-01}');
SELECT atTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestampset '{2000-01-01}');

select atTimestampSet(tfloat '[1@2000-01-02, 2@2000-01-04, 1@2000-01-05]',
  timestampset '{2000-01-01, 2000-01-02, 2000-01-03, 2000-01-04, 2000-01-05, 2000-01-06}');
select atTimestampSet(tfloat '(1@2000-01-02, 2@2000-01-04, 1@2000-01-05)',
  timestampset '{2000-01-01, 2000-01-02, 2000-01-03, 2000-01-04, 2000-01-05, 2000-01-06}');

SELECT minusTimestampSet(tbool 't@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '1@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '1.5@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext 'AAA@2000-01-01', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestampset '{2000-01-01}');
SELECT minusTimestampSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestampset '{2000-01-01}');

SELECT atPeriod(tbool 't@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '1@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '1.5@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext 'AAA@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT atPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT minusPeriod(tbool 't@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '1@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '1.5@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext 'AAA@2000-01-01', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', period '[2000-01-01,2000-01-02]');
SELECT minusPeriod(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', period '[2000-01-01,2000-01-02]');

SELECT atPeriodSet(tbool 't@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '1@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '1.5@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext 'AAA@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT atPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT minusPeriodSet(tbool 't@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '1@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '1.5@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext 'AAA@2000-01-01', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', periodset '{[2000-01-01,2000-01-02]}');
SELECT minusPeriodSet(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', periodset '{[2000-01-01,2000-01-02]}');

SELECT intersectsTimestamp(tbool 't@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tint '1@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tfloat '1.5@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', timestamp '2000-01-01');
SELECT intersectsTimestamp(ttext 'AAA@2000-01-01', timestamp '2000-01-01');
SELECT intersectsTimestamp(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', timestamp '2000-01-01');
SELECT intersectsTimestamp(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', timestamp '2000-01-01');
SELECT intersectsTimestamp(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', timestamp '2000-01-01');

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
SELECT integral(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT integral(tfloat '1.5@2000-01-01');
SELECT integral(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT integral(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT integral(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

SELECT twAvg(tint '1@2000-01-01');
SELECT twAvg(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}');
SELECT twAvg(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]');
SELECT twAvg(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}');

SELECT twAvg(tfloat '1.5@2000-01-01');
SELECT twAvg(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}');
SELECT twAvg(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]');
SELECT twAvg(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}');

-------------------------------------------------------------------------------
-- Comparison functions and B-tree indexing
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
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}';
SELECT tint '1@2000-01-01' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]';
SELECT tint '1@2000-01-01' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';
SELECT tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}' < tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}';

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

------------------------------------------------------------------------------
