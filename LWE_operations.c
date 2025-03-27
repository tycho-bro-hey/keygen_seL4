#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define LWE_OPS_CHANNEL 4
#define DECRYPTION_CHANNEL 5

#define MAX_N_COLS 4
#define CIPHERTEXT_LEN (MAX_N_COLS + 1)

// Shared memory regions (set by system description)
uintptr_t ct_shared_mem_vaddr;
uintptr_t op_shared_mem_vaddr;

void copy_ciphertext() {
    int *ct_src = (int *)ct_shared_mem_vaddr;
    int *ct_dest = (int *)op_shared_mem_vaddr;

    microkit_dbg_puts("LWE_OPS: Reading ciphertext from ct_shared_mem and copying to op_shared_mem...\n");

    for (int i = 0; i < CIPHERTEXT_LEN; i++) {
        ct_dest[i] = ct_src[i];

        // Debug output
        char buf[16];
        snprintf(buf, sizeof(buf), "%d ", ct_dest[i]);
        microkit_dbg_puts(buf);
    }

    microkit_dbg_puts("\nLWE_OPS: Ciphertext copied. Notifying decryption domain...\n");

    microkit_notify(DECRYPTION_CHANNEL);
}

void init(void) {
    microkit_dbg_puts("LWE_OPS: Ready to receive ciphertext...\n");
}

void notified(microkit_channel channel) {
    if (channel == LWE_OPS_CHANNEL) {
        copy_ciphertext();
    }
}
