/***********************************************************************
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
 * @brief Default encoding conversion functions
 */

extern int koi8r_to_mic(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int mic_to_koi8r(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int iso_to_mic(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int mic_to_iso(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win1251_to_mic(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int mic_to_win1251(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win866_to_mic(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int mic_to_win866(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest,  int len, bool noError);
extern int koi8r_to_win1251(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win1251_to_koi8r(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int koi8r_to_win866(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win866_to_koi8r(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win866_to_win1251(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win1251_to_win866(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int iso_to_koi8r(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int koi8r_to_iso(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int iso_to_win1251(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win1251_to_iso(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int iso_to_win866(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win866_to_iso(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int euc_cn_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_euc_cn(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int euc_jp_to_sjis(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int sjis_to_euc_jp(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int euc_jp_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_euc_jp(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int sjis_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_sjis(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int euc_kr_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_euc_kr(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int euc_tw_to_big5(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int big5_to_euc_tw(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int euc_tw_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_euc_tw(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int big5_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_big5(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int euc_jis_2004_to_shift_jis_2004(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int shift_jis_2004_to_euc_jis_2004(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int latin1_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_latin1(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int latin3_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_latin3(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int latin4_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_latin4(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int latin2_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_latin2(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int win1250_to_mic(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int mic_to_win1250(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int latin2_to_win1250(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int win1250_to_latin2(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int big5_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_big5(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int utf8_to_koi8r(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int koi8r_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_koi8u(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int koi8u_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int euc_cn_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_euc_cn(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int euc_jp_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_euc_jp(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int euc_kr_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_euc_kr(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int euc_tw_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_euc_tw(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int euc_jis_2004_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_euc_jis_2004(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int gb18030_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_gb18030(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int gbk_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_gbk(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int iso8859_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_iso8859(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
  
extern int iso8859_1_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_iso8859_1(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int johab_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_johab(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int sjis_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_sjis(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int shift_jis_2004_to_utf8(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);
extern int utf8_to_shift_jis_2004(int src_id, int dest_id, unsigned char *src,
  unsigned char *dest, int len, bool noError);

extern int uhc_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);
extern int utf8_to_uhc(int src_id, int dest_id, unsigned char *src, unsigned char *dest,
  int len, bool noError);

extern int win_to_utf8(int src_id, int dest_id, unsigned char *src, unsigned char *dest, int len,
  bool noError);
extern int utf8_to_win(int src_id, int dest_id, unsigned char *src, unsigned char *dest, int len,
  bool noError);
