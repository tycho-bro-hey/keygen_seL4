#include <stdint.h>
#include <microkit.h>
#include "printf.h" 

#define PK_CONSUMER_CHANNEL 3
#define MAX_N 4
#define MAX_N_COLS 4

// Public key shared memory region (set by system description file)
uintptr_t pk_shared_mem_vaddr;

void read_from_shared_memory() {
    int *shared_mem = (int *)pk_shared_mem_vaddr;

    microkit_dbg_puts("PK_CONSUMER: Public Key from Shared Memory:\n");
    int offset = 0;
    for (int i = 0; i < MAX_N; i++) {
        char buf[64] = {0};
        int pos = 0;
        for (int j = 0; j < MAX_N_COLS + 1; j++) {
            pos += snprintf(buf + pos, sizeof(buf) - pos, "%3d ", shared_mem[offset++]);
        }
        microkit_dbg_puts(buf);
        microkit_dbg_puts("\n");
    }
}

void init(void) {
    microkit_dbg_puts("\n");
    microkit_dbg_puts("PK_CONSUMER: Waiting for public key data...\n");
}

void notified(microkit_channel channel) {
    if (channel == PK_CONSUMER_CHANNEL) {
        read_from_shared_memory();
    }
}
