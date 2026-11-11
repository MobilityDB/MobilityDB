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
