#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define DECRYPTION_CHANNEL 5
#define MAX_N 4
#define MAX_N_COLS 4
#define Q 1024
#define T 16  // Same as used in encryption (message space size)

// Shared memory pointers (patched in via LWE.system)
uintptr_t sk_shared_mem_vaddr;
uintptr_t op_shared_mem_vaddr;

// Reduce value mod q into range [-q/2, q/2)
static int reduce_mod_q(int value, int q) {
    int r = value % q;
    if (r < 0) r += q;          // bring to [0, q)
    if (r >= q / 2) r -= q;     // center to [-q/2, q/2)
    return r;
}

// LWE decryption for integer messages
static int decrypt(int *ciphertext, int *secret_key, int N, int t, int q) {
    // Step 1: inner product of secret key and c1
    int inner_product = 0;
    for (int i = 0; i < N; i++) {
        inner_product += secret_key[i] * ciphertext[i];
    }

    // Step 2: difference = c2 - (s · c1)
    int difference = ciphertext[N] - inner_product;

    // Step 3: center mod q
    difference = reduce_mod_q(difference, q);

    // Step 4: scale rounding
    int alpha = q / t;
    int recovered_message = (difference + (alpha / 2)) / alpha;

    return recovered_message;
}

void decrypt_from_shared_memory() {
    int *sk_mem = (int *)sk_shared_mem_vaddr;
    int *ct_mem = (int *)op_shared_mem_vaddr;

    int secret_key[MAX_N];
    int ciphertext[MAX_N_COLS + 1];

    // Copy secret key and ciphertext from shared memory
    for (int i = 0; i < MAX_N; i++) {
        secret_key[i] = sk_mem[i];
    }

    for (int i = 0; i < MAX_N_COLS + 1; i++) {
        ciphertext[i] = ct_mem[i];
    }

    // Decrypt the message
    int result = decrypt(ciphertext, secret_key, MAX_N, T, Q);

    // Print result
    microkit_dbg_puts("DECRYPTION: Recovered message = ");
    char buf[12];
    snprintf(buf, sizeof(buf), "%d\n", result);
    microkit_dbg_puts(buf);
}

void init(void) {
    microkit_dbg_puts("DECRYPTION: Waiting for ciphertext...\n");
}

void notified(microkit_channel channel) {
    if (channel == DECRYPTION_CHANNEL) {
        decrypt_from_shared_memory();
    }
}
