-------------------------------------------------------------------------------
-- Temporal equal
-------------------------------------------------------------------------------

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-- A segment passing through the circular buffer inside it
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)' #= cbuffer 'Cbuffer(Point(1 1), 0.3)';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.3)' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)';
SELECT tcbuffer '[Cbuffer(Point(0.1 1), 0.5)@2001-01-01, Cbuffer(Point(0.4 1), 0.5)@2001-01-04)' #= cbuffer 'Cbuffer(Point(0.3 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(0.2 0.2), 0.2)@2001-01-01, Cbuffer(Point(0.4 0.4), 0.4)@2001-01-03)' #= cbuffer 'Cbuffer(Point(0.3 0.3), 0.3)';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #= tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-------------------------------------------------------------------------------
-- Temporal not equal
-------------------------------------------------------------------------------

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> cbuffer 'Cbuffer(Point(1 1), 0.5)';

SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.5)' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

-- A segment passing through the circular buffer inside it
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)' #<> cbuffer 'Cbuffer(Point(1 1), 0.3)';
SELECT cbuffer 'Cbuffer(Point(1 1), 0.3)' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-03)';
SELECT tcbuffer '[Cbuffer(Point(0.1 1), 0.5)@2001-01-01, Cbuffer(Point(0.4 1), 0.5)@2001-01-04)' #<> cbuffer 'Cbuffer(Point(0.3 1), 0.5)';
SELECT tcbuffer '[Cbuffer(Point(0.2 0.2), 0.2)@2001-01-01, Cbuffer(Point(0.4 0.4), 0.4)@2001-01-03)' #<> cbuffer 'Cbuffer(Point(0.3 0.3), 0.3)';

SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer 'Cbuffer(Point(1 1), 0.5)@2001-01-01';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '{Cbuffer(Point(1 1), 0.3)@2001-01-01, Cbuffer(Point(1 1), 0.5)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03}';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}' #<> tcbuffer '{[Cbuffer(Point(1 1), 0.2)@2001-01-01, Cbuffer(Point(1 1), 0.4)@2001-01-02, Cbuffer(Point(1 1), 0.5)@2001-01-03], [Cbuffer(Point(2 2), 0.6)@2001-01-04, Cbuffer(Point(2 2), 0.6)@2001-01-05]}';

/* Errors */
SELECT tcbuffer 'Cbuffer(Point(1 1),1)@2001-01-01' #= tcbuffer 'SRID=3812;Cbuffer(Point(1 1),1)@2001-01-01';

-------------------------------------------------------------------------------/

-------------------------------------------------------------------------------

-- Ever and always overlaps: a value and a spatiotemporal box

SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' && stbox 'STBOX XT(((8,0),(10,2)),[2001-01-01 00:00, 2001-01-01 10:00])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' ?&& stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 04:00, 2001-01-01 06:00])';
SELECT stbox 'STBOX XT(((4,4),(6,6)),[2001-01-01 08:00, 2001-01-01 10:00])' ?&& tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' %&& stbox 'STBOX X((-1,-1),(11,11))';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01 00:00, Cbuffer(Point(10 10), 1)@2001-01-01 10:00]' %&& stbox 'STBOX X((0,0),(10,10))';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(10 10), 1)@2001-01-02]' ?&& stbox 'STBOX T([2001-01-03, 2001-01-04])';
SELECT tcbuffer '[Cbuffer(Point(0 0), 1)@2001-01-01, Cbuffer(Point(10 10), 1)@2001-01-02]' %&& stbox 'STBOX T([2001-01-01, 2001-01-04])';
/* Errors */
SELECT tcbuffer 'Cbuffer(Point(0 0), 1)@2001-01-01' ?&& stbox 'SRID=4326;STBOX X((0,0),(1,1))';

-- Ever and always overlaps: two values, whose boxes are compared between
-- their instants too, where two moving buffers may overlap for a moment

-- Two buffers passing through each other between two instants
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]';
-- Boxes touching at one moment between two instants, and missing by 1/2
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(2 -6),1)@2001-01-01, Cbuffer(Point(2 6),1)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(2.5 -6),1)@2001-01-01, Cbuffer(Point(2.5 6),1)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(500000 6000000),1)@2001-01-01, Cbuffer(Point(500000 6000000),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(500002 5999994),1)@2001-01-01, Cbuffer(Point(500002 6000006),1)@2001-01-03]';
-- A contact at an excluded bound, and at an included one
SELECT tcbuffer '(Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(2 0),1)@2001-01-01, Cbuffer(Point(30 0),1)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]' ?&& tcbuffer '[Cbuffer(Point(2 0),1)@2001-01-01, Cbuffer(Point(30 0),1)@2001-01-03]';
-- Values synchronized between their instants
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(8 0),1)@2001-01-05]' ?&& tcbuffer '[Cbuffer(Point(4 4),1)@2001-01-03, Cbuffer(Point(4 -4),1)@2001-01-07]';
SELECT tcbuffer '{Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(5 0),1)@2001-01-02}' ?&& tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]';
SELECT tcbuffer 'Cbuffer(Point(8 0),1)@2001-01-02' ?&& tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]';
-- Always: radii growing along, apart strictly between two instants, a step
-- value jumping at its instant, and a sequence set
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(10 0),3)@2001-01-03]' %&& tcbuffer '[Cbuffer(Point(1 1),1)@2001-01-01, Cbuffer(Point(11 1),3)@2001-01-03]';
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-03]' %&& tcbuffer '[Cbuffer(Point(2 0),1)@2001-01-01, Cbuffer(Point(6 0),1)@2001-01-02, Cbuffer(Point(2 0),1)@2001-01-03]';
SELECT tcbuffer 'Interp=Step;[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-02, Cbuffer(Point(10 0),1)@2001-01-03]' %&& tcbuffer '[Cbuffer(Point(10 0),1)@2001-01-01, Cbuffer(Point(10 0),1)@2001-01-03]';
SELECT tcbuffer '{[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-02], [Cbuffer(Point(0 0),1)@2001-01-03, Cbuffer(Point(0 0),1)@2001-01-04]}' %&& tcbuffer '[Cbuffer(Point(1 0),1)@2001-01-01, Cbuffer(Point(1 0),1)@2001-01-04]';
-- Values sharing no time
SELECT tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-01, Cbuffer(Point(0 0),1)@2001-01-02]' ?&& tcbuffer '[Cbuffer(Point(0 0),1)@2001-01-03, Cbuffer(Point(0 0),1)@2001-01-04]';
/* Errors */
SELECT tcbuffer 'Cbuffer(Point(0 0),1)@2001-01-01' ?&& tcbuffer 'SRID=3812;Cbuffer(Point(0 0),1)@2001-01-01';
