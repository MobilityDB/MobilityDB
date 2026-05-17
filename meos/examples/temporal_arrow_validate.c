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
 * Three value-leaf tiers are exercised:
 *
 * - The fully decomposed nested-Struct leaf, represented by the temporal
 *   circular buffer (`Struct{x,y,r}`) and the temporal pose. A 2D pose
 *   decomposes to `Struct{x,y,theta}` (three Float64 children); a 3D pose
 *   to `Struct{x,y,z,W,X,Y,Z}` (seven Float64 children, the trailing four
 *   being the orientation quaternion). The circular buffer is kept as the
 *   control discharged by the first conformance probe; the pose is
 *   validated here independently for both its 2D and its 3D field set.
 * - The opaque LargeBinary leaf and the rigid-geometry struct leaf:
 *   temporal geometry and geography carry their per-instant value as an
 *   opaque int64-offset LargeBinary "Z" leaf; a temporal rigid geometry
 *   carries a `Struct{ref:LargeBinary, ...pose fields}` whose leading
 *   "ref" child is the shared reference geometry as EWKB. These use a
 *   different value-leaf encoding (LargeBinary + int64 offsets) than the
 *   decomposed tier.
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
    char leafbuf[96];
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
              {
                /* Report the resolved child count so the pose 2D
                 * Struct{x,y,theta} (3) and 3D Struct{x,y,z,W,X,Y,Z}
                 * (7) field sets are visible in the verdict, distinct
                 * from the circular buffer Struct{x,y,r} (3). */
                snprintf(leafbuf, sizeof(leafbuf),
                  "decomposed Struct value leaf (%d children)",
                  (int) vleaf->n_children);
                leafdesc = leafbuf;
              }
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
 * @brief Validate one temporal pose value (the fully decomposed
 * nested-Struct value leaf: a 2D pose is `Struct{x,y,theta}`, a 3D pose is
 * `Struct{x,y,z,W,X,Y,Z}`)
 *
 * @details A temporal pose is built the canonical way through its string
 * input function #tpose_in; the 2D form is `Pose(Point(x y),theta)` and the
 * 3D form is `Pose(Point Z(x y z),W,X,Y,Z)` with a unit orientation
 * quaternion. nanoarrow walks into the value-leaf Struct and FULL-validates
 * its three (2D) or seven (3D) Float64 children, distinct from the circular
 * buffer `Struct{x,y,r}` representative; the pose field set is exercised
 * here on its own evidence.
 */
static int
validate_tpose(const char *label, const char *in)
{
  return validate_temp(label, tpose_in(in));
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

int
main(void)
{
  meos_initialize();

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

  /* ---- Decomposed tier: temporal pose ----
   * Validated independently of the circular buffer representative. A 2D
   * pose decomposes to a Struct{x,y,theta} (three Float64 children); a 3D
   * pose to a Struct{x,y,z,W,X,Y,Z} (seven Float64 children, the trailing
   * four being the orientation quaternion). Both field sets are exercised
   * across instant, sequence (inclusive and exclusive bounds, single
   * instant, negative theta) and sequence set. The literals mirror the
   * canonical 103_tpose_arrow pg_regress coverage; a pose is linearly
   * interpolated, so an exclusive upper bound needs no value repeat. */
  /* 2D poses: Struct{x,y,theta}. */
  rc |= validate_tpose("tpose-2d-instant",
    "Pose(Point(1 1),0.5)@2000-01-01");
  rc |= validate_tpose("tpose-2d-instant-srid",
    "SRID=3812;Pose(Point(1 2),1)@2000-01-01");
  rc |= validate_tpose("tpose-2d-single-instant-seq",
    "[Pose(Point(1 1),0.2)@2000-01-01]");
  rc |= validate_tpose("tpose-2d-sequence-inclusive",
    "[Pose(Point(1 1),0.2)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02, "
    "Pose(Point(3 3),0.9)@2000-01-03]");
  rc |= validate_tpose("tpose-2d-sequence-exclusive-negatives",
    "(Pose(Point(-1 -2),-0.5)@2000-01-01, "
    "Pose(Point(0 0),0)@2000-01-02, "
    "Pose(Point(3 1),-1.25)@2000-01-03)");
  rc |= validate_tpose("tpose-2d-discrete",
    "{Pose(Point(1 1),0.5)@2000-01-01, Pose(Point(2 2),1)@2000-01-02, "
    "Pose(Point(1 1),-0.5)@2000-01-03}");
  rc |= validate_tpose("tpose-2d-sequence-set",
    "{[Pose(Point(1 1),0.2)@2000-01-01, Pose(Point(2 2),0.5)@2000-01-02], "
    "[Pose(Point(3 3),0.6)@2000-01-04, Pose(Point(4 4),0.8)@2000-01-05]}");
  /* 3D poses: Struct{x,y,z,W,X,Y,Z} with a unit orientation quaternion. */
  rc |= validate_tpose("tpose-3d-instant",
    "Pose(Point Z(1 1 1),0.5,0.5,0.5,0.5)@2000-01-01");
  rc |= validate_tpose("tpose-3d-instant-srid",
    "SRID=3812;Pose(Point Z(1 2 3),1,0,0,0)@2000-01-01");
  rc |= validate_tpose("tpose-3d-single-instant-seq",
    "[Pose(Point Z(1 1 1),1,0,0,0)@2000-01-01]");
  rc |= validate_tpose("tpose-3d-sequence-inclusive",
    "[Pose(Point Z(1 1 1),0.5,0.5,0.5,0.5)@2000-01-01, "
    "Pose(Point Z(2 2 2),1,0,0,0)@2000-01-02]");
  rc |= validate_tpose("tpose-3d-discrete",
    "{Pose(Point Z(1 1 1),1,0,0,0)@2000-01-01, "
    "Pose(Point Z(2 2 2),0,1,0,0)@2000-01-02}");
  rc |= validate_tpose("tpose-3d-sequence-set",
    "{[Pose(Point Z(1 1 1),1,0,0,0)@2000-01-01, "
    "Pose(Point Z(2 2 2),0,0,0,1)@2000-01-02], "
    "[Pose(Point Z(3 3 3),0,1,0,0)@2000-01-04]}");

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

  meos_finalize();
  printf("==== %s ====\n", rc ? "OVERALL FAIL" : "OVERALL PASS");
  return rc ? 1 : 0;
}
