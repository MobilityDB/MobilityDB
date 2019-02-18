/*****************************************************************************
 *
 * spgiststat.c
 *		Function belonging to the initial implementation of SP-GiST taken from
 *		https://www.postgresql.org/message-id/29780.1324160816@sss.pgh.pa.us
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

// #include <access/spgist_private.h>
#include "TemporalTypes.h"

/* These definitions are taken from <catalog/pg_am.h> */
#define GIST_AM_OID 783
#define SPGIST_AM_OID 4000

/* This definition is taken from <catalog/pg_class.h> */
#define		  RELKIND_INDEX			  'i'	/* secondary index */

/* These definition are taken from <access/spgist_private.h> */

/*
 * Contents of page special space on SPGiST index pages
 */
typedef struct SpGistPageOpaqueData
{
	uint16		flags;			/* see bit definitions below */
	uint16		nRedirection;	/* number of redirection tuples on page */
	uint16		nPlaceholder;	/* number of placeholder tuples on page */
	/* note there's no count of either LIVE or DEAD tuples ... */
	uint16		spgist_page_id; /* for identification of SP-GiST indexes */
} SpGistPageOpaqueData;

typedef SpGistPageOpaqueData *SpGistPageOpaque;

typedef struct SpGistInnerTupleData
{
	unsigned int tupstate:2,	/* LIVE/REDIRECT/DEAD/PLACEHOLDER */
				allTheSame:1,	/* all nodes in tuple are equivalent */
				nNodes:13,		/* number of nodes within inner tuple */
				prefixSize:16;	/* size of prefix, or 0 if none */
	uint16		size;			/* total size of inner tuple */
	/* On most machines there will be a couple of wasted bytes here */
	/* prefix datum follows, then nodes */
} SpGistInnerTupleData;

typedef SpGistInnerTupleData *SpGistInnerTuple;

/* Flag bits in page special space */
#define SPGIST_META			(1<<0)
#define SPGIST_DELETED		(1<<1)	/* never set, but keep for afterwards
									 * compatibility */
#define SPGIST_LEAF			(1<<2)
#define SPGIST_NULLS		(1<<3)

#define SpGistPageGetOpaque(page) ((SpGistPageOpaque) PageGetSpecialPointer(page))
#define SpGistPageIsMeta(page) (SpGistPageGetOpaque(page)->flags & SPGIST_META)
#define SpGistPageIsDeleted(page) (SpGistPageGetOpaque(page)->flags & SPGIST_DELETED)
#define SpGistPageIsLeaf(page) (SpGistPageGetOpaque(page)->flags & SPGIST_LEAF)
#define SpGistPageStoresNulls(page) (SpGistPageGetOpaque(page)->flags & SPGIST_NULLS)

#define SPGIST_ROOT_BLKNO		 (1)	/* root for normal entries */

#define IS_INDEX(r) ((r)->rd_rel->relkind == RELKIND_INDEX)
#define IS_GIST(r) ((r)->rd_rel->relam == GIST_AM_OID)
#define IS_SPGIST(r) ((r)->rd_rel->relam == SPGIST_AM_OID)

/*****************************************************************************/

PG_FUNCTION_INFO_V1(spgiststat);

PGDLLEXPORT Datum
spgiststat(PG_FUNCTION_ARGS)
{
	text	   *name = PG_GETARG_TEXT_P(0);
	RangeVar   *relvar;
	Relation	index;
	BlockNumber blkno;
	BlockNumber totalPages = 0,
				innerPages = 0,
				leafPages = 0,
				emptyPages = 0,
				deletedPages = 0;
	double		usedSpace = 0.0;
	char		res[1024];
	int			bufferSize = -1;
	int64		innerTuples = 0,
				leafTuples = 0,
				nAllTheSame = 0,
				nLeafPlaceholder = 0,
				nInnerPlaceholder = 0,
				nLeafRedirect = 0,
				nInnerRedirect = 0;

	relvar = makeRangeVarFromNameList(textToQualifiedNameList(name));
	index = relation_openrv(relvar, AccessExclusiveLock);

	if (!IS_INDEX(index) || !IS_SPGIST(index))
		elog(ERROR, "relation \"%s\" is not an SPGiST index",
			 RelationGetRelationName(index));

	totalPages = RelationGetNumberOfBlocks(index);

	/* It was SPGIST_HEAD_BLKNO but the constant is no longer defined */
	for (blkno = SPGIST_ROOT_BLKNO; blkno < totalPages; blkno++)
	{
		Buffer	  buffer;
		Page		page;
		int		 pageFree;

		buffer = ReadBuffer(index, blkno);
		LockBuffer(buffer, BUFFER_LOCK_SHARE);

		page = BufferGetPage(buffer);

		if (PageIsNew(page) || SpGistPageIsDeleted(page))
		{
			deletedPages++;
			UnlockReleaseBuffer(buffer);
			continue;
		}

		if (SpGistPageIsLeaf(page))
		{
			leafPages++;
			leafTuples += PageGetMaxOffsetNumber(page);
			nLeafPlaceholder += SpGistPageGetOpaque(page)->nPlaceholder;
			nLeafRedirect += SpGistPageGetOpaque(page)->nRedirection;
		}
		else
		{
			int	 i,
					max;

			innerPages++;
			max = PageGetMaxOffsetNumber(page);
			innerTuples += max;
			nInnerPlaceholder += SpGistPageGetOpaque(page)->nPlaceholder;
			nInnerRedirect += SpGistPageGetOpaque(page)->nRedirection;
			for (i = FirstOffsetNumber; i <= max; i++)
			{
				SpGistInnerTuple it;

				it = (SpGistInnerTuple) PageGetItem(page,
													PageGetItemId(page, i));
				if (it->allTheSame)
					nAllTheSame++;
			}
		}

		if (bufferSize < 0)
			bufferSize = BufferGetPageSize(buffer)
				- MAXALIGN(sizeof(SpGistPageOpaqueData))
				- SizeOfPageHeaderData;

		pageFree = PageGetExactFreeSpace(page);

		usedSpace += bufferSize - pageFree;

		if (pageFree == bufferSize)
			emptyPages++;

		UnlockReleaseBuffer(buffer);
	}

	index_close(index, AccessExclusiveLock);

	totalPages--;			   /* discount metapage */

	snprintf(res, sizeof(res),
			 "totalPages:		%u\n"
			 "deletedPages:	  %u\n"
			 "innerPages:		%u\n"
			 "leafPages:		 %u\n"
			 "emptyPages:		%u\n"
			 "usedSpace:		 %.2f kbytes\n"
			 "freeSpace:		 %.2f kbytes\n"
			 "fillRatio:		 %.2f%%\n"
			 "leafTuples:		" INT64_FORMAT "\n"
			 "innerTuples:	   " INT64_FORMAT "\n"
			 "innerAllTheSame:   " INT64_FORMAT "\n"
			 "leafPlaceholders:  " INT64_FORMAT "\n"
			 "innerPlaceholders: " INT64_FORMAT "\n"
			 "leafRedirects:	 " INT64_FORMAT "\n"
			 "innerRedirects:	" INT64_FORMAT,
			 totalPages, deletedPages, innerPages, leafPages, emptyPages,
			 usedSpace / 1024.0,
		  (((double) bufferSize) * ((double) totalPages) - usedSpace) / 1024,
	   100.0 * (usedSpace / (((double) bufferSize) * ((double) totalPages))),
			 leafTuples, innerTuples, nAllTheSame,
			 nLeafPlaceholder, nInnerPlaceholder,
			 nLeafRedirect, nInnerRedirect);

	PG_RETURN_TEXT_P(CStringGetTextDatum(res));
}