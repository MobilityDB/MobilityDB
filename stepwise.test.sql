-- drop extension mobilitydb cascade
-- create extension mobilitydb cascade

select pg_backend_pid()

-- Constructors 
select tfloatseq(ARRAY[tfloat '1@2000-01-01', tfloat '2@2000-01-03', tfloat '3@2000-01-05'], true, true, false)
"Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

select astext(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false))
"Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

select asewkt(setSRID(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false), 4326))
"SRID=4326;Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

select astext(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false))
"Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

select asewkt(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false))
"SRID=4326;Interp=Stepwise;[POINT(1 1)@2000-01-01 00:00:00+01, POINT(2 2)@2000-01-03 00:00:00+01, POINT(3 3)@2000-01-05 00:00:00+01]"

-- Casting 
select tintseq(ARRAY[tint '1@2000-01-01', tint '2@2000-01-03', tint '3@2000-01-05'])::tfloat
"Interp=Stepwise;[1@2000-01-01 00:00:00+01, 2@2000-01-03 00:00:00+01, 3@2000-01-05 00:00:00+01]"

-- Restriction Functions
select atTimestamp(tfloatseq(ARRAY[tfloat '1@2000-01-01', tfloat '2@2000-01-03', tfloat '3@2000-01-05'], true, true, false), '2000-01-02')
"1@2000-01-02 00:00:00+01"

select astext(atTimestamp(tgeompointseq(ARRAY[tgeompoint 'Point(1 1)@2000-01-01', tgeompoint 'Point(2 2)@2000-01-03', tgeompoint 'Point(3 3)@2000-01-05'], true, true, false), '2000-01-02'))
"POINT(1 1)@2000-01-02 00:00:00+01"

select asewkt(atTimestamp(tgeogpointseq(ARRAY[tgeogpoint 'Point(1 1)@2000-01-01', tgeogpoint 'Point(2 2)@2000-01-03', tgeogpoint 'Point(3 3)@2000-01-05'], true, true, false), '2000-01-02'))
"SRID=4326;POINT(1 1)@2000-01-02 00:00:00+01"


