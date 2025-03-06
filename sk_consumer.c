#include <stdint.h>
#include <microkit.h>
#include "printf.h" 

#define SK_CONSUMER_CHANNEL 2
#define MAX_N 4

// Secret key shared memory region (set by system description file)
uintptr_t sk_shared_mem_vaddr;

void read_from_shared_memory() {
    int *shared_mem = (int *)sk_shared_mem_vaddr;

    microkit_dbg_puts("SK_CONSUMER: Secret Key from Shared Memory: ");
    for (int i = 0; i < MAX_N; i++) {
        char buf[4];
        snprintf(buf, sizeof(buf), "%d ", shared_mem[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

void init(void) {
    microkit_dbg_puts("\n");
    microkit_dbg_puts("SK_CONSUMER: Waiting for secret key data...\n");
}

void notified(microkit_channel channel) {
    if (channel == SK_CONSUMER_CHANNEL) {
        read_from_shared_memory();
    }
}
