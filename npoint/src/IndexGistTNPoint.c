/*****************************************************************************
 *
 * IndexGistTPoint.c
 *	  R-tree GiST index for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * GiST Compress methods for tnpoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_tnpoint_compress);

PGDLLEXPORT Datum
gist_tnpoint_compress(PG_FUNCTION_ARGS)
{
    GISTENTRY* entry = (GISTENTRY *) PG_GETARG_POINTER(0);

    if (entry->leafkey)
    {
        GISTENTRY	*retval = palloc(sizeof(GISTENTRY));

        gistentryinit(*retval, call_function1(tnpoint_to_gbox, entry->key),
                      entry->rel, entry->page, entry->offset, false);

        PG_RETURN_POINTER(retval);
    }

    PG_RETURN_POINTER(entry);
}

