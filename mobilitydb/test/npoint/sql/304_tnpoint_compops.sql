-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #= npoint 'Npoint(1, 0.5)';

SELECT npoint 'Npoint(1, 0.5)' #= tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #= tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #= tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #= tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #= tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #= tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #= tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #= tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #= tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #= tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #= tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #= tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #= tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #= tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #<> npoint 'Npoint(1, 0.5)';

SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #<> tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #<> tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #<> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' #<> tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #<> tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #<> tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #<> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}' #<> tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #<> tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #<> tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #<> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]' #<> tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #<> tnpoint 'Npoint(1, 0.5)@2001-01-01';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #<> tnpoint '{Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.5)@2001-01-02, Npoint(1, 0.5)@2001-01-03}';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #<> tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03]';
SELECT tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}' #<> tnpoint '{[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.4)@2001-01-02, Npoint(1, 0.5)@2001-01-03], [Npoint(2, 0.6)@2001-01-04, Npoint(2, 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box

SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]' ?&& stbox(tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]');
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]' %&& stbox(tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]');
SELECT stbox 'STBOX T([2001-01-05, 2001-01-06])' ?&& tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]' %&& stbox 'STBOX T([2001-01-02, 2001-01-06])';
/* Errors */
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' ?&& stbox 'STBOX X((0,0),(1,1))';

-- Ever and always overlaps: two values, at one place of the map where the
-- points of their routes coincide, as ST_Equals compares them, a crossing of
-- two routes included, where = and <> compare the route and the fraction

-- Routes 0 and 1 cross: the fractions the cast of the crossing point gives
SELECT tnpoint 'Npoint(0, 0.676259981511564)@2001-01-01' ?&& tnpoint 'Npoint(1, 0.3501533481390591)@2001-01-01';
SELECT tnpoint 'Npoint(0, 0.676259981511564)@2001-01-01' ?= tnpoint 'Npoint(1, 0.3501533481390591)@2001-01-01';
SELECT tnpoint 'Npoint(0, 0.676259981511564)@2001-01-01' ?&& tnpoint 'Npoint(1, 0.36)@2001-01-01';
-- Two positions of route 1 a hundred-millionth of a millionth of its length
-- apart are two places of the map
SELECT tnpoint 'Npoint(1, 0.5)@2001-01-01' ?&& tnpoint 'Npoint(1, 0.50000000000001)@2001-01-01';
-- Passing the crossing together, and an hour apart
SELECT tnpoint '[Npoint(0, 0.6)@2001-01-01, Npoint(0, 0.676259981511564)@2001-01-02, Npoint(0, 0.7)@2001-01-03]' ?&& tnpoint '[Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.3501533481390591)@2001-01-02, Npoint(1, 0.4)@2001-01-03]';
SELECT tnpoint '[Npoint(0, 0.6)@2001-01-01, Npoint(0, 0.676259981511564)@2001-01-02, Npoint(0, 0.7)@2001-01-03]' ?&& tnpoint '[Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.3501533481390591)@2001-01-02 01:00, Npoint(1, 0.4)@2001-01-03]';
-- One route: opposite directions meet once, one behind the other never
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.8)@2001-01-03]' ?&& tnpoint '[Npoint(1, 0.8)@2001-01-01, Npoint(1, 0.2)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.8)@2001-01-03]' %&& tnpoint '[Npoint(1, 0.8)@2001-01-01, Npoint(1, 0.2)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]' ?&& tnpoint '[Npoint(1, 0.3)@2001-01-01, Npoint(1, 0.7)@2001-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]' %&& tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-03]';
-- Values sharing no time
SELECT tnpoint '[Npoint(1, 0.2)@2001-01-01, Npoint(1, 0.6)@2001-01-02]' ?&& tnpoint '[Npoint(1, 0.2)@2001-01-03, Npoint(1, 0.6)@2001-01-04]';
