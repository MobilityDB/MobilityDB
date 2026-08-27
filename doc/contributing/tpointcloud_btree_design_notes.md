<!--
  MobilityDB — B-tree Ordering of Temporal Point Cloud Values: Design Notes
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# B-tree ordering of `tpcpoint` and `tpcpatch`

The `tpcpoint_btree_ops` and `tpcpatch_btree_ops` opclasses produce a total order over their
values, and that order is byte-wise on the underlying pgPointCloud serialisation rather than
geometric. What follows is what a caller writing `ORDER BY` on such a column gets.

## `tpcpoint`

- **`pcid` is the primary discriminator.** Points with a smaller `pcid` sort before points with a
  larger one, whatever their coordinates, so mixing pcids in one column groups the rows by schema
  before anything else.
- **Within one `pcid` the order follows the schema's on-disk dimension layout**, typically X, Y and
  Z encoded as `int32` after the per-dimension scale is applied. Two points a small Euclidean
  distance apart can sort far apart when their X dimensions differ, X being compared first.
- **Equality is exact-bytes equality.** Two pcpoints holding the same dimension values under the
  same schema are equal; the same coordinates encoded under a schema of a different scale or a
  different dimension order are not.

## `tpcpatch`

The same three properties hold, the comparison reading the patch header and then its compressed
payload, so two patches holding the same points in a different insertion order are not equal.

## What to order on instead

For an order that means something spatially, project to a `tgeompoint` through the schema-aware
cast and use the PostGIS KNN operator `<->` on the result, or use the GiST KNN operator `|=|`
directly on the temporal point cloud value. For an order that means something temporally, project
to `startTimestamp` or to the time span and order on that.
