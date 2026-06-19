/*-------------------------------------------------------------------------
 *
 * pg_conversion.h
 *	  definition of the "conversion" system catalog (pg_conversion)
 *
 * Portions Copyright (c) 1996-2025, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_conversion.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_CONVERSION_H
#define PG_CONVERSION_H

typedef int (*pg_con_fn)(int,int,unsigned char *,unsigned char *,int,bool);

extern pg_con_fn FindDefaultConversion(int32 for_encoding, int32 to_encoding);

#endif /* PG_CONVERSION_H */
