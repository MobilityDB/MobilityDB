/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief A program that tests that a span, a set and a temporal value hold
 * finite timestamps and dates only
 * @details `infinity` and `-infinity` have no position on the time line a
 * span, a set or a temporal value bounds and interpolates over, so MEOS
 * refuses them where a timestamp or a date enters such a value
 * (#ensure_finite_timestamptz, #ensure_finite_date), as it refuses a
 * non-finite number (`nan_refusal_test.c`). The program reads `infinity` and
 * `-infinity` through each kind of entry point and verifies that each case
 * ends in an error: never a value, never NULL without an error, never a
 * crash. The cases are
 * - the text readers of the time spans, sets and span sets, of the temporal
 *   values and of the boxes;
 * - their binary readers, a value holding a sentinel instant being written as
 *   hex WKB and the bytes of that instant overwritten with an infinite
 *   timestamp or date;
 * - the MF-JSON reader of a temporal float, its sentinel instant overwritten
 *   with an infinite one;
 * - the constructors, conversions and set operations taking a timestamp or a
 *   date.
 *
 * Its controls verify that a finite input is still answered, that a malformed
 * timestamp reports its own error only, and that a predicate over an infinite
 * timestamp answers without an error. Each case runs in a child process, so a
 * case that crashes is reported as such and the others still run.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o infinite_time_refusal_test infinite_time_refusal_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <meos.h>
#include <meos_geo.h>

static int failures = 0, cases = 0;

/* The outcome of a case: 0 an error and no value, 1 a value, 3 a setup that
 * produced no sentinel to overwrite, so that the case would test nothing, 4 a
 * value returned with an error, 5 the expected answer, which no error may
 * accompany */
typedef int (*case_fn)(void);

static void
check(const char *what, case_fn fn)
{
  cases++;
  fflush(stdout);
  pid_t pid = fork();
  if (pid == 0)
  {
    meos_errno_reset();
    int r = fn();
    int err = meos_errno();
    if (r == 3)
      _exit(3);
    if (r == 4)
      _exit(err ? 4 : 1);
    if (r == 5)
      _exit(err ? 6 : 0);
    /* NULL without an error is a refusal nobody is told about */
    _exit(err ? 0 : (r == 0 ? 7 : 1));
  }
  int st;
  waitpid(pid, &st, 0);
  const char *verdict = NULL;
  if (WIFSIGNALED(st))
    verdict = "crashes";
  else if (WEXITSTATUS(st) == 1)
    verdict = "accepts the value";
  else if (WEXITSTATUS(st) == 3)
    verdict = "could not be set up";
  else if (WEXITSTATUS(st) == 4)
    verdict = "raises an error and still returns a value";
  else if (WEXITSTATUS(st) == 6)
    verdict = "raises an error over an input it answers";
  else if (WEXITSTATUS(st) == 7)
    verdict = "returns NULL without an error";
  else if (WEXITSTATUS(st) != 0)
    verdict = "exits";
  if (verdict)
  {
    printf("  %s: %s\n", what, verdict);
    failures++;
  }
}

/* A case returning a value fails whatever the error number says */
static int
result(void *v)
{
  free(v);
  return v ? 4 : 0;
}

/* The timestamps read */
static TimestampTz
pinf(void)
{
  return timestamptz_in("infinity", -1);
}
static TimestampTz
ninf(void)
{
  return timestamptz_in("-infinity", -1);
}
static TimestampTz
sentinel(void)
{
  return timestamptz_in("2001-01-01 00:00:00+00", -1);
}

/* The little-endian hex of an integer of @p n bytes */
static void
hex_le(uint64_t v, int n, char *out)
{
  for (int i = 0; i < n; i++)
    sprintf(out + 2 * i, "%02X", (unsigned) ((v >> (8 * i)) & 0xFF));
  out[2 * n] = '\0';
}

/* Replace the bytes of the sentinel instant in a hex WKB string by those of
 * an infinite timestamp, or of an infinite date when @p date */
static bool
patch_hex(char *hex, bool date, bool positive)
{
  char from[17], to[17];
  if (date)
  {
    hex_le((uint32_t) date_in("2001-01-01"), 4, from);
    hex_le((uint32_t) date_in(positive ? "infinity" : "-infinity"), 4, to);
  }
  else
  {
    hex_le((uint64_t) sentinel(), 8, from);
    hex_le((uint64_t) (positive ? pinf() : ninf()), 8, to);
  }
  char *p = hex ? strstr(hex, from) : NULL;
  if (p)
    memcpy(p, to, strlen(to));
  return p != NULL;
}

/* Text readers */
static int tstzspan_lower_text(void) { return result(tstzspan_in("[-infinity, 2001-01-01]")); }
static int tstzspan_upper_text(void) { return result(tstzspan_in("[2001-01-01, infinity]")); }
static int tstzset_text(void) { return result(tstzset_in("{2001-01-01, infinity}")); }
static int tstzset_alone_text(void) { return result(tstzset_in("{infinity}")); }
static int tstzspanset_text(void) { return result(tstzspanset_in("{[-infinity, 2001-01-01]}")); }
static int datespan_text(void) { return result(datespan_in("[2001-01-01, infinity]")); }
static int dateset_text(void) { return result(dateset_in("{-infinity}")); }
static int datespanset_text(void) { return result(datespanset_in("{[2001-01-01, infinity]}")); }
static int tfloat_inst_text(void) { return result(tfloat_in("1@infinity")); }
static int tfloat_ninst_text(void) { return result(tfloat_in("1@-infinity")); }
static int tfloat_seq_text(void) { return result(tfloat_in("[1@2001-01-01, 2@infinity]")); }
static int tfloat_disc_text(void) { return result(tfloat_in("{1@-infinity, 2@2001-01-01}")); }
static int tgeompoint_text(void) { return result(tgeompoint_in("Point(1 1)@infinity")); }
static int tbox_text(void) { return result(tbox_in("TBOXFLOAT XT([1, 2],[2001-01-01, infinity])")); }
static int stbox_text(void) { return result(stbox_in("STBOX T([-infinity, 2001-01-01])")); }

/* Binary readers, the sentinel overwritten by each infinite timestamp */
#define WKB_CASES(name, date, setup, writer, reader) \
  static int name##_wkb(bool positive) \
  { \
    __typeof__(setup) v = (setup); \
    size_t size; \
    char *hex = writer; \
    free(v); \
    int r = patch_hex(hex, date, positive) ? result(reader(hex)) : 3; \
    free(hex); \
    return r; \
  } \
  static int name##_wkb_pinf(void) { return name##_wkb(true); } \
  static int name##_wkb_ninf(void) { return name##_wkb(false); }

WKB_CASES(tstzspan, false, tstzspan_in("[2001-01-01 00:00:00+00, 2001-01-02 00:00:00+00]"),
  span_as_hexwkb(v, 0x08 | 0x04, &size), span_from_hexwkb)
WKB_CASES(tstzset, false, tstzset_in("{2001-01-01 00:00:00+00, 2001-01-02 00:00:00+00}"),
  set_as_hexwkb(v, 0x08 | 0x04, &size), set_from_hexwkb)
WKB_CASES(tstzspanset, false, tstzspanset_in("{[2001-01-01 00:00:00+00, 2001-01-02 00:00:00+00]}"),
  spanset_as_hexwkb(v, 0x08 | 0x04, &size), spanset_from_hexwkb)
WKB_CASES(datespan, true, datespan_in("[2001-01-01, 2001-01-05)"),
  span_as_hexwkb(v, 0x08 | 0x04, &size), span_from_hexwkb)
WKB_CASES(dateset, true, dateset_in("{2001-01-01, 2001-01-05}"),
  set_as_hexwkb(v, 0x08 | 0x04, &size), set_from_hexwkb)
WKB_CASES(tfloat, false, tfloat_in("[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00]"),
  temporal_as_hexwkb(v, 0x08 | 0x04, &size), temporal_from_hexwkb)
WKB_CASES(tbox, false, tbox_in("TBOXFLOAT XT([1, 2],[2001-01-01 00:00:00+00, 2001-01-02 00:00:00+00])"),
  tbox_as_hexwkb(v, 0x08 | 0x04, &size), tbox_from_hexwkb)

/* The MF-JSON reader, the sentinel instant overwritten by an infinite one */
static int
tfloat_mfjson(const char *token)
{
  Temporal *t = tfloat_in("[1@2001-01-01 00:00:00+00, 2@2001-01-02 00:00:00+00]");
  size_t size;
  char *doc = temporal_as_mfjson(t, 0, 0, 15, NULL, &size);
  free(t);
  const char *instant = "2001-01-02T00:00:00+00";
  char *p = doc ? strstr(doc, instant) : NULL;
  if (! p)
  {
    free(doc);
    return 3;
  }
  size_t n = strlen(token), m = strlen(instant);
  char *patched = malloc(strlen(doc) - m + n + 1);
  memcpy(patched, doc, (size_t) (p - doc));
  strcpy(patched + (p - doc), token);
  strcat(patched, p + m);
  free(doc);
  int r = result(tfloat_from_mfjson(patched));
  free(patched);
  return r;
}
static int tfloat_mfjson_pinf(void) { return tfloat_mfjson("infinity"); }
static int tfloat_mfjson_ninf(void) { return tfloat_mfjson("-infinity"); }

/* Constructors, conversions and set operations */
static int tstzspan_make_lower(void) { return result(tstzspan_make(ninf(), sentinel(), true, true)); }
static int tstzspan_make_upper(void) { return result(tstzspan_make(sentinel(), pinf(), true, true)); }
static int datespan_make_upper(void) { return result(datespan_make(date_in("2001-01-01"), date_in("infinity"), true, true)); }
static int tstz_to_set(void) { return result(timestamptz_to_set(pinf())); }
static int tstz_to_span(void) { return result(timestamptz_to_span(ninf())); }
static int tstz_to_spanset(void) { return result(timestamptz_to_spanset(pinf())); }
static int date_to_set_inf(void) { return result(date_to_set(date_in("-infinity"))); }
static int date_to_span_inf(void) { return result(date_to_span(date_in("infinity"))); }
static int date_to_spanset_inf(void) { return result(date_to_spanset(date_in("infinity"))); }
static int tfloatinst_make_inf(void) { return result(tfloatinst_make(1.0, pinf())); }
static int tstz_to_tbox(void) { return result(timestamptz_to_tbox(pinf())); }
static int float_tstz_to_tbox(void) { return result(float_timestamptz_to_tbox(1.0, ninf())); }
static int tstz_to_stbox(void) { return result(timestamptz_to_stbox(pinf())); }
static int int_tstz_to_tbox(void) { return result(int_timestamptz_to_tbox(1, pinf())); }

static int
numspan_tstz_to_tbox(void)
{
  Span *s = intspan_in("[1, 2]");
  int r = result(numspan_timestamptz_to_tbox(s, ninf()));
  free(s);
  return r;
}
static int
geo_tstz_to_stbox(void)
{
  GSERIALIZED *gs = geom_in("Point(1 1)", -1);
  int r = result(geo_timestamptz_to_stbox(gs, pinf()));
  free(gs);
  return r;
}

static int
union_set_inf(void)
{
  Set *s = tstzset_in("{2001-01-01}");
  int r = result(union_set_timestamptz(s, pinf()));
  free(s);
  return r;
}
static int
minus_span_inf(void)
{
  Span *s = tstzspan_in("[2001-01-01, 2001-01-02]");
  int r = result(minus_timestamptz_span(pinf(), s));
  free(s);
  return r;
}

/* Controls: a finite input is answered, a malformed one reports its error */
static int
answered(void *v)
{
  free(v);
  return v ? 5 : 1;
}
static int finite_tstzspan(void) { return answered(tstzspan_in("[2001-01-01, 2001-01-02]")); }
static int finite_tfloat(void) { return answered(tfloat_in("[1@2001-01-01, 2@2001-01-02]")); }
static int finite_to_span(void) { return answered(timestamptz_to_span(sentinel())); }
static int finite_datespan(void) { return answered(datespan_make(date_in("2001-01-01"), date_in("2001-01-05"), true, false)); }
static int malformed_tfloat(void) { return result(tfloat_in("1@garbage")); }
static int
predicate_inf(void)
{
  Span *s = tstzspan_in("[2001-01-01, 2001-01-02]");
  bool c = contains_span_timestamptz(s, pinf());
  free(s);
  return c ? 1 : 5;
}

/* A malformed timestamp reports its own error only: an error handler that
 * keeps the last message reads it */
static char last_message[256];
static void
last_message_handler(int level, int code, const char *msg)
{
  (void) level; (void) code;
  snprintf(last_message, sizeof(last_message), "%s", msg);
}
static int
malformed_message(void)
{
  meos_initialize_error_handler(last_message_handler);
  void *v = tfloat_in("1@garbage");
  free(v);
  if (v)
    return 4;
  return strstr(last_message, "garbage") ? 0 : 1;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  printf("Text readers:\n");
  check("tstzspan_in, -infinity lower bound", tstzspan_lower_text);
  check("tstzspan_in, infinity upper bound", tstzspan_upper_text);
  check("tstzset_in", tstzset_text);
  check("tstzset_in, infinity alone", tstzset_alone_text);
  check("tstzspanset_in", tstzspanset_text);
  check("datespan_in", datespan_text);
  check("dateset_in", dateset_text);
  check("datespanset_in", datespanset_text);
  check("tfloat_in, instant at infinity", tfloat_inst_text);
  check("tfloat_in, instant at -infinity", tfloat_ninst_text);
  check("tfloat_in, sequence", tfloat_seq_text);
  check("tfloat_in, discrete sequence", tfloat_disc_text);
  check("tgeompoint_in", tgeompoint_text);
  check("tbox_in", tbox_text);
  check("stbox_in", stbox_text);
  printf("Binary readers:\n");
  check("span_from_hexwkb of a tstzspan, infinity", tstzspan_wkb_pinf);
  check("span_from_hexwkb of a tstzspan, -infinity", tstzspan_wkb_ninf);
  check("set_from_hexwkb of a tstzset, infinity", tstzset_wkb_pinf);
  check("set_from_hexwkb of a tstzset, -infinity", tstzset_wkb_ninf);
  check("spanset_from_hexwkb of a tstzspanset, infinity", tstzspanset_wkb_pinf);
  check("spanset_from_hexwkb of a tstzspanset, -infinity", tstzspanset_wkb_ninf);
  check("span_from_hexwkb of a datespan, infinity", datespan_wkb_pinf);
  check("span_from_hexwkb of a datespan, -infinity", datespan_wkb_ninf);
  check("set_from_hexwkb of a dateset, infinity", dateset_wkb_pinf);
  check("set_from_hexwkb of a dateset, -infinity", dateset_wkb_ninf);
  check("temporal_from_hexwkb of a tfloat, infinity", tfloat_wkb_pinf);
  check("temporal_from_hexwkb of a tfloat, -infinity", tfloat_wkb_ninf);
  check("tbox_from_hexwkb, infinity", tbox_wkb_pinf);
  check("tbox_from_hexwkb, -infinity", tbox_wkb_ninf);
  printf("MF-JSON readers:\n");
  check("tfloat_from_mfjson, infinity", tfloat_mfjson_pinf);
  check("tfloat_from_mfjson, -infinity", tfloat_mfjson_ninf);
  printf("Constructors, conversions and set operations:\n");
  check("tstzspan_make, -infinity lower bound", tstzspan_make_lower);
  check("tstzspan_make, infinity upper bound", tstzspan_make_upper);
  check("datespan_make, infinity upper bound", datespan_make_upper);
  check("timestamptz_to_set", tstz_to_set);
  check("timestamptz_to_span", tstz_to_span);
  check("timestamptz_to_spanset", tstz_to_spanset);
  check("date_to_set", date_to_set_inf);
  check("date_to_span", date_to_span_inf);
  check("date_to_spanset", date_to_spanset_inf);
  check("tfloatinst_make", tfloatinst_make_inf);
  check("timestamptz_to_tbox", tstz_to_tbox);
  check("float_timestamptz_to_tbox", float_tstz_to_tbox);
  check("timestamptz_to_stbox", tstz_to_stbox);
  check("int_timestamptz_to_tbox", int_tstz_to_tbox);
  check("numspan_timestamptz_to_tbox", numspan_tstz_to_tbox);
  check("geo_timestamptz_to_stbox", geo_tstz_to_stbox);
  check("union_set_timestamptz", union_set_inf);
  check("minus_timestamptz_span", minus_span_inf);
  printf("Controls:\n");
  check("tstzspan_in, finite", finite_tstzspan);
  check("tfloat_in, finite", finite_tfloat);
  check("timestamptz_to_span, finite", finite_to_span);
  check("datespan_make, finite", finite_datespan);
  check("tfloat_in, malformed timestamp", malformed_tfloat);
  check("tfloat_in, malformed timestamp, its own message", malformed_message);
  check("contains_span_timestamptz, infinity", predicate_inf);

  printf("%d of %d case(s) failed\n", failures, cases);
  meos_finalize();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
