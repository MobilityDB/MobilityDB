-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
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
-- timeBucket
-------------------------------------------------------------------------------

SELECT timeBucket(tbool 't@2000-01-01', '2 days');
SELECT timeBucket(tbool '{t@2000-01-01, f@2000-01-02, t@2000-01-03}', '2 days');
SELECT timeBucket(tbool '[t@2000-01-01, f@2000-01-02, t@2000-01-03]', '2 days');
SELECT timeBucket(tbool '{[t@2000-01-01, f@2000-01-02, t@2000-01-03],[t@2000-01-04, t@2000-01-05]}', '2 days');
SELECT timeBucket(tint '1@2000-01-01', '2 days');
SELECT timeBucket(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', '2 days');
SELECT timeBucket(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', '2 days');
SELECT timeBucket(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', '2 days');
SELECT timeBucket(tfloat '1.5@2000-01-01', '2 days');
SELECT timeBucket(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', '2 days');
SELECT timeBucket(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '2 days');
SELECT timeBucket(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', '2 days');
SELECT timeBucket(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '2 days');
SELECT timeBucket(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', '2 days');
SELECT timeBucket(ttext 'AAA@2000-01-01', '2 days');
SELECT timeBucket(ttext '{AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03}', '2 days');
SELECT timeBucket(ttext '[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03]', '2 days');
SELECT timeBucket(ttext '{[AAA@2000-01-01, BBB@2000-01-02, AAA@2000-01-03],[CCC@2000-01-04, CCC@2000-01-05]}', '2 days');

-------------------------------------------------------------------------------
-- rangeBucket
-------------------------------------------------------------------------------

SELECT rangeBucket(tint '1@2000-01-01', 2);
SELECT rangeBucket(tint '{1@2000-01-01, 2@2000-01-02, 1@2000-01-03}', 2);
SELECT rangeBucket(tint '[1@2000-01-01, 2@2000-01-02, 1@2000-01-03]', 2);
SELECT rangeBucket(tint '{[1@2000-01-01, 2@2000-01-02, 1@2000-01-03],[3@2000-01-04, 3@2000-01-05]}', 2);
SELECT rangeBucket(tfloat '1.5@2000-01-01', 0.5);
SELECT rangeBucket(tfloat '{1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03}', 0.5);
SELECT rangeBucket(tfloat '[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5);
SELECT rangeBucket(tfloat 'Interp=Stepwise;[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03]', 0.5);
SELECT rangeBucket(tfloat '{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5);
SELECT rangeBucket(tfloat 'Interp=Stepwise;{[1.5@2000-01-01, 2.5@2000-01-02, 1.5@2000-01-03],[3.5@2000-01-04, 3.5@2000-01-05]}', 0.5);

-------------------------------------------------------------------------------
