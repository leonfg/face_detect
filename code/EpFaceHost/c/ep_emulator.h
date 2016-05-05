/**
 * Header file for emulation routines and data structures which allow compiling
 * and running core code on host. Activated only if DEVICE_EMULATION is defined.
 */

#ifdef DEVICE_EMULATION

#ifndef EP_EMULATOR_H
#define EP_EMULATOR_H

#include "ep_data_types.h"

typedef struct {
    EpCoreBank1 bank1;
    EpCoreBank2 bank2;
    EpCoreBank3 bank3;
} __attribute__((packed)) EpCoreMemory;

typedef struct {
    EpDRAMBuf common_memory;
} __attribute__((packed)) EpDRAMMemory;

/// Emulated core memory
extern EpCoreMemory core_memory;

/// Emulated shared memory
extern EpDRAMMemory dram_memory;


#ifdef __cplusplus
extern "C" {
#endif

void device_dump_buffers(char const *const file_name);

/**
 * @return 0
 */
int e_open(char const *const eServIP, unsigned short const eServPort);

/**
 * @return 0
 */
int e_close();

/**
 * Calls memcpy(buf, from_addr, count);
 */
size_t e_read(void volatile const *const from_addr, void volatile *const buf, size_t const count);

/**
 * Calls memcpy(to_addr, buf, count);
 */
size_t e_write(void volatile *const to_addr, const void volatile *const buf, size_t const count);

/**
 * @return 2084
 */
unsigned int e_coreid_origin(void);
/**
 * @return 2084
 */
unsigned int e_get_coreid();

/**
 * Process task list on core
 */
void device_process_tasks(void);

#ifdef __cplusplus
}
#endif

#endif//EP_EMULATOR_H

#endif//DEVICE_EMULATION
