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
 * @brief The style parameters of a buffer
 * @details A parameter string names the end cap, the join, the mitre limit,
 * the quadrant segments and the side of a buffer. A key the buffer does not
 * know, a key without a value and a value its key does not accept are each
 * refused as an invalid argument value; a single-sided buffer is refused as a
 * feature the buffer does not support. Every accepted string answers a
 * buffer.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o buffer_params_test \
 *   buffer_params_test.c -L/usr/local/lib -lmeos -lm
 * ./buffer_params_test
 * @endcode
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_geo.h>

/* The buffer of @p geo by 1 in the style @p params, asserted to answer or to
 * be refused with the error @p err */
static void
buffer_params(const GSERIALIZED *geo, const char *params, int err)
{
  meos_errno_reset();
  GSERIALIZED *buf = geom_buffer(geo, 1.0, params);
  int errno_buf = meos_errno();
  printf("geom_buffer(line, 1, \"%s\"): %d, errno %d\n", params, buf != NULL,
    errno_buf);
  assert(errno_buf == err);
  assert((buf != NULL) == (err == 0));
  free(buf);
}

int
main(void)
{
  meos_initialize();
  meos_initialize_noexit_error_handler();

  GSERIALIZED *line = geom_in("Linestring(0 0,10 0,10 10)", -1);
  assert(line != NULL);

  /* Every style the parameters name */
  buffer_params(line, "", 0);
  buffer_params(line, "endcap=flat join=bevel", 0);
  buffer_params(line, "endcap=square join=mitre mitre_limit=2", 0);
  buffer_params(line, "endcap=butt join=miter miter_limit=2 quad_segs=8", 0);
  buffer_params(line, "side=both", 0);

  /* A value its key does not accept, a key without a value and an unknown
   * key are invalid argument values */
  buffer_params(line, "endcap=pointy", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "join=pointy", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "quad_segs=0", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "side=up", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "endcap=", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "endcap", MEOS_ERR_INVALID_ARG_VALUE);
  buffer_params(line, "colour=red", MEOS_ERR_INVALID_ARG_VALUE);

  /* A single-sided buffer is a feature the buffer does not support */
  buffer_params(line, "side=left", MEOS_ERR_FEATURE_NOT_SUPPORTED);

  free(line);
  meos_finalize();
  printf("buffer_params_test: all assertions hold\n");
  return 0;
}
