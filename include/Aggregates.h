#ifndef MOBILITYDB_AGGREGATES_H
#define MOBILITYDB_AGGREGATES_H

#include <TemporalTypes.h>

TemporalSeq **
temporalseq_tagg2(TemporalSeq **sequences1, int count1, TemporalSeq **sequences2,
                  int count2, Datum (*operator)(Datum, Datum), bool crossings, int *newcount) ;

TemporalS *
temporalseq_tavg_finalfn(TemporalSeq **sequences, int count) ;

#endif