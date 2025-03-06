#include <stdint.h>
#include <microkit.h>
#include "printf.h"  

#define NOTIFY_CHANNEL 1
#define SK_CONSUMER_CHANNEL 2
#define PK_CONSUMER_CHANNEL 3

#define MAX_N 4  
#define MAX_N_COLS 4
#define MOD_Q_VALUE 1024

// Shared memory regions (set by system description file)
uintptr_t sk_shared_mem_vaddr;
uintptr_t pk_shared_mem_vaddr;

// Random seed
static unsigned int rand_seed = 42;  

static int simple_rand() {
    rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return rand_seed;
}

static int randomUniformInt(int s) {
    int range = 2 * s + 1;
    return (simple_rand() % range) - s;
}

static int mod_q(int q) {
    int r = simple_rand() % q;
    if (2 * r >= q) {
        r -= q;
    }
    return r;
}

static void generate_lattice(int lattice[MAX_N][MAX_N_COLS], int q) {
    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS; j++) {
            lattice[i][j] = mod_q(q);
        }
    }
}

static void generate_sk(int secret_key[MAX_N]) {
    for (int i = 0; i < MAX_N; i++) {
        secret_key[i] = simple_rand() % 2;  
    }
}

static void matrix_vector_multiply(int matrix[MAX_N][MAX_N_COLS], int vector[MAX_N], int result[MAX_N]) {
    for (int i = 0; i < MAX_N; i++) {
        result[i] = 0;
        for (int j = 0; j < MAX_N_COLS; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
}

static void generate_pk(int lattice[MAX_N][MAX_N_COLS], int secret_key[MAX_N], int uniform_vector[MAX_N], int public_key[MAX_N][MAX_N_COLS + 1]) {
    int mv_result[MAX_N];
    matrix_vector_multiply(lattice, secret_key, mv_result);

    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS; j++) {
            public_key[i][j] = lattice[i][j];
        }
        public_key[i][MAX_N_COLS] = mv_result[i] + uniform_vector[i];  
    }
}

static void write_to_shared_memory(int secret_key[MAX_N], int public_key[MAX_N][MAX_N_COLS + 1]) {
    int *sk_mem = (int *)sk_shared_mem_vaddr;
    int *pk_mem = (int *)pk_shared_mem_vaddr;

    for (int i = 0; i < MAX_N; i++) {
        sk_mem[i] = secret_key[i];
    }

    int offset = 0;
    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS + 1; j++) {
            pk_mem[offset++] = public_key[i][j];
        }
    }
}

void init(void) {
    microkit_dbg_puts("KEY GENERATION: Ready to generate keys...\n");
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        int lattice[MAX_N][MAX_N_COLS];
        int secret_key[MAX_N];
        int error_vector[MAX_N];
        int public_key[MAX_N][MAX_N_COLS + 1];

        generate_lattice(lattice, MOD_Q_VALUE);
        generate_sk(secret_key);
        generate_pk(lattice, secret_key, error_vector, public_key);

        write_to_shared_memory(secret_key, public_key);

        microkit_notify(SK_CONSUMER_CHANNEL);
        microkit_notify(PK_CONSUMER_CHANNEL);
    }
}
