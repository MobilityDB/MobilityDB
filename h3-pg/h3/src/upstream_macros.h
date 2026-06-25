/*
 * Copyright 2025 Zacharias Knudsen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Macros/etc copied from upsteam (needed for Debian packaging) */

#ifndef H3_UPSTREAM_MACROS_H
#define H3_UPSTREAM_MACROS_H

#include "h3api.h"

/* SOURCE coordijk.h */

/** @brief H3 digit representing ijk+ axes direction.
 * Values will be within the lowest 3 bits of an integer.
 */
typedef enum {
    /** H3 digit in center */
    CENTER_DIGIT = 0,
    /** H3 digit in k-axes direction */
    K_AXES_DIGIT = 1,
    /** H3 digit in j-axes direction */
    J_AXES_DIGIT = 2,
    /** H3 digit in j == k direction */
    JK_AXES_DIGIT = J_AXES_DIGIT | K_AXES_DIGIT, /* 3 */
    /** H3 digit in i-axes direction */
    I_AXES_DIGIT = 4,
    /** H3 digit in i == k direction */
    IK_AXES_DIGIT = I_AXES_DIGIT | K_AXES_DIGIT, /* 5 */
    /** H3 digit in i == j direction */
    IJ_AXES_DIGIT = I_AXES_DIGIT | J_AXES_DIGIT, /* 6 */
    /** H3 digit in the invalid direction */
    INVALID_DIGIT = 7,
    /** Valid digits will be less than this value. Same value as INVALID_DIGIT.
     */
    NUM_DIGITS = INVALID_DIGIT,
    /** Child digit which is skipped for pentagons */
    PENTAGON_SKIPPED_DIGIT = K_AXES_DIGIT /* 1 */
} Direction;

/* SOURCE h3Index.h */

/** The number of bits in a single H3 resolution digit. */
#define H3_PER_DIGIT_OFFSET 3

/** 1's in the 3 bits of res 15 digit bits, 0's everywhere else. */
#define H3_DIGIT_MASK ((uint64_t)(7))

/**
 * Gets the resolution res integer digit (0-7) of h3.
 */
#define H3_GET_INDEX_DIGIT(h3, res)                                        \
    ((Direction)((((h3) >> ((MAX_H3_RES - (res)) * H3_PER_DIGIT_OFFSET)) & \
                  H3_DIGIT_MASK)))

#endif /* H3_UPSTREAM_MACROS_H */
