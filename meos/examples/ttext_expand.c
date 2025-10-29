/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief A simple program that generates a given number of ttext instants,
 * appends the instant into a sequence set where each sequence of the sequence
 * set has a fixed number of instants defined by a compiler constant.
 * The program outputs the number of sequences and, for the last sequence,
 * the number of instants and the last value.
 *
 * The instants are generated so they are not redundant, that is, all input
 * instants will appear in the final sequence.
 *
 * The program can be build as follows
 * @code
 * gcc -Wall -g -I/usr/local/include -o ttext_expand ttext_expand.c -L/usr/local/lib -lmeos
 * @endcode
 *
 * The output of the program when MEOS is built with the flag -DDEBUG_EXPAND=1
 * to show debug messages for the expandable data structures is as follows
 * @code
 * Total number of instants generated: 50000
 * Maximum number of instants in a sequence: 500
 * Generating the instants (one '*' marker every 100 instants)
 * * Seq -> 128 * Seq -> 256 * Seq -> 512 *************************************
 * * SS -> 128 ****************************************************************
 * Number of instants in the sequence set: 50000
 * Number of sequences: 100, Maximum number of sequences : 128
 * Number of instants in the last sequence: 500, Last value : BBBBBBBBB
 * The program took 0.000000 seconds to execute
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <meos.h>
/* The expandable functions are in the internal MEOS API */
#include <meos_internal.h>

/* Maximum number of instants */
#define MAX_NO_INSTS 50000000
/* Initial number of instants allocated when creating a sequence */
#define INITIAL_INSTS_SEQ 64
/* Initial number of sequences allocated when creating a sequence set */
#define INITIAL_SEQUENCES_SEQSET 64
/* Maximum number of instants in a sequence */
#define MAX_INSTS_SEQ 50000
/* Number of instants in a batch for printing a marker */
#define NO_INSTS_BATCH 500000
/* Maximum length in characters of the text values in the instants */
#define MAX_LEN_TEXT 10
/* State whether a message is shown every time a sequence set is expanded */
#define EXPAND_SEQ true
/* Determine when the composing sequences are compacted
 * - True: sequences are compacted before adding them to the sequence set
 * - False: sequences are compacted when compacting the sequence set
 */
#define COMPACT_COMP_SEQS true
/* Determine whether debug messages showing the expansion of sequences and
 * and sequence sets are shown */
#define DEBUG false

/* Main program */
int main(void)
{
  /* Initialize MEOS */
  meos_initialize();

  /* Get start time */
  clock_t tm;
  tm = clock();

  /* Sequence and sequence set values for accumulating the input instants */
  TSequence *seq = NULL, *seq1 = NULL;
  TSequenceSet *ss = NULL;
  /* Interval to add */
  Interval *onehour = interval_in("1 hour", -1);
  /* Iterator variables */
  int i;
  /* Seed the random number generator with the current time in seconds. */
  srandom(time(0));

  printf("Total number of instants generated: %d\n", MAX_NO_INSTS);
  printf("Maximum number of instants in a sequence: %d\n", MAX_INSTS_SEQ);
  printf("Generating the instants (one '*' marker every %d instants)\n",
    NO_INSTS_BATCH);

  TimestampTz t = timestamptz_in("2000-01-01", -1);
  for (i = 0; i < MAX_NO_INSTS; i++)
  {
    /* Generate the instant */
    /* Use a random generator to set the length of the text value */
    int len = random() % MAX_LEN_TEXT + 1;
    char *value = malloc(sizeof(char) * (len + 2));
    memset(value, i % 2 == 0 ? 'A' : 'B', len);
    value[len] = '\0';
    text *txt = cstring2text(value);
    t = add_timestamptz_interval(t, onehour);
    TInstant *inst = ttextinst_make(txt, t);
    free(value); free(txt);
    /* Test whether it is the first instant read */
    if (! seq)
      /* Create an expandable temporal sequence that can store
       * INITIAL_INSTS_SEQ instants and store the first instant.
       * Notice that we do not use MAX_INSTS_SEQ to illustrate the
       * #tsequence_compact() function */
      seq = tsequence_make_exp(&inst, 1, INITIAL_INSTS_SEQ, true, true,
        STEP, false);
    else
    {
      if (seq->count < MAX_INSTS_SEQ)
      {
        int maxcount = seq->maxcount;
        /* We are sure that the result is a temporal sequence */
        seq = (TSequence *) tsequence_append_tinstant(seq, inst, 0.0, NULL,
          true);
        /* Print a marker when the sequence has been expanded */
        if (DEBUG && EXPAND_SEQ && maxcount != seq->maxcount)
        {
          printf(" Seq -> %d ", seq->maxcount);
          fflush(stdout);
        }
      }
      else
      {
        /* If requested, compact the sequence to remove unused extra space */
        seq1 = COMPACT_COMP_SEQS ? tsequence_compact(seq) : seq;
        if (! ss)
          ss = tsequenceset_make_exp(&seq1, 1, INITIAL_SEQUENCES_SEQSET,
            false);
        else
          ss = tsequenceset_append_tsequence(ss, seq1, true);
        free(seq);
        if (COMPACT_COMP_SEQS)
          free(seq1);
        /* Create a new sequence containing the last instant generated */
        seq = tsequence_make_exp(&inst, 1, INITIAL_INSTS_SEQ, true, true,
          STEP, false);
      }
    }
    free(inst);

    /* Print a '*' marker every X instants generated */
    if (i % NO_INSTS_BATCH == 0)
    {
      printf("*");
      fflush(stdout);
    }
  }
  /* Add the last sequence to the sequence set */
  seq1 = COMPACT_COMP_SEQS ? tsequence_compact(seq) : seq;
  ss = tsequenceset_append_tsequence(ss, seq1, true);
  free(seq);
  if (COMPACT_COMP_SEQS)
    free(seq1);

  /* Compact the sequence set */
  TSequenceSet *ss1 = tsequenceset_compact(ss);
  free(ss);

  /* Print information about the sequence set */
  printf("\nTotal number of instants in the sequence set: %d\n", ss1->totalcount);
  printf("Number of sequences: %d, Maximum number of sequences : %d\n",
    ss1->count, ss1->maxcount);

  /* Print information about the last sequence */
  seq = temporal_end_sequence((Temporal *) ss1);
  text *txt = ttext_end_value((Temporal *) seq);
  char *str = text_to_cstring(txt);
  printf("Number of instants in the last sequence: %d, Last value : %s\n",
    seq->count, str);

  /* Calculate the elapsed time */
  tm = clock() - tm;
  double time_taken = ((double) tm) / CLOCKS_PER_SEC;
  printf("The program took %f seconds to execute\n", time_taken);

  /* Free memory */
  free(ss1); free(onehour); free(txt); free(str); free(seq);

  /* Finalize MEOS */
  meos_finalize();

  /* Return */
  return EXIT_SUCCESS;
}
