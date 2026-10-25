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
 * @brief The hexadecimal conversions `%a` and `%A` of the printf family a
 * program reaches through the MEOS headers
 * @details The MEOS headers carry the formatting functions of PostgreSQL, so
 * `printf`, `snprintf` and their siblings in a program that includes them are
 * those functions. A double printed with `%a` is written exactly, and read
 * back by `strtod` it is the same double. Every value below is printed through
 * the MEOS `snprintf`, and the text must read back as the value, equal the C
 * library's own text for the same conversion, and be as long as the count the
 * call returns. The C library's `snprintf` is reached through a function
 * defined before the MEOS headers are included.
 *
 * To compile and run:
 * @code
 * gcc -Wall -g -I/usr/local/include -o printf_hexfloat_test \
 *   printf_hexfloat_test.c -L/usr/local/lib -lmeos -lm
 * ./printf_hexfloat_test
 * @endcode
 */

#include <assert.h>
#include <float.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Format a double with the C library's snprintf
 */
static int
libc_format(char *buf, size_t size, const char *fmt, double value)
{
  return snprintf(buf, size, fmt, value);
}

#include <meos.h>

int
main(void)
{
  meos_initialize();

  const double values[] = {1.0, 0.1, -2.5, 15.0, 0.0, -0.0, DBL_MAX, DBL_MIN,
    DBL_TRUE_MIN, 1.0 / 3.0, 6376089.29314323, -7.2606503141306343e-152};
  const char *formats[] = {"%a", "%A", "%.3a", "%14a", "%-14a|", "%+a",
    "%.0a", "%.20a"};
  const size_t nvalues = sizeof(values) / sizeof(values[0]);
  const size_t nformats = sizeof(formats) / sizeof(formats[0]);
  int checked = 0;
  for (size_t i = 0; i < nvalues; i++)
  {
    char text[128], expected[128];
    /* The text of the exact conversion reads back as the value */
    int len = snprintf(text, sizeof(text), "%a", values[i]);
    assert(len > 0 && (size_t) len == strlen(text));
    double back = strtod(text, NULL);
    assert(memcmp(&back, &values[i], sizeof(double)) == 0);
    for (size_t j = 0; j < nformats; j++)
    {
      len = snprintf(text, sizeof(text), formats[j], values[i]);
      int explen = libc_format(expected, sizeof(expected), formats[j],
        values[i]);
      assert(len > 0 && (size_t) len == strlen(text));
      assert(len == explen && strcmp(text, expected) == 0);
      checked++;
    }
  }
  printf("%d hexadecimal conversions of %d doubles match the C library, "
    "and each %%a reads back as its double: %a\n", checked, (int) nvalues,
    0.1);

  meos_finalize();
  return EXIT_SUCCESS;
}
