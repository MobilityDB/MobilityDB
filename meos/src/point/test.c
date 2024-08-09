/**
 * @brief Return a range value from given arguments
 */
RangeType *
multirange_to_range(TypeCacheEntry *typcache, MultirangeType *mr)
{
  RangeBound lower, upper, bound_tmp;
  bool empty;
  RangeType *range = multirange_get_range(typcache, mr, 0);
  range_deserialize(typcache, range, &lower, &bound_tmp, &empty);
  range = multirange_get_range(typcache, mr, mr->rangeCount - 1);
  range_deserialize(typcache, range, &bound_tmp, &upper, &empty);
  return make_range(typcache, &lower, &upper, false, NULL);
}
