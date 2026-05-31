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
 * @brief Validate the MEOS closure-type Arrow C Data Interface export
 * against a canonical external Arrow consumer
 *
 * @details set, span and spanset are the MEOS closure types over a single
 * infinite base domain; tbox and stbox are the closure types over a product
 * (multi dimensional) domain (a box is literally a product of spans). This
 * builds one value of every set, span, span set, temporal box and
 * spatiotemporal box variant, converts each through #meos_set_to_arrow /
 * #meos_span_to_arrow / #meos_spanset_to_arrow / #meos_tbox_to_arrow /
 * #meos_stbox_to_arrow, and hands the produced
 * `ArrowSchema`/`ArrowArray` to nanoarrow, a small dependency-free
 * implementation of the Arrow C Data Interface. nanoarrow imports the schema
 * and array with no MEOS knowledge and runs full specification validation
 * (`ArrowArrayViewInitFromSchema` + `ArrowArrayViewSetArray` +
 * `ArrowArrayViewValidate` at the FULL level), which walks the entire nested
 * `Struct{ ..., List<...> }` tree including the per-base-type value leaf. The
 * export is also self-checked through the matching round-trip as a control.
 *
 * @code
 * gcc -Wall -g -I/usr/local/include -Inanoarrow -o setspan_arrow_validate \
 *   setspan_arrow_validate.c nanoarrow/nanoarrow.c -L/usr/local/lib -lmeos
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
#include <meos_geo.h>

static int g_rc = 0;

/* Run nanoarrow FULL-validate on a produced schema/array pair. */
static int
nano_validate(const char *label, struct ArrowSchema *schema,
  struct ArrowArray *array)
{
  int rc = 0;
  struct ArrowError error;
  struct ArrowArrayView view;
  memset(&view, 0, sizeof(view));
  if (ArrowArrayViewInitFromSchema(&view, schema, &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewInitFromSchema: %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
    goto done;
  }
  if (ArrowArrayViewSetArray(&view, array, &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewSetArray: %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
    goto done;
  }
  if (ArrowArrayViewValidate(&view, NANOARROW_VALIDATION_LEVEL_FULL,
      &error) != NANOARROW_OK)
  {
    printf("[%s] FAIL: nanoarrow ArrowArrayViewValidate(FULL): %s\n", label,
      ArrowErrorMessage(&error));
    rc = 1;
  }
done:
  ArrowArrayViewReset(&view);
  return rc;
}

static void
validate_set(const char *label, Set *s, char *(*printer)(const Set *))
{
  if (! s) { printf("[%s] FAIL: input NULL\n", label); g_rc = 1; return; }
  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_set_to_arrow(s, &schema, &array))
  { printf("[%s] FAIL: meos_set_to_arrow\n", label); g_rc = 1;
    free(s); return; }
  int rc = nano_validate(label, &schema, &array);
  Set *back = meos_set_arrow_roundtrip(s);
  if (! back) { printf("[%s] FAIL: roundtrip NULL\n", label); rc = 1; }
  else
  {
    char *a = printer(s); char *b = printer(back);
    if (strcmp(a, b) != 0)
    { printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
        label, a, b); rc = 1; }
    free(a); free(b); free(back);
  }
  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);
  g_rc |= rc;
  schema.release(&schema);
  array.release(&array);
  free(s);
}

static void
validate_span(const char *label, Span *s, char *(*printer)(const Span *))
{
  if (! s) { printf("[%s] FAIL: input NULL\n", label); g_rc = 1; return; }
  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_span_to_arrow(s, &schema, &array))
  { printf("[%s] FAIL: meos_span_to_arrow\n", label); g_rc = 1;
    free(s); return; }
  int rc = nano_validate(label, &schema, &array);
  Span *back = meos_span_arrow_roundtrip(s);
  if (! back) { printf("[%s] FAIL: roundtrip NULL\n", label); rc = 1; }
  else
  {
    char *a = printer(s); char *b = printer(back);
    if (strcmp(a, b) != 0)
    { printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
        label, a, b); rc = 1; }
    free(a); free(b); free(back);
  }
  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);
  g_rc |= rc;
  schema.release(&schema);
  array.release(&array);
  free(s);
}

static void
validate_ss(const char *label, SpanSet *s,
  char *(*printer)(const SpanSet *))
{
  if (! s) { printf("[%s] FAIL: input NULL\n", label); g_rc = 1; return; }
  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_spanset_to_arrow(s, &schema, &array))
  { printf("[%s] FAIL: meos_spanset_to_arrow\n", label); g_rc = 1;
    free(s); return; }
  int rc = nano_validate(label, &schema, &array);
  SpanSet *back = meos_spanset_arrow_roundtrip(s);
  if (! back) { printf("[%s] FAIL: roundtrip NULL\n", label); rc = 1; }
  else
  {
    char *a = printer(s); char *b = printer(back);
    if (strcmp(a, b) != 0)
    { printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
        label, a, b); rc = 1; }
    free(a); free(b); free(back);
  }
  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);
  g_rc |= rc;
  schema.release(&schema);
  array.release(&array);
  free(s);
}

static void
validate_tbox(const char *label, TBox *box)
{
  if (! box) { printf("[%s] FAIL: input NULL\n", label); g_rc = 1; return; }
  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_tbox_to_arrow(box, &schema, &array))
  { printf("[%s] FAIL: meos_tbox_to_arrow\n", label); g_rc = 1;
    free(box); return; }
  int rc = nano_validate(label, &schema, &array);
  TBox *back = meos_tbox_arrow_roundtrip(box);
  if (! back) { printf("[%s] FAIL: roundtrip NULL\n", label); rc = 1; }
  else
  {
    char *a = tbox_out(box, 15); char *b = tbox_out(back, 15);
    if (strcmp(a, b) != 0)
    { printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
        label, a, b); rc = 1; }
    free(a); free(b); free(back);
  }
  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);
  g_rc |= rc;
  schema.release(&schema);
  array.release(&array);
  free(box);
}

static void
validate_stbox(const char *label, STBox *box)
{
  if (! box) { printf("[%s] FAIL: input NULL\n", label); g_rc = 1; return; }
  struct ArrowSchema schema;
  struct ArrowArray array;
  if (! meos_stbox_to_arrow(box, &schema, &array))
  { printf("[%s] FAIL: meos_stbox_to_arrow\n", label); g_rc = 1;
    free(box); return; }
  int rc = nano_validate(label, &schema, &array);
  STBox *back = meos_stbox_arrow_roundtrip(box);
  if (! back) { printf("[%s] FAIL: roundtrip NULL\n", label); rc = 1; }
  else
  {
    char *a = stbox_out(box, 15); char *b = stbox_out(back, 15);
    if (strcmp(a, b) != 0)
    { printf("[%s] FAIL: self round-trip mismatch\n  in : %s\n  out: %s\n",
        label, a, b); rc = 1; }
    free(a); free(b); free(back);
  }
  if (rc == 0)
    printf("[%s] PASS: nanoarrow FULL-validate + self round-trip\n", label);
  g_rc |= rc;
  schema.release(&schema);
  array.release(&array);
  free(box);
}

static char *p_fset(const Set *s)     { return floatset_out(s, 15); }
static char *p_fspan(const Span *s)   { return floatspan_out(s, 15); }
static char *p_fss(const SpanSet *s)  { return floatspanset_out(s, 15); }
static char *p_gset(const Set *s)     { return spatialset_as_text(s, 15); }

int
main(void)
{
  meos_initialize();

  /* Sets: int, bigint, float, text, date, tstz, geometry, geography */
  validate_set("intset", intset_in("{1, 2, 3, 5, 8}"), intset_out);
  validate_set("bigintset",
    bigintset_in("{10000000000, 20000000000}"), bigintset_out);
  validate_set("floatset",
    floatset_in("{1.5, -3.25, 7, 42.125}"), p_fset);
  validate_set("textset",
    textset_in("{\"alpha\", \"beta\", \"gamma\"}"), textset_out);
  validate_set("dateset",
    dateset_in("{2000-01-01, 2000-06-15, 2001-12-31}"), dateset_out);
  validate_set("tstzset",
    tstzset_in("{2000-01-01 08:00:00+00, 2000-01-02 09:30:00+00}"),
    tstzset_out);
  validate_set("geomset",
    geomset_in("{Point(1 1), Point(2 3), Point(4 5)}"), p_gset);
  validate_set("geomset-srid",
    geomset_in("SRID=4326;{Point(1 1), Point(2 3)}"), p_gset);
  validate_set("geogset",
    geogset_in("{Point(1 1), Point(2 3)}"), p_gset);

  /* Spans: int, bigint, float, date, tstz */
  validate_span("intspan", intspan_in("[1, 10)"), intspan_out);
  validate_span("bigintspan",
    bigintspan_in("[100000000000, 200000000000]"), bigintspan_out);
  validate_span("floatspan", floatspan_in("[1.5, 9.75)"), p_fspan);
  validate_span("datespan",
    datespan_in("(2000-01-01, 2001-01-01]"), datespan_out);
  validate_span("tstzspan",
    tstzspan_in("[2000-01-01 00:00:00+00, 2000-12-31 23:59:59+00)"),
    tstzspan_out);

  /* Span sets: int, bigint, float, date, tstz */
  validate_ss("intspanset",
    intspanset_in("{[1, 3), [5, 8), [10, 12)}"), intspanset_out);
  validate_ss("bigintspanset",
    bigintspanset_in("{[1000000000, 2000000000], [9000000000, 9999999999]}"),
    bigintspanset_out);
  validate_ss("floatspanset",
    floatspanset_in("{[1.5, 2.5), (3, 4], [10.25, 11.75)}"), p_fss);
  validate_ss("datespanset",
    datespanset_in("{[2000-01-01, 2000-02-01), [2000-06-01, 2000-07-01)}"),
    datespanset_out);
  validate_ss("tstzspanset",
    tstzspanset_in("{[2000-01-01 00:00:00+00, 2000-01-02 00:00:00+00), "
      "[2000-03-01 00:00:00+00, 2000-04-01 00:00:00+00)}"),
    tstzspanset_out);

  /* Temporal boxes: time x value product domain, flag-gated dimensions
   * (X int, X float, T only, XT int, XT float) */
  validate_tbox("tbox-int-xt",
    tbox_in("TBOXINT XT([1,10),[2000-01-01,2000-01-05])"));
  validate_tbox("tbox-float-xt",
    tbox_in("TBOXFLOAT XT([1.5,9.75],[2000-01-01,2000-12-31))"));
  validate_tbox("tbox-int-x", tbox_in("TBOXINT X([1,10))"));
  validate_tbox("tbox-float-x", tbox_in("TBOXFLOAT X([1.5,9.75])"));
  validate_tbox("tbox-t", tbox_in("TBOX T([2000-01-01,2000-01-05])"));

  /* Spatiotemporal boxes: space x time product domain, flag-gated
   * dimensions (X 2D, Z 3D, T only, XT, ZT, SRID, geodetic) */
  validate_stbox("stbox-x", stbox_in("STBOX X((1,1),(2,2))"));
  validate_stbox("stbox-z", stbox_in("STBOX Z((1,1,1),(2,2,2))"));
  validate_stbox("stbox-t", stbox_in("STBOX T([2000-01-01,2000-01-05])"));
  validate_stbox("stbox-xt",
    stbox_in("STBOX XT(((1,1),(5,5)),[2000-01-01,2000-01-05])"));
  validate_stbox("stbox-zt",
    stbox_in("STBOX ZT(((1,1,1),(5,5,5)),[2000-01-01,2000-01-05])"));
  validate_stbox("stbox-srid",
    stbox_in("SRID=5676;STBOX X((1,1),(2,2))"));
  validate_stbox("stbox-geod-t",
    stbox_in("GEODSTBOX T([2000-01-01,2000-01-02])"));
  validate_stbox("stbox-geod-x",
    stbox_in("GEODSTBOX X((1,1),(1,1))"));
  validate_stbox("stbox-geod-srid-xt",
    stbox_in("SRID=4326;GEODSTBOX XT(((1,1),(5,5)),"
      "[2000-01-01,2000-01-05])"));

  meos_finalize();
  printf("\n%s\n", g_rc == 0 ? "OVERALL PASS" : "OVERALL FAIL");
  return g_rc;
}
