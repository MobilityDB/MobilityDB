
/* MEOS */
#include <meos.h>
#include <point/tpoint_gist.h>
#include <general/temporal.h>

/**
 * @brief Increase the first box to include the second one
 */
void
stbox_adjust(void *bbox1, void *bbox2){
  STBox *box1 = (STBox *) bbox1;
  STBox *box2 = (STBox *) bbox2;
  box1->xmin = FLOAT8_MIN(box1->xmin, box2->xmin);
  box1->xmax = FLOAT8_MAX(box1->xmax, box2->xmax);
  box1->ymin = FLOAT8_MIN(box1->ymin, box2->ymin);
  box1->ymax = FLOAT8_MAX(box1->ymax, box2->ymax);
  box1->zmin = FLOAT8_MIN(box1->zmin, box2->zmin);
  box1->zmax = FLOAT8_MAX(box1->zmax, box2->zmax);
  TimestampTz tmin = Min(DatumGetTimestampTz(box1->period.lower),
    DatumGetTimestampTz(box2->period.lower));
  TimestampTz tmax = Max(DatumGetTimestampTz(box1->period.upper),
    DatumGetTimestampTz(box2->period.upper));
  box1->period.lower = TimestampTzGetDatum(tmin);
  box1->period.upper = TimestampTzGetDatum(tmax);
  return;
}