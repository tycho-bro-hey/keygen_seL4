#include <stdint.h>
#include <microkit.h>
#include "printf.h" 

#define CONSUMER_CHANNEL 2
#define MAX_N 4
#define MAX_N_COLS 4

uintptr_t shared_mem_vaddr;

void read_from_shared_memory() {
    int *shared_mem = (int *)shared_mem_vaddr;

    microkit_dbg_puts("CONSUMER: Secret Key from Shared Memory: ");
    for (int i = 0; i < MAX_N; i++) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%d ", shared_mem[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");

    microkit_dbg_puts("CONSUMER: Public Key from Shared Memory:\n");
    int offset = MAX_N;
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
    microkit_dbg_puts("CONSUMER: Waiting for key data...\n");
}


void notified(microkit_channel channel) {
    if (channel == CONSUMER_CHANNEL) {
        read_from_shared_memory();
    }
}
