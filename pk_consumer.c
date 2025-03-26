#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define PK_CONSUMER_CHANNEL 3
#define MAX_N 4
#define MAX_N_COLS 4
#define Q 1024
#define S 1
#define PLAINTEXT_MSG 1

// Shared memory symbols (must match .system file)
uintptr_t pk_shared_mem_vaddr;
uintptr_t ct_shared_mem_vaddr;

// PRNG seed
static unsigned int rand_seed = 123;

static int simple_rand() {
    rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return rand_seed;
}

static int randomUniformInt(int s) {
    int range = 2 * s + 1;
    return (simple_rand() % range) - s;
}

static int reduce_mod_q(int value, int q) {
    int r = value % q;
    if (r < 0) r += q;
    return r - (q / 2);
}

// Aᵗ * r
static void matrix_vector_multiply(
    int matrix[MAX_N][MAX_N_COLS],
    int vector[MAX_N],
    int result[MAX_N],
    int q
) {
    for (int j = 0; j < MAX_N_COLS; j++) {
        result[j] = 0;
        for (int i = 0; i < MAX_N; i++) {
            result[j] += matrix[i][j] * vector[i];
        }
        result[j] = reduce_mod_q(result[j], q);
    }
}

void encrypt_and_store_ciphertext() {
    int *shared_mem = (int *)pk_shared_mem_vaddr;
    int *ct_mem = (int *)ct_shared_mem_vaddr;

    // Reconstruct full public key: A | b
    int public_key[MAX_N][MAX_N_COLS + 1];
    int offset = 0;
    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS + 1; j++) {
            public_key[i][j] = shared_mem[offset++];
        }
    }

    // Copy A portion of public key
    int A_matrix[MAX_N][MAX_N_COLS];
    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS; j++) {
            A_matrix[i][j] = public_key[i][j];
        }
    }

    // Sample r ∈ {-1, 0, 1}
    int r[MAX_N];
    for (int i = 0; i < MAX_N; i++) {
        r[i] = randomUniformInt(1);
    }

    // Compute c1 = Aᵗ * r
    int c1[MAX_N_COLS];
    matrix_vector_multiply(A_matrix, r, c1, Q);

    // Compute c2 = bᵗ * r + noise + msg*(q/2)
    int c2 = 0;
    for (int i = 0; i < MAX_N; i++) {
        c2 += public_key[i][MAX_N_COLS] * r[i];
    }
    c2 += randomUniformInt(S);
    c2 += PLAINTEXT_MSG * (Q / 2);
    c2 = reduce_mod_q(c2, Q);

    // Write ciphertext to shared memory
    for (int i = 0; i < MAX_N_COLS; i++) {
        ct_mem[i] = c1[i];
    }
    ct_mem[MAX_N_COLS] = c2;

    // Print
    microkit_dbg_puts("PK_CONSUMER: Ciphertext written = ");
    for (int i = 0; i < MAX_N_COLS + 1; i++) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d ", ct_mem[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
    microkit_notify(4);  // notify sk_consumer
}

void init(void) {
    microkit_dbg_puts("PK_CONSUMER: Waiting for public key data...\n");
}

void notified(microkit_channel channel) {
    if (channel == PK_CONSUMER_CHANNEL) {
        encrypt_and_store_ciphertext();
    }
}
