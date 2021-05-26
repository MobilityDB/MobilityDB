-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #= npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #= npoint 'Npoint(1, 0.5)';

SELECT npoint 'Npoint(1, 0.5)' #= tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT npoint 'Npoint(1, 0.5)' #= tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #= tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #= tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #= tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #= tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #= tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #= tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #= tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #= tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #= tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #= tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #= tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #= tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #= tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #= tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #= tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #= tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #<> npoint 'Npoint(1, 0.5)';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #<> npoint 'Npoint(1, 0.5)';

SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT npoint 'Npoint(1, 0.5)' #<> tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #<> tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #<> tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #<> tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint 'Npoint(1, 0.5)@2000-01-01' #<> tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #<> tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #<> tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #<> tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}' #<> tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #<> tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #<> tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #<> tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]' #<> tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #<> tnpoint 'Npoint(1, 0.5)@2000-01-01';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #<> tnpoint '{Npoint(1, 0.3)@2000-01-01, Npoint(1, 0.5)@2000-01-02, Npoint(1, 0.5)@2000-01-03}';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #<> tnpoint '[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03]';
SELECT tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}' #<> tnpoint '{[Npoint(1, 0.2)@2000-01-01, Npoint(1, 0.4)@2000-01-02, Npoint(1, 0.5)@2000-01-03], [Npoint(2, 0.6)@2000-01-04, Npoint(2, 0.6)@2000-01-05]}';

-------------------------------------------------------------------------------/
