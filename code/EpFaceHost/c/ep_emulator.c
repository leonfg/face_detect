/* <title of the code in this file>
   Copyright (C) 2012 Adapteva, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program, see the file COPYING.  If not, see
   <http://www.gnu.org/licenses/>. */

/**
 * Emulation routines and data structures which allow compiling and running core code on host.
 * Activated only if DEVICE_EMULATION is defined.
 */

#ifdef DEVICE_EMULATION

#include <opencv/cv.h>

#include "ep_emulator.h"

EpCoreMemory core_memory;
#define BANK1 (core_memory.bank1)
#define BANK2 (core_memory.bank2)
#define BANK3 (core_memory.bank3)

EpDRAMMemory dram_memory;

/**
 * @return pointer to shared memory buffer
 */
static EpDRAMBuf *get_sram_origin() {
    return (EpDRAMBuf*)&dram_memory;
}

/**
 * global time counter
 */
int64 emulated_timer;

/**
 * Start timer.
 * @return start value of timer
 */
static unsigned int start_timer() {
    emulated_timer = cvGetTickCount();
    return ~0;
}

/**
 * Stop timer.
 * @return stop value of timer
 */
static unsigned int stop_timer() {
    double const time = (cvGetTickCount() - emulated_timer) / cvGetTickFrequency();
    return ~((unsigned int)0) - cvRound(time * CORE_FREQUENCY);
}

/**
 * Emulate DMA data transfer.
 * calls memcpy(dst, src, size)
 */
static void dma_transfer (
    void       volatile *const dst,
    void const volatile *const src,
    unsigned int         const size,
    int                  const wait
) {
    memcpy( (void *)dst, (void const *)src, size );
}

/**
 * Increment shared variable
 * @param val pointer on variable for increment
 * @param max_val max value of variable
 * @return unmodified (*val) value
 */
static int atomic_increment(int volatile *const val, int const max_val) {

    int const cur_val = *val;
    if(cur_val < max_val)
        *val = cur_val + 1;

    return cur_val;
}


//Including actual core code
#include "../../EpFaceCore_commonlib/src/device_routines.h"

int e_open(char const *const eServIP, unsigned short const eServPort) {
    return 0;
}

int e_close() {
    return 0;
}

size_t e_read(void volatile const *const from_addr, void volatile *const buf, size_t const count) {
    memcpy( (void *)buf, (void *)from_addr, count );
    return count;
}

size_t e_write(void volatile *const to_addr, const void volatile *const buf, size_t const count) {
    memcpy( (void *)to_addr, (void *)buf, count );
    return count;
}

unsigned int e_coreid_origin(void) {
    return 2084;
}

unsigned int e_get_coreid() {
    return 2084;
}

#endif//DEVICE_EMULATION
