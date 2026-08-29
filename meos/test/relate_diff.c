/* SPDX-License-Identifier: PostgreSQL */
/**
 * @file
 * @brief Check the DE-9IM matrix of the WKT geometry pairs read on the
 * standard input, one `wkt1|wkt2` or `wkt1|wkt2|expected` record per line
 * @details A record without an expected matrix prints the nine-character
 * matrix #geom_relate answers, or the word `UNSUPPORTED` where it answers
 * none. A record with an expected matrix is checked against it and counted,
 * the exit status reporting whether every record passed. A pair left
 * unanswered counts as a failure, so a coverage gap shows up as a red run
 * rather than as a silently skipped record. The expected matrix follows the
 * DE-9IM pattern alphabet, so `T` accepts any non-empty intersection and `*`
 * accepts anything.
 *
 * The engine under test is the native one, reached through #geom_relate, so
 * the answers are its own whether or not the library carries GEOS. The corpus
 * of expected matrices comes from the GEOS suite through
 * `relate_harvest_geos.py`, so the two answer the same question about the
 * same records.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <meos.h>
#include <meos_geo.h>

/**
 * @brief Return true if a matrix satisfies an expected DE-9IM pattern
 */
static bool
matrix_matches(const char *matrix, const char *pattern)
{
  for (int i = 0; i < 9; i++)
  {
    if (pattern[i] == '*')
      continue;
    if (pattern[i] == 'T')
    {
      if (matrix[i] == 'F')
        return false;
      continue;
    }
    if (pattern[i] != matrix[i])
      return false;
  }
  return true;
}

int
main(int argc, char **argv)
{
  (void) argc; (void) argv;
  meos_initialize();
  /* A record the engine cannot answer must not end the run, so the handler
   * returns instead of exiting */
  meos_initialize_noexit_error_handler();
  /* The corpus holds geometries of any size, and a truncated line would be
   * reported as a parse failure of the engine rather than of the reader */
  char *line = NULL;
  size_t linesize = 0;
  int checked = 0, passed = 0, uncovered = 0;
  while (getline(&line, &linesize, stdin) > 0)
  {
    char *nl = strchr(line, '\n');
    if (nl)
      *nl = '\0';
    /* A corpus file carries its provenance in leading comment lines */
    if (line[0] == '#')
      continue;
    char *sep = strchr(line, '|');
    if (! sep)
      continue;
    *sep = '\0';
    char *second = sep + 1;
    char *expected = strchr(second, '|');
    if (expected)
      *expected++ = '\0';

    GSERIALIZED *gs1 = geom_in(line, -1);
    GSERIALIZED *gs2 = geom_in(second, -1);
    if (! gs1 || ! gs2)
    {
      printf("PARSE-ERROR\n");
      if (gs1) free(gs1);
      if (gs2) free(gs2);
      continue;
    }
    char *matrix = geom_relate(gs1, gs2);
    bool covered = (matrix != NULL);
    free(gs1); free(gs2);

    if (! expected)
      printf("%s\n", covered ? matrix : "UNSUPPORTED");
    else if (! covered)
    {
      uncovered++;
      printf("UNANSWERED expected=%s  %s|%s\n", expected, line, second);
    }
    else
    {
      checked++;
      if (matrix_matches(matrix, expected))
        passed++;
      else
        printf("FAIL got=%s expected=%s  %s|%s\n", matrix, expected, line,
          second);
    }
    if (matrix)
      free(matrix);
  }
  free(line);
  meos_finalize();
  if (checked || uncovered)
  {
    printf("%d of %d checked records pass, %d unanswered\n", passed, checked,
      uncovered);
    return (passed == checked && uncovered == 0) ? 0 : 1;
  }
  return 0;
}
