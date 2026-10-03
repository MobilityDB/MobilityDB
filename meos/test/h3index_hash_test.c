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
 * @brief A program that tests the 32-bit and the seeded 64-bit hash of an
 * H3 cell
 * @details The hash of an h3index is the hash PostgreSQL computes over a
 * 64-bit integer, as for the quadbin and S2 cells, so a hash join or a hash
 * aggregation groups the same cells in every engine.
 *
 * The program verifies that #h3index_hash and #h3index_hash_extended answer
 * the values #quadbin_hash and #quadbin_hash_extended answer on the same bit
 * pattern, and that the seeded hash with seed 0 carries the 32-bit hash in
 * its low 32 bits, as PostgreSQL requires of the second support function of
 * a hash operator class.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o h3index_hash_test h3index_hash_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_h3.h>
#include <meos_quadbin.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* A resolution 10 cell */
  H3Index cell = 0x8a1fb46622dffffULL;

  /* The 32-bit hash is the one of the same 64 bits as a quadbin */
  uint32_t hash = h3index_hash(cell);
  assert(hash == quadbin_hash(cell));

  /* The seeded hash is the one of the same 64 bits as a quadbin */
  uint64_t hash7 = h3index_hash_extended(cell, 7);
  assert(hash7 == quadbin_hash_extended(cell, 7));

  /* With seed 0 the low 32 bits are the 32-bit hash */
  uint64_t hash0 = h3index_hash_extended(cell, 0);
  assert((uint32_t) hash0 == hash);

  printf("h3index_hash(%llx): %u\n", (unsigned long long) cell, hash);
  printf("h3index_hash_extended(%llx, 7): %llu\n", (unsigned long long) cell,
    (unsigned long long) hash7);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
