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




--
-- Name: tbl_tgeompoint3d_big; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public.tbl_tgeompoint3d_big (
    k integer,
    temp public.tgeompoint
);

explain analyze select * from tbl_tgeompoint3d_big where temp <</ 'LineString(20 20 20, 50 50 50)'::geometry;
explain analyze select * from tbl_tgeompoint3d_big where 'LineString(20 20 20, 50 50 50)'::geometry <</ temp;
explain analyze select * from tbl_tgeompoint3d_big where temp />> 'LineString(20 20 20, 50 50 50)'::geometry::geometry;
explain analyze select * from tbl_tgeompoint3d_big where 'LineString(20 20 20, 50 50 50)'::geometry />> temp;
explain analyze select * from tbl_tgeompoint3d_big where temp &</ 'LineString(20 20 20, 50 50 50)'::geometry;
explain analyze select * from tbl_tgeompoint3d_big where 'LineString(20 20 20, 50 50 50)'::geometry &</ temp;
explain analyze select * from tbl_tgeompoint3d_big where temp /&> 'LineString(20 20 20, 50 50 50)'::geometry;
explain analyze select * from tbl_tgeompoint3d_big where 'LineString(20 20 20, 50 50 50)'::geometry /&> temp;
