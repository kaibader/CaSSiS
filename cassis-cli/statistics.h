/*!
 * Statistical helper functions (runtime measurements)
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) CLI.
 *
 * Copyright (C) 2010,2011
 *     Kai Christian Bader <mail@kaibader.de>
 *
 * CaSSiS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * CaSSiS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with CaSSiS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CASSIS_STATISTICS_H_
#define CASSIS_STATISTICS_H_

#include <cstdio>
#include <sys/time.h>

/*!
 * Dump statistical information:
 * - Memory consumption of the current process.
 * - Runtime since the last dump (delta-t).
 */
void dumpStats(const char *comment) {
    // Snapshot counter:
    static unsigned int snapshot = 0;
    snapshot++;

    // Fetch runtime (msecs) when entering this function.
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long long msecs_current = ((unsigned long long) tv.tv_sec) * 1000
            + tv.tv_usec / 1000;
    static unsigned long long msecs_last = 0;
    if (msecs_last == 0)
        msecs_last = msecs_current;

    // The programs memory consumption is stored here.
    size_t size = 0;
    size_t resident = 0;
    size_t share = 0;

    // Fetch the system stats (memory consumption).
    FILE *statm = fopen("/proc/self/statm", "r");
    if (statm) {
        size_t size_p = 0;
        size_t resident_p = 0;
        size_t share_p = 0;
        fscanf(statm, "%lu %lu %lu", &size_p, &resident_p, &share_p);
        fclose(statm);
        size_t pagesize = getpagesize();
        size = (size_t) size_p * pagesize;
        resident = (size_t) resident_p * pagesize;
        share = (size_t) share_p * pagesize;
    }

    // Dump the statistics with reduced accuracy for the memory (kB).
    fprintf(stderr, "\n######################################"
            "######################################\n"
            "# Snapshot %d: \"%s\"\n"
            "# Runtime (delta): %llu ms\n"
            "# Memory usage:    size     %9li kB\n"
            "#                  resident %9li kB\n"
            "#                  shared   %9li kB\n"
            "######################################"
            "######################################\n\n", snapshot, comment,
            (msecs_current - msecs_last), size / 1000, resident / 1000,
            share / 1000);

    // Store current time...
    msecs_last = msecs_current;
}

#endif /* CASSIS_STATISTICS_H_ */
