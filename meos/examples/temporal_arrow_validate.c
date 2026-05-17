/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief Validate the MEOS Arrow C Data Interface export against a canonical
 * external Arrow consumer
 *
 * @details This builds representative temporal values, converts each through
 * #meos_temporal_to_arrow, and hands the produced `ArrowSchema`/`ArrowArray`
 * to nanoarrow, a small dependency-free implementation of the Arrow C Data
 * Interface. nanoarrow imports the schema and array with no MEOS knowledge
 * and runs full specification validation (`ArrowArrayViewInitFromSchema` +
 * `ArrowArrayViewSetArray` + `ArrowArrayViewValidate` at the FULL level),
 * which walks the entire nested
 * `Struct{ ..., seqs:List<Struct{ ..., insts:List<Struct{t, v}>}>}` tree.
 * The export is also self-checked through #meos_temporal_arrow_roundtrip as
 * a control.
 *
 * Two value-leaf tiers are exercised:
 *
 * - The fully decomposed nested-Struct leaf, represented by the temporal
 *   circular buffer (`Struct{x,y,r}`). This is the tier discharged by the
 *   first conformance probe and is kept here as a control.
 * - The opaque LargeBinary leaf and the rigid-geometry struct leaf:
 *   temporal geometry and geography carry their per-instant value as an
 *   opaque int64-offset LargeBinary "Z" leaf; a temporal rigid geometry
 *   carries a `Struct{ref:LargeBinary, ...pose fields}` whose leading
 *   "ref" child is the shared reference geometry as EWKB. These use a
 *   different value-leaf encoding (LargeBinary + int64 offsets) than the
 *   decomposed tier and are validated here for the first time.
 *
 * @code
 * gcc -Wall -g -I/usr/local/include -Inanoarrow -o temporal_arrow_validate \
 *   temporal_arrow_validate.c nanoarrow/nanoarrow.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* nanoarrow is the canonical external Arrow C Data Interface oracle. It
 * supplies the ArrowSchema / ArrowArray / ArrowArrayStream ABI structs
 * (verbatim from the Arrow specification format/abi.h) and the C Data
 * Interface guard macro ARROW_C_DATA_INTERFACE. MEOS vendors the SAME two
 * structs in its own ABI-identical header (include guard
 * MEOS_ARROW_C_DATA_INTERFACE_H, same upstream source). Including nanoarrow
 * first and pre-defining MEOS's include guard makes the single canonical
 * definition apply on both sides; libmeos was compiled against the
 * ABI-identical copy, so the binary interface is unchanged. */
#include "nanoarrow/nanoarrow.h"
#define MEOS_ARROW_C_DATA_INTERFACE_H
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include <meos_geo.h>
#include <meos_pose.h>
#include <meos_rgeo.h>
#include <meos_pointcloud.h>
/* A standalone MEOS program builds and registers a point-cloud value
 * exactly as meos/examples/tpcbox_rtree.c and
 * meos/test/pointcloud_valgrind.c do (in a PG backend mobilitydb_init
 * registers the schema lazily from the pointcloud_formats catalog and
 * PC_Patch builds the patch). pc_api.h / pc_api_internal.h are
 * pgPointCloud's vendored headers (PCSCHEMA, pc_schema_from_xml, the
 * PCPOINTLIST / PCPATCH builders); pointcloud/pgsql_compat.h is the MEOS
 * serialization shim (meos_pc_patch_serialize, SERIALIZED_PATCH). */
#include <pointcloud/pgsql_compat.h>
#include "pc_api.h"
#include "pc_api_internal.h"

/**
 * @brief Hand one already-built temporal value's Arrow export to the
 * external nanoarrow consumer for full specification validation, then
 * self-check it through the MEOS round-trip as a control
 *
 * @details @p temp is consumed (freed) by this function. nanoarrow imports
 * the schema and array with zero MEOS knowledge and validates the whole
 * nested tree at the FULL level (validity buffers, list offset buffers,
 * the LargeBinary/Struct value leaf, child counts, lengths and null
 * counts). The MEOS self round-trip is only a control on top of the
 * external verdict.
 */
static int
validate_temp(const char *label, Temporal *temp)
{
  if (! temp)
  {
    printf("[%s] FAIL: constructor returned NULL\n", label);
    return 1;
  }

  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_temporal_to_arrow(temp, &schema, &array))
  {
    printf("[%s] FAIL: meos_temporal_to_arrow\n", label);
    free(temp);
    return 1;
  }

  int rc = 0;
  struct ArrowError error;
  struct ArrowArrayView view;
  memset(&view, 0, sizeof(view));

  /* Independent external parse of the MEOS-produced schema. */
  if (ArrowArrayViewInitFromSchema(&view, &schema, &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewInitFromSchema: %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
    goto done;
  }
  /* Bind the MEOS-produced array: nanoarrow reads every buffer, derives
   * sizes from the list offsets, and checks the list/struct child wiring
   * (including the int64-offset LargeBinary value leaf). */
  if (ArrowArrayViewSetArray(&view, &array, &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewSetArray: %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
    goto done;
  }
  /* Full Arrow C Data Interface content validation. */
  if (ArrowArrayViewValidate(&view, NANOARROW_VALIDATION_LEVEL_FULL,
      &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewValidate(FULL): %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
    goto done;
  }

  /* Descend the externally parsed view to the per-instant value leaf and
   * report the storage type nanoarrow resolved for it. The path is the
   * fixed contract
   * top:Struct -> seqs:List -> seq:Struct -> insts:List -> inst:Struct
   * -> [t, v]; the value leaf is children[1] of the inst struct. This
   * proves nanoarrow actually walked into and FULL-validated the
   * value-leaf encoding (the opaque LargeBinary "Z" leaf for temporal
   * geometry/geography, the Struct{ref:LargeBinary,...} for a rigid
   * geometry), not merely the outer skeleton. */
  {
    struct ArrowArrayView *v = &view;            /* top struct */
    const char *leafdesc = NULL;
    if (v->n_children == 5)
    {
      struct ArrowArrayView *seqs = v->children[4];        /* seqs list */
      if (seqs->n_children == 1)
      {
        struct ArrowArrayView *seq = seqs->children[0];    /* seq struct */
        if (seq->n_children == 3)
        {
          struct ArrowArrayView *insts = seq->children[2]; /* insts list */
          if (insts->n_children == 1)
          {
            struct ArrowArrayView *inst = insts->children[0]; /* inst */
            if (inst->n_children == 2)
            {
              struct ArrowArrayView *vleaf = inst->children[1];
              if (vleaf->storage_type == NANOARROW_TYPE_LARGE_BINARY)
                leafdesc = "LargeBinary value leaf";
              else if (vleaf->storage_type == NANOARROW_TYPE_STRUCT &&
                vleaf->n_children >= 1 &&
                vleaf->children[0]->storage_type ==
                  NANOARROW_TYPE_LARGE_BINARY)
                leafdesc = "Struct value leaf with LargeBinary ref child";
              else if (vleaf->storage_type == NANOARROW_TYPE_STRUCT)
                leafdesc = "decomposed Struct value leaf";
            }
          }
        }
      }
    }
    if (! leafdesc)
    {
      printf("[%s] FAIL: could not locate the value leaf in the parsed "
        "view\n", label);
      rc = 1;
      goto done;
    }
    printf("[%s] value leaf: %s\n", label, leafdesc);
  }

  /* Control: MEOS reads its own export back to the same value. */
  Temporal *back = meos_temporal_arrow_roundtrip(temp);
  if (! back)
  {
    printf("[%s] FAIL: meos_temporal_arrow_roundtrip\n", label);
    rc = 1;
    goto done;
  }
  char *s1 = temporal_out(temp, 6);
  char *s2 = temporal_out(back, 6);
  if (strcmp(s1, s2) != 0)
  {
    printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
      label, s1, s2);
    rc = 1;
  }
  free(s1);
  free(s2);
  free(back);

  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);

done:
  ArrowArrayViewReset(&view);
  schema.release(&schema);
  array.release(&array);
  free(temp);
  return rc;
}

/**
 * @brief Validate one temporal circular buffer value (the fully decomposed
 * nested-Struct value leaf, kept as the decomposed-tier control)
 */
static int
validate_cbuffer(const char *label, const char *in)
{
  return validate_temp(label, tcbuffer_in(in));
}

/**
 * @brief Validate one temporal geometry value (opaque int64-offset
 * LargeBinary "Z" value leaf)
 */
static int
validate_tgeometry(const char *label, const char *in)
{
  return validate_temp(label, tgeometry_in(in));
}

/**
 * @brief Validate one temporal geography value (opaque int64-offset
 * LargeBinary "Z" value leaf, geodetic flag carried verbatim)
 */
static int
validate_tgeography(const char *label, const char *in)
{
  return validate_temp(label, tgeography_in(in));
}

/**
 * @brief Validate one temporal rigid geometry value (the value leaf is a
 * Struct whose leading "ref" child is the shared reference geometry as a
 * LargeBinary EWKB, followed by the per-instant pose fields)
 *
 * @details A temporal rigid geometry has no string input function; it is
 * built the canonical way, from a reference geometry plus a temporal pose
 * via #geo_tpose_to_trgeo (the same path the Arrow reader uses to
 * reconstruct it).
 */
static int
validate_trgeo(const char *label, const char *geo_wkt, const char *tpose_in_str)
{
  GSERIALIZED *gs = geom_in(geo_wkt, -1);
  if (! gs)
  {
    printf("[%s] FAIL: geom_in returned NULL\n", label);
    return 1;
  }
  Temporal *tp = tpose_in(tpose_in_str);
  if (! tp)
  {
    printf("[%s] FAIL: tpose_in returned NULL\n", label);
    free(gs);
    return 1;
  }
  Temporal *trgeo = geo_tpose_to_trgeo(gs, tp);
  free(gs);
  free(tp);
  return validate_temp(label, trgeo);
}

/* Canonical pcid = 1 schema (three int32_t X/Y/Z dimensions scaled by
 * 0.01), byte-identical to the one
 * mobilitydb/datagen/pointcloud/random_tpcpoint.sql installs in
 * pointcloud_formats and the one tpc_wkb_roundtrip.c's
 * PCPOINT_HEX_PCID1_111 layout matches. The point-cloud value leaf is
 * serialized with its pcid only; the schema is resolved out-of-band, so
 * registering it once lets meos_temporal_to_arrow compute the bbox and
 * the round-trip rebuild every tpcpoint/tpcpatch value. */
#define PCID1_SCHEMA_XML \
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" \
  "<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\"" \
  " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">" \
  "<pc:dimension><pc:position>1</pc:position><pc:size>4</pc:size>" \
  "<pc:description>X coordinate</pc:description><pc:name>X</pc:name>" \
  "<pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale>" \
  "</pc:dimension>" \
  "<pc:dimension><pc:position>2</pc:position><pc:size>4</pc:size>" \
  "<pc:description>Y coordinate</pc:description><pc:name>Y</pc:name>" \
  "<pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale>" \
  "</pc:dimension>" \
  "<pc:dimension><pc:position>3</pc:position><pc:size>4</pc:size>" \
  "<pc:description>Z coordinate</pc:description><pc:name>Z</pc:name>" \
  "<pc:interpretation>int32_t</pc:interpretation><pc:scale>0.01</pc:scale>" \
  "</pc:dimension>" \
  "<pc:metadata><Metadata name=\"srid\">0</Metadata></pc:metadata>" \
  "</pc:PointCloudSchema>"

/* A pcid = 1 SERIALIZED_POINT in hex: [4-byte vl_len_ slot][4-byte pcid
 * little-endian = 1][3 x 4-byte int32 dimensions]. The layout and the
 * (1,1,1) seed are the ones documented and exercised by the canonical
 * meos/examples/tpc_wkb_roundtrip.c; distinct points only change the
 * three trailing int32 little-endian dimension words. The hex-WKB body
 * is parsed schema-free so no schema is needed to build a tpcpoint. */
#define PCPOINT_HEX_111 "0000000001000000010000000100000001000000"
#define PCPOINT_HEX_222 "0000000001000000020000000200000002000000"
#define PCPOINT_HEX_333 "0000000001000000030000000300000003000000"

/* The MEOS schema cache, populated by main(). */
static PCSCHEMA *pcid1_schema = NULL;

/**
 * @brief Build a tpcpoint instant from a serialized hex point body
 * @details @p hexbody is parsed schema-free by pcpoint_hex_in (mirroring
 * tpc_wkb_roundtrip.c). The value Datum is the varlena pointer; MEOS's
 * Datum is uintptr_t so the (Datum) cast is the portable equivalent of
 * PostgreSQL's PointerGetDatum.
 */
static TInstant *
tpcpoint_inst(const char *hexbody, const char *ts)
{
  Pcpoint *pt = pcpoint_hex_in(hexbody);
  if (! pt)
    return NULL;
  return tinstant_make((Datum) pt, T_TPCPOINT, pg_timestamptz_in(ts, -1));
}

/**
 * @brief Build a tpcpatch instant from an explicit list of X/Y/Z points
 * @details Mirrors meos/test/pointcloud_valgrind.c: assemble a
 * PCPOINTLIST against the registered pcid = 1 PCSCHEMA, form an
 * uncompressed PCPATCH and serialize it to the SERIALIZED_PATCH varlena
 * that is the tpcpatch instant value. This is the canonical way a
 * standalone MEOS program builds a patch (in a PG backend PC_Patch does
 * it). The bytes are built fresh in-process, not hand-encoded.
 */
static TInstant *
tpcpatch_inst(const double (*pts)[3], int npts, const char *ts)
{
  PCPOINTLIST *pl = pc_pointlist_make(npts);
  PCDIMENSION *xd = pc_schema_get_dimension(pcid1_schema, 0);
  PCDIMENSION *yd = pc_schema_get_dimension(pcid1_schema, 1);
  PCDIMENSION *zd = pc_schema_get_dimension(pcid1_schema, 2);
  for (int i = 0; i < npts; i++)
  {
    PCPOINT *pt = pc_point_make(pcid1_schema);
    pc_double_to_ptr(pt->data + xd->byteoffset, xd->interpretation,
      pts[i][0]);
    pc_double_to_ptr(pt->data + yd->byteoffset, yd->interpretation,
      pts[i][1]);
    pc_double_to_ptr(pt->data + zd->byteoffset, zd->interpretation,
      pts[i][2]);
    pc_pointlist_add_point(pl, pt);
  }
  PCPATCH *patch = (PCPATCH *) pc_patch_uncompressed_from_pointlist(pl);
  SERIALIZED_PATCH *ser = meos_pc_patch_serialize(patch, NULL);
  if (! ser)
    return NULL;
  return tinstant_make((Datum) ser, T_TPCPATCH,
    pg_timestamptz_in(ts, -1));
}

/**
 * @brief Validate one already-built temporal point-cloud point value
 * (the per-instant value is an opaque int64-offset LargeBinary "Z" leaf
 * whose name "pcpoint" discriminates it from the general-geometry leaf)
 */
static int
validate_tpcpoint(const char *label, Temporal *temp)
{
  return validate_temp(label, temp);
}

/**
 * @brief Validate one already-built temporal point-cloud patch value
 * (same opaque LargeBinary "Z" leaf encoding as tpcpoint, leaf name
 * "pcpatch")
 */
static int
validate_tpcpatch(const char *label, Temporal *temp)
{
  return validate_temp(label, temp);
}

int
main(void)
{
  meos_initialize();

  /* Pre-populate the MEOS schema cache for pcid = 1 the canonical
   * standalone way (meos/examples/tpcbox_rtree.c). The point-cloud
   * value leaf serializes the pcid only; meos_temporal_to_arrow and the
   * round-trip resolve the schema out-of-band from this cache, exactly
   * as the PG backend resolves it from the pointcloud_formats catalog. */
  pcid1_schema = pc_schema_from_xml(PCID1_SCHEMA_XML);
  if (! pcid1_schema)
  {
    printf("[setup] FAIL: pc_schema_from_xml(pcid 1)\n");
    meos_finalize();
    return 1;
  }
  pcid1_schema->pcid = 1;
  meos_pc_schema_register(1, pcid1_schema);

  int rc = 0;

  /* ---- Decomposed-tier control: temporal circular buffer ---- */
  rc |= validate_cbuffer("cbuffer-instant",
    "Cbuffer(Point(1 2),0.5)@2000-01-01");
  rc |= validate_cbuffer("cbuffer-sequence-inclusive",
    "[Cbuffer(Point(1 2),0.5)@2000-01-01, "
    "Cbuffer(Point(3 4),1.5)@2000-01-02]");
  rc |= validate_cbuffer("cbuffer-sequence-exclusive-negatives",
    "(Cbuffer(Point(-1 -2),0.25)@2000-01-01, "
    "Cbuffer(Point(0 0),3)@2000-01-02, "
    "Cbuffer(Point(0 0),3)@2000-01-03)");
  rc |= validate_cbuffer("cbuffer-sequence-set",
    "{[Cbuffer(Point(1 1),0.5)@2000-01-01, "
    "Cbuffer(Point(2 2),1)@2000-01-02], "
    "[Cbuffer(Point(3 3),1.5)@2000-01-03, "
    "Cbuffer(Point(4 4),2)@2000-01-04]}");

  /* ---- Opaque tier: temporal geometry (LargeBinary "Z" leaf) ----
   * The literals mirror the canonical 024_temporal_arrow pg_regress
   * coverage (instant polygon/linestring, explicit SRID, 3D, discrete
   * mixed-geometry set, step sequence, linear point sequence, sequence
   * set). A temporal geometry sequence with a general geometry value is
   * step interpolated, so an exclusive upper bound would have to repeat
   * the last value; the canonical coverage uses closed bounds and a
   * negative-coordinate point exercises the negative-buffer path. */
  /* Instant: one pseudo-sequence of one instant. */
  rc |= validate_tgeometry("tgeometry-instant-polygon",
    "Polygon((1 1,4 4,7 1,1 1))@2000-01-01");
  /* Instant with an explicit SRID. */
  rc |= validate_tgeometry("tgeometry-instant-srid",
    "SRID=3812;Polygon((1 1,4 4,7 1,1 1))@2000-01-01");
  /* Instant with a 3D linestring value. */
  rc |= validate_tgeometry("tgeometry-instant-3d",
    "Linestring(1 1 1,2 2 2)@2000-01-01");
  /* Single-instant sequence. */
  rc |= validate_tgeometry("tgeometry-single-instant-seq",
    "[Point(1 2)@2000-01-01]");
  /* Discrete sequence mixing point, linestring, polygon values. */
  rc |= validate_tgeometry("tgeometry-discrete-mixed",
    "{Point(1 1)@2000-01-01, Linestring(1 1,3 3)@2000-01-02, "
    "Polygon((1 1,4 4,7 1,1 1))@2000-01-03}");
  /* Step sequence (closed bounds; the step rule needs no value repeat
   * when the upper bound is inclusive). */
  rc |= validate_tgeometry("tgeometry-step-sequence",
    "Interp=Step;[Linestring(1 1,2 2)@2000-01-01, Point(2 2)@2000-01-02, "
    "Point(1 1)@2000-01-03]");
  /* Linear point sequence with a negative coordinate. */
  rc |= validate_tgeometry("tgeometry-sequence-linear-negatives",
    "[Point(-1 -2)@2000-01-01, Point(0 0)@2000-01-02, "
    "Point(3 1)@2000-01-03]");
  /* Sequence set. */
  rc |= validate_tgeometry("tgeometry-sequence-set",
    "{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], "
    "[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}");

  /* ---- Opaque tier: temporal geography (LargeBinary "Z" leaf) ----
   * Same opaque encoding as temporal geometry with the geodetic flag
   * carried verbatim; literals mirror the canonical 024 coverage. */
  rc |= validate_tgeography("tgeography-instant",
    "Point(1 2)@2000-01-01");
  rc |= validate_tgeography("tgeography-single-instant-seq",
    "[Point(1 2)@2000-01-01]");
  rc |= validate_tgeography("tgeography-sequence-inclusive",
    "[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02, "
    "Point(3 3)@2000-01-03]");
  rc |= validate_tgeography("tgeography-sequence-set",
    "{[Point(1 1)@2000-01-01, Point(2 2)@2000-01-02], "
    "[Point(3 3)@2000-01-03, Point(4 4)@2000-01-04]}");

  /* ---- Opaque tier: temporal rigid geometry ----
   * value leaf = Struct{ref:LargeBinary EWKB, ...pose fields}. */
  /* Instant. */
  rc |= validate_trgeo("trgeo-instant",
    "Polygon((1 1,2 2,3 1,1 1))",
    "Pose(Point(1 1),0.5)@2000-01-01");
  /* Instant with an explicit SRID on the reference geometry. */
  rc |= validate_trgeo("trgeo-instant-srid",
    "SRID=4326;Polygon((1 1,2 2,3 1,1 1))",
    "SRID=4326;Pose(Point(1 2),0.5)@2000-01-01");
  /* Single-instant sequence. */
  rc |= validate_trgeo("trgeo-single-instant-seq",
    "Polygon((1 1,2 2,3 1,1 1))",
    "[Pose(Point(1 1),0.3)@2000-01-01]");
  /* Multi-instant linear sequence. */
  rc |= validate_trgeo("trgeo-sequence",
    "Polygon((1 1,2 2,3 1,1 1))",
    "[Pose(Point(1 1),0.3)@2000-01-01, Pose(Point(1 1),0.4)@2000-01-02, "
    "Pose(Point(1 1),0.5)@2000-01-03]");
  /* Step sequence (exclusive bounds with the step-interpolation rule). */
  rc |= validate_trgeo("trgeo-step-sequence",
    "Polygon((1 1,2 2,3 1,1 1))",
    "Interp=Step;[Pose(Point(1 2),0.5)@2000-01-01, "
    "Pose(Point(3 4),0.5)@2000-01-02]");
  /* Sequence set. */
  rc |= validate_trgeo("trgeo-sequence-set",
    "Polygon((1 1,2 2,3 1,1 1))",
    "Interp=Step;{[Pose(Point(1 1),0.2)@2000-01-01, "
    "Pose(Point(1 1),0.4)@2000-01-02, Pose(Point(1 1),0.5)@2000-01-03], "
    "[Pose(Point(2 2),0.6)@2000-01-04, Pose(Point(2 2),0.6)@2000-01-05]}");

  /* ---- Opaque tier: temporal point cloud point (LargeBinary "Z" leaf
   * named "pcpoint") ----
   * tpcpoint defaults to step interpolation, so sequences use closed
   * bounds; the shapes mirror the canonical 450_tpc_arrow pg_regress
   * coverage (instant, discrete sequence, step sequence, sequence set)
   * plus a single-instant sequence consistent with the other tiers. */
  rc |= validate_tpcpoint("tpcpoint-instant",
    (Temporal *) tpcpoint_inst(PCPOINT_HEX_111, "2024-01-01"));
  rc |= validate_tpcpoint("tpcpoint-single-instant-seq",
    (Temporal *) tsequence_make(
      (TInstant *[]){ tpcpoint_inst(PCPOINT_HEX_111, "2024-01-01") },
      1, true, true, STEP, true));
  rc |= validate_tpcpoint("tpcpoint-discrete-sequence",
    (Temporal *) tsequence_make(
      (TInstant *[]){ tpcpoint_inst(PCPOINT_HEX_111, "2024-01-01"),
        tpcpoint_inst(PCPOINT_HEX_222, "2024-01-02"),
        tpcpoint_inst(PCPOINT_HEX_333, "2024-01-03") },
      3, true, true, DISCRETE, true));
  rc |= validate_tpcpoint("tpcpoint-step-sequence",
    (Temporal *) tsequence_make(
      (TInstant *[]){ tpcpoint_inst(PCPOINT_HEX_111, "2024-01-01"),
        tpcpoint_inst(PCPOINT_HEX_222, "2024-01-02"),
        tpcpoint_inst(PCPOINT_HEX_333, "2024-01-03") },
      3, true, true, STEP, true));
  rc |= validate_tpcpoint("tpcpoint-sequence-set",
    (Temporal *) tsequenceset_make(
      (TSequence *[]){
        tsequence_make(
          (TInstant *[]){ tpcpoint_inst(PCPOINT_HEX_111, "2024-01-01"),
            tpcpoint_inst(PCPOINT_HEX_222, "2024-01-02") },
          2, true, true, STEP, true),
        tsequence_make(
          (TInstant *[]){ tpcpoint_inst(PCPOINT_HEX_333, "2024-01-03") },
          1, true, true, STEP, true) },
      2, true));

  /* ---- Opaque tier: temporal point cloud patch (LargeBinary "Z" leaf
   * named "pcpatch") ----
   * Patches are built fresh in-process from explicit X/Y/Z point lists
   * against the registered pcid = 1 schema, mirroring the canonical
   * 450_tpc_arrow PC_Patch(ARRAY[PC_MakePoint(1, ...)]) coverage. */
  {
    static const double s12[][3] = { {1, 1, 1}, {2, 2, 2} };
    static const double s56[][3] = { {5, 5, 5}, {6, 6, 6} };
    static const double s3[][3]  = { {3, 3, 3} };
    rc |= validate_tpcpatch("tpcpatch-instant",
      (Temporal *) tpcpatch_inst(s12, 2, "2024-01-01"));
    rc |= validate_tpcpatch("tpcpatch-single-instant-seq",
      (Temporal *) tsequence_make(
        (TInstant *[]){ tpcpatch_inst(s12, 2, "2024-01-01") },
        1, true, true, STEP, true));
    rc |= validate_tpcpatch("tpcpatch-discrete-sequence",
      (Temporal *) tsequence_make(
        (TInstant *[]){ tpcpatch_inst(s12, 2, "2024-01-01"),
          tpcpatch_inst(s56, 2, "2024-01-02"),
          tpcpatch_inst(s3, 1, "2024-01-03") },
        3, true, true, DISCRETE, true));
    rc |= validate_tpcpatch("tpcpatch-step-sequence",
      (Temporal *) tsequence_make(
        (TInstant *[]){ tpcpatch_inst(s12, 2, "2024-01-01"),
          tpcpatch_inst(s56, 2, "2024-01-02"),
          tpcpatch_inst(s3, 1, "2024-01-03") },
        3, true, true, STEP, true));
    rc |= validate_tpcpatch("tpcpatch-sequence-set",
      (Temporal *) tsequenceset_make(
        (TSequence *[]){
          tsequence_make(
            (TInstant *[]){ tpcpatch_inst(s12, 2, "2024-01-01"),
              tpcpatch_inst(s56, 2, "2024-01-02") },
            2, true, true, STEP, true),
          tsequence_make(
            (TInstant *[]){ tpcpatch_inst(s3, 1, "2024-01-03") },
            1, true, true, STEP, true) },
        2, true));
  }

  meos_finalize();
  printf("==== %s ====\n", rc ? "OVERALL FAIL" : "OVERALL PASS");
  return rc ? 1 : 0;
}
