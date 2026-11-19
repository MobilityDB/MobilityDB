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
 * @brief A program that tests how the text inputs read the SRID they are
 * given under the noexit error handler
 * @details Every text input reads an `SRID=` specification as the PostGIS
 * parser reads it, so the program takes #geom_in, which is that parser, as the
 * reference: for each SRID text, every other input answers the SRID #geom_in
 * answers, a negative SRID reading as the unknown SRID 0 and an SRID above the
 * maximum folding into the reserved range, and refuses with an error a
 * specification #geom_in refuses.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o srid_text_test srid_text_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>
#include <meos_cbuffer.h>
#include <meos_pose.h>

/* The SRID a text input answers, -1 when it refuses the text with an error */
static int32_t
read_srid(int kind, const char *srid_text)
{
  char buf[256];
  int32_t result = -1;
  switch (kind)
  {
    case 0:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;Point(1 1)", srid_text);
      GSERIALIZED *v = geom_in(buf, -1);
      if (v) result = geo_srid(v);
      free(v);
      break;
    }
    case 1:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;Cbuffer(Point(1 1),0.5)", srid_text);
      Cbuffer *v = cbuffer_in(buf);
      if (v) result = cbuffer_srid(v);
      free(v);
      break;
    }
    case 2:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;Pose(Point(1 1),0.5)", srid_text);
      Pose *v = pose_in(buf);
      if (v) result = pose_srid(v);
      free(v);
      break;
    }
    case 3:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;PoseChain(Pose(Point(1 2),0))",
        srid_text);
      PoseChain *v = posechain_in(buf);
      if (v) result = posechain_srid(v);
      free(v);
      break;
    }
    case 4:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;STBOX X((1,1),(2,2))", srid_text);
      STBox *v = stbox_in(buf);
      if (v) result = stbox_srid(v);
      free(v);
      break;
    }
    case 5:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;[Point(1 1)@2001-01-01, "
        "Point(2 2)@2001-01-02]", srid_text);
      Temporal *v = tgeompoint_in(buf);
      if (v) result = tspatial_srid(v);
      free(v);
      break;
    }
    case 6:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;{[Cbuffer(Point(1 1),0.5)@2001-01-01,"
        " Cbuffer(Point(2 2),0.5)@2001-01-02]}", srid_text);
      Temporal *v = tcbuffer_in(buf);
      if (v) result = tspatial_srid(v);
      free(v);
      break;
    }
    default:
    {
      snprintf(buf, sizeof(buf), "SRID=%s;{\"Point(1 1)\", \"Point(2 2)\"}",
        srid_text);
      Set *v = geomset_in(buf);
      if (v) result = spatialset_srid(v);
      free(v);
    }
  }
  /* A refusal raises an error, an answer leaves none */
  assert((result == -1) == (meos_errno() != 0));
  meos_errno_reset();
  return result;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_timezone("UTC");
  meos_initialize_noexit_error_handler();

  const char *inputs[] = {"geom_in", "cbuffer_in", "pose_in", "posechain_in",
    "stbox_in", "tgeompoint_in", "tcbuffer_in", "geomset_in"};
  const char *srids[] = {"3857", "0", "-5", "999999", "1000000", "1000001",
    "999999999", "12x", "", "x"};
  int ninputs = (int) (sizeof(inputs) / sizeof(inputs[0]));
  int nsrids = (int) (sizeof(srids) / sizeof(srids[0]));
  for (int i = 0; i < nsrids; i++)
  {
    int32_t expected = read_srid(0, srids[i]);
    printf("SRID=%s: geom_in %s", srids[i], expected == -1 ? "refuses" : "");
    if (expected != -1)
      printf("answers %d", expected);
    for (int k = 1; k < ninputs; k++)
    {
      int32_t got = read_srid(k, srids[i]);
      if (got != expected)
        printf("\n  %s answers %d", inputs[k], got);
      assert(got == expected);
    }
    printf(", as every other input\n");
  }

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
