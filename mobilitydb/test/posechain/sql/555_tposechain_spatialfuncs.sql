-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Restriction to a box, and ever and always overlaps with a box: a chain lies
-- in a box at a time when any of its joints lies in it
-------------------------------------------------------------------------------

-- The elbow in the box, the hand out
SELECT asText(atStbox(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01', stbox 'STBOX X((-1,-1),(1,1))'));
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01' ?&& stbox 'STBOX X((-1,-1),(1,1))';
-- The hand in the box, the elbow out
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01' ?&& stbox 'STBOX X((9,-1),(11,1))';
-- No joint in the box
SELECT asText(atStbox(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01', stbox 'STBOX X((4,-1),(6,1))'));
SELECT asText(minusStbox(tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01', stbox 'STBOX X((4,-1),(6,1))'));
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01' ?&& stbox 'STBOX X((4,-1),(6,1))';

-- The elbow, the first link, crosses the box between two instants
SELECT asText(atStbox(tposechain '[PoseChain(Pose(Point(-5 0), 0), Pose(Point(0 20), 0))@2001-01-01, PoseChain(Pose(Point(5 0), 0), Pose(Point(0 20), 0))@2001-01-02]', stbox 'STBOX X((-1,-1),(1,1))'));
SELECT tposechain '[PoseChain(Pose(Point(-5 0), 0), Pose(Point(0 20), 0))@2001-01-01, PoseChain(Pose(Point(5 0), 0), Pose(Point(0 20), 0))@2001-01-02]' ?&& stbox 'STBOX X((-1,-1),(1,1))';
SELECT tposechain '[PoseChain(Pose(Point(-5 0), 0), Pose(Point(0 20), 0))@2001-01-01, PoseChain(Pose(Point(5 0), 0), Pose(Point(0 20), 0))@2001-01-02]' %&& stbox 'STBOX X((-1,-1),(1,1))';
-- The hand, composed of two links, held in the box until the next instant
SELECT asText(atStbox(tposechain '[PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01, PoseChain(Pose(Point(0 5), 0), Pose(Point(10 0), 0))@2001-01-02]', stbox 'STBOX X((9,-1),(11,1))'));
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01, PoseChain(Pose(Point(0 5), 0), Pose(Point(10 0), 0))@2001-01-02]' %&& stbox 'STBOX X((9,-1),(11,1))';

-- The box first, a box with a period, and a box with only a period
SELECT stbox 'STBOX X((-1,-1),(1,1))' ?&& tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01';
SELECT stbox 'STBOX X((-1,-1),(1,1))' %&& tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01';
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0), Pose(Point(10 0), 0))@2001-01-01' ?&& stbox 'STBOX XT(((-1,-1),(1,1)),[2001-01-02, 2001-01-03])';
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]' ?&& stbox 'STBOX T([2001-01-03, 2001-01-04])';
SELECT tposechain '[PoseChain(Pose(Point(0 0), 0))@2001-01-01, PoseChain(Pose(Point(1 1), 0))@2001-01-02]' %&& stbox 'STBOX T([2001-01-01, 2001-01-04])';

/* Errors */
SELECT atStbox(tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01', stbox 'STBOX T([2001-01-01, 2001-01-04])');
SELECT tposechain 'PoseChain(Pose(Point(0 0), 0))@2001-01-01' ?&& stbox 'SRID=4326;STBOX X((0,0),(1,1))';

-------------------------------------------------------------------------------
