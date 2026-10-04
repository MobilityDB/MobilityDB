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
 * @brief A program that tests the 32-bit and the seeded 64-bit hash of a
 * numeric value and of a JSONB value holding one
 * @details The values are those PostgreSQL 18 answers for
 * `hash_numeric(v)`, `hash_numeric_extended(v, seed)` and
 * `jsonb_hash_extended(jsonb, seed)`, so a set, a JSONB value or a temporal
 * value hashed by MEOS groups and joins as PostgreSQL does.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o numeric_hash_test numeric_hash_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <meos.h>
#include <meos_json.h>
#include <pg_numeric.h>

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* The values and the hashes PostgreSQL answers for them */
  const char *values[] = {"1", "2", "3.5", "123456789", "-7"};
  const uint64_t hash7[] = {0x6fb927ba36b367aeULL, 0x3a414145619e02eaULL,
    0x0f82ea71ec89021eULL, 0xaa0dba4fdb13dbdcULL, 0x786b6dd1c1603bdbULL};
  const uint64_t hash0[] = {0x97cff9054ef7e348ULL, 0xb7201cabf69100daULL,
    0x515527c5fdd9b722ULL, 0x98314a9e865541eaULL, 0x83603bfe45db0d7dULL};
  const uint32_t hash32[] = {0x4ef7e348U, 0xf69100daU, 0xfdd9b722U,
    0x865541eaU, 0x45db0d7dU};

  for (int i = 0; i < 5; i++)
  {
    Numeric num = numeric_in(values[i], -1);
    /* The seeded hash keeps its 64 bits */
    assert(numeric_hash_extended(num, 7) == hash7[i]);
    assert(numeric_hash_extended(num, 0) == hash0[i]);
    /* The 32-bit hash is the low half of the hash with seed 0 */
    assert(numeric_hash(num) == hash32[i]);
    assert((uint32_t) numeric_hash_extended(num, 0) == numeric_hash(num));
    printf("numeric_hash_extended(%s, 7): %016llx\n", values[i],
      (unsigned long long) numeric_hash_extended(num, 7));
    free(num);
  }

  /* A JSONB value hashes its numbers through the same function */
  Jsonb *jb = jsonb_in("{\"a\": 1}");
  assert(jsonb_hash_extended(jb, 7) == 0x5414c9c89847c67aULL);
  printf("jsonb_hash_extended({\"a\": 1}, 7): %016llx\n",
    (unsigned long long) jsonb_hash_extended(jb, 7));
  free(jb);

  /* Finalize MEOS */
  meos_finalize();
  return EXIT_SUCCESS;
}
