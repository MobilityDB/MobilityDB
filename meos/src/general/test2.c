/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal integer and an integer.
 * @sqlop @p #>
 */
Temporal *
tgt_tint_int(const Temporal *temp, int i)
{
  return tcomp_temporal_base(temp, Int32GetDatum(i), T_INT4, &datum2_gt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal float and a float.
 * @sqlop @p #>
 */
Temporal *
tgt_tfloat_float(const Temporal *temp, double d)
{
  return tcomp_temporal_base(temp, Float8GetDatum(d), T_FLOAT8, &datum2_gt2,
    INVERT_NO);
}

/**
 * @ingroup libmeos_temporal_comp
 * @brief Return the temporal greater than of a temporal text and a text.
 * @sqlop @p #>
 */
Temporal *
tgt_ttext_text(const Temporal *temp, const text *txt)
{
  return tcomp_temporal_base(temp, PointerGetDatum(txt), T_TEXT, &datum2_gt2,
    INVERT_NO);
}
