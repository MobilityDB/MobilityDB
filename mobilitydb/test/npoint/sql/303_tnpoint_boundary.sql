-------------------------------------------------------------------------------
-- Boundary-condition coverage for npoint position clamping at [0, 1].
-- Documented contract: 0.0 (start of route) and 1.0 (end of route) are
-- valid; values outside [0, 1] are rejected at the constructor.
-------------------------------------------------------------------------------

-- Endpoints are valid
SELECT npoint(1, 0.0);
SELECT npoint(1, 1.0);

-- Just-inside values are valid
SELECT npoint(1, 0.0001);
SELECT npoint(1, 0.9999);

-- Interpolating to geometry at the endpoints lands on the route's start
-- and end points respectively; both are well-defined.
SELECT round(geometry(npoint(1, 0.0))::geometry, 6);
SELECT round(geometry(npoint(1, 1.0))::geometry, 6);

-- Position parameter outside [0, 1] is rejected at construction
/* Errors */
SELECT npoint(1, -0.0001);
SELECT npoint(1, 1.0001);
SELECT npoint(1, -1);
SELECT npoint(1, 2);

-- Same boundary contract on the WKT input path
SELECT npoint 'Npoint(1, 0.0)';
SELECT npoint 'Npoint(1, 1.0)';
/* Errors */
SELECT npoint 'Npoint(1, -0.0001)';
SELECT npoint 'Npoint(1, 1.0001)';

-- nsegment endpoints: nsegment_set normalises (pos1, pos2) via Min / Max,
-- so a constructor call with the positions reversed returns a normalised
-- segment rather than an error.
SELECT nsegment(1, 1.0, 0.0);
SELECT nsegment(1, 0.0, 1.0);

-- nsegment also rejects out-of-range positions at construction
/* Errors */
SELECT nsegment(1, -0.0001, 1.0);
SELECT nsegment(1, 0.0, 1.0001);

-------------------------------------------------------------------------------
