/******************************************************************************
 * Constructors
 ******************************************************************************/

SELECT npoint(1, 0.5);

SELECT nregion(1, 0.2, 0.6);
SELECT nregion(1);
SELECT nregion(1, 0.5);
SELECT nregion(npoint '(1,0.5)');
SELECT nregion_agg(nregion(gid)) FROM ways WHERE gid <= 10;

/******************************************************************************
 * Accessing values
 ******************************************************************************/

SELECT route(npoint '(1,0.5)');
SELECT pos(npoint '(1,0.5)');
SELECT * FROM segments(nregion '{(1,0.2,0.6),(2,0,1)}');

/******************************************************************************
 * Conversions between network and space
 ******************************************************************************/

SELECT in_space(npoint '(1,0.2)');
SELECT in_space(nregion '{(1,0.2,0.6),(2,0,1)}');
SELECT point_in_network(ST_SetSRID(ST_MakePoint(4.3560493, 50.8504975), 4326));
SELECT geometry_in_network(ST_MakeEnvelope(4.3, 50.8, 4.4, 50.9, 4326));

/******************************************************************************/
