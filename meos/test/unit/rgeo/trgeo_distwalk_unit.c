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
 * @brief Test of the functions of the walk of meos/src/rgeo/trgeo_distwalk.c
 * @details Each function of the walk gives a #DistFun that must be equal to
 * an expression of the placed geometry.  The test compares the two for
 * random bodies and segments, with a moving and a static second body.
 *
 * CMake builds the test in a MEOS build with rigid geometries, see
 * CMakeLists.txt in this directory:
 * @code
 * cmake -S . -B build -DMEOS=ON -DPOSE=ON -DRGEO=ON
 * make -C build && (cd build && ctest -R trgeo_dist)
 * @endcode
 * NDEBUG is not defined, so that the functions also run their own checks,
 * see distwalk_check().
 *
 * The last check compares the rotation of a segment with
 * #posesegm_interpolate(), including the rule for a half turn.  If the two
 * differ, the walk computes the distance to a body at another position than
 * the one that MobilityDB gives.
 */
#include <stdlib.h>
#include "rgeo/trgeo_distwalk.h"

extern void meos_initialize(void);
extern void meos_finalize(void);
extern Pose *pose_make_2d(double x, double y, double theta, bool geodetic,
  int32 srid);
extern Pose *posesegm_interpolate(const Pose *start, const Pose *end,
  double ratio);

static uint64_t rs = 0x9E3779B97F4A7C15ULL;
static double rnd(double lo, double hi)
{
  rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17;
  return lo + (hi - lo) * ((double) (rs >> 11) / 9007199254740992.0);
}

static double angdiff(double a, double b)
{
  double d = fmod(a - b + 3.0 * M_PI, 2.0 * M_PI) - M_PI;
  return fabs(d);
}

int main(void)
{
  meos_initialize();
  const int TRIALS = 40000, NT = 17;
  double e_tr = 0, e_cr = 0, e_vv = 0, e_pa = 0, e_lvl = 0, e_perp = 0;
  double e_omega = 0;
  int nstatic = 0;

  for (int k = 0; k < TRIALS; k++)
  {
    /* Two rigid bodies with random reference edges/vertices and poses */
    DistMotion ma, mb;
    distmotion_set(&ma, rnd(-20, 20), rnd(-20, 20), rnd(-M_PI, M_PI),
      rnd(-20, 20), rnd(-20, 20), rnd(-M_PI, M_PI));
    bool statictarget = (k % 3 == 0);
    if (statictarget)
    {
      distmotion_set_static(&mb, rnd(-20, 20), rnd(-20, 20));
      nstatic++;
    }
    else
      distmotion_set(&mb, rnd(-20, 20), rnd(-20, 20), rnd(-M_PI, M_PI),
        rnd(-20, 20), rnd(-20, 20), rnd(-M_PI, M_PI));
    DistSegm seg;
    distsegm_set(&seg, &ma, &mb);

    DistRefEdge ea, eb;
    distrefedge_set(&ea, rnd(-5, 5), rnd(-5, 5), rnd(-5, 5), rnd(-5, 5));
    distrefedge_set(&eb, rnd(-5, 5), rnd(-5, 5), rnd(-5, 5), rnd(-5, 5));
    DistRefVert va, vb;
    distrefvert_set(&va, rnd(-5, 5), rnd(-5, 5));
    if (statictarget)
      distrefvert_set(&vb, 0.0, 0.0);
    else
      distrefvert_set(&vb, rnd(-5, 5), rnd(-5, 5));

    DistFun ftr, fcr, fvv, fpa;
    distwalk_transition(&seg, &ea, &vb, &ftr);
    distwalk_cross(&seg, &ea, &vb, &fcr);
    distwalk_vertdist2(&seg, &va, &vb, &fvv);
    distwalk_parallel(&seg, &ea, &eb, &fpa);

    for (int j = 0; j < NT; j++)
    {
      double t = (double) j / (NT - 1);
      double qsx, qsy, qex, qey, px, py, ax, ay, bsx, bsy, bex, bey;
      distmotion_place(&ma, ea.sx, ea.sy, t, &qsx, &qsy);
      distmotion_place(&ma, ea.ex, ea.ey, t, &qex, &qey);
      distmotion_place(&mb, vb.x, vb.y, t, &px, &py);
      distmotion_place(&ma, va.x, va.y, t, &ax, &ay);
      distmotion_place(&mb, eb.sx, eb.sy, t, &bsx, &bsy);
      distmotion_place(&mb, eb.ex, eb.ey, t, &bex, &bey);

      double ux = qex - qsx, uy = qey - qsy;
      double N = (px - qsx) * ux + (py - qsy) * uy;
      double G = (px - qsx) * uy - (py - qsy) * ux;
      double D2 = (px - ax) * (px - ax) + (py - ay) * (py - ay);
      double P = ux * (bey - bsy) - uy * (bex - bsx);
      double sc = 1.0;

      double d;
      d = fabs(distfun_eval(&ftr, t) - N) / (1 + fabs(N));
      if (d > e_tr) e_tr = d;
      d = fabs(distfun_eval(&fcr, t) - G) / (1 + fabs(G));
      if (d > e_cr) e_cr = d;
      d = fabs(distfun_eval(&fvv, t) - D2) / (1 + fabs(D2));
      if (d > e_vv) e_vv = d;
      d = fabs(distfun_eval(&fpa, t) - P) / (1 + fabs(P));
      if (d > e_pa) e_pa = d;
      (void) sc;

      /* s(t) = 1 is the same function at level len^2 */
      double N1 = (px - qex) * ux + (py - qey) * uy;
      d = fabs((distfun_eval(&ftr, t) - ea.len * ea.len) - N1) /
        (1 + fabs(N1));
      if (d > e_lvl) e_lvl = d;

      /* inside a vertex-to-edge interval, |G|/len is the distance */
      double s = N / (ea.len * ea.len);
      if (s > 0.02 && s < 0.98)
      {
        double fx = qsx + s * ux, fy = qsy + s * uy;
        double dd = hypot(px - fx, py - fy);
        d = fabs(fabs(G) / ea.len - dd) / (1 + dd);
        if (d > e_perp) e_perp = d;
      }
    }

    /* motion_omega must reproduce posesegm_interpolate exactly */
    if (k < 4000)
    {
      double th1 = rnd(-M_PI, M_PI), th2 = rnd(-M_PI, M_PI);
      DistMotion mm;
      distmotion_set(&mm, 0, 0, th1, 0, 0, th2);
      Pose *p1 = pose_make_2d(0, 0, th1, false, 0);
      Pose *p2 = pose_make_2d(0, 0, th2, false, 0);
      for (int j = 0; j <= 8; j++)
      {
        double r = (double) j / 8.0;
        Pose *pi = posesegm_interpolate(p1, p2, r);
        double d = angdiff(mm.th0 + mm.w * r, pi->data[2]);
        if (d > e_omega) e_omega = d;
        free(pi);
      }
      free(p1); free(p2);
    }
  }

  printf("builders vs direct evaluation, %d segments x %d samples\n"
         "  (%d with a static target)\n", TRIALS, NT, nstatic);
  printf("  transition  N(t)                    rel err %.3e\n", e_tr);
  printf("  cross       G(t)                    rel err %.3e\n", e_cr);
  printf("  vertex-vertex squared distance      rel err %.3e\n", e_vv);
  printf("  parallel edges                      rel err %.3e\n", e_pa);
  printf("  s(t)=1 is the same f at level L^2   rel err %.3e\n", e_lvl);
  printf("  |G|/L is the distance when 0<s<1    rel err %.3e\n", e_perp);
  printf("  motion_omega vs posesegm_interpolate    err %.3e rad\n", e_omega);
  bool ok = e_tr < 1e-11 && e_cr < 1e-11 && e_vv < 1e-11 && e_pa < 1e-11 &&
    e_lvl < 1e-11 && e_perp < 1e-11 && e_omega < 1e-11;
  printf("\n%s\n", ok ? "all checks passed" : "FAILURES");
  meos_finalize();
  return ok ? 0 : 1;
}
