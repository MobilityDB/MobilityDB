-- Comparison operators (=, <>, <, <=, >=, >, cmp, B-tree opclass) for the
-- static pcpoint / pcpatch base types. Uses the ergonomic pcpoint/pcpatch
-- constructors (same pattern as 420_tpcpoint.sql / 430_tpcpatch.sql).

-------------------------------------------------------------------------------
-- pcpoint
-------------------------------------------------------------------------------

-- Equality of two identical points built independently.
SELECT pcpoint(1, 1.0, 1.0, 1.0) = pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) = pcpoint(1, 2.0, 2.0, 2.0);

SELECT pcpoint(1, 1.0, 1.0, 1.0) <> pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) <> pcpoint(1, 2.0, 2.0, 2.0);

-- cmp() is reflexive and zero only for equal values.
SELECT cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 1.0, 1.0, 1.0)) = 0;
SELECT cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <> 0;

-- The </<=/>/>= operators agree with the sign of cmp().
SELECT (pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) < 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) <= pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <= 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) > pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) > 0);
SELECT (pcpoint(1, 1.0, 1.0, 1.0) >= pcpoint(1, 2.0, 2.0, 2.0)) =
  (cmp(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) >= 0);
-- Antisymmetry.
SELECT (pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 2.0, 2.0, 2.0)) =
  (pcpoint(1, 2.0, 2.0, 2.0) > pcpoint(1, 1.0, 1.0, 1.0));
-- Irreflexivity of the strict operators.
SELECT pcpoint(1, 1.0, 1.0, 1.0) < pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) > pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) <= pcpoint(1, 1.0, 1.0, 1.0);
SELECT pcpoint(1, 1.0, 1.0, 1.0) >= pcpoint(1, 1.0, 1.0, 1.0);

-- ORDER BY over three distinct values produces a sequence that is
-- non-decreasing with respect to the <= operator (a genuine B-tree total
-- order, whatever its relationship to the coordinate magnitudes).
WITH v(pt) AS (
  VALUES (pcpoint(1, 1.0, 1.0, 1.0)), (pcpoint(1, 2.0, 2.0, 2.0)),
    (pcpoint(1, 3.0, 3.0, 3.0))
), ordered AS (
  SELECT pt, row_number() OVER (ORDER BY pt) AS rn FROM v
)
SELECT (SELECT count(*) FROM ordered) = 3
  AND bool_and(a.pt <= b.pt)
FROM ordered a JOIN ordered b ON b.rn = a.rn + 1;

-------------------------------------------------------------------------------
-- pcpatch
-------------------------------------------------------------------------------

-- Equality of two identical patches built independently.
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) =
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) =
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0));

SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <>
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <>
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0));

-- cmp() is reflexive and zero only for equal values.
SELECT cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))) = 0;
SELECT cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
  pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) <> 0;

-- The </<=/>/>= operators agree with the sign of cmp().
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) < 0);
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) >
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (cmp(pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)),
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) > 0);
-- Antisymmetry.
SELECT (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
    pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0))) =
  (pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0)) >
    pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)));
-- Irreflexivity of the strict operators.
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));
SELECT pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0)) <=
  pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0));

-- ORDER BY over three distinct values produces a sequence that is
-- non-decreasing with respect to the <= operator.
WITH v(pa) AS (
  VALUES
    (pcpatch(pcpoint(1, 1.0, 1.0, 1.0), pcpoint(1, 2.0, 2.0, 2.0))),
    (pcpatch(pcpoint(1, 3.0, 3.0, 3.0), pcpoint(1, 4.0, 4.0, 4.0))),
    (pcpatch(pcpoint(1, 5.0, 5.0, 5.0), pcpoint(1, 6.0, 6.0, 6.0)))
), ordered AS (
  SELECT pa, row_number() OVER (ORDER BY pa) AS rn FROM v
)
SELECT (SELECT count(*) FROM ordered) = 3
  AND bool_and(a.pa <= b.pa)
FROM ordered a JOIN ordered b ON b.rn = a.rn + 1;

-------------------------------------------------------------------------------

-------------------------------------------------------------------------------
-- Values of different pcids
-------------------------------------------------------------------------------

-- Two more pcids with the schema of the pcid 1. The pcid 256 has a low byte
-- of zero, so its bytes in memory, least significant first on a
-- little-endian machine, are below those of the pcids 1 and 2.
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 2, srid, schema FROM pointcloud_formats WHERE pcid = 1;
INSERT INTO pointcloud_formats (pcid, srid, schema)
SELECT 256, srid, schema FROM pointcloud_formats WHERE pcid = 1;

-- Values of different pcids are ordered as their pcids, whatever the
-- dimension values they store.
SELECT pcpoint(2, 9.0, 9.0, 9.0) < pcpoint(256, 1.0, 1.0, 1.0);
SELECT cmp(pcpoint(256, 1.0, 1.0, 1.0), pcpoint(1, 9.0, 9.0, 9.0));
SELECT pcpatch(pcpoint(2, 9.0, 9.0, 9.0), pcpoint(2, 9.0, 9.0, 9.0)) <
  pcpatch(pcpoint(256, 1.0, 1.0, 1.0), pcpoint(256, 1.0, 1.0, 1.0));
SELECT cmp(pcpatch(pcpoint(256, 1.0, 1.0, 1.0), pcpoint(256, 1.0, 1.0, 1.0)),
  pcpatch(pcpoint(1, 9.0, 9.0, 9.0), pcpoint(1, 9.0, 9.0, 9.0)));

-- ORDER BY over values of the three pcids, and over the values of one pcid
-- by the dimension values they store.
WITH v(pt) AS (
  VALUES (pcpoint(256, 1.0, 1.0, 1.0)), (pcpoint(2, 5.0, 5.0, 5.0)),
    (pcpoint(1, 9.0, 9.0, 9.0)), (pcpoint(2, 4.0, 4.0, 4.0))
)
SELECT array_agg(pcid(pt) ORDER BY pt) FROM v;
WITH v(pa) AS (
  VALUES (pcpatch(pcpoint(256, 1.0, 1.0, 1.0), pcpoint(256, 2.0, 2.0, 2.0))),
    (pcpatch(pcpoint(2, 5.0, 5.0, 5.0), pcpoint(2, 6.0, 6.0, 6.0))),
    (pcpatch(pcpoint(1, 9.0, 9.0, 9.0), pcpoint(1, 9.0, 9.0, 9.0)))
)
SELECT array_agg(pcid(pa) ORDER BY pa) FROM v;

DELETE FROM pointcloud_formats WHERE pcid IN (2, 256);

-------------------------------------------------------------------------------
