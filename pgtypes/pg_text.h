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
 * @brief Functions for base and time types corresponding to the external (SQL)
 * PostgreSQL functions
 */

#ifndef PG_TEXT_H
#define PG_TEXT_H

typedef struct varlena;
typedef struct varlena text;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef unsigned int Oid;

/*****************************************************************************/

/* Functions for text */

extern uint32 char_hash(char c);
extern uint64 char_hash_extended(char c, uint64 seed);
extern text *cstring_to_text(const char *str);
extern text *icu_unicode_version(void);
extern text *int32_to_bin(int32 num);
extern text *int32_to_hex(int32 num);
extern text *int32_to_oct(int32 num);
extern text *int64_to_bin(int64 num);
extern text *int64_to_hex(int64 num);
extern text *int64_to_oct(int64 num);
extern char *text_to_cstring(const text *txt);
extern text *text_cat(const text *txt1, const text *txt2);
extern int text_cmp(const text *txt1, const text *txt2, Oid collid);
extern text *text_concat(text **textarr, int count);
extern text *text_concat_ws(text **textarr, int count, const text *sep);
extern text *text_copy(const text *txt);
extern bool text_eq(const text *txt1, const text *txt2);
extern bool text_ge(const text *txt1, const text *txt2);
extern bool text_gt(const text *txt1, const text *txt2);
extern uint32 text_hash(const text *txt, Oid collid);
extern uint64 text_hash_extended(const text *txt, uint64 seed, Oid collid);
extern text *text_in(const char *str);
extern text *text_initcap(const text *txt);
extern text *text_larger(const text *txt1, const text *txt2);
extern bool text_le(const text *txt1, const text *txt2);
extern text *text_left(const text *txt, int n);
extern int32 text_len(const text *txt);
extern text *text_lower(const text *txt);
extern bool text_lt(const text *txt1, const text *txt2);
extern bool text_ne(const text *txt1, const text *txt2);
extern int32 text_octetlen(const text *txt);
extern char *text_out(const text *txt);
extern text *text_overlay(const text *txt1, const text *txt2, int from, int count);
extern text *text_overlay_no_len(const text *txt1, const text *txt2, int from);
extern bool text_pattern_ge(const text *txt1, const text *txt2);
extern bool text_pattern_gt(const text *txt1, const text *txt2);
extern bool text_pattern_le(const text *txt1, const text *txt2);
extern bool text_pattern_lt(const text *txt1, const text *txt2);
extern int32 text_pos(const text *txt, const text *search);
extern text *text_replace(const text *txt, const text *from, const text *to);
extern text *text_reverse(const text *txt);
extern text *text_right(const text *txt, int n);
extern text *text_smaller(const text *txt1, const text *txt2);
extern text *text_split_part(const text *txt, const text *sep, int fldnum);
extern bool text_starts_with(const text *txt1, const text *txt2);
extern text *text_substr(const text *txt, int32 start, int32 length);
extern text *text_substr_no_len(const text *txt, int32 start);
extern text *text_upper(const text *txt);
extern text *textcat_text_text(const text *txt1, const text *txt2);
extern bool unicode_assigned(const text *txt);
extern bool unicode_is_normalized(const text *txt, const text *fmt);
extern text *unicode_normalize_func(const text *txt, const text *fmt);
extern text *unicode_version(void);
extern text *unistr(const text *txt);

/*****************************************************************************/

#endif /* PG_TEXT_H */
