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
 * @brief MEOS-side producer for the full-surface zero-MEOS Arrow and Parquet
 * consumption demo
 *
 * @details This is the MEOS-linked half of the demo. It builds a
 * representative population of temporal values covering the ENTIRE temporal
 * base type surface the per-type Arrow conformance covers — the scalar tier
 * (tint, tfloat, tbool, ttext, tbigint), the H3 index (th3index), the
 * decomposed point tier (tgeompoint, tgeogpoint), the decomposed extended
 * tier (tcbuffer, tnpoint, tpose in 2D and 3D), and the opaque tier
 * (tgeometry, tgeography, trgeometry, tpcpoint, tpcpatch). Each type's
 * population is sized so that, written with a small Parquet row-group size,
 * the resulting Parquet file spans several row groups. The original
 * tgeompoint and tfloat cases are kept inside the expanded set as the
 * regression guard.
 *
 * For each value it calls the conformance-proven #meos_temporal_to_arrow
 * kernel and exposes the produced `ArrowSchema`/`ArrowArray` over the
 * documented Arrow C Data Interface so that a zero-MEOS consumer (pyarrow,
 * loaded by the bridge with no libmeos symbols of its own) can import the
 * array purely through that ABI. It also emits, per row, the decomposed
 * value-leaf payload exactly as MEOS itself reports it for the original
 * `Temporal*` (computed through the SAME per-type accessors the
 * #meos_temporal_to_arrow kernel uses on the original value, NOT through
 * the Arrow export), so that a separate zero-MEOS process can verify the
 * Parquet round-trip bit/value-exactly without ever touching MEOS and with
 * no MEOS encoding re-implemented anywhere.
 *
 * `temporal_arrow.c` is untouched: this is a consumer-side demo built only
 * on the already-conformance-proven export kernel. The producer is the
 * MEOS-linked half and legitimately uses the MEOS internal headers — it is
 * the canonical oracle. Only the consumer is zero-MEOS.
 *
 * The values are laid out in SRID blocks (the first half SRID 4326, the
 * second half SRID 3857) for the SRID-bearing types so that the flat
 * top-level `srid` Arrow column carries block structure a Parquet reader
 * can use for honest row-group statistics pruning. The deeply nested
 * `seqs` column intentionally carries no such flat structure; the demo
 * measures and reports exactly what the nested schema does and does not
 * let a Parquet engine prune, per type.
 *
 * Built as a shared library; the Python bridge loads it with ctypes and
 * calls the exported functions. The bridge itself links no MEOS symbols.
 *
 * @code
 * gcc -Wall -fPIC -shared -I/usr/local/include -I<repo>/meos/include \
 *   -o libtemporal_arrow_parquet_producer.so \
 *   temporal_arrow_parquet_producer.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <meos.h>
#include <meos_arrow.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_geo.h>
#include "arrow_c_data_interface.h"

/* The producer is the MEOS-linked half and is the canonical oracle. It
 * uses only the installed PUBLIC MEOS headers — the per-type value
 * accessors MEOS itself exposes. No MEOS encoding is re-implemented; the
 * oracle reads the original Temporal* through MEOS's own accessors and
 * decomposes each instant into exactly the fields the
 * meos_temporal_to_arrow kernel puts in the value leaf. Each extended
 * subsystem header is gated by the build flag that compiles its
 * subsystem into libmeos. */
#if CBUFFER
  #include <meos_cbuffer.h>
#endif
#if NPOINT
  #include <meos_npoint.h>
#endif
#if POSE
  #include <meos_pose.h>
#endif
#if RGEO
  #include <meos_rgeo.h>
#endif
#if H3
  #include <meos_h3.h>
#endif
#if POINTCLOUD
  #include <meos_pointcloud.h>
  #include <pointcloud/pgsql_compat.h>
  #include "pc_api.h"
  #include "pc_api_internal.h"
#endif

/* Opaque MEOS value-object pointers reached from a Datum. DatumGetPointer
 * is the documented public MEOS macro (meos.h); these one-line casts are
 * the public equivalent of PostgreSQL's PG_GETARG_*_P and are NOT a
 * re-implementation of any encoding — the pointer is only ever passed to
 * MEOS's own public accessor functions. */
#if CBUFFER
  #define ProdGetCbufferP(d)  ((const Cbuffer *) DatumGetPointer(d))
#endif
#if NPOINT
  #define ProdGetNpointP(d)   ((const Npoint *)  DatumGetPointer(d))
#endif
#if POSE
  #define ProdGetPoseP(d)     ((const Pose *)    DatumGetPointer(d))
#endif
#if POINTCLOUD
  #define ProdGetPcpointP(d)  ((const Pcpoint *) DatumGetPointer(d))
  #define ProdGetPcpatchP(d)  ((const Pcpatch *) DatumGetPointer(d))
#endif

/* Population size per type. A few hundred rows per type so that, written
 * with a small Parquet row-group size, every per-type file spans several
 * row groups and a predicate on a flat top-level column can skip some. */
#define N_PER_TYPE 800

/* Two SRID blocks for the SRID-bearing types: the first half carries
 * SRID 4326, the second half SRID 3857. This is real block structure on
 * the flat top-level Arrow column, the only place row-group statistics
 * pruning is honestly available for this nested schema. */
#define SRID_A 4326
#define SRID_B 3857

/* The full base-type surface the per-type Arrow conformance covers. The
 * tag string is the stable per-type Parquet file suffix. */
typedef enum
{
  K_TINT, K_TFLOAT, K_TBOOL, K_TTEXT, K_TBIGINT,
  K_TGEOMPOINT, K_TGEOGPOINT,
  K_TCBUFFER, K_TNPOINT, K_TPOSE2D, K_TPOSE3D,
  K_TGEOMETRY, K_TGEOGRAPHY, K_TRGEOMETRY,
  K_TH3INDEX, K_TPCPOINT, K_TPCPATCH,
  K_NTYPES
} TypeKind;

static const char *kind_tag[K_NTYPES] = {
  "tint", "tfloat", "tbool", "ttext", "tbigint",
  "tgeompoint", "tgeogpoint",
  "tcbuffer", "tnpoint", "tpose2d", "tpose3d",
  "tgeometry", "tgeography", "trgeometry",
  "th3index", "tpcpoint", "tpcpatch"
};

/* Which kinds are actually built into this libmeos (flag-gated). A kind
 * that is not built is honestly skipped, never faked. */
static int kind_built[K_NTYPES];

typedef struct
{
  Temporal *t;
  TypeKind  kind;
} Row;

static Row *g_rows = NULL;
static int  g_count = 0;

#if POINTCLOUD
/* The pcid = 1 schema (three int32_t X/Y/Z dimensions scaled by 0.01).
 * The point-cloud value leaf serializes the pcid only; the schema is
 * resolved out-of-band, so registering it once lets the kernel build the
 * bbox and read every tpcpoint/tpcpatch value. */
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

/* A pcid = 1 SERIALIZED_POINT in hex, layout from the canonical
 * tpc_wkb_roundtrip.c: [4-byte vl_len_ slot][4-byte pcid LE = 1][3 x
 * 4-byte int32 LE dimensions]. Distinct points only change the three
 * trailing int32 LE dimension words; the body is parsed schema-free. */
static char pchex[64];
static const char *
pcpoint_hex(int a, int b, int c)
{
  snprintf(pchex, sizeof(pchex),
    "0000000001000000%02x000000%02x000000%02x000000",
    a & 0xff, b & 0xff, c & 0xff);
  return pchex;
}

static PCSCHEMA *pcid1_schema = NULL;

static TInstant *
tpcpoint_inst(int a, int b, int c, const char *ts)
{
  Pcpoint *pt = pcpoint_hex_in(pcpoint_hex(a, b, c));
  if (! pt)
    return NULL;
  return tinstant_make((Datum) pt, T_TPCPOINT, timestamptz_in(ts, -1));
}

/* Build a tpcpatch instant from explicit X/Y/Z points, mirroring the
 * canonical meos/test/pointcloud_valgrind.c: assemble a PCPOINTLIST
 * against the registered pcid = 1 schema, form an uncompressed PCPATCH
 * and serialize it. The bytes are built fresh, never hand-encoded. */
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
  return tinstant_make((Datum) ser, T_TPCPATCH, timestamptz_in(ts, -1));
}
#endif /* POINTCLOUD */

/**
 * @brief Build the full-surface temporal value population
 *
 * @details One contiguous block of #N_PER_TYPE rows per built type. Each
 * row is a short two- or three-instant value so the nested seqs/insts
 * lists are non-trivial. SRID-bearing types split their block into a
 * SRID 4326 first half and a SRID 3857 second half so the flat top-level
 * srid Arrow column carries honest row-group block structure. Types whose
 * subsystem is not compiled into this libmeos are honestly skipped.
 */
static void
build_population(void)
{
  for (int kk = 0; kk < K_NTYPES; kk++)
    kind_built[kk] = 1;
#if ! CBUFFER
  kind_built[K_TCBUFFER] = 0;
#endif
#if ! NPOINT
  kind_built[K_TNPOINT] = 0;
#endif
#if ! POSE
  kind_built[K_TPOSE2D] = 0;
  kind_built[K_TPOSE3D] = 0;
#endif
#if ! RGEO
  kind_built[K_TRGEOMETRY] = 0;
#endif
#if ! H3
  kind_built[K_TH3INDEX] = 0;
#endif
#if ! POINTCLOUD
  kind_built[K_TPCPOINT] = 0;
  kind_built[K_TPCPATCH] = 0;
#endif

  int nbuilt = 0;
  for (int kk = 0; kk < K_NTYPES; kk++)
    if (kind_built[kk])
      nbuilt++;
  g_rows = malloc(sizeof(Row) * (size_t) nbuilt * N_PER_TYPE);
  g_count = 0;
  char buf[1024];

  for (int kk = 0; kk < K_NTYPES; kk++)
  {
    if (! kind_built[kk])
      continue;
    for (int i = 0; i < N_PER_TYPE; i++)
    {
      int srid = (i < N_PER_TYPE / 2) ? SRID_A : SRID_B;
      int day = (i % 27) + 1;
      Temporal *t = NULL;
      switch ((TypeKind) kk)
      {
        case K_TINT:
        {
          int a = (i % 211) - 50, b = a + ((i % 5) - 2);
          snprintf(buf, sizeof(buf),
            "Interp=Step;[%d@2000-01-%02d, %d@2000-02-%02d, %d@2000-03-%02d]",
            a, day, b, day, b, day);
          t = tint_in(buf);
          break;
        }
        case K_TFLOAT:
        {
          double f0 = (i % 101) * 0.5 - 10.0;
          double f1 = f0 + ((i % 7) - 3) * 1.5;
          snprintf(buf, sizeof(buf),
            "[%.6f@2000-01-%02d, %.6f@2000-02-%02d]", f0, day, f1, day);
          t = tfloat_in(buf);
          break;
        }
        case K_TBOOL:
        {
          const char *v0 = (i & 1) ? "true" : "false";
          const char *v1 = (i & 2) ? "true" : "false";
          snprintf(buf, sizeof(buf),
            "Interp=Step;[%s@2000-01-%02d, %s@2000-02-%02d, %s@2000-03-%02d]",
            v0, day, v1, day, v1, day);
          t = tbool_in(buf);
          break;
        }
        case K_TTEXT:
        {
          snprintf(buf, sizeof(buf),
            "Interp=Step;[\"row %d alpha\"@2000-01-%02d, "
            "\"row %d beta\"@2000-02-%02d, "
            "\"row %d beta\"@2000-03-%02d]",
            i, day, i, day, i, day);
          t = ttext_in(buf);
          break;
        }
        case K_TBIGINT:
        {
          /* Non-sentinel values only; never INT64_MAX (the deferred
           * type-MAX sentinel crash). Large but well within range. */
          long long a = 9000000000000000000LL - (long long) i * 7;
          long long b = a - (long long) (i % 11);
          snprintf(buf, sizeof(buf),
            "Interp=Step;[%lld@2000-01-%02d, %lld@2000-02-%02d, "
            "%lld@2000-03-%02d]", a, day, b, day, b, day);
          t = tbigint_in(buf);
          break;
        }
        case K_TGEOMPOINT:
        {
          double x0 = (i % 97) + 0.25, y0 = (i % 89) + 0.5;
          snprintf(buf, sizeof(buf),
            "SRID=%d;[Point(%.4f %.4f)@2000-01-%02d, "
            "Point(%.4f %.4f)@2000-02-%02d]",
            srid, x0, y0, day, x0 + 1.0, y0 + 2.0, day);
          t = tgeompoint_in(buf);
          break;
        }
        case K_TGEOGPOINT:
        {
          double lon = ((i % 71) * 0.5) - 17.0;
          double lat = ((i % 53) * 0.5) - 13.0;
          snprintf(buf, sizeof(buf),
            "[Point(%.4f %.4f)@2000-01-%02d, "
            "Point(%.4f %.4f)@2000-02-%02d]",
            lon, lat, day, lon + 0.5, lat + 0.25, day);
          t = tgeogpoint_in(buf);
          break;
        }
#if CBUFFER
        case K_TCBUFFER:
        {
          double x0 = (i % 61) + 0.5, y0 = (i % 47) + 0.25;
          double r = ((i % 9) + 1) * 0.5;
          snprintf(buf, sizeof(buf),
            "SRID=%d;[Cbuffer(Point(%.4f %.4f),%.4f)@2000-01-%02d, "
            "Cbuffer(Point(%.4f %.4f),%.4f)@2000-02-%02d]",
            srid, x0, y0, r, day, x0 + 1.0, y0 + 1.0, r + 0.5, day);
          t = tcbuffer_in(buf);
          break;
        }
#endif
#if NPOINT
        case K_TNPOINT:
        {
          /* npoint value = route id + fraction; SRID is network-derived
           * (not in the value), so the srid slot stays 0. Routes 1..3. */
          int rid = (i % 3) + 1;
          double p0 = ((i % 50) + 1) / 100.0;
          double p1 = ((i % 40) + 5) / 100.0;
          snprintf(buf, sizeof(buf),
            "Interp=Step;[NPoint(%d, %.6f)@2000-01-%02d, "
            "NPoint(%d, %.6f)@2000-02-%02d, "
            "NPoint(%d, %.6f)@2000-03-%02d]",
            rid, p0, day, rid, p1, day, rid, p1, day);
          t = tnpoint_in(buf);
          break;
        }
#endif
#if POSE
        case K_TPOSE2D:
        {
          double x0 = (i % 73) + 0.5, y0 = (i % 67) + 0.25;
          double th = ((i % 31) - 15) * 0.1;
          snprintf(buf, sizeof(buf),
            "SRID=%d;[Pose(Point(%.4f %.4f),%.4f)@2000-01-%02d, "
            "Pose(Point(%.4f %.4f),%.4f)@2000-02-%02d]",
            srid, x0, y0, th, day, x0 + 1.0, y0 + 0.5, th + 0.05, day);
          t = tpose_in(buf);
          break;
        }
        case K_TPOSE3D:
        {
          double x0 = (i % 41) + 0.5, y0 = (i % 37) + 0.25;
          double z0 = (i % 29) + 0.75;
          /* A normalized quaternion (rotation about z by angle a). */
          double a = ((i % 24) * 0.13);
          double qw = cos(a / 2.0), qz = sin(a / 2.0);
          snprintf(buf, sizeof(buf),
            "SRID=%d;[Pose(Point(%.4f %.4f %.4f),%.6f,%.6f,%.6f,%.6f)"
            "@2000-01-%02d, "
            "Pose(Point(%.4f %.4f %.4f),%.6f,%.6f,%.6f,%.6f)@2000-02-%02d]",
            srid, x0, y0, z0, qw, 0.0, 0.0, qz, day,
            x0 + 1.0, y0 + 0.5, z0 + 0.25, qw, 0.0, 0.0, qz, day);
          t = tpose_in(buf);
          break;
        }
#endif
        case K_TGEOMETRY:
        {
          double x0 = (i % 59) + 0.5, y0 = (i % 43) + 0.25;
          if (i & 1)
            snprintf(buf, sizeof(buf),
              "SRID=%d;Interp=Step;[Polygon((%.3f %.3f,%.3f %.3f,"
              "%.3f %.3f,%.3f %.3f))@2000-01-%02d, "
              "Linestring(%.3f %.3f,%.3f %.3f)@2000-02-%02d, "
              "Linestring(%.3f %.3f,%.3f %.3f)@2000-03-%02d]",
              srid, x0, y0, x0 + 3, y0 + 3, x0 + 6, y0, x0, y0, day,
              x0, y0, x0 + 2, y0 + 1, day, x0, y0, x0 + 2, y0 + 1, day);
          else
            snprintf(buf, sizeof(buf),
              "SRID=%d;[Point(%.4f %.4f)@2000-01-%02d, "
              "Point(%.4f %.4f)@2000-02-%02d]",
              srid, x0, y0, day, x0 + 1.0, y0 + 1.0, day);
          t = tgeometry_in(buf);
          break;
        }
        case K_TGEOGRAPHY:
        {
          double lon = ((i % 61) * 0.5) - 15.0;
          double lat = ((i % 47) * 0.5) - 11.0;
          snprintf(buf, sizeof(buf),
            "[Point(%.4f %.4f)@2000-01-%02d, "
            "Point(%.4f %.4f)@2000-02-%02d]",
            lon, lat, day, lon + 0.5, lat + 0.25, day);
          t = tgeography_in(buf);
          break;
        }
#if RGEO
        case K_TRGEOMETRY:
        {
          double x0 = (i % 53) + 0.5, y0 = (i % 41) + 0.25;
          double th = ((i % 21) - 10) * 0.1;
          snprintf(buf, sizeof(buf),
            "SRID=%d;Polygon((0 0,2 0,2 1,0 1,0 0))", srid);
          GSERIALIZED *gs = geom_in(buf, -1);
          snprintf(buf, sizeof(buf),
            "SRID=%d;[Pose(Point(%.4f %.4f),%.4f)@2000-01-%02d, "
            "Pose(Point(%.4f %.4f),%.4f)@2000-02-%02d]",
            srid, x0, y0, th, day, x0 + 1.0, y0 + 0.5, th + 0.05, day);
          Temporal *tp = tpose_in(buf);
          if (gs && tp)
            t = geo_tpose_to_trgeometry(gs, tp);
          if (gs) free(gs);
          if (tp) free(tp);
          break;
        }
#endif
#if H3
        case K_TH3INDEX:
        {
          /* Real valid H3 cells (mirrors 290_th3index_arrow coverage). */
          static const char *cells[] = {
            "831c02fffffffff", "831c00fffffffff", "880326b885fffff",
            "880326b88dfffff", "831c04fffffffff" };
          const char *c0 = cells[i % 5];
          const char *c1 = cells[(i + 1) % 5];
          snprintf(buf, sizeof(buf),
            "Interp=Step;[%s@2001-01-%02d, %s@2001-02-%02d, "
            "%s@2001-03-%02d]", c0, day, c1, day, c1, day);
          t = th3index_in(buf);
          break;
        }
#endif
#if POINTCLOUD
        case K_TPCPOINT:
        {
          char d1[16], d2[16], d3[16];
          snprintf(d1, sizeof(d1), "2024-01-%02d", day);
          snprintf(d2, sizeof(d2), "2024-02-%02d", day);
          snprintf(d3, sizeof(d3), "2024-03-%02d", day);
          TInstant *insts[3] = {
            tpcpoint_inst(1 + (i % 7), 2 + (i % 5), 3 + (i % 3), d1),
            tpcpoint_inst(4 + (i % 6), 5 + (i % 4), 6 + (i % 2), d2),
            tpcpoint_inst(4 + (i % 6), 5 + (i % 4), 6 + (i % 2), d3) };
          if (insts[0] && insts[1] && insts[2])
            t = (Temporal *) tsequence_make(insts, 3, true, true, STEP,
              true);
          break;
        }
        case K_TPCPATCH:
        {
          char d1[16], d2[16];
          snprintf(d1, sizeof(d1), "2024-01-%02d", day);
          snprintf(d2, sizeof(d2), "2024-02-%02d", day);
          double a = i % 9, b = (i % 5) + 1;
          double s1[][3] = { {a, a + 1, a + 2}, {b, b + 1, b + 2} };
          double s2[][3] = { {a + 3, a + 4, a + 5} };
          TInstant *insts[2] = {
            tpcpatch_inst(s1, 2, d1), tpcpatch_inst(s2, 1, d2) };
          if (insts[0] && insts[1])
            t = (Temporal *) tsequence_make(insts, 2, true, true, STEP,
              true);
          break;
        }
#endif
        default:
          break;
      }
      g_rows[g_count].t = t;
      g_rows[g_count].kind = (TypeKind) kk;
      g_count++;
    }
  }
}

/**
 * @brief Initialize MEOS and build the population
 * @return The number of temporal values produced, or -1 on failure
 */
int
producer_init(void)
{
  meos_initialize();
#if POINTCLOUD
  /* Pre-populate the MEOS pcid = 1 schema cache the canonical standalone
   * way (meos/examples/tpcbox_rtree.c). The point-cloud value leaf
   * serializes the pcid only; the kernel resolves the schema out-of-band
   * from this cache, exactly as the PG backend resolves it from the
   * pointcloud_formats catalog. */
  pcid1_schema = pc_schema_from_xml(PCID1_SCHEMA_XML);
  if (! pcid1_schema)
  {
    fprintf(stderr, "producer_init: pc_schema_from_xml(pcid 1) failed\n");
    return -1;
  }
  pcid1_schema->pcid = 1;
  meos_pc_schema_register(1, pcid1_schema);
#endif
  build_population();
  for (int i = 0; i < g_count; i++)
    if (! g_rows[i].t)
    {
      fprintf(stderr, "producer_init: value %d (kind %s) is NULL\n",
        i, kind_tag[g_rows[i].kind]);
      return -1;
    }
  return g_count;
}

/** @brief Number of temporal values in the population */
int
producer_count(void)
{
  return g_count;
}

/** @brief Stable per-row Parquet-file tag (the base type name) */
const char *
producer_kind_tag(int i)
{
  if (i < 0 || i >= g_count)
    return "";
  return kind_tag[g_rows[i].kind];
}

/** @brief Comma-separated list of the base types that were actually built
 * into this libmeos (the genuine demo surface; flag-gated). */
const char *
producer_built_surface(void)
{
  static char s[512];
  s[0] = '\0';
  for (int kk = 0; kk < K_NTYPES; kk++)
    if (kind_built[kk])
    {
      if (s[0])
        strncat(s, ",", sizeof(s) - strlen(s) - 1);
      strncat(s, kind_tag[kk], sizeof(s) - strlen(s) - 1);
    }
  return s;
}

/**
 * @brief Export value @p i over the Arrow C Data Interface
 *
 * @details The caller (the zero-MEOS Python bridge) owns two
 * `ArrowSchema` and `ArrowArray` C structs and passes their addresses.
 * This fills them through the conformance-proven #meos_temporal_to_arrow
 * kernel. The consumer then imports them purely through the documented
 * ABI; the consumer never links or calls any MEOS symbol.
 *
 * @return 1 on success, 0 on failure
 */
int
producer_export(int i, void *schema_addr, void *array_addr)
{
  if (i < 0 || i >= g_count)
    return 0;
  struct ArrowSchema *schema = (struct ArrowSchema *) schema_addr;
  struct ArrowArray *array = (struct ArrowArray *) array_addr;
  return meos_temporal_to_arrow(g_rows[i].t, schema, array) ? 1 : 0;
}

/* Append the decomposed value-leaf payload for one instant value, exactly
 * as the meos_temporal_to_arrow kernel decomposes it, computed here from
 * the original value with MEOS's OWN accessors (never the Arrow export,
 * never a re-implemented encoding). */
static void
append_leaf(FILE *f, TypeKind kind, const Temporal *t, TimestampTz ts)
{
  switch (kind)
  {
    case K_TINT:
    {
      int v = 0;
      tint_value_at_timestamptz(t, ts, false, &v);
      fprintf(f, "%d", v);
      break;
    }
    case K_TFLOAT:
    {
      double v = 0.0;
      tfloat_value_at_timestamptz(t, ts, false, &v);
      fprintf(f, "%.17g", v);
      break;
    }
    case K_TBOOL:
    {
      bool v = false;
      tbool_value_at_timestamptz(t, ts, false, &v);
      fprintf(f, "%d", v ? 1 : 0);
      break;
    }
    case K_TTEXT:
    {
      text *txt = NULL;
      ttext_value_at_timestamptz(t, ts, false, &txt);
      char *c = text_to_cstring(txt);
      /* Hex-encode the UTF-8 bytes so the payload is delimiter-safe and
       * byte-exact regardless of content. */
      for (unsigned char *p = (unsigned char *) c; *p; p++)
        fprintf(f, "%02x", *p);
      free(c);
      free(txt);
      break;
    }
    case K_TBIGINT:
    {
      int64 v = 0;
      tbigint_value_at_timestamptz(t, ts, false, &v);
      fprintf(f, "%lld", (long long) v);
      break;
    }
    case K_TGEOMPOINT:
    case K_TGEOGPOINT:
    {
      Datum d;
      temporal_value_at_timestamptz(t, ts, false, &d);
      const GSERIALIZED *gs = DatumGetGserializedP(d);
      bool hasz = MEOS_FLAGS_GET_Z(t->flags);
      if (hasz)
      {
        const POINT3DZ *p = GSERIALIZED_POINT3DZ_P(gs);
        fprintf(f, "%.17g:%.17g:%.17g", p->x, p->y, p->z);
      }
      else
      {
        const POINT2D *p = GSERIALIZED_POINT2D_P(gs);
        fprintf(f, "%.17g:%.17g", p->x, p->y);
      }
      break;
    }
#if CBUFFER
    case K_TCBUFFER:
    {
      Datum d;
      temporal_value_at_timestamptz(t, ts, false, &d);
      const Cbuffer *cb = ProdGetCbufferP(d);
      /* cbuffer_point / cbuffer_radius are MEOS's own public accessors;
       * the leaf is Struct{x,y,r}. */
      GSERIALIZED *cg = cbuffer_point(cb);
      const POINT2D *p = GSERIALIZED_POINT2D_P(cg);
      fprintf(f, "%.17g:%.17g:%.17g", p->x, p->y, cbuffer_radius(cb));
      free(cg);
      break;
    }
#endif
#if NPOINT
    case K_TNPOINT:
    {
      Datum d;
      temporal_value_at_timestamptz(t, ts, false, &d);
      const Npoint *np = ProdGetNpointP(d);
      fprintf(f, "%lld:%.17g", (long long) npoint_route(np),
        npoint_position(np));
      break;
    }
#endif
#if POSE
    case K_TPOSE2D:
    {
      Pose *po = NULL;
      tpose_value_at_timestamptz(t, ts, false, &po);
      GSERIALIZED *pt = pose_to_point(po);
      const POINT2D *p = GSERIALIZED_POINT2D_P(pt);
      fprintf(f, "%.17g:%.17g:%.17g", p->x, p->y, pose_rotation(po));
      free(pt);
      free(po);
      break;
    }
    case K_TPOSE3D:
    {
      Pose *po = NULL;
      tpose_value_at_timestamptz(t, ts, false, &po);
      GSERIALIZED *pt = pose_to_point(po);
      const POINT3DZ *p = GSERIALIZED_POINT3DZ_P(pt);
      int qcount;
      double *q = pose_orientation(po, &qcount);   /* {W,X,Y,Z} */
      fprintf(f, "%.17g:%.17g:%.17g:%.17g:%.17g:%.17g:%.17g",
        p->x, p->y, p->z, q[0], q[1], q[2], q[3]);
      free(q);
      free(pt);
      free(po);
      break;
    }
#endif
    case K_TGEOMETRY:
    case K_TGEOGRAPHY:
    {
      Datum d;
      temporal_value_at_timestamptz(t, ts, false, &d);
      const GSERIALIZED *gs = DatumGetGserializedP(d);
      /* The opaque LargeBinary "Z" leaf carries exactly geo_as_ewkb's
       * bytes (extended NDR WKB); geo_as_hexewkb is its canonical hex
       * rendering. The consumer hexes the leaf bytes and compares. */
      char *h = geo_as_hexewkb(gs, "NDR");
      for (char *p = h; *p; p++)
        fputc((unsigned char) tolower((unsigned char) *p), f);
      free(h);
      break;
    }
#if RGEO
    case K_TRGEOMETRY:
    {
      /* Value leaf = Struct{ref:LargeBinary EWKB, ...pose fields}. The
       * reference geometry is shared (constant across instants); the
       * per-instant variable part is the pose, reached through MEOS's
       * own trgeometry_to_tpose + tpose value accessor. The constant ref
       * EWKB hex is appended once per row after a '#' (handled by the
       * caller). */
      Temporal *tp = trgeometry_to_tpose(t);
      Pose *po = NULL;
      tpose_value_at_timestamptz(tp, ts, false, &po);
      GSERIALIZED *pt = pose_to_point(po);
      bool is3d = MEOS_FLAGS_GET_Z(tp->flags);
      if (is3d)
      {
        const POINT3DZ *p = GSERIALIZED_POINT3DZ_P(pt);
        int qcount;
        double *q = pose_orientation(po, &qcount);
        fprintf(f, "%.17g:%.17g:%.17g:%.17g:%.17g:%.17g:%.17g",
          p->x, p->y, p->z, q[0], q[1], q[2], q[3]);
        free(q);
      }
      else
      {
        const POINT2D *p = GSERIALIZED_POINT2D_P(pt);
        fprintf(f, "%.17g:%.17g:%.17g", p->x, p->y, pose_rotation(po));
      }
      free(pt);
      free(po);
      free(tp);
      break;
    }
#endif
#if H3
    case K_TH3INDEX:
    {
      uint64 v = 0;
      th3index_value_at_timestamptz(t, ts, false, &v);
      fprintf(f, "%llu", (unsigned long long) v);
      break;
    }
#endif
#if POINTCLOUD
    case K_TPCPOINT:
    case K_TPCPATCH:
    {
      Datum d;
      temporal_value_at_timestamptz(t, ts, false, &d);
      /* The opaque "Z" leaf carries the raw varlena BODY (VARDATA, i.e.
       * the serialized value without its 4-byte varlena header).
       * pc{point,patch}_as_hexwkb is MEOS's own canonical hex of the
       * WHOLE varlena (header + body). Dropping the leading
       * 2 * VARHDRSZ = 8 hex chars (the 4-byte varlena length header,
       * a documented PostgreSQL constant, not a re-derived encoding)
       * yields exactly the leaf body — derived purely from MEOS's
       * public canonical serializer. */
      char *h = (kind == K_TPCPOINT)
        ? pcpoint_as_hexwkb(ProdGetPcpointP(d))
        : pcpatch_as_hexwkb(ProdGetPcpatchP(d));
      for (char *p = h + 8; *p; p++)
        fputc((unsigned char) tolower((unsigned char) *p), f);
      free(h);
      break;
    }
#endif
    default:
      fprintf(f, "?");
      break;
  }
}

/**
 * @brief Write the per-row decomposed value-leaf ground truth to @p path
 *
 * @details One line per value, in population order:
 *
 *   row_id|kind|subtype|interp|srid|t0:LEAF0,t1:LEAF1,...[#REFHEX]
 *
 * `kind` is the base type tag, `subtype`/`interp` are MEOS's own
 * #temporal_subtype / #temporal_interp strings, `srid` is the SRID the
 * Arrow top-level slot carries (#tspatial_srid for the SRID-bearing
 * types, 0 otherwise), `t` is the integer microsecond timestamp, and
 * `LEAF` is the value-leaf payload exactly as the kernel decomposes it,
 * computed from the original `Temporal*` with MEOS's own per-type
 * accessors. For a rigid geometry the constant reference-geometry EWKB
 * hex is appended once after a `#`. A zero-MEOS process reads the same
 * fields back from Parquet and compares them value-exact.
 *
 * @return 1 on success, 0 on failure
 */
int
producer_write_decomposed(const char *path)
{
  FILE *f = fopen(path, "w");
  if (! f)
    return 0;
  for (int i = 0; i < g_count; i++)
  {
    const Temporal *t = g_rows[i].t;
    TypeKind kind = g_rows[i].kind;
    const char *sub = temporal_subtype(t);
    const char *itp = temporal_interp(t);
    /* srid: EXACTLY the kernel's top-level slot rule
     * (temporal_arrow.c: is_point || is_cbuffer || is_pose || is_trgeo).
     * tgeometry/tgeography carry 0 in the slot — their SRID travels
     * inside the opaque EWKB leaf; tnpoint/tpc/scalars carry 0 too. */
    int srid = 0;
    if (kind == K_TGEOMPOINT || kind == K_TGEOGPOINT ||
        kind == K_TCBUFFER || kind == K_TPOSE2D || kind == K_TPOSE3D ||
        kind == K_TRGEOMETRY)
      srid = tspatial_srid(t);
    int nc = 0;
    TimestampTz *tss = temporal_timestamps(t, &nc);
    fprintf(f, "%d|%s|%s|%s|%d|", i, kind_tag[kind], sub, itp, srid);
    for (int k = 0; k < nc; k++)
    {
      if (k)
        fputc(',', f);
      fprintf(f, "%lld:", (long long) tss[k]);
      append_leaf(f, kind, t, tss[k]);
    }
#if RGEO
    if (kind == K_TRGEOMETRY)
    {
      GSERIALIZED *ref = trgeometry_geom(t);
      char *h = geo_as_hexewkb(ref, "NDR");
      fputc('#', f);
      for (char *p = h; *p; p++)
        fputc((unsigned char) tolower((unsigned char) *p), f);
      free(h);
      free(ref);
    }
#endif
    fputc('\n', f);
    free(tss);
  }
  fclose(f);
  return 1;
}

/**
 * @brief MEOS-side control: reconstruct value @p i from its own Arrow
 * export and return whether it is canonical-string-exact to the original
 *
 * @details Only a control on top of the external zero-MEOS verdict.
 *
 * @return 1 if the self round-trip matches, 0 otherwise
 */
int
producer_selfcheck(int i)
{
  if (i < 0 || i >= g_count)
    return 0;
  Temporal *back = meos_temporal_arrow_roundtrip(g_rows[i].t);
  if (! back)
    return 0;
  char *a = temporal_out(g_rows[i].t, 15);
  char *b = temporal_out(back, 15);
  int ok = (strcmp(a, b) == 0);
  free(a);
  free(b);
  free(back);
  return ok;
}

/** @brief Finalize MEOS and free the population */
void
producer_finalize(void)
{
  for (int i = 0; i < g_count; i++)
    free(g_rows[i].t);
  free(g_rows);
  g_rows = NULL;
  g_count = 0;
  meos_finalize();
}
