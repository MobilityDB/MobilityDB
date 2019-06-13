--BerlinMOD SF0.005
--TGEOMPOINT op PERIOD
explain analyze select * from trips where trip <<# period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]';
explain analyze select * from trips where period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]' <<# trip;
explain analyze select * from trips where trip #>> period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]';
explain analyze select * from trips where period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]' #>> trip;
explain analyze select * from trips where trip #&> period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]';
explain analyze select * from trips where period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]' #&> trip;
explain analyze select * from trips where trip &<# period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]';
explain analyze select * from trips where period '[2007-05-29 10:00:00, 2007-05-29 11:00:00]' &<# trip;

--TGEOMPOINT op GEOMETRY(LINESTRING)
explain analyze select * from trips where trip && 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry && trip;
explain analyze select * from trips where trip @> 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry @> trip;
explain analyze select * from trips where trip <@ 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry <@ trip;
explain analyze select * from trips where trip << 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry << trip;
explain analyze select * from trips where trip >> 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry >> trip;
explain analyze select * from trips where trip &< 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry &< trip;
explain analyze select * from trips where trip &> 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry &> trip;
explain analyze select * from trips where trip <<| 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry <<| trip;
explain analyze select * from trips where trip |>> 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry |>> trip;
explain analyze select * from trips where trip &<| 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry &<| trip;
explain analyze select * from trips where trip |&> 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry;
explain analyze select * from trips where 'LineString(13.39879 52.53, 13.49879 52.66)'::geometry |&> trip;

--TGEOMPOINT op TGEOMPOINT(Sequence)
explain analyze select * from trips where trip && '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint && trip;
explain analyze select * from trips where trip @> '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint @> trip;
explain analyze select * from trips where trip <@ '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint <@ trip;
explain analyze select * from trips where trip << '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint << trip;
explain analyze select * from trips where trip >> '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint >> trip;
explain analyze select * from trips where trip &< '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint &< trip;
explain analyze select * from trips where trip &> '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint &> trip;
explain analyze select * from trips where trip <<| '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint <<| trip;
explain analyze select * from trips where trip |>> '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint |>> trip;
explain analyze select * from trips where trip &<| '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint &<| trip;
explain analyze select * from trips where trip |&> '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint;
explain analyze select * from trips where '[point(13.39879 52.53)@2007-05-29 10:00:00, point(13.49879 52.66)@2007-05-29 11:00:00]'::tgeompoint |&> trip;

--TGEOMPOINT op STBOX
explain analyze select * from trips where trip && 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox && trip;
explain analyze select * from trips where trip @> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox @> trip;
explain analyze select * from trips where trip <@ 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox <@ trip;
explain analyze select * from trips where trip << 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox << trip;
explain analyze select * from trips where trip >> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox >> trip;
explain analyze select * from trips where trip &< 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &< trip;
explain analyze select * from trips where trip &> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &> trip;
explain analyze select * from trips where trip <<| 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox <<| trip;
explain analyze select * from trips where trip |>> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox |>> trip;
explain analyze select * from trips where trip &<| 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &<| trip;
explain analyze select * from trips where trip |&> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox |&> trip;

--STBOX op STBOX
create table trips_stbox as select trip::stbox from trips;

explain analyze select * from trips_stbox where trip && 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox && trip;
explain analyze select * from trips_stbox where trip @> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox @> trip;
explain analyze select * from trips_stbox where trip <@ 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox <@ trip;
explain analyze select * from trips_stbox where trip << 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox << trip;
explain analyze select * from trips_stbox where trip >> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox >> trip;
explain analyze select * from trips_stbox where trip &< 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &< trip;
explain analyze select * from trips_stbox where trip &> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &> trip;
explain analyze select * from trips_stbox where trip <<| 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox <<| trip;
explain analyze select * from trips_stbox where trip |>> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox |>> trip;
explain analyze select * from trips_stbox where trip &<| 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox &<| trip;
explain analyze select * from trips_stbox where trip |&> 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox;;
explain analyze select * from trips_stbox where 'STBOX T((13.398789,52.529999,2.337408e+14),(13.498791,52.660004,2.337444e+14))'::stbox |&> trip;




--Z-DIM
--Planes table
--TGEOMPOINT op GEOMETRY
explain analyze select * from plane_3d where plane_pos <</ 'LineString Z(10 51.1 2000, 30 51.6 2700)'::geometry;
explain analyze select * from plane_3d where 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry <</ plane_pos;
explain analyze select * from plane_3d where plane_pos />> 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry;
explain analyze select * from plane_3d where 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry />> plane_pos;
explain analyze select * from plane_3d where plane_pos &</ 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry;
explain analyze select * from plane_3d where 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry &</ plane_pos;
explain analyze select * from plane_3d where plane_pos /&> 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry;
explain analyze select * from plane_3d where 'LineString Z(20 51.1 400, 30 51.6 2000)'::geometry /&> plane_pos;

--TGEOMPOINT op STBOX
explain analyze select * from plane_3d where plane_pos <</ 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox;
explain analyze select * from plane_3d where 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox <</ plane_pos;
explain analyze select * from plane_3d where plane_pos />> 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox;
explain analyze select * from plane_3d where 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox />> plane_pos;
explain analyze select * from plane_3d where plane_pos &</ 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox;
explain analyze select * from plane_3d where 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox &</ plane_pos;
explain analyze select * from plane_3d where plane_pos /&> 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox;
explain analyze select * from plane_3d where 'STBOX ZT((20,51.099998,400,6.023772e+14),(30,51.600002,2000,6.024636e+14))'::stbox /&> plane_pos;

--TGEOMPOINT op TGEOMPOINT
explain analyze select * from plane_3d where plane_pos << tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]';
explain analyze select * from plane_3d where tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]'::geometry <</ plane_pos;
explain analyze select * from plane_3d where plane_pos />> tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]';
explain analyze select * from plane_3d where tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]' />> plane_pos;
explain analyze select * from plane_3d where plane_pos &</ tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]';
explain analyze select * from plane_3d where tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]' &</ plane_pos;
explain analyze select * from plane_3d where plane_pos /&> tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]';
explain analyze select * from plane_3d where tgeompoint(sequence)'[POINT Z(20 51.1 400)@2019-02-02, POINT Z(30 51.6 2000)@2019-02-03]' /&> plane_pos;

