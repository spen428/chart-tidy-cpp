/**
 *  chart-tidy - A tool for automatically fixing Guitar Hero III song charts.
 *
 *  Copyright (C) 2016  lykat1
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

/**
 * Defines some of the constants associated with the FeedBack charting program.
 */

// Time durations
static const unsigned int DURATION_1_1 = 768; // One measure
static const unsigned int DURATION_1_2 = DURATION_1_1 / 2; // 1/2 a measure
static const unsigned int DURATION_1_3 = DURATION_1_1 / 3; // etc..
static const unsigned int DURATION_1_4 = DURATION_1_1 / 4;
static const unsigned int DURATION_1_6 = DURATION_1_1 / 6;
static const unsigned int DURATION_1_8 = DURATION_1_1 / 8;
static const unsigned int DURATION_1_12 = DURATION_1_1 / 12;
static const unsigned int DURATION_1_16 = DURATION_1_1 / 16;
static const unsigned int DURATION_1_24 = DURATION_1_1 / 24;
static const unsigned int DURATION_1_32 = DURATION_1_1 / 32;
static const unsigned int DURATION_1_48 = DURATION_1_1 / 48;
static const unsigned int DURATION_1_64 = DURATION_1_1 / 64;

inline unsigned long durationToMillis(const unsigned int bpmT,
        const unsigned int dur) {
    return ((60000000UL * dur) / ((unsigned long) bpmT * DURATION_1_4));
}
