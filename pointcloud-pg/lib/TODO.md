To Do
=====

- Add in dimensional stats caching to speed up dimensional compression in batch cases

- (?) convert PCBYTES to use PCDIMENSION* instead of holding all values as dupes
- (??) convert PCBYTES handling to pass-by-reference instead of pass-by-value
- implement PC\_PatchAvg/PC\_PatchMin/PC\_PatchMax as C functions against patches with dimensional and uncompressed implementations
- TESTS for pc\_patch\_dimensional\_from\_uncompressed() and pc\_patch\_dimensional\_compress()

- Update pc\_patch\_from\_patchlist() to merge GHT patches without decompression
- Update pc\_patch\_from\_patchlist() to merge dimensional patches directly

- Before doing dimensional compression, sort by geohash (actually by a localized geohash based on the patch bounds). This will (?) enhance the autocorrelation of values and improve run-length encoding in particular

- Compute PCSTATS in WKB reading code for all patch variants, not just uncompressed
  - compute stats in libght
  - compute stats of dimensional
- Remove extents in favour of PCSTATS
- Make PCSTATS a static member of the PCPATCH, not a pointer

- PC\_Filter[GreaterThan|LessThan|Between](patch, dimension, value) should note the relevant stats and short-circuit return either all or none of the patch, as necessary

Use Cases to Support
--------------------

- frustrum filtering
- raster overlaying
- filtering on attribute values
- filtering on spatial polygons (in *and* out)
- find the k nearest points to point P
- gridding/binning ("avg intensity per cell", "max z per cell", "agv red per cell", "rgb into grid/picture")
- reprojection / re-schema

More Functions
--------------

- PC\_FilterPolygon(patch, wkb) returns patch
- PC\_Filter(patch, dimension, expression) returns patch
- PC\_Get(pcpatch, dimname) returns Array(numeric)

- PC\_Transform(pcpatch, newpcid) 
