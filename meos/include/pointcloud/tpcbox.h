/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @brief Internal declarations for the TPCBox bounding-box type.
 */

#ifndef __TPCBOX_H__
#define __TPCBOX_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pointcloud.h>

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetTpcboxP(X)         ((TPCBox *) DatumGetPointer(X))
#define TpcboxPGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_TPCBOX_P(X)      DatumGetTpcboxP(PG_GETARG_DATUM(X))
#define PG_RETURN_TPCBOX_P(X)      return TpcboxPGetDatum(X)

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_tpcbox_tpcbox(const TPCBox *box1, const TPCBox *box2);

/* Bounding box operators */

extern bool tpcbox_contains(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_contained(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overlaps(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_same(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_adjacent(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_left(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_right(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overleft(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overright(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_below(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_above(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overbelow(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overabove(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_front(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_back(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overfront(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overback(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_before(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_after(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overbefore(const TPCBox *box1, const TPCBox *box2);
extern bool tpcbox_overafter(const TPCBox *box1, const TPCBox *box2);

/* Input/output functions */

extern TPCBox *tpcbox_parse(const char **str);

/*****************************************************************************/

#endif /* __TPCBOX_H__ */
