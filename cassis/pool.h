/*!
 * pThread pool
 *
 * This file is part of the
 * Comprehensive and Sensitive Signature Search (CaSSiS) library.
 *
 * Copyright (C) 2011,2012
 *     Kai Christian Bader <mail@kaibader.de>
 *     Christian Grothoff <christian@grothoff.org>
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

/*!
 * Returns the number of (real) processors or, if defined, the number
 * from the 'PTHREADS_NUM_THREADS' environment variable.
 */
int num_processors();

/*!
 * See pthread_create.
 */
typedef void *(*WorkFun)(void *arg);

/*!
 * Schedule the given work function to be run from the pool.
 */
void pool_run(WorkFun fun, void *arg);

/*!
 * Wait for all work given to the pool to complete.
 */
void pool_barrier();

/*!
 * Initialize pool.
 * If num_threads= 0, the number of processors is used.
 */
void pool_init(int num_threads = 0);

/*!
 * Destroy pool.
 */
void pool_shutdown();
