#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define DECRYPTION_CHANNEL 4
#define MAX_N 4
#define MAX_N_COLS 4
#define Q 1024

// Shared memory pointers (patched in from LWE.system)
uintptr_t sk_shared_mem_vaddr;
uintptr_t ct_shared_mem_vaddr;

// Center value modulo q into [-q/2, q/2)
static int reduce_mod_q(int value, int q) {
    int r = value % q;
    if (r < 0) r += q;
    return r - (q / 2);
}

int decrypt(int *ciphertext, int *secret_key, int N, int q) {
    // compute inner product of secret key and c1
    int inner_product = 0;
    for (int i = 0; i < N; i++) {
        inner_product += secret_key[i] * ciphertext[i];
    }

    // subtract from c2
    int difference = ciphertext[N] - inner_product;

    // center mod q
    difference = reduce_mod_q(difference, q);

    // threshold decision
    if (difference > q / 4 || difference < -q / 4) {
        return 1;
    } else {
        return 0;
    }
}

void decrypt_from_shared_memory() {
    int *sk_mem = (int *)sk_shared_mem_vaddr;
    int *ct_mem = (int *)ct_shared_mem_vaddr;

    int secret_key[MAX_N];
    int ciphertext[MAX_N_COLS + 1];

    // copy from shared memory
    for (int i = 0; i < MAX_N; i++) {
        secret_key[i] = sk_mem[i];
    }

    for (int i = 0; i < MAX_N_COLS + 1; i++) {
        ciphertext[i] = ct_mem[i];
    }

    // perform decryption
    int result = decrypt(ciphertext, secret_key, MAX_N, Q);

    // print result
    microkit_dbg_puts("DECRYPTION: Message = ");
    if (result == 1) {
        microkit_dbg_puts("1\n");
    } else {
        microkit_dbg_puts("0\n");
    }
}

void init(void) {
    microkit_dbg_puts("DECRYPTION: Waiting for ciphertext...\n");
}

void notified(microkit_channel channel) {
    if (channel == DECRYPTION_CHANNEL) {
        decrypt_from_shared_memory();
    }
}
