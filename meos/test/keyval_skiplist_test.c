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
 * @brief Test the key-value mode of the skiplist
 * @details A program that tests the skiplist in its key-value mode, in which
 * a MEOS program accumulates one value per key, as the AIS example
 * `ais_expand_skiplist.c` does per vessel. The list is filled in the three
 * ways the mode allows:
 * (i)   one value at a time, searching for its key and inserting the value
 * only when the key is absent, then updating the stored value in place;
 * (ii)  sorted batches of values whose key is a field of the value;
 * (iii) sorted batches of keys given apart from their values.
 *
 * Every batch after the first holds keys that are already in the list, so
 * its values are merged with the stored ones by the merge function of the
 * list, which is tried in its two forms: one that updates the stored value
 * in place and returns it, and one that returns a new value.
 *
 * After every step the list must hold each key once, in increasing order,
 * with the sum of the counts given for it, and its length must be the number
 * of keys.
 *
 * The program can be built as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o keyval_skiplist_test keyval_skiplist_test.c -L/usr/local/lib -lmeos
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <meos.h>
#include <meos_internal.h>

/* Number of distinct keys */
#define NUM_KEYS 40
/* Number of values given one at a time */
#define NUM_SINGLE 1000
/* Number of batches */
#define NUM_BATCHES 4

/* A value whose key is one of its fields */
typedef struct
{
  int key;
  int count;
} Record;

static int
record_cmp(void *a, void *b)
{
  int k1 = ((Record *) a)->key, k2 = ((Record *) b)->key;
  return (k1 > k2) - (k1 < k2);
}

static int
key_cmp(void *a, void *b)
{
  int k1 = *(int *) a, k2 = *(int *) b;
  return (k1 > k2) - (k1 < k2);
}

/* Add the count of the second value into the first one and return it */
static void *
record_merge_in_place(void *left, void *right)
{
  ((Record *) left)->count += ((Record *) right)->count;
  return left;
}

/* Return a new value whose count is the sum of the counts of both values */
static void *
record_merge_new(void *left, void *right)
{
  Record *result = malloc(sizeof(Record));
  result->key = ((Record *) left)->key;
  result->count = ((Record *) left)->count + ((Record *) right)->count;
  return result;
}

/**
 * @brief Return the number of defects of a list against the expected counts,
 * where a zero count stands for an absent key
 */
static int
check_list(const char *step, const SkipList *list, const int *expected,
  bool keyed)
{
  int defects = 0, prev = -1, n = 0;
  for (int cur = list->elems[0].next[0]; cur != list->tail;
       cur = list->elems[cur].next[0])
  {
    Record *rec = (Record *) list->elems[cur].value;
    int key = keyed ? *(int *) list->elems[cur].key : rec->key;
    if (key <= prev)
    {
      printf("%s: key %d follows key %d\n", step, key, prev);
      defects++;
    }
    else if (key >= NUM_KEYS || rec->count != expected[key])
    {
      printf("%s: key %d has count %d, expected %d\n", step, key,
        rec->count, key < NUM_KEYS ? expected[key] : 0);
      defects++;
    }
    prev = key;
    n++;
  }
  int want = 0;
  for (int k = 0; k < NUM_KEYS; k++)
    if (expected[k])
      want++;
  if (n != want || list->length != want)
  {
    printf("%s: %d elements and length %d, expected %d keys\n", step, n,
      list->length, want);
    defects++;
  }
  return defects;
}

/**
 * @brief Fill a list one value at a time, inserting a key only when it is
 * absent and updating the stored value in place
 */
static int
test_single(void)
{
  int expected[NUM_KEYS] = {0};
  SkipList *list = skiplist_make(0, sizeof(Record), &record_cmp,
    &record_merge_in_place);
  for (int i = 0; i < NUM_SINGLE; i++)
  {
    Record rec = { .key = rand() % NUM_KEYS, .count = 1 };
    int pos = skiplist_search(list, NULL, &rec);
    if (pos < 0)
    {
      Record init = { .key = rec.key, .count = 0 };
      Record *init_p = &init;
      skiplist_splice(list, NULL, (void **) &init_p, 1, NULL, false,
        SKIPLIST_KEYVALUE);
      pos = skiplist_search(list, NULL, &rec);
      if (pos < 0)
      {
        printf("single: key %d not found after its insertion\n", rec.key);
        skiplist_free(list);
        return 1;
      }
    }
    ((Record *) list->elems[pos].value)->count += rec.count;
    expected[rec.key] += rec.count;
  }
  int defects = check_list("single", list, expected, false);
  skiplist_free(list);
  return defects;
}

/**
 * @brief Fill a list with sorted batches, the keys being given apart from the
 * values when `keyed` is true and being a field of the values otherwise
 * @details Batch `b` holds the keys that are multiples of `b + 1` and gives
 * each of them the count `b + 1`, so that every batch after the first holds
 * keys already in the list as well as new ones.
 */
static int
test_batches(bool keyed, void *(*merge_fn)(void *, void *), const char *name)
{
  int expected[NUM_KEYS] = {0};
  int defects = 0;
  char step[64];
  SkipList *list = keyed ?
    skiplist_make(sizeof(int), sizeof(Record), &key_cmp, merge_fn) :
    skiplist_make(0, sizeof(Record), &record_cmp, merge_fn);
  int keys[NUM_KEYS];
  Record recs[NUM_KEYS];
  void *keyptrs[NUM_KEYS], *recptrs[NUM_KEYS];
  for (int b = 0; b < NUM_BATCHES; b++)
  {
    int count = 0;
    for (int k = 0; k < NUM_KEYS; k += b + 1)
    {
      keys[count] = k;
      recs[count].key = k;
      recs[count].count = b + 1;
      keyptrs[count] = &keys[count];
      recptrs[count] = &recs[count];
      expected[k] += b + 1;
      count++;
    }
    skiplist_splice(list, keyed ? keyptrs : NULL, recptrs, count, NULL, false,
      SKIPLIST_KEYVALUE);
    snprintf(step, sizeof(step), "%s, batch %d", name, b + 1);
    defects += check_list(step, list, expected, keyed);
  }
  skiplist_free(list);
  return defects;
}

int
main(void)
{
  meos_initialize();
  srand(1);
  int failures = 0;

  failures += test_single();
  failures += test_batches(false, &record_merge_in_place,
    "embedded keys, merge in place");
  failures += test_batches(false, &record_merge_new,
    "embedded keys, merge into a new value");
  failures += test_batches(true, &record_merge_in_place,
    "separate keys, merge in place");
  failures += test_batches(true, &record_merge_new,
    "separate keys, merge into a new value");

  if (failures == 0)
    printf("key-value skiplist test: all tests passed\n");
  meos_finalize();
  return failures ? 1 : 0;
}
