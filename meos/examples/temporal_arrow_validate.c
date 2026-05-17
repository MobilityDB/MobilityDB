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
 * @details This builds several temporal circular buffer values (instant,
 * inclusive sequence, exclusive-bounds sequence with negative coordinates,
 * and sequence set), converts each through #meos_temporal_to_arrow, and
 * hands the produced `ArrowSchema`/`ArrowArray` to nanoarrow, a small
 * dependency-free implementation of the Arrow C Data Interface. nanoarrow
 * imports the schema and array with no MEOS knowledge and runs full
 * specification validation (`ArrowArrayViewInitFromSchema` +
 * `ArrowArrayViewSetArray` + `ArrowArrayViewValidate` at the FULL level),
 * which walks the entire nested
 * `Struct{ ..., seqs:List<Struct{ ..., insts:List<Struct{t, v:Struct{x,y,r}}>}>}`
 * tree. tcbuffer exercises the fully decomposed nested-Struct value leaf, so
 * it is the representative probe for the decomposed export tier. The export
 * is also self-checked through #meos_temporal_arrow_roundtrip as a control.
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

/**
 * @brief Validate one temporal circular buffer value's Arrow export with the
 * external nanoarrow consumer and the MEOS self round-trip
 */
static int
validate_one(const char *label, const char *in)
{
  Temporal *temp = tcbuffer_in(in);
  if (! temp)
  {
    printf("[%s] FAIL: tcbuffer_in returned NULL\n", label);
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
   * sizes from the list offsets, and checks the list/struct child wiring. */
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

int
main(void)
{
  meos_initialize();

  int rc = 0;
  /* Instant: one pseudo-sequence of one instant. */
  rc |= validate_one("instant",
    "Cbuffer(Point(1 2),0.5)@2000-01-01");
  /* Inclusive-bounds sequence. */
  rc |= validate_one("sequence-inclusive",
    "[Cbuffer(Point(1 2),0.5)@2000-01-01, "
    "Cbuffer(Point(3 4),1.5)@2000-01-02]");
  /* Exclusive-bounds sequence with negative coordinates (tcbuffer is step
   * interpolation, so an exclusive upper bound repeats the last value). */
  rc |= validate_one("sequence-exclusive-negatives",
    "(Cbuffer(Point(-1 -2),0.25)@2000-01-01, "
    "Cbuffer(Point(0 0),3)@2000-01-02, "
    "Cbuffer(Point(0 0),3)@2000-01-03)");
  /* Sequence set: several component sequences. */
  rc |= validate_one("sequence-set",
    "{[Cbuffer(Point(1 1),0.5)@2000-01-01, "
    "Cbuffer(Point(2 2),1)@2000-01-02], "
    "[Cbuffer(Point(3 3),1.5)@2000-01-03, "
    "Cbuffer(Point(4 4),2)@2000-01-04]}");

  meos_finalize();
  printf("==== %s ====\n", rc ? "OVERALL FAIL" : "OVERALL PASS");
  return rc ? 1 : 0;
}
