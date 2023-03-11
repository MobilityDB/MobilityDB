﻿-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2023, PostGIS contributors
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

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' << tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

/* Errors */
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << stbox 'SRID=4326;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' >> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &< tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<| tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' |&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tstzspan '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' <<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tstzspan '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #>> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tstzspan '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' &<# tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------

SELECT tstzspan '[2000-01-01,2000-01-02]' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tstzspan '[2000-01-01,2000-01-02]' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tstzspan '[2000-01-01,2000-01-02]' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tstzspan '[2000-01-01,2000-01-02]' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tstzspan '[2000-01-01,2000-01-02]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tstzspan '[2000-01-01,2000-01-02]';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> stbox 'SRID=5676;STBOX XT(((1.0,1.0),(2.0,2.0)),[2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{NPoint(1,0.5)@2000-01-01, NPoint(2,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03]' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';
SELECT tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}' #&> tnpoint '{[NPoint(1,0.4)@2000-01-01, NPoint(1,0.5)@2000-01-02, NPoint(1,0.7)@2000-01-03],[Npoint(3,0.5)@2000-01-04, NPoint(3,0.5)@2000-01-05]}';

-------------------------------------------------------------------------------
/* Errors */

SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' << tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' &< tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' >> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' &> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' <<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' &<| tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' |>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX T([2000-01-01,2000-01-02])' |&> tnpoint 'NPoint(1,0.5)@2000-01-01';

SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' <<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' &<# tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #>> tnpoint 'NPoint(1,0.5)@2000-01-01';
SELECT stbox 'STBOX X((1.0,1.0),(2.0,2.0))' #&> tnpoint 'NPoint(1,0.5)@2000-01-01';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' << stbox 'STBOX T([2000-01-01,2000-01-02])';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &< stbox 'STBOX T([2000-01-01,2000-01-02])';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' >>  stbox 'STBOX T([2000-01-01,2000-01-02])' ;
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &>  stbox 'STBOX T([2000-01-01,2000-01-02])';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<| stbox 'STBOX T([2000-01-01,2000-01-02])';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<| stbox 'STBOX T([2000-01-01,2000-01-02])';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01'|>>  stbox 'STBOX T([2000-01-01,2000-01-02])' ;
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' |&>  stbox 'STBOX T([2000-01-01,2000-01-02])';

SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' <<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' &<# stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #>> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';
SELECT tnpoint 'NPoint(1,0.5)@2000-01-01' #&> stbox 'STBOX X((1.0,1.0),(2.0,2.0))';

-------------------------------------------------------------------------------

