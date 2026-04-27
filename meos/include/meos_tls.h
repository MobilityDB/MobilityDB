/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Thread-local-storage qualifier macro used by MEOS to mark
 * per-thread state. Kept in a stand-alone header so that vendored
 * PostgreSQL headers (e.g. pgtime.h) can pick it up without pulling in
 * the full meos.h.
 */

#ifndef __MEOS_TLS_H__
#define __MEOS_TLS_H__

/*
 * Thread-local storage qualifier. C11 _Thread_local is supported by GCC,
 * Clang, and MSVC 2019 16.10+; older compilers fall back to vendor
 * extensions. If none of these is available the macro expands to
 * nothing and MEOS state remains process-global (legacy behaviour).
 */
#if defined(__cplusplus) && __cplusplus >= 201103L
#define MEOS_TLS thread_local
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && \
    !defined(__STDC_NO_THREADS__)
#define MEOS_TLS _Thread_local
#elif defined(_MSC_VER)
#define MEOS_TLS __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)
#define MEOS_TLS __thread
#else
#define MEOS_TLS  /* not supported; falls back to non-thread-safe globals */
#endif

#endif /* __MEOS_TLS_H__ */
