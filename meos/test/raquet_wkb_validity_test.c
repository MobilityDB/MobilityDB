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
 * @brief A program that tests how the reading of a Raquet tile from its
 * Well-Known Binary representation reports dimensions no tile can have
 * @details The WKB of a Raquet tile writes its width and height as 32-bit
 * integers, while a tile holds between 1 and 65535 pixels a side. The program
 * verifies that a WKB stating a width or a height outside that range is
 * refused with MEOS_ERR_WKB_INPUT under the noexit error handler, whatever
 * payload follows, and that the dimensions a tile can have are read back.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o raquet_wkb_validity_test raquet_wkb_validity_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_raster.h>

/* Offsets of the width and height in the WKB of a tile: the endian flag, the
 * QUADBIN cell and the pixel type precede them */
#define WIDTH_OFFSET 10
#define HEIGHT_OFFSET 14
/* Size of the header of the WKB of a tile, before its pixels */
#define HEADER_SIZE 27

/**
 * @brief Write a 32-bit integer in little-endian order
 */
static void
put_int32(uint8_t *p, int32_t v)
{
  uint32_t u = (uint32_t) v;
  for (int i = 0; i < 4; i++)
    p[i] = (uint8_t) (u >> (8 * i));
}

/**
 * @brief Return the tile read from a copy of a WKB stating the dimensions
 * given, followed by a payload of the bytes given
 */
static Raquet *
read_dims(const uint8_t *wkb, int32_t width, int32_t height, size_t payload)
{
  size_t size = HEADER_SIZE + payload;
  uint8_t *buf = calloc(size, 1);
  assert(buf);
  memcpy(buf, wkb, HEADER_SIZE);
  put_int32(buf + WIDTH_OFFSET, width);
  put_int32(buf + HEIGHT_OFFSET, height);
  Raquet *result = raquet_from_wkb(buf, size);
  free(buf);
  return result;
}

/* Main program */
int main(void)
{
  /* Initialize MEOS and install the error handler that reports through
   * meos_errno instead of exiting */
  meos_initialize();
  meos_initialize_noexit_error_handler();

  /* A one-pixel tile and its WKB in little-endian order */
  uint8_t pixels[1] = { 7 };
  Raquet *rq = raquet_make(0x480fffffffffffffULL, 1, 1, MEOS_PT_UINT8, 0.0,
    false, pixels, 1);
  assert(rq);
  size_t size;
  uint8_t *wkb = raquet_as_wkb(rq, WKB_NDR, &size);
  assert(wkb && size == HEADER_SIZE + 1);

  /* Each dimension outside [1, 65535] is refused, with a payload as long as
   * the dimension read modulo 65536 would ask for, so that no shortage of
   * bytes refuses it instead */
  const int32_t bad[] = { INT32_MIN, -65537, -1, 0, 65536, 65537, 131073,
    INT32_MAX };
  const int nbad = sizeof(bad) / sizeof(bad[0]);
  for (int i = 0; i < nbad; i++)
  {
    size_t payload = (uint16_t) bad[i] ? (uint16_t) bad[i] : 1;
    for (int side = 0; side < 2; side++)
    {
      int32_t width = side ? 1 : bad[i], height = side ? bad[i] : 1;
      meos_errno_reset();
      Raquet *res = read_dims(wkb, width, height, payload);
      printf("raquet_from_wkb(%d x %d): %s, errno %d\n", width, height,
        res ? "a tile" : "refused", meos_errno());
      assert(res == NULL);
      assert(meos_errno() == MEOS_ERR_WKB_INPUT);
      free(res);
    }
  }

  /* The dimensions a tile can have are read back */
  const int32_t good[][2] = { { 1, 1 }, { 2, 3 }, { 65535, 1 }, { 1, 65535 } };
  for (int i = 0; i < 4; i++)
  {
    meos_errno_reset();
    Raquet *res = read_dims(wkb, good[i][0], good[i][1],
      (size_t) good[i][0] * good[i][1]);
    printf("raquet_from_wkb(%d x %d): %s, errno %d\n", good[i][0], good[i][1],
      res ? "a tile" : "refused", meos_errno());
    assert(res && meos_errno() == 0);
    assert(raquet_width(res) == good[i][0]);
    assert(raquet_height(res) == good[i][1]);
    free(res);
  }

  free(wkb); free(rq);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
