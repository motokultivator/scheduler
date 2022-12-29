/******************************************************************

  Author: Aleksandar Rikalo <arikalo@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307, USA.
  The GNU General Public License is contained in the file COPYING.
*/

#include <stdint.h>

#define MAX_STACK_SIZE      (1024 * 1024 * 4)
#define MIN_STACK_SIZE      16384
#define RED_ZONE_STACK_SIZE 65536 // A gap between stacks, not really allocated.

#define MAX_TASKS      1024

#if !defined(NUM_THREADS)
#define NUM_THREADS    1
#endif

#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

#define MUTEX volatile uint8_t
#define INIT(x) (x = 0)
#define LOCK(x) while (0 != __sync_val_compare_and_swap(&x, 0, 1))
#define UNLOCK(x) (x = 0)

typedef void (*entry_ptr)(void*);

int init();
uint64_t task(entry_ptr entry, void *arg);
void run();
void yld();
void cleanup();
