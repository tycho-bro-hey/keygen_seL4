#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define ENCRYPTION_CHANNEL 3
#define LWE_OPS_CHANNEL 4
#define MAX_N 4
#define MAX_N_COLS 4
#define Q 1024
#define S 1     
#define T 16      // message space: integers in [-T/2, T/2]

// Shared memory
uintptr_t pk_shared_mem_vaddr;
uintptr_t ct_shared_mem_vaddr;

// Simple PRNG
static int simple_rand() {
    static unsigned int rand_seed = 123;
    rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return rand_seed;
}

// Sample from uniform [-s, s]
static int randomUniformInt(int s) {
    int range = 2 * s + 1;
    return (simple_rand() % range) - s;
}

// Reduce mod q to range [-q/2, q/2)
static int reduce_mod_q(int value, int q) {
    int r = value % q;
    if (r < 0) r += q;        
    if (r >= q / 2) r -= q;    
    return r;
}

// Integer message encryption using public key
static void encrypt_integer_message(
    int public_key[MAX_N][MAX_N_COLS + 1],
    int message,
    int ciphertext[MAX_N_COLS + 1]
) {
    int r[MAX_N];
    int alpha = Q / T;

    // r ∈ {−1, 0, 1}^n
    for (int i = 0; i < MAX_N; i++) {
        r[i] = randomUniformInt(1);
    }

    // c1 = r' * A
    for (int j = 0; j < MAX_N_COLS; j++) {
        ciphertext[j] = 0;
        for (int i = 0; i < MAX_N; i++) {
            ciphertext[j] += public_key[i][j] * r[i];
        }
        ciphertext[j] = reduce_mod_q(ciphertext[j], Q);
    }

    // c2 = r' * b + m * alpha + e
    int c2 = 0;
    for (int i = 0; i < MAX_N; i++) {
        c2 += public_key[i][MAX_N_COLS] * r[i];
    }
    c2 += message * alpha;
    c2 += randomUniformInt(S);
    c2 = reduce_mod_q(c2, Q);

    ciphertext[MAX_N_COLS] = c2;
}

// Read public key from shared memory, encrypt message, store ciphertext
void encrypt_and_store_ciphertext() {
    int (*public_key)[MAX_N_COLS + 1] = (int (*)[MAX_N_COLS + 1])pk_shared_mem_vaddr;
    int *ct_mem = (int *)ct_shared_mem_vaddr;
    int ciphertext[MAX_N_COLS + 1];

    // Example message to encrypt
    int message = 2;

    encrypt_integer_message(public_key, message, ciphertext);

    microkit_dbg_puts("ENCRYPTION: Ciphertext = ");
    for (int i = 0; i < MAX_N_COLS + 1; i++) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%d ", ciphertext[i]);
        microkit_dbg_puts(buf);
        ct_mem[i] = ciphertext[i];  // write to shared memory
    }
    microkit_dbg_puts("\n");

    microkit_notify(LWE_OPS_CHANNEL);  // notify decryption PD
}

void init(void) {
    microkit_dbg_puts("ENCRYPTION: Waiting to be notified...\n");
}

void notified(microkit_channel channel) {
    if (channel == ENCRYPTION_CHANNEL) {
        encrypt_and_store_ciphertext();
    }
}
