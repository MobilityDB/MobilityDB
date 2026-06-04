#!/usr/bin/env python3
"""Insert missing DocBook ``<indexterm>`` entries for ``C6`` alignment gaps.

This is `tools/close_c6_gaps.py`. It complements
``check_sql_doc_alignment.py``: each gap is a public SQL function name
absent from the DocBook index but documented via operator symbols, and
this script inserts a function-name ``<indexterm>`` entry before the
closing tag of the relevant section.
"""
import re
import os
import tempfile
from collections import defaultdict

DOC_DIR = os.path.join(os.path.dirname(__file__), "..", "doc")

# ---------------------------------------------------------------------------
# Mapping: fn_name → (chapter_file_basename, section_xml_id)
# SKIP means "no suitable DocBook section — do not add".
# The section_xml_id is the xml:id of the listitem or sect that CONTAINS
# the target content.  We find the enclosing <sect1>/<sect2> and insert
# before its closing tag.
# ---------------------------------------------------------------------------
SKIP = "SKIP"

MAPPING = {
    # -----------------------------------------------------------------------
    # doc/box_types.xml
    # -----------------------------------------------------------------------

    # binary I/O section (xml:id on listitem inside sect1 box_input_output)
    "stboxfrombinary":       ("box_types.xml", "boxFromBinary"),
    "tboxfrombinary":        ("box_types.xml", "boxFromBinary"),
    "stboxfromhexwkb":       ("box_types.xml", "boxFromBinary"),
    "tboxfromhexwkb":        ("box_types.xml", "boxFromBinary"),

    # splitting (xml:id on sect1)
    "tboxes":                ("box_types.xml", "box_splitting"),
    "stboxes":               ("box_types.xml", "box_splitting"),

    # set operations (xml:id on sect1)
    "stbox_union":           ("box_types.xml", "box_set_ops"),
    "tbox_union":            ("box_types.xml", "box_set_ops"),
    "stbox_intersection":    ("box_types.xml", "box_set_ops"),
    "tbox_intersection":     ("box_types.xml", "box_set_ops"),

    # topological + positional operators (xml:id on sect1)
    "stbox_overlaps":        ("box_types.xml", "box_topo_pos"),
    "stbox_contains":        ("box_types.xml", "box_topo_pos"),
    "stbox_contained":       ("box_types.xml", "box_topo_pos"),
    "stbox_same":            ("box_types.xml", "box_topo_pos"),
    "stbox_adjacent":        ("box_types.xml", "box_topo_pos"),
    "stbox_left":            ("box_types.xml", "box_topo_pos"),
    "stbox_right":           ("box_types.xml", "box_topo_pos"),
    "stbox_overleft":        ("box_types.xml", "box_topo_pos"),
    "stbox_overright":       ("box_types.xml", "box_topo_pos"),
    "stbox_below":           ("box_types.xml", "box_topo_pos"),
    "stbox_above":           ("box_types.xml", "box_topo_pos"),
    "stbox_overbelow":       ("box_types.xml", "box_topo_pos"),
    "stbox_overabove":       ("box_types.xml", "box_topo_pos"),
    "stbox_front":           ("box_types.xml", "box_topo_pos"),
    "stbox_back":            ("box_types.xml", "box_topo_pos"),
    "stbox_overfront":       ("box_types.xml", "box_topo_pos"),
    "stbox_overback":        ("box_types.xml", "box_topo_pos"),
    "stbox_before":          ("box_types.xml", "box_topo_pos"),
    "stbox_after":           ("box_types.xml", "box_topo_pos"),
    "stbox_overbefore":      ("box_types.xml", "box_topo_pos"),
    "stbox_overafter":       ("box_types.xml", "box_topo_pos"),
    "tbox_left":             ("box_types.xml", "box_topo_pos"),
    "tbox_right":            ("box_types.xml", "box_topo_pos"),
    "tbox_overleft":         ("box_types.xml", "box_topo_pos"),
    "tbox_overright":        ("box_types.xml", "box_topo_pos"),
    "tbox_before":           ("box_types.xml", "box_topo_pos"),
    "tbox_after":            ("box_types.xml", "box_topo_pos"),
    "tbox_overbefore":       ("box_types.xml", "box_topo_pos"),
    "tbox_overafter":        ("box_types.xml", "box_topo_pos"),
    "tbox_overlaps":         ("box_types.xml", "box_topo_pos"),
    "tbox_contains":         ("box_types.xml", "box_topo_pos"),
    "tbox_contained":        ("box_types.xml", "box_topo_pos"),
    "tbox_same":             ("box_types.xml", "box_topo_pos"),
    "tbox_adjacent":         ("box_types.xml", "box_topo_pos"),

    # comparisons (xml:id on sect1)
    "stbox_eq":              ("box_types.xml", "box_comparisons"),
    "stbox_ne":              ("box_types.xml", "box_comparisons"),
    "stbox_lt":              ("box_types.xml", "box_comparisons"),
    "stbox_le":              ("box_types.xml", "box_comparisons"),
    "stbox_gt":              ("box_types.xml", "box_comparisons"),
    "stbox_ge":              ("box_types.xml", "box_comparisons"),
    "tbox_eq":               ("box_types.xml", "box_comparisons"),
    "tbox_ne":               ("box_types.xml", "box_comparisons"),
    "tbox_lt":               ("box_types.xml", "box_comparisons"),
    "tbox_le":               ("box_types.xml", "box_comparisons"),
    "tbox_gt":               ("box_types.xml", "box_comparisons"),
    "tbox_ge":               ("box_types.xml", "box_comparisons"),

    # -----------------------------------------------------------------------
    # doc/set_span_types.xml
    # -----------------------------------------------------------------------

    # FromBinary (xml:id on listitem)
    "bigintsetfrombinary":       ("set_span_types.xml", "setspan_FromBinary"),
    "bigintspanfrombinary":      ("set_span_types.xml", "setspan_FromBinary"),
    "bigintspansetfrombinary":   ("set_span_types.xml", "setspan_FromBinary"),
    "datespanfrombinary":        ("set_span_types.xml", "setspan_FromBinary"),
    "datespansetfrombinary":     ("set_span_types.xml", "setspan_FromBinary"),
    "datesetfrombinary":         ("set_span_types.xml", "setspan_FromBinary"),
    "floatsetfrombinary":        ("set_span_types.xml", "setspan_FromBinary"),
    "floatspanfrombinary":       ("set_span_types.xml", "setspan_FromBinary"),
    "floatspansetfrombinary":    ("set_span_types.xml", "setspan_FromBinary"),
    "intsetfrombinary":          ("set_span_types.xml", "setspan_FromBinary"),
    "intspanfrombinary":         ("set_span_types.xml", "setspan_FromBinary"),
    "intspansetfrombinary":      ("set_span_types.xml", "setspan_FromBinary"),
    "textsetfrombinary":         ("set_span_types.xml", "setspan_FromBinary"),
    "tstzsetfrombinary":         ("set_span_types.xml", "setspan_FromBinary"),
    "tstzspanfrombinary":        ("set_span_types.xml", "setspan_FromBinary"),
    "tstzspansetfrombinary":     ("set_span_types.xml", "setspan_FromBinary"),

    # FromHexWKB (xml:id on listitem)
    "bigintsetfromhexwkb":       ("set_span_types.xml", "setspan_FromHexWKB"),
    "bigintspanfromhexwkb":      ("set_span_types.xml", "setspan_FromHexWKB"),
    "bigintspansetfromhexwkb":   ("set_span_types.xml", "setspan_FromHexWKB"),
    "datespanfromhexwkb":        ("set_span_types.xml", "setspan_FromHexWKB"),
    "datespansetfromhexwkb":     ("set_span_types.xml", "setspan_FromHexWKB"),
    "datesetfromhexwkb":         ("set_span_types.xml", "setspan_FromHexWKB"),
    "floatsetfromhexwkb":        ("set_span_types.xml", "setspan_FromHexWKB"),
    "floatspanfromhexwkb":       ("set_span_types.xml", "setspan_FromHexWKB"),
    "floatspansetfromhexwkb":    ("set_span_types.xml", "setspan_FromHexWKB"),
    "intsetfromhexwkb":          ("set_span_types.xml", "setspan_FromHexWKB"),
    "intspanfromhexwkb":         ("set_span_types.xml", "setspan_FromHexWKB"),
    "intspansetfromhexwkb":      ("set_span_types.xml", "setspan_FromHexWKB"),
    "textsetfromhexwkb":         ("set_span_types.xml", "setspan_FromHexWKB"),
    "tstzsetfromhexwkb":         ("set_span_types.xml", "setspan_FromHexWKB"),
    "tstzspanfromhexwkb":        ("set_span_types.xml", "setspan_FromHexWKB"),
    "tstzspansetfromhexwkb":     ("set_span_types.xml", "setspan_FromHexWKB"),

    # set / span / spanset constructors (xml:id on listitem)
    "intset":                    ("set_span_types.xml", "set"),
    "floatset":                  ("set_span_types.xml", "set"),
    "dateset":                   ("set_span_types.xml", "set"),
    "tstzset":                   ("set_span_types.xml", "set"),

    "intspan":                   ("set_span_types.xml", "span"),
    "floatspan":                 ("set_span_types.xml", "span"),
    "datespan":                  ("set_span_types.xml", "span"),
    "tstzspan":                  ("set_span_types.xml", "span"),

    "intspanset":                ("set_span_types.xml", "spanset"),
    "floatspanset":              ("set_span_types.xml", "spanset"),
    "datespanset":               ("set_span_types.xml", "spanset"),
    "tstzspanset":               ("set_span_types.xml", "spanset"),

    # textset_cat (xml:id on listitem)
    "textset_cat":               ("set_span_types.xml", "textset_concat"),

    # spann / timespan / valuespan / spanset_round (xml:id on listitem)
    "spann":                     ("set_span_types.xml", "floatsetspan_round"),
    "timespan":                  ("set_span_types.xml", "floatsetspan_round"),
    "valuespan":                 ("temporal_types_p2.xml", "ttype_restrictions"),

    # set operations (xml:id on sect1)
    "set_union":                 ("set_span_types.xml", "setspan_set_ops"),
    "set_minus":                 ("set_span_types.xml", "setspan_set_ops"),
    "set_intersection":          ("set_span_types.xml", "setspan_set_ops"),
    "span_union":                ("set_span_types.xml", "setspan_set_ops"),
    "span_minus":                ("set_span_types.xml", "setspan_set_ops"),
    "span_intersection":         ("set_span_types.xml", "setspan_set_ops"),

    # topo/pos operators (xml:id on sect1)
    "span_overlaps":             ("set_span_types.xml", "setspan_topo_pos"),
    "span_contains":             ("set_span_types.xml", "setspan_topo_pos"),
    "span_contained":            ("set_span_types.xml", "setspan_topo_pos"),
    "span_adjacent":             ("set_span_types.xml", "setspan_topo_pos"),
    "span_left":                 ("set_span_types.xml", "setspan_topo_pos"),
    "span_right":                ("set_span_types.xml", "setspan_topo_pos"),
    "span_overleft":             ("set_span_types.xml", "setspan_topo_pos"),
    "span_overright":            ("set_span_types.xml", "setspan_topo_pos"),
    "set_overlaps":              ("set_span_types.xml", "setspan_topo_pos"),
    "set_overleft":              ("set_span_types.xml", "setspan_topo_pos"),
    "set_overright":             ("set_span_types.xml", "setspan_topo_pos"),
    "set_contained":             ("set_span_types.xml", "setspan_topo_pos"),
    "set_contains":              ("set_span_types.xml", "setspan_topo_pos"),
    "set_left":                  ("set_span_types.xml", "setspan_topo_pos"),
    "set_right":                 ("set_span_types.xml", "setspan_topo_pos"),

    # distance (xml:id on sect1)
    "span_distance":             ("set_span_types.xml", "setspan_distance"),
    "set_distance":              ("set_span_types.xml", "setspan_distance"),

    # comparisons (xml:id on sect1)
    "span_eq":                   ("set_span_types.xml", "setspan_comparisons"),
    "span_ne":                   ("set_span_types.xml", "setspan_comparisons"),
    "span_lt":                   ("set_span_types.xml", "setspan_comparisons"),
    "span_le":                   ("set_span_types.xml", "setspan_comparisons"),
    "span_gt":                   ("set_span_types.xml", "setspan_comparisons"),
    "span_ge":                   ("set_span_types.xml", "setspan_comparisons"),
    "set_eq":                    ("set_span_types.xml", "setspan_comparisons"),
    "set_ne":                    ("set_span_types.xml", "setspan_comparisons"),
    "set_lt":                    ("set_span_types.xml", "setspan_comparisons"),
    "set_le":                    ("set_span_types.xml", "setspan_comparisons"),
    "set_gt":                    ("set_span_types.xml", "setspan_comparisons"),
    "set_ge":                    ("set_span_types.xml", "setspan_comparisons"),
    "spanset_eq":                ("set_span_types.xml", "setspan_comparisons"),
    "spanset_ne":                ("set_span_types.xml", "setspan_comparisons"),
    "spanset_lt":                ("set_span_types.xml", "setspan_comparisons"),
    "spanset_le":                ("set_span_types.xml", "setspan_comparisons"),
    "spanset_gt":                ("set_span_types.xml", "setspan_comparisons"),
    "spanset_ge":                ("set_span_types.xml", "setspan_comparisons"),

    # spatial set from binary/hexwkb/ewkb/ewkt/text (xml:id on listitem)
    "geomsetfrombinary":         ("set_span_types.xml", "spatialset_SRID"),
    "geomsetfromhexwkb":         ("set_span_types.xml", "spatialset_SRID"),
    "geomsetfromewkb":           ("set_span_types.xml", "spatialset_SRID"),
    "geomsetfromewkt":           ("set_span_types.xml", "spatialset_SRID"),
    "geomsetfromtext":           ("set_span_types.xml", "spatialset_SRID"),
    "geogsetfrombinary":         ("set_span_types.xml", "spatialset_SRID"),
    "geogsetfromhexwkb":         ("set_span_types.xml", "spatialset_SRID"),
    "geogsetfromewkb":           ("set_span_types.xml", "spatialset_SRID"),
    "geogsetfromewkt":           ("set_span_types.xml", "spatialset_SRID"),
    "geogsetfromtext":           ("set_span_types.xml", "spatialset_SRID"),

    # -----------------------------------------------------------------------
    # doc/temporal_types_p1.xml
    # -----------------------------------------------------------------------

    # from-binary / from-hexwkb / from-mfjson (xml:id on listitem)
    "tboolfrombinary":           ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tintfrombinary":            ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tfloatfrombinary":          ("temporal_types_p1.xml", "ttypeFromBinary"),
    "ttextfrombinary":           ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tboolfromhexwkb":           ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tintfromhexwkb":            ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tfloatfromhexwkb":          ("temporal_types_p1.xml", "ttypeFromBinary"),
    "ttextfromhexwkb":           ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tboolfrommfjson":           ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tintfrommfjson":            ("temporal_types_p1.xml", "ttypeFromBinary"),
    "tfloatfrommfjson":          ("temporal_types_p1.xml", "ttypeFromBinary"),
    "ttextfrommfjson":           ("temporal_types_p1.xml", "ttypeFromBinary"),

    # base constructors (xml:id on listitem)
    "tbool":                     ("temporal_types_p1.xml", "ttype_const"),
    "tint":                      ("temporal_types_p1.xml", "ttype_const"),
    "tfloat":                    ("temporal_types_p1.xml", "ttype_const"),
    "ttext":                     ("temporal_types_p1.xml", "ttype_const"),

    # instant / seq / seqset constructors (xml:id on listitem)
    "tboolinst":                 ("temporal_types_p1.xml", "ttypeSeq"),
    "tintinst":                  ("temporal_types_p1.xml", "ttypeSeq"),
    "tfloatinst":                ("temporal_types_p1.xml", "ttypeSeq"),
    "ttextinst":                 ("temporal_types_p1.xml", "ttypeSeq"),
    "tboolseq":                  ("temporal_types_p1.xml", "ttypeSeq"),
    "tintseq":                   ("temporal_types_p1.xml", "ttypeSeq"),
    "tfloatseq":                 ("temporal_types_p1.xml", "ttypeSeq"),
    "ttextseq":                  ("temporal_types_p1.xml", "ttypeSeq"),
    "tboolseqset":               ("temporal_types_p1.xml", "ttypeSeq"),
    "tintseqset":                ("temporal_types_p1.xml", "ttypeSeq"),
    "tfloatseqset":              ("temporal_types_p1.xml", "ttypeSeq"),
    "ttextseqset":               ("temporal_types_p1.xml", "ttypeSeq"),
    "tboolseqsetgaps":           ("temporal_types_p1.xml", "ttypeSeq"),
    "tintseqsetgaps":            ("temporal_types_p1.xml", "ttypeSeq"),
    "tfloatseqsetgaps":          ("temporal_types_p1.xml", "ttypeSeq"),
    "ttextseqsetgaps":           ("temporal_types_p1.xml", "ttypeSeq"),

    # interp (xml:id on listitem)
    "interp":                    ("temporal_types_p1.xml", "ttype_interp"),

    # getValues / positions / gettime (xml:id on listitem)
    "positions":                 ("temporal_types_p1.xml", "ttype_getValues"),
    "gettime":                   ("temporal_types_p1.xml", "ttype_getValues"),

    # memSize / getbin (xml:id on listitem)
    "getbin":                    ("temporal_types_p1.xml", "ttype_memSize"),

    # -----------------------------------------------------------------------
    # doc/temporal_types_p2.xml
    # -----------------------------------------------------------------------

    # bbox operators (xml:id on sect1)
    "temporal_above":            ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_adjacent":         ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_after":            ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_back":             ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_before":           ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_below":            ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_front":            ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_left":             ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overabove":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overafter":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overback":         ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overbefore":       ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overbelow":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overfront":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overlaps":         ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overleft":         ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_overright":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_right":            ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_contained":        ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_contains":         ("temporal_types_p2.xml", "ttype_bbox"),
    "temporal_same":             ("temporal_types_p2.xml", "ttype_bbox"),

    # comparisons (xml:id on sect1)
    "temporal_eq":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_ne":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_lt":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_le":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_gt":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_ge":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_teq":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_tne":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_tlt":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_tle":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_tgt":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "temporal_tge":              ("temporal_types_p2.xml", "ttype_comparisons"),

    # ever/always comparisons — these live in the ever_always_comparison sect2
    "always_eq":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "always_ne":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "always_lt":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "always_le":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "always_gt":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "always_ge":                 ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_eq":                   ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_ne":                   ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_lt":                   ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_le":                   ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_gt":                   ("temporal_types_p2.xml", "ever_always_comparison"),
    "ever_ge":                   ("temporal_types_p2.xml", "ever_always_comparison"),

    # restrictions (xml:id on sect1)
    "atvalue":                   ("temporal_types_p2.xml", "ttype_restrictions"),
    "minusvalue":                ("temporal_types_p2.xml", "ttype_restrictions"),
    "volume":                    ("temporal_types_p2.xml", "ttype_restrictions"),
    "valueweight":               ("temporal_types_p2.xml", "ttype_restrictions"),

    # ttext_cat — comparisons section (closest fit)
    "ttext_cat":                 ("temporal_types_p2.xml", "ttype_comparisons"),

    # tbool operators — comparisons section
    "tbool_and":                 ("temporal_types_p2.xml", "ttype_comparisons"),
    "tbool_or":                  ("temporal_types_p2.xml", "ttype_comparisons"),
    "tbool_not":                 ("temporal_types_p2.xml", "ttype_comparisons"),

    # tnumber arithmetic
    "tnumber_add":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "tnumber_sub":               ("temporal_types_p2.xml", "ttype_comparisons"),
    "tnumber_mult":              ("temporal_types_p2.xml", "ttype_comparisons"),
    "tnumber_div":               ("temporal_types_p2.xml", "ttype_comparisons"),

    # miscellaneous (xml:id on sect1)
    "spacetimesplit":            ("temporal_types_p2.xml", "ttype_miscellaneous"),

    # -----------------------------------------------------------------------
    # doc/temporal_spatial_p1.xml
    # -----------------------------------------------------------------------

    # from-binary / from-hexwkb / from-mfjson / from-text/ewkb/ewkt
    # (xml:id on listitem tspatialFromBinary inside sect1 tgeo_inout)
    "tgeompointfrombinary":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfrombinary":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfrombinary":       ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfrombinary":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeompointfromhexewkb":     ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfromhexewkb":     ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfromhexewkb":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfromhexewkb":     ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeompointfrommfjson":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfrommfjson":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfrommfjson":       ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfrommfjson":      ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeompointfromewkb":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfromewkb":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfromewkb":         ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfromewkb":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeompointfromewkt":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfromewkt":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfromewkt":         ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfromewkt":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeompointfromtext":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeogpointfromtext":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeometryfromtext":         ("temporal_spatial_p1.xml", "tspatialFromBinary"),
    "tgeographyfromtext":        ("temporal_spatial_p1.xml", "tspatialFromBinary"),

    # tgeompoint / tgeogpoint constructor aliases (xml:id on listitem tgeo_const
    # inside sect1 tgeo_constructors)
    "tgeompoint":                ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeogpoint":                ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeompointinst":            ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeogpointinst":            ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeompointseq":             ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeogpointseq":             ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeompointseqset":          ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeogpointseqset":          ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeompointseqsetgaps":      ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeogpointseqsetgaps":      ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeometryinst":             ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeographyinst":            ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeometryseqsetgaps":       ("temporal_spatial_p1.xml", "tgeo_constructors"),
    "tgeographyseqsetgaps":      ("temporal_spatial_p1.xml", "tgeo_constructors"),

    # tgeo_teq / tgeo_tne — accessors section
    "tgeo_teq":                  ("temporal_spatial_p1.xml", "tgeo_accessors"),
    "tgeo_tne":                  ("temporal_spatial_p1.xml", "tgeo_accessors"),

    # tgeo_affine — transform_gk, translate, transscale (xml:id on listitem
    # tgeo_affine inside sect1 tgeo_transformations)
    "transform_gk":              ("temporal_spatial_p1.xml", "tgeo_affine"),
    "translate":                 ("temporal_spatial_p1.xml", "tgeo_affine"),
    "transscale":                ("temporal_spatial_p1.xml", "tgeo_affine"),

    # distance / time_distance / nearestapproachdistance / tdistance / tcovers
    # — modifications section (closest available)
    "nearestapproachdistance":   ("temporal_spatial_p1.xml", "tgeo_modifications"),
    "tdistance":                 ("temporal_spatial_p1.xml", "tgeo_modifications"),
    "tcovers":                   ("temporal_spatial_p1.xml", "tgeo_modifications"),
    "time_distance":             ("temporal_spatial_p1.xml", "tgeo_modifications"),
    "distance":                  ("temporal_spatial_p1.xml", "tgeo_modifications"),

    # -----------------------------------------------------------------------
    # doc/temporal_network_points.xml
    # -----------------------------------------------------------------------

    # npoint comparisons (xml:id on sect2)
    "npoint_eq":                 ("temporal_network_points.xml", "npoint_comparisons"),
    "npoint_ne":                 ("temporal_network_points.xml", "npoint_comparisons"),
    "npoint_lt":                 ("temporal_network_points.xml", "npoint_comparisons"),
    "npoint_le":                 ("temporal_network_points.xml", "npoint_comparisons"),
    "npoint_gt":                 ("temporal_network_points.xml", "npoint_comparisons"),
    "npoint_ge":                 ("temporal_network_points.xml", "npoint_comparisons"),

    # nsegment comparisons (same sect2)
    "nsegment_eq":               ("temporal_network_points.xml", "npoint_comparisons"),
    "nsegment_ne":               ("temporal_network_points.xml", "npoint_comparisons"),
    "nsegment_lt":               ("temporal_network_points.xml", "npoint_comparisons"),
    "nsegment_le":               ("temporal_network_points.xml", "npoint_comparisons"),
    "nsegment_gt":               ("temporal_network_points.xml", "npoint_comparisons"),
    "nsegment_ge":               ("temporal_network_points.xml", "npoint_comparisons"),

    # npoint_same
    "same":                      ("temporal_network_points.xml", "npoint_comparisons"),

    # npoint from-binary/hexwkb/ewkb/ewkt/text (xml:id on sect2)
    "npointfrombinary":          ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointfromewkb":            ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointfromewkt":            ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointfromhexewkb":         ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointfromtext":            ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointsetfrombinary":       ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointsetfromewkb":         ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointsetfromewkt":         ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointsetfromhexwkb":       ("temporal_network_points.xml", "npoint_constructor_functions"),
    "npointsetfromtext":         ("temporal_network_points.xml", "npoint_constructor_functions"),

    # tnpoint from-binary/hexwkb/seqsetgaps (xml:id on sect1)
    "tnpointfrombinary":         ("temporal_network_points.xml", "tnpoint_constructors"),
    "tnpointfromhexwkb":         ("temporal_network_points.xml", "tnpoint_constructors"),
    "tnpointseqsetgaps":         ("temporal_network_points.xml", "tnpoint_constructors"),

    # create_trip (xml:id on sect1)
    "create_trip":               ("temporal_network_points.xml", "tnpoint_constructors"),

    # tnpoint topological operators (xml:id on sect1)
    "overlaps_rid":              ("temporal_network_points.xml", "tnpoint_bbox_ops"),
    "contained_rid":             ("temporal_network_points.xml", "tnpoint_bbox_ops"),
    "contains_rid":              ("temporal_network_points.xml", "tnpoint_bbox_ops"),
    "same_rid":                  ("temporal_network_points.xml", "tnpoint_bbox_ops"),

    # routes (xml:id on sect1)
    "route":                     ("temporal_network_points.xml", "tnpoint_accessors"),

    # -----------------------------------------------------------------------
    # doc/temporal_circular_buffers.xml
    # -----------------------------------------------------------------------

    # cbuffer comparison / spatial operators (xml:id on sect2)
    "cbuffer_eq":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_ne":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_lt":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_le":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_gt":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_ge":                ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_contains":          ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_covers":            ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_disjoint":          ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_dwithin":           ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_intersects":        ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_same":              ("temporal_circular_buffers.xml", "cbuffer_static_operators"),
    "cbuffer_touches":           ("temporal_circular_buffers.xml", "cbuffer_static_operators"),

    # cbufferset from-binary / hexwkb (xml:id on listitem tcbufferFromBinary
    # inside sect1 tcbuffer_inout)
    "cbuffersetfrombinary":      ("temporal_circular_buffers.xml", "tcbufferFromBinary"),
    "cbuffersetfromhexwkb":      ("temporal_circular_buffers.xml", "tcbufferFromBinary"),

    # tcbuffer seqsetgaps (xml:id on listitem inside sect1 tcbuffer_constructors)
    "tcbufferseqsetgaps":        ("temporal_circular_buffers.xml", "tcbufferSeqSet"),

    # -----------------------------------------------------------------------
    # doc/temporal_poses.xml
    # -----------------------------------------------------------------------

    # pose set from-binary / hexwkb / ewkb / ewkt / text (xml:id on listitem
    # poseFromBinary inside sect2 pose_inout)
    "posesetfrombinary":         ("temporal_poses.xml", "poseFromBinary"),
    "posesetfromhexwkb":         ("temporal_poses.xml", "poseFromBinary"),
    "posesetfromewkb":           ("temporal_poses.xml", "poseFromBinary"),
    "posesetfromewkt":           ("temporal_poses.xml", "poseFromBinary"),
    "posesetfromtext":           ("temporal_poses.xml", "poseFromBinary"),

    # tpose from-binary / hexwkb / mfjson (xml:id on listitem tposeFromBinary
    # inside sect1 tpose_inout)
    "tposefrombinary":           ("temporal_poses.xml", "tposeFromBinary"),
    "tposefromhexwkb":           ("temporal_poses.xml", "tposeFromBinary"),
    "tposefrommfjson":           ("temporal_poses.xml", "tposeFromBinary"),

    # tpose seqsetgaps (xml:id on listitem inside sect1 tpose_constructors)
    "tposeseqsetgaps":           ("temporal_poses.xml", "tposeSeqSet"),

    # pose comparisons (xml:id on sect2)
    "pose_eq":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_ne":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_lt":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_le":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_gt":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_ge":                   ("temporal_poses.xml", "pose_comparison"),
    "pose_same":                 ("temporal_poses.xml", "pose_comparison"),
}

# ---------------------------------------------------------------------------
# Helper: build cluster of indexterm lines
# ---------------------------------------------------------------------------


def build_indexterm_block(fn_names: list) -> str:
    """Return a comment + cluster of <indexterm> lines."""
    lines = ["\t\t\t\t<!-- Function names for the operators above -->\n"]
    for fn in sorted(fn_names):
        lines.append(
            f"\t\t\t\t<indexterm significance=\"normal\">"
            f"<primary><varname>{fn}</varname></primary></indexterm>\n"
        )
    return "".join(lines)


# ---------------------------------------------------------------------------
# Core: find the position just before the closing sect tag that encloses
# the element with xml:id="section_id".
# ---------------------------------------------------------------------------

def find_enclosing_sect_close(content: str, section_id: str):
    """Locate the closing tag of the outermost section with the given xml:id.

    Returns ``(insert_pos, tag_name)`` pointing just before the matching
    ``</sect1>`` or ``</sect2>``, or ``(None, None)`` if not found.
    """
    id_pat = re.compile(r'xml:id="' + re.escape(section_id) + r'"')
    m = id_pat.search(content)
    if not m:
        return None, None

    id_pos = m.start()

    # Walk forward from position 0 counting depth for each sect level,
    # record which sect is "currently open" when we reach id_pos.
    stack = []  # list of (tag_name, open_start_pos)
    pos = 0
    open_re = re.compile(r'<(sect[12])\b')
    close_re = re.compile(r'</(sect[12])>')

    while pos < id_pos:
        m_open = open_re.search(content, pos, id_pos)
        m_close = close_re.search(content, pos, id_pos)

        if m_open is None and m_close is None:
            break
        if m_open is not None and (m_close is None or m_open.start() < m_close.start()):
            stack.append((m_open.group(1), m_open.start()))
            pos = m_open.end()
        else:
            if stack:
                stack.pop()
            pos = m_close.end()

    if not stack:
        return None, None

    # The innermost enclosing sect is stack[-1]
    tag_name, sect_start = stack[-1]

    # Now from sect_start, find the matching close tag
    depth = 0
    pos = sect_start
    open_tag_re = re.compile(r'<(' + re.escape(tag_name) + r')\b')
    close_tag_re = re.compile(r'</' + re.escape(tag_name) + r'>')

    while pos < len(content):
        m_open = open_tag_re.search(content, pos)
        m_close = close_tag_re.search(content, pos)

        if m_close is None:
            return None, None

        if m_open is not None and m_open.start() < m_close.start():
            depth += 1
            pos = m_open.end()
        else:
            depth -= 1
            if depth == 0:
                return m_close.start(), tag_name
            pos = m_close.end()

    return None, None


def process_file(filepath: str, groups: dict):
    """Insert indexterm entries for each section group and return the count.

    ``groups`` maps each ``section_id`` to a list of function names. The
    new entries are inserted before the section's enclosing closing tag.
    """
    with open(filepath, "r", encoding="utf-8") as fh:
        content = fh.read()

    total_added = 0
    inserts = []  # list of (insert_pos, block_str, count)

    for section_id, fn_names in groups.items():
        # Skip functions already indexed
        new_fns = []
        for fn in fn_names:
            needle = f"<varname>{fn}</varname></primary></indexterm>"
            if needle in content:
                # already present
                pass
            else:
                new_fns.append(fn)

        if not new_fns:
            continue

        insert_pos, _tag_name = find_enclosing_sect_close(content, section_id)
        if insert_pos is None:
            print(f"  WARNING: section '{section_id}' not found in {os.path.basename(filepath)}")
            continue

        block = build_indexterm_block(new_fns)
        inserts.append((insert_pos, block, len(new_fns)))

    # Apply in reverse order so positions stay valid
    inserts.sort(key=lambda x: x[0], reverse=True)
    for insert_pos, block, count in inserts:
        content = content[:insert_pos] + block + content[insert_pos:]
        total_added += count

    if total_added > 0:
        with open(filepath, "w", encoding="utf-8") as fh:
            fh.write(content)

    return total_added


def main():
    """Entry point for the CLI."""
    # Group by (file, section_id)
    groups_by_file = defaultdict(lambda: defaultdict(list))
    skipped = []

    for fn_name, dest in MAPPING.items():
        if dest == SKIP:
            skipped.append(fn_name)
            continue
        filename, section_id = dest
        groups_by_file[filename][section_id].append(fn_name)

    # Report any names in c6_clean.txt NOT in MAPPING
    c6_path = os.environ.get("C6_CLEAN_FILE", os.path.join(tempfile.gettempdir(), "c6_clean.txt"))
    if os.path.exists(c6_path):
        with open(c6_path, encoding="utf-8") as fh:
            gap_names = set(fh.read().split())
        unmapped = gap_names - set(MAPPING.keys())
        if unmapped:
            print(f"\nWARNING: {len(unmapped)} names from c6_clean.txt have no mapping:")
            for n in sorted(unmapped):
                print(f"  UNMAPPED: {n}")

    if skipped:
        print(f"\nSKIPPED (no section / already indexed): {len(skipped)}")
        for n in sorted(skipped):
            print(f"  SKIP: {n}")

    grand_total = 0
    print()
    for filename, section_groups in sorted(groups_by_file.items()):
        filepath = os.path.join(DOC_DIR, filename)
        if not os.path.exists(filepath):
            print(f"FILE NOT FOUND: {filepath}")
            continue
        added = process_file(filepath, section_groups)
        grand_total += added
        print(f"{filename}: +{added} indexterm entries")

    print(f"\nTotal indexterm entries added: {grand_total}")


if __name__ == "__main__":
    main()
