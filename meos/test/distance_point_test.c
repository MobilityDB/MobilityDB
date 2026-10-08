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
 * @brief A program that tests that the temporal distance of a point pair is the
 * distance between two points
 * @details The distance between two points is an exact rational of their
 * coordinates under a square root, and the answer is the double nearest it, a
 * tie going to the double whose last bit is even. Neither `hypot` of the
 * coordinate differences nor the root of their rounded squares is that double
 * on every pair: they are formulas with roundings of their own. The squared
 * form also leaves the range of a double long before the distance does, so it
 * answers that points a representable distance apart are INFINITELY far apart,
 * and that points a representable distance apart are in the SAME PLACE.
 *
 * Every expected value below is a constant, never a call to a formula, and
 * each is the one the CGAL judge `cgal_ptdist_round` accepts: it compares the
 * exact squared distance with the exact squares of the midpoints to the two
 * neighbours of the answer.
 *
 * The first part asks only what the integers settle: a 3-4-5 triangle scaled by
 * a power of two has every coordinate and every side exactly representable, and
 * `9 + 16 = 25` holds exactly, so a pair five units apart is five units apart
 * at every one of those scales and the answer carries no rounding at all.
 *
 * The second part carries the two pairs the squared form cannot represent, and
 * neither needs an oracle to check. The points `(0, 0)` and `(1e200, 1e200)`
 * stand `1.414213562373095e+200` apart, a perfectly ordinary double, while
 * `1e200 * 1e200` is infinity; and `(0, 0)` and `(1e-200, 1e-200)` stand
 * `1.414213562373095e-200` apart while `1e-200 * 1e-200` underflows to zero.
 * A kernel that squares answers `inf` and `0` for these.
 *
 * The third part carries the case that needs no overflow and no underflow at
 * all: `1e39` squares to an ordinary double whose root is `1e39` exactly, so
 * every intermediate value is representable, and the distance is nonetheless
 * above `FLT_MAX` -- the float32 maximum, which bounds no double distance. A
 * walk whose running minimum starts there answers that starting value for every
 * pair farther apart than it.
 *
 * The fourth part carries EXACT TIES, which no rounded formula is asked to
 * break: the integers of a Pythagorean triple whose hypotenuse `c` is an odd
 * integer in [2^53, 2^54), so the distance is exactly the midpoint of the two
 * doubles `c - 1` and `c + 1`, and the answer is the even one of them. The
 * first point then moves by a small `eps` either way along the first axis,
 * which leaves the rounded coordinate difference where it is and puts the
 * exact distance just below or just above that midpoint. A hypotenuse of two
 * integers is 1 modulo 4, so its tie rounds DOWN, and three coordinates carry
 * the tie that rounds UP. The fifth part carries three pairs of real AIS
 * vertices, in projected metres, on which `hypot` is not the nearest double.
 * The sixth asks a circular buffer at rest against a point, whose distance is
 * the distance less the radius: at the ties `5 - r` of a 3-4-5 triangle, and
 * where the radius is within a few units in the last place of the distance,
 * so that subtracting it from the rounded distance cancels and leaves an
 * error of percents rather than of a unit in the last place.
 * The seventh asks a point and a circular buffer at rest against one segment,
 * whose nearest point is the foot of the perpendicular: a pair in projected
 * metres, a radius a few units in the last place below the distance, a point
 * within 1e-7 of the segment's line, segments at 2^-500 and 2^500 and one
 * spanning +-1e308, and points whose distance is exactly the midpoint of two
 * doubles.
 * The eighth asks a moving point against a geometry, whose nearest approach
 * is the distance of its path to the geometry: parallel segments one or two
 * units in the last place apart, the points of a multipoint, and a line its
 * path crosses, where the distance is 0.
 *
 * Each part asks the nearest approach too, which reaches the same per-element
 * distance through the synchronous walk.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o distance_point_test distance_point_test.c -L/usr/local/lib -lmeos -lm
 * @endcode
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>

/**
 * @brief Return a temporal point that stays where it is over the period every
 * case shares, so the distance of a pair is the distance of two points
 */
static Temporal *
still(double x, double y)
{
  char buffer[256];
  snprintf(buffer, sizeof(buffer),
    "[POINT(%.17g %.17g)@2001-01-01, POINT(%.17g %.17g)@2001-01-02]",
    x, y, x, y);
  return tgeompoint_in(buffer);
}

/**
 * @brief Return true when a point geometry stands at the given coordinates
 * @details The comparison is of the binary forms, which carry every bit of
 * the coordinates, against a point written with 17 significant digits, which
 * read back a double exactly, as #still writes one. A comparison within a
 * tolerance would take two points 1e-200 apart for one
 */
static bool
point_at(const GSERIALIZED *gs, double x, double y)
{
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "POINT(%.17g %.17g)", x, y);
  GSERIALIZED *expected = geom_in(buffer, -1);
  char *hex1 = geo_as_hexewkb(gs, "XDR"), *hex2 = geo_as_hexewkb(expected, "XDR");
  bool result = hex1 && hex2 && strcmp(hex1, hex2) == 0;
  free(hex1); free(hex2); free(expected);
  return result;
}

/**
 * @brief Assert that every entry answering the distance of a pair of resting
 * points gives the distance between those points: the temporal distance of two
 * temporal points, the temporal distance of one against the other as a
 * geometry, the nearest approach of the two temporal points and of the first
 * against the second as a geometry, which the resting pair makes equal to it,
 * and the shortest line of the first against the second, which runs from the
 * one point to the other
 * @details Built on #still, as every case of this program is
 */
static void
every_entry(double px, double py, double qx, double qy, double expected)
{
  Temporal *p = still(px, py), *q = still(qx, qy);
  assert(p != NULL && q != NULL);

  /* two temporal points */
  Temporal *dist = tdistance_tgeo_tgeo(p, q, true);
  assert(dist != NULL);
  int count;
  double *values = tfloat_values(dist, &count);
  assert(count == 1);
  assert(values[0] == expected);
  free(values);
  free(dist);

  /* a temporal point and the second point as a geometry */
  char buffer[256];
  snprintf(buffer, sizeof(buffer), "POINT(%.17g %.17g)", qx, qy);
  GSERIALIZED *gs = geom_in(buffer, -1);
  assert(gs != NULL);
  Temporal *dist2 = tdistance_tgeo_geo(p, gs, true);
  assert(dist2 != NULL);
  values = tfloat_values(dist2, &count);
  assert(count == 1);
  assert(values[0] == expected);
  free(values);
  free(dist2);

  /* the nearest approach of two temporal points, and of a temporal point and a
   * geometry, which neither operand moving makes the distance itself */
  assert(nad_tgeo_tgeo(p, q, true) == expected);
  assert(nad_tgeo_geo(p, gs, true) == expected);

  /* the shortest line of a resting point to a point runs from the one to the
   * other, however near or far apart they stand */
  GSERIALIZED *line = shortestline_tgeo_geo(p, gs, true);
  assert(line != NULL);
  GSERIALIZED *start = line_point_n(line, 1), *end = line_point_n(line, 2);
  assert(point_at(start, px, py) && point_at(end, qx, qy));
  free(start);
  free(end);
  free(line);

  free(gs);
  free(p);
  free(q);
}

/**
 * @brief Return a temporal point with three coordinates that stays where it is
 * over the period every case shares, as #still does in two
 */
static Temporal *
still3d(double x, double y, double z)
{
  char buffer[320];
  snprintf(buffer, sizeof(buffer),
    "[POINT Z (%.17g %.17g %.17g)@2001-01-01, "
    "POINT Z (%.17g %.17g %.17g)@2001-01-02]", x, y, z, x, y, z);
  return tgeompoint_in(buffer);
}

/**
 * @brief Assert that the entries answering the distance of a pair of resting
 * points with three coordinates give the distance between those points: the
 * temporal distance of two temporal points and of one against the other as a
 * geometry, and the nearest approach of the two temporal points
 * @details The three-coordinate twin of #every_entry
 */
static void
every_entry3d(const double *p3, const double *q3, double expected)
{
  Temporal *p = still3d(p3[0], p3[1], p3[2]);
  Temporal *q = still3d(q3[0], q3[1], q3[2]);
  assert(p != NULL && q != NULL);

  Temporal *dist = tdistance_tgeo_tgeo(p, q, true);
  assert(dist != NULL);
  int count;
  double *values = tfloat_values(dist, &count);
  assert(count == 1);
  assert(values[0] == expected);
  free(values);
  free(dist);

  char buffer[256];
  snprintf(buffer, sizeof(buffer), "POINT Z (%.17g %.17g %.17g)", q3[0],
    q3[1], q3[2]);
  GSERIALIZED *gs = geom_in(buffer, -1);
  assert(gs != NULL);
  Temporal *dist2 = tdistance_tgeo_geo(p, gs, true);
  assert(dist2 != NULL);
  values = tfloat_values(dist2, &count);
  assert(count == 1);
  assert(values[0] == expected);
  free(values);
  free(dist2);

  assert(nad_tgeo_tgeo(p, q, true) == expected);

  free(gs);
  free(p);
  free(q);
}

/**
 * @brief Assert that the nearest approach of a circular buffer resting at a
 * centre to a point is the expected distance less the radius
 */
static void
buffer_entry(double cx, double cy, double r, double px, double py,
  double expected)
{
  char buffer[320];
  snprintf(buffer, sizeof(buffer), "[Cbuffer(Point(%.17g %.17g),%.17g)"
    "@2001-01-01, Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-02]", cx, cy, r,
    cx, cy, r);
  Temporal *temp = tcbuffer_in(buffer);
  snprintf(buffer, sizeof(buffer), "POINT(%.17g %.17g)", px, py);
  GSERIALIZED *gs = geom_in(buffer, -1);
  assert(temp != NULL && gs != NULL);
  assert(nad_tcbuffer_geo(temp, gs) == expected);
  free(gs);
  free(temp);
}

/**
 * @brief Assert that the nearest approach of a point, or of a circular buffer
 * of the given radius, resting at a centre to a segment is the expected
 * distance less the radius
 */
static void
segment_entry(double cx, double cy, double r, double ax, double ay, double bx,
  double by, double expected)
{
  char buffer[320];
  if (r == 0.0)
    snprintf(buffer, sizeof(buffer), "[POINT(%.17g %.17g)@2001-01-01, "
      "POINT(%.17g %.17g)@2001-01-02]", cx, cy, cx, cy);
  else
    snprintf(buffer, sizeof(buffer), "[Cbuffer(Point(%.17g %.17g),%.17g)"
      "@2001-01-01, Cbuffer(Point(%.17g %.17g),%.17g)@2001-01-02]", cx, cy, r,
      cx, cy, r);
  Temporal *temp = (r == 0.0) ? tgeompoint_in(buffer) : tcbuffer_in(buffer);
  snprintf(buffer, sizeof(buffer), "LINESTRING(%.17g %.17g, %.17g %.17g)", ax,
    ay, bx, by);
  GSERIALIZED *gs = geom_in(buffer, -1);
  assert(temp != NULL && gs != NULL);
  double d = (r == 0.0) ? nad_tgeo_geo(temp, gs, true) :
    nad_tcbuffer_geo(temp, gs);
  assert(d == expected);
  free(gs);
  free(temp);
}

/* The nearest approach of a moving point to a geometry, which is the distance
 * of its path to the geometry, against the double nearest the exact one; the
 * moving twin of #segment_entry */
static void
moving_entry(const char *trip, const char *wkt, double expected)
{
  Temporal *temp = tgeompoint_in(trip);
  GSERIALIZED *gs = geom_in(wkt, -1);
  assert(temp != NULL && gs != NULL);
  double d = nad_tgeo_geo(temp, gs, true);
  assert(d == expected);
  free(gs);
  free(temp);
}

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();
  meos_initialize_timezone("UTC");

  /* The 3-4-5 triangle at every scale its products stay exact over: the
   * distance is five units and carries no rounding */
  int asked = 0;
  for (int exponent = -30; exponent <= 30; exponent++)
  {
    double s = ldexp(1.0, exponent);
    every_entry(0, 0, 3 * s, 4 * s, 5 * s);
    every_entry(3 * s, 4 * s, 0, 0, 5 * s);
    /* Two points in the same place are no distance apart */
    every_entry(3 * s, 4 * s, 3 * s, 4 * s, 0.0);
    asked += 3;
  }

  /* The two pairs whose squares leave the range of a double while their
   * distance does not: the first is finite, where a squared form answers
   * infinity, and the second is not zero, where a squared form answers that
   * the points coincide. The constants are 1.414213562373095e+200 and
   * 1.414213562373095e-200 */
  const double far = 0x1.d8f9811335b57p+664;
  const double near = 0x1.151f68876f410p-664;
  assert(isfinite(far) && far > 1.0e200);
  assert(near > 0.0 && near < 1.0e-199);
  /* the squared form really does lose both */
  assert(! isfinite(sqrt(1e200 * 1e200 + 1e200 * 1e200)));
  assert(sqrt(1e-200 * 1e-200 + 1e-200 * 1e-200) == 0.0);

  every_entry(0, 0, 1e200, 1e200, far);
  every_entry(0, 0, 1e-200, 1e-200, near);
  /* and with one coordinate alone, where the square overflows just as surely */
  every_entry(0, 0, 1e200, 0, 1e200);
  every_entry(0, 0, 1e-200, 0, 1e-200);
  asked += 4;

  /* A multipoint whose farther point comes first, so that once the first is
   * read, a bound on the distance decides whether the nearer one is: the
   * nearest approach and the shortest line reach the nearer point at both ends
   * of the range, where the square of either distance is not a double */
  const double multi[][2] = {{2e-200, 1e-200}, {2e200, 1e200}};
  for (int i = 0; i < 2; i++)
  {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "MULTIPOINT(%.17g 0, %.17g 0)",
      multi[i][0], multi[i][1]);
    Temporal *p = still(0, 0);
    GSERIALIZED *gs = geom_in(buffer, -1);
    assert(p != NULL && gs != NULL);
    assert(nad_tgeo_geo(p, gs, true) == multi[i][1]);
    GSERIALIZED *line = shortestline_tgeo_geo(p, gs, true);
    assert(line != NULL);
    GSERIALIZED *end = line_point_n(line, 2);
    assert(point_at(end, multi[i][1], 0));
    free(end);
    free(line);
    free(gs);
    free(p);
  }
  asked += 2;

  /* The case that needs NO overflow and no underflow: 1e39 squares to an
   * ordinary double and the root of that square is 1e39 exactly, so every
   * intermediate value is representable -- and the distance is still above
   * FLT_MAX, which is the float32 maximum and no bound on a double distance at
   * all. A walk whose running minimum starts there answers its own starting
   * value for every pair farther apart than that */
  assert(isfinite(1e39 * 1e39));
  assert(sqrt(1e39 * 1e39) == 1e39);
  assert(1e39 > (double) FLT_MAX);
  every_entry(0, 0, 1e39, 0, 1e39);
  /* 1.4142135623730948e+39 */
  every_entry(0, 0, 1e39, 1e39, 0x1.09fbe7fbf5380p+130);
  asked += 2;

  /* EXACT TIES. 6420259854305159^2 + 6420260338747080^2 = 9079618902470041^2,
   * the hypotenuse odd, so the distance is the midpoint of the doubles
   * 9079618902470040 and 9079618902470042 and rounds to the first, whose last
   * bit is even; moved by eps the other way, it rounds up. Each scale by a
   * power of two is exact */
  const double a2 = 6420259854305159.0, b2 = 6420260338747080.0;
  const double down2 = 0x1.020eec1b056ccp+53, up2 = 0x1.020eec1b056cdp+53;
  /* 902950485365053^2 + 8863009057253318^2 + 3702825674676854^2 =
   * 9647754511025943^2, a hypotenuse 3 modulo 4, so the tie rounds UP to
   * 9647754511025944 */
  const double a3[3] = {902950485365053.0, 8863009057253318.0,
    3702825674676854.0};
  const double down3 = 0x1.1234a7241438bp+53, up3 = 0x1.1234a7241438cp+53;
  const int scales[3] = {-600, 0, 600};
  for (int i = 0; i < 3; i++)
  {
    int e = scales[i];
    double eps = ldexp(1.0, e - 20);
    double qx = ldexp(a2, e), qy = ldexp(b2, e);
    every_entry(0, 0, qx, qy, ldexp(down2, e));
    every_entry(eps, 0, qx, qy, ldexp(down2, e));
    every_entry(- eps, 0, qx, qy, ldexp(up2, e));
    const double q3[3] = {ldexp(a3[0], e), ldexp(a3[1], e), ldexp(a3[2], e)};
    const double o3[3] = {0, 0, 0}, pe[3] = {eps, 0, 0}, me[3] = {- eps, 0, 0};
    every_entry3d(o3, q3, ldexp(up3, e));
    every_entry3d(pe, q3, ldexp(down3, e));
    every_entry3d(me, q3, ldexp(up3, e));
    asked += 6;
  }

  /* Real AIS vertices in projected metres on which `hypot` is not the nearest
   * double: 608374.7395221737, 665373.8546375738 and 638693.3597953286 */
  every_entry(407910.75106787268, 5992281.7599931061, 965422.17747977306,
    6235799.3838087982, 0x1.290ed7aa2a67dp+19);
  every_entry(825741.35049306252, 6030501.7351777172, 171449.35708826367,
    6151432.9296712717, 0x1.44e3bb5930e5bp+19);
  every_entry(171453.53092184389, 6151432.617599302, 803556.61574629182,
    6059918.14878665, 0x1.37dcab83717e3p+19);
  asked += 3;

  /* A circular buffer at rest. The point stands exactly 5 from the centre and
   * the radius is j * 2^-51, so 5 - r is the midpoint of two doubles and rounds
   * to the even one */
  buffer_entry(0, 0, ldexp(1.0, -51), 3, 4, 0x1.4000000000000p+2);
  buffer_entry(0, 0, ldexp(3.0, -51), 3, 4, 0x1.3fffffffffffep+2);
  buffer_entry(0, 0, ldexp(5.0, -51), 3, 4, 0x1.3fffffffffffep+2);
  buffer_entry(0, 0, ldexp(7.0, -51), 3, 4, 0x1.3fffffffffffcp+2);
  /* Radii a few units in the last place below the distance, in projected
   * metres: 1.2387501666881207e-12, 8.420064680985333e-13 and
   * 3.8441253815593e-12 */
  buffer_entry(658164.288, 5107229.515, 790.7534945110075, 658704.4783326653,
    5106652.032971209, 0x1.5cad5b4bd131ep-40);
  buffer_entry(403480.345, 5356924.818, 245.95254677553052, 403718.29958717315,
    5356862.606508227, 0x1.da01eba69dad5p-41);
  buffer_entry(350907.494, 5247885.866, 1021.3601095091641, 351595.69282486296,
    5248640.5572287515, 0x1.0e819b4674be7p-38);
  /* A point 2^-440 off the circle of radius 2^100, whose distance to the
   * buffer, about 2^-881 squared over twice the radius, is 2^-981 */
  buffer_entry(0, 0, 0x1p+100, 0x1p+100, 0x1p-440, 0x1p-981);
  /* A centre and a point whose difference, one and a half times the largest
   * double, is beyond it, the radius half of it, the distance to the buffer
   * the largest double */
  buffer_entry(-DBL_MAX / 2, 0, DBL_MAX / 2, DBL_MAX, 0, DBL_MAX);
  asked += 9;

  printf("%d distances answered as the distance between two points, 61 scales, "
    "4 pairs whose squares leave the range of a double, 2 multipoints whose "
    "nearer point a squared bound would skip, 2 beyond FLT_MAX whose every "
    "intermediate value is representable, 18 exact ties in two and three "
    "coordinates, 3 real pairs on which hypot is not the nearest double and 9 "
    "circular buffers at rest\n", asked);

  /* A point and a circular buffer at rest against one segment */
  int segments = 8;
  segment_entry(516962.74227634474, 5107948.610185236, 0, 516931.81200000003,
    5108222.836, 516882.48163457355, 5107913.724103649, 0x1.270a244e55fa5p+6);
  segment_entry(522708.31187730626, 5394638.409778467, 31.30297357509829,
    522836.172, 5394682.178, 522393.30386973545, 5394639.504396664,
    0x1.720bde420e71dp-43);
  segment_entry(444701.4571792798, 5328803.155586255, 0, 444840.779,
    5328781.864, 444558.37996268016, 5328825.0210837815, 0x1.a5ce9ea9cb377p-24);
  segment_entry(-2.4086662620552625e-152, -2.7425309156430855e-151, 0,
    1.4867657329013218e-151, -1.973264725736441e-151, 1.3305241696727391e-152,
    7.838752007775803e-152, 0x1.3cbb240cac384p-501);
  segment_entry(-1.5285551538279573e+150, -2.0949832398599417e+149, 0,
    -1.8044225536862182e+150, 3.656650249579651e+148, -2.5327694085907134e+150,
    -9.107457317795018e+149, 0x1.cd54da28c5143p+496);
  segment_entry(1.9065672791409535e+299, 2.2529152754140272e+297, 0, -1e+308,
    0, 1e+308, 0, 0x1.b8f089ae83eb9p+987);
  /* Exactly the midpoint of two doubles: 5j for an odd j, at two scales */
  segment_entry(-5.866124092063631e+29, 4.399593069047723e+29, 0,
    -2.4338891524382005e+32, -3.2451855365842673e+32, 2.4338891524382005e+32,
    3.2451855365842673e+32, 0x1.2829e07aa8fd4p+99);
  segment_entry(-1.0082578453796613e-07, 7.56193384034746e-08, 0,
    -4.57763671875e-05, -6.103515625e-05, 4.57763671875e-05, 6.103515625e-05,
    0x1.0ea6f398bec6ep-23);
  printf("%d points and circular buffers at rest against a segment answered as "
    "the nearest double\n", segments);

  /* A moving point against parallel segments one or two units in the last
   * place apart, against the points of a multipoint, and across a line its
   * path crosses, where the distance is 0 */
  int moving = 3;
  moving_entry("[POINT(657655.292 5181685.274)@2001-01-01, "
    "POINT(657705.292 5181685.274)@2001-01-02, "
    "POINT(657755.292 5181685.274)@2001-01-03, "
    "POINT(657805.292 5181685.274)@2001-01-04, "
    "POINT(657855.292 5181685.274)@2001-01-05]",
    "MULTILINESTRING((657650.8387083027 5181686.215331687, "
    "657837.1602424956 5181686.215331687), "
    "(657661.8867602018 5181686.215331687, "
    "657932.2051257079 5181686.215331687), "
    "(657675.1708392231 5181686.215331684, "
    "657854.8218127734 5181686.215331684))", 0x1.e1f639f800000p-1);
  moving_entry("[POINT(467291.059 5256659.43686811)@2001-01-01, "
    "POINT(467341.059 5256657.007441363)@2001-01-02, "
    "POINT(467391.059 5256657.95398316)@2001-01-03, "
    "POINT(467441.059 5256656.494698682)@2001-01-04, "
    "POINT(467491.059 5256651.3944048835)@2001-01-05]",
    "MULTIPOINT(467441.059 5256657.135135818, "
    "467441.6994371363 5256656.494698682, "
    "467441.059 5256655.854261545, 467440.4185628637 5256656.494698682)",
    0x1.321ce1b245485p-6);
  moving_entry("[POINT(652154.8599123628 5004032.085512782)@2001-01-01, "
    "POINT(652513.6326831725 5003847.107986104)@2001-01-02]",
    "LINESTRING(652703.8964077407 5003786.774758782, "
    "652322.368562004 5004155.573411469, 652318.1317795934 5003650.016050734, "
    "652060.2892375863 5004251.273621884, 652367.7547813747 5004318.638264828, "
    "652509.4005680797 5004005.870055652, 652703.4259387131 5003687.951227436)",
    0.0);
  printf("%d moving points against a geometry answered as the nearest "
    "double\n", moving);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
