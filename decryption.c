#include <stdint.h>
#include <microkit.h>
#include "printf.h" 

#define SK_CONSUMER_CHANNEL 4
#define MAX_N_COLS 4

// Shared memory variables (set via keygen.system)
uintptr_t sk_shared_mem_vaddr;
uintptr_t ct_shared_mem_vaddr; 

void read_ciphertext_from_shared_memory() {
    int *ct_mem = (int *)ct_shared_mem_vaddr;

    microkit_dbg_puts("SK_CONSUMER: Ciphertext received = ");
    for (int i = 0; i < MAX_N_COLS + 1; i++) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d ", ct_mem[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

void init(void) {
    microkit_dbg_puts("\n");
    microkit_dbg_puts("SK_CONSUMER: Waiting for secret key and ciphertext...\n");
}

void notified(microkit_channel channel) {
    if (channel == SK_CONSUMER_CHANNEL) {
        read_ciphertext_from_shared_memory();
    }
}
