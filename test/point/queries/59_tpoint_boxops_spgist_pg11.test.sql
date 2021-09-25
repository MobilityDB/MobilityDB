-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
--
-- Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2021, PostGIS contributors
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

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

ALTER TABLE test_geoboundboxops ADD spgistidx BIGINT;

-------------------------------------------------------------------------------

CREATE INDEX tbl_tgeompoint_spgist_idx ON tbl_tgeompoint USING SPGIST(temp);
CREATE INDEX tbl_tgeogpoint_spgist_idx ON tbl_tgeogpoint USING SPGIST(temp);

-------------------------------------------------------------------------------
-- <type> op tgeompoint

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g && temp )
WHERE op = '&&' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g @> temp )
WHERE op = '@>' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g <@ temp )
WHERE op = '<@' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g -|- temp )
WHERE op = '-|-' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geometry, tbl_tgeompoint WHERE g ~= temp )
WHERE op = '~=' AND leftarg = 'geometry' AND rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeompoint WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeompoint WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeompoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeompoint WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tgeompoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_stbox, tbl_tgeompoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- <type> op tgeogpoint

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography, tbl_tgeogpoint WHERE g && temp )
WHERE op = '&&' AND leftarg = 'geogcollection' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography, tbl_tgeogpoint WHERE g @> temp )
WHERE op = '@>' AND leftarg = 'geogcollection' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography, tbl_tgeogpoint WHERE g <@ temp )
WHERE op = '<@' AND leftarg = 'geogcollection' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography, tbl_tgeogpoint WHERE g -|- temp )
WHERE op = '-|-' AND leftarg = 'geogcollection' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geography, tbl_tgeogpoint WHERE g ~= temp )
WHERE op = '~=' AND leftarg = 'geogcollection' AND rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t && temp )
WHERE op = '&&' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t @> temp )
WHERE op = '@>' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t <@ temp )
WHERE op = '<@' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t -|- temp )
WHERE op = '-|-' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestamptz, tbl_tgeogpoint WHERE t ~= temp )
WHERE op = '~=' AND leftarg = 'timestamptz' AND rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts && temp )
WHERE op = '&&' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts @> temp )
WHERE op = '@>' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts <@ temp )
WHERE op = '<@' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts -|- temp )
WHERE op = '-|-' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_timestampset, tbl_tgeogpoint WHERE ts ~= temp )
WHERE op = '~=' AND leftarg = 'timestampset' AND rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p && temp )
WHERE op = '&&' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p @> temp )
WHERE op = '@>' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p <@ temp )
WHERE op = '<@' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p -|- temp )
WHERE op = '-|-' AND leftarg = 'period' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_period, tbl_tgeogpoint WHERE p ~= temp )
WHERE op = '~=' AND leftarg = 'period' AND rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps && temp )
WHERE op = '&&' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps @> temp )
WHERE op = '@>' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps <@ temp )
WHERE op = '<@' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps -|- temp )
WHERE op = '-|-' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_periodset, tbl_tgeogpoint WHERE ps ~= temp )
WHERE op = '~=' AND leftarg = 'periodset' AND rightarg = 'tgeogpoint';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geodstbox, tbl_tgeogpoint WHERE b && temp )
WHERE op = '&&' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geodstbox, tbl_tgeogpoint WHERE b @> temp )
WHERE op = '@>' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geodstbox, tbl_tgeogpoint WHERE b <@ temp )
WHERE op = '<@' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geodstbox, tbl_tgeogpoint WHERE b -|- temp )
WHERE op = '-|-' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_geodstbox, tbl_tgeogpoint WHERE b ~= temp )
WHERE op = '~=' AND leftarg = 'stbox' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------
-- tgeompoint op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp && g )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp @> g )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp <@ g )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp -|- g )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_geometry WHERE temp ~= g )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'geometry';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint, tbl_stbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'stbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeompoint t1, tbl_tgeompoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeompoint' AND rightarg = 'tgeompoint';

-------------------------------------------------------------------------------
-- tgeogpoint op <type>

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geography WHERE temp && g )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geography WHERE temp @> g )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geography WHERE temp <@ g )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geography WHERE temp -|- g )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'geogcollection';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geography WHERE temp ~= g )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'geogcollection';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp && t )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp @> t )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp <@ t )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp -|- t )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestamptz WHERE temp ~= t )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'timestamptz';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp && ts )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp @> ts )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp <@ ts )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp -|- ts )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_timestampset WHERE temp ~= ts )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'timestampset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp && p )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp @> p )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp <@ p )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp -|- p )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'period';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_period WHERE temp ~= p )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'period';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp && ps )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp @> ps )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp <@ ps )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp -|- ps )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_periodset WHERE temp ~= ps )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'periodset';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geodstbox WHERE temp && b )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geodstbox WHERE temp @> b )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geodstbox WHERE temp <@ b )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geodstbox WHERE temp -|- b )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint, tbl_geodstbox WHERE temp ~= b )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'stbox';

UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp && t2.temp )
WHERE op = '&&' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp @> t2.temp )
WHERE op = '@>' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp <@ t2.temp )
WHERE op = '<@' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp -|- t2.temp )
WHERE op = '-|-' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';
UPDATE test_geoboundboxops
SET spgistidx = ( SELECT count(*) FROM tbl_tgeogpoint t1, tbl_tgeogpoint t2 WHERE t1.temp ~= t2.temp )
WHERE op = '~=' AND leftarg = 'tgeogpoint' AND rightarg = 'tgeogpoint';

-------------------------------------------------------------------------------

DROP INDEX IF EXISTS tbl_tgeompoint_spgist_idx;
DROP INDEX IF EXISTS tbl_tgeogpoint_spgist_idx;

-------------------------------------------------------------------------------

SELECT * FROM test_geoboundboxops
WHERE noidx <> spgistidx
ORDER BY op, leftarg, rightarg;

DROP TABLE test_geoboundboxops;

-------------------------------------------------------------------------------
