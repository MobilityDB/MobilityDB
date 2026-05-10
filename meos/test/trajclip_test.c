/*****************************************************************************
 *
 * Verification harness for the Clipper2-backed trajectory-vs-polygon clip.
 * Each case below corresponds to a known correctness pitfall in the
 * earlier parity-sweep implementation (triangle: dropped diagonal edges
 * via an inverted horizontal-edge filter; diamond: monotonic parity
 * accumulator that never recognised exits). The harness prints whether
 * each output matches the expected inside-polygon segment.
 *
 * Build:
 *   gcc -Wall -g -O2 -I<prefix>/include -o trajclip_test trajclip_test.c \
 *     -L<prefix>/lib -lmeos
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>

static int
run_case(const char *name, const char *traj_in, const char *poly_in,
  const char *expect_out)
{
  Temporal *traj = (Temporal *) tgeompoint_in(traj_in);
  GSERIALIZED *poly = (GSERIALIZED *) geom_in(poly_in, -1);
  if (! traj || ! poly)
  {
    printf("[FAIL] %s — parse failed\n", name);
    return 1;
  }
  Temporal *clipped = tgeo_at_geom(traj, poly);
  char *got = clipped ? tspatial_as_text(clipped, 6) : strdup("(empty)");
  int pass = expect_out ? (strcmp(got, expect_out) == 0) : (clipped == NULL);
  printf("%s %s\n  in:       %s\n  poly:     %s\n  expected: %s\n  got:      %s\n",
    pass ? "[PASS]" : "[FAIL]", name, traj_in, poly_in,
    expect_out ? expect_out : "(empty)", got);
  free(got);
  if (clipped) free(clipped);
  free(traj);
  free(poly);
  return pass ? 0 : 1;
}

int
main(void)
{
  meos_initialize();
  meos_initialize_timezone("UTC");

  int fails = 0;

  /* Bug A repro — entry/exit on a triangle. The pre-fix fast clip dropped
   * every diagonal edge and returned an empty result. */
  fails += run_case("triangle entry/exit",
    "[POINT(0 5)@2026-01-01 00:00:00, POINT(10 5)@2026-01-01 02:00:00]",
    "POLYGON((0 0,10 0,5 10,0 0))",
    "{[POINT(2.5 5)@2026-01-01 00:30:00+00, POINT(7.5 5)@2026-01-01 01:30:00+00]}");

  /* Bug B repro — diamond. The parity sweep treated the exit at (10,0) as
   * a second entry and kept the inside-flag set through the trajectory's
   * end at (11,0). */
  fails += run_case("diamond entry/exit",
    "[POINT(-1 0)@2026-01-01 00:00:00, POINT(11 0)@2026-01-01 12:00:00]",
    "POLYGON((5 -5,10 0,5 5,0 0,5 -5))",
    "{[POINT(0 0)@2026-01-01 01:00:00+00, POINT(10 0)@2026-01-01 11:00:00+00]}");

  /* Trajectory entirely inside polygon — full sequence preserved. */
  fails += run_case("fully inside",
    "[POINT(2 2)@2026-01-01 00:00:00, POINT(3 3)@2026-01-01 01:00:00]",
    "POLYGON((0 0,10 0,10 10,0 10,0 0))",
    "{[POINT(2 2)@2026-01-01 00:00:00+00, POINT(3 3)@2026-01-01 01:00:00+00]}");

  /* Trajectory entirely outside polygon — empty result. */
  fails += run_case("fully outside",
    "[POINT(20 20)@2026-01-01 00:00:00, POINT(30 30)@2026-01-01 01:00:00]",
    "POLYGON((0 0,10 0,10 10,0 10,0 0))",
    NULL);

  /* Trajectory passes through a hole — should produce two inside-segments. */
  fails += run_case("polygon with hole",
    "[POINT(0 5)@2026-01-01 00:00:00, POINT(10 5)@2026-01-01 02:00:00]",
    "POLYGON((0 0,10 0,10 10,0 10,0 0),(3 3,7 3,7 7,3 7,3 3))",
    "{[POINT(0 5)@2026-01-01 00:00:00+00, POINT(3 5)@2026-01-01 00:36:00+00], [POINT(7 5)@2026-01-01 01:24:00+00, POINT(10 5)@2026-01-01 02:00:00+00]}");

  /* Quantisation regression: trajectory starts inside, exits on a slanted
   * segment whose CLIP_SCALE-quantised exit vertex is a few units off the
   * source segment's exact line. The original exact-collinearity test in
   * map_clipper_vertex_to_t fell through to the defensive fallback
   * (returning the sequence start), collapsing the inside-span and
   * dropping the entire result. Coordinates derive from
   * tnpoint_to_tgeompoint(Npoint(1, 0.2/0.4/0.5)) on the npoint test
   * fixture (route 1 in tbl_ways) — the failure that lit up
   * 056_tpoint_spatialfuncs_tbl, 087_tnpoint_spatialfuncs and
   * 087_tnpoint_spatialfuncs_tbl on PR #18's CI. */
  fails += run_case("quantised exit on slanted segment",
    "[POINT(69.790369 81.452098)@2000-01-01,"
    " POINT(55.752648 78.953813)@2000-01-02,"
    " POINT(48.718663 77.764071)@2000-01-03]",
    "POLYGON((50 50,50 100,100 100,100 50,50 50))",
    "{[POINT(69.790369 81.452098)@2000-01-01 00:00:00+00,"
    " POINT(55.752648 78.953813)@2000-01-02 00:00:00+00,"
    " POINT(50 77.980799)@2000-01-02 19:37:41.053145+00]}");

  /* Multi-path open-path output where each output path is two crossings on
   * the same segment (no input vertex inside the path). The earlier
   * cross_sq * other_lensq cross-multiply trick overflowed __int128 when
   * one candidate segment had a large cross product (~3.9e17), wrapping
   * the comparison and picking a wrong-but-near-collinear segment for the
   * boundary vertex — yielding a span_set with overlapping spans, which
   * temporal_merge_array (used by the at/minus round-trip in
   * 056_tpoint_spatialfuncs_tbl) rejects with "values cannot overlap on
   * time". Coordinates from tbl_tgeompoint k=52 vs tbl_geom k=25 in the
   * 056 fixture. */
  fails += run_case("multi-path overlap (int128 cross-mult overflow)",
    "[POINT(16.3546500261873 56.44954051822424)@2001-03-29 12:17:00+02,"
    " POINT(45.61119144782424 95.10431154631078)@2001-03-29 12:22:00+02,"
    " POINT(98.94688022322953 65.34966584295034)@2001-03-29 12:24:00+02,"
    " POINT(80.89245427399874 5.186562286689878)@2001-03-29 12:27:00+02,"
    " POINT(20.968852005898952 33.17127930931747)@2001-03-29 12:31:00+02]",
    "POLYGON((56.72718310374483 50.53721809824145,"
    "69.14995349943638 47.429162031039596,"
    "85.17494197003543 6.433171266689897,"
    "44.539342168718576 6.923481728881598,"
    "29.645028244704008 36.87097877264023,"
    "56.72718310374483 50.53721809824145))",
    "{[POINT(82.963951 12.089449)@2001-03-29 10:26:39.347482+00,"
    " POINT(81.280652 6.48016)@2001-03-29 10:26:56.129728+00],"
    " [POINT(78.038715 6.519277)@2001-03-29 10:27:11.429509+00,"
    " POINT(34.666556 26.774361)@2001-03-29 10:30:05.13933+00]}");

  meos_finalize();
  printf("\n%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
