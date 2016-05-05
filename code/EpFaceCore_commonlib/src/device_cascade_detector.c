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

/*
 * These functions will run on the Epiphany device
 */
/*
#include <e_common.h>
#include <e_coreid.h>
#include <e_mutex.h>
#include <e_ctimers.h>
#include <e_dma.h>
*/
#include "e_lib.h"
#include "../../EpFaceHost/c/ep_data_types.h"

//EpCoreBank1 BANK1 SECTION(".text_bank1");
#define BANK1 0x2000


//Almost two memory banks are used to store image data:
//EpCoreBank2 BANK2 SECTION(".text_bank2"); //Second part of image data

//Part of last memory bank is for classifier:
//EpCoreBank3 BANK3 SECTION(".text_bank3"); //It is supposed that stack is less than 512 bytes!
#define BANK3 0x6000
//(EpCoreBank3 *)BANK3

//e_mutex_t mutex;


/**
 * @return global pointer to mutex in first core
 */

/**
 * Get pointer on begin of Shared memory structure.
 * @return pointer on origin of SRAM
 */
static EpDRAMBuf volatile *get_sram_origin() {
	return (EpDRAMBuf*)0x8f000000;
}

/**
 * Start timer.
 * Reset timer on max value and start it.
 * @return value of timer on start.
 */
static unsigned int start_timer() {

    e_ctimer_stop(E_CTIMER_0); //ToDo: remove this
    //e_ctimer_set(E_CTIMER_0, E_CTIMER_CLK, E_CTIMER_MAX);
    e_ctimer_set(E_CTIMER_0, E_CTIMER_MAX);
    e_ctimer_start(E_CTIMER_0, E_CTIMER_CLK);
    return e_ctimer_get(E_CTIMER_0);

	//return 0;
}

/**
 * Stop timer.
 * @return current value of timer.
 */
static unsigned int stop_timer() {
    e_ctimer_stop(E_CTIMER_0);
    return e_ctimer_get(E_CTIMER_0);
    //return 0;
}

/**
 * Copy memory buffer using DMA.
 * @param dst : pointer to destination memory location.
 * @param src : pointer to source memory location.
 * @param size: size of memory block in BYTES. Must be non-zero.
 * @param wait: flag of waiting (if non-zero then wait until transfer is finished).
 */
static void dma_transfer (
    void       volatile *const dst,
    void const volatile *const src,
    unsigned int         const size,
    int                  const wait
) {
#if 1
    if(wait) {

        /*
         * Function e_dma_busy(E_DMA_0) is unreliable for waiting for data transfer to actually finish
         * since last bytes may still be on the way from src to dst when DMA became free.
         * So, instead we test here for the last byte to be actually transferred.
         */
        int const last_index = size - 1;
        char const last_source_byte = *( (char const volatile *)src + last_index );
        char volatile *const last_dst_byte_ptr = (char volatile *)dst + last_index;
        *last_dst_byte_ptr = ~last_source_byte;

    //    e_dma_copy(E_DMA_0, (void *)dst, (void *)src, size, E_ALIGN_AUTO);
		e_dma_copy((void *)dst, (void *)src, size);
        while(*last_dst_byte_ptr != last_source_byte);

    } else {

     //   e_dma_copy(E_DMA_0, (void *)dst, (void *)src, size, E_ALIGN_AUTO);
		e_dma_copy((void *)dst, (void *)src, size);
    }
#endif
    //e_dma_copy((void *)dst, (void *)src, size);
    //memcpy(dst, src, size);
    return;
}

/**
 * Increment shared variable
 * @param val pointer on variable for increment
 * @param max_val max value of variable
 * @return unmodified (*val) value
 */
static int atomic_increment(int volatile *const val, int const max_val) {
    e_mutex_t *mutex_p = (void *)0x00004000;

    e_mutex_lock(0, 0, mutex_p);
    int const cur_val = *val;
    if(cur_val < max_val)
        *val = cur_val + 1;
    e_mutex_unlock(0, 0, mutex_p);

    return cur_val;
}

/**
 * Decrement shared variable
 * @param val pointer on variable for decrement
 * @param min_val min value of variable
 * @return unmodified (*val) value
 */
static int atomic_decrement(int volatile * const val, int const min_val) {
    e_mutex_t *mutex_p = (void *)0x00004000;

    e_mutex_lock(0, 0, mutex_p);
    int const cur_val = *val;
    if(cur_val > min_val)
        *val = cur_val - 1;
    e_mutex_unlock(0, 0, mutex_p);

    return cur_val;
}

#include "device_routines.h"

int mc_core_common_go()
{
#if 1
    unsigned row = 0;
    unsigned col = 0;
    e_coreid_t coreid = 0;
    e_mutex_t *mutex = NULL;
    // Must be initialized ONLY on one core
    mutex = (int *)0x00004000;
    coreid = e_get_coreid();
    e_coords_from_coreid(coreid, &row, &col);
    if ( (0==row) && (0==col) )
    {
        e_mutex_init(0, 0, mutex, NULL);
    }
#endif
    ((EpCoreBank1 *)BANK1)->timer.core_id = e_get_coreid();

    while (1) {
        while(atomic_decrement(&get_sram_origin()->control_info.start_cores, 0) == 0);
        device_process_tasks();
    }

    return 0;
}
    
int main(void)
{
	int status;

	status = 0;

	/* jump to multicore common code */
	mc_core_common_go();
#if 0
       while(1){
      get_sram_origin()->control_info.unused=1;
      //((EpDRAMBuf*)0x8f000000)->control_info.unused=1;
       }
#endif
	return status;
}

