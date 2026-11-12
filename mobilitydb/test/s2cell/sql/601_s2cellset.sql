-- Static s2cellset SQL type: parser, output, constructor, accessors, the cast
-- from a single cell, set operations, and the btree opclass.
--
-- Every cell here is at level 29, whose level bit sits at position 2, so the
-- literals end in 4, c and 14. A token ending in 8 sets bit 3, which is odd
-- and encodes no level, and the type rejects it.

-------------------------------------------------------------------------------
-- Input and output
--
-- A set prints its elements in ascending identifier order, which is Hilbert
-- order, and duplicates collapse.
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c0000004}';

-- The text and binary forms: EWKT, EWKB and HexEWKB state the SRID 4326 of a
-- cell, and every form reads back
SELECT asEWKT(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT asHexEWKB(s2cellset '{47c3c444c000000c}', 'XDR');
SELECT s2cellsetFromText(asText(s2cellset '{47c3c444c000000c, 47c3c444c0000004}'))
  = s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellsetFromEWKT(asEWKT(s2cellset '{47c3c444c000000c, 47c3c444c0000004}'))
  = s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellsetFromBinary(asBinary(s2cellset '{47c3c444c000000c, 47c3c444c0000004}'))
  = s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellsetFromEWKB(asEWKB(s2cellset '{47c3c444c000000c, 47c3c444c0000004}'))
  = s2cellset '{47c3c444c000000c, 47c3c444c0000004}';
SELECT s2cellsetFromHexWKB(asHexWKB(s2cellset '{47c3c444c000000c}'))
  = s2cellset '{47c3c444c000000c}';
SELECT s2cellsetFromHexEWKB(asHexEWKB(s2cellset '{47c3c444c000000c}'))
  = s2cellset '{47c3c444c000000c}';

-- A cell states SRID 4326, so another SRID is refused in every input form
SELECT s2cellsetFromEWKT('SRID=3857;{47c3c444c000000c}');
SELECT s2cellsetFromHexEWKB(replace(asHexEWKB(s2cellset '{47c3c444c000000c}', 'XDR'),
  '000010E6', '00000F11'));
SELECT s2cell '47c3c444c0000008';

-------------------------------------------------------------------------------
-- Constructor and accessors
-------------------------------------------------------------------------------

SELECT set(ARRAY[s2cell '47c3c444c000000c', s2cell '47c3c444c0000004']);
SELECT numValues(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT startValue(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT endValue(s2cellset '{47c3c444c000000c, 47c3c444c0000004}');
SELECT valueN(s2cellset '{47c3c444c000000c, 47c3c444c0000004}', 1);
SELECT valueN(s2cellset '{47c3c444c000000c, 47c3c444c0000004}', 3);

-------------------------------------------------------------------------------
-- The cast from a single cell, and set membership
-------------------------------------------------------------------------------

SELECT (s2cell '47c3c444c0000004')::s2cellset;
SELECT s2cell '47c3c444c0000004' <@ s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cell '47c3c444c0000014' <@ s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' @> s2cell '47c3c444c0000004';

-------------------------------------------------------------------------------
-- Set operations
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c0000004}' + s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c000000c}' * s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004, 47c3c444c000000c}' - s2cellset '{47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' && s2cellset '{47c3c444c0000004, 47c3c444c000000c}';
SELECT s2cellset '{47c3c444c0000004}' && s2cellset '{47c3c444c0000014}';

-------------------------------------------------------------------------------
-- Comparison, which the btree operator class carries
-------------------------------------------------------------------------------

SELECT s2cellset '{47c3c444c0000004}' = s2cellset '{47c3c444c0000004}';
SELECT s2cellset '{47c3c444c0000004}' < s2cellset '{47c3c444c000000c}';

-------------------------------------------------------------------------------
