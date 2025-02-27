#include <stdint.h>
#include <microkit.h>
#include "printf.h"  

#define NOTIFY_CHANNEL 1
#define CONSUMER_CHANNEL 2
#define MAX_N 4  
#define MAX_N_COLS 4
#define MOD_Q_VALUE 1024

// Shared memory region (set by system description file)
uintptr_t shared_mem_vaddr;

// Seed for the random number generator
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

static void generate_lattice(int lattice[MAX_N][MAX_N_COLS], int n, int N, int q) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < N; j++) {
            lattice[i][j] = mod_q(q);
        }
    }
}

static void generate_sk(int secret_key[MAX_N], int N) {
    for (int i = 0; i < N; i++) {
        secret_key[i] = simple_rand() % 2;  
    }
}

static void matrix_vector_multiply(int matrix[MAX_N][MAX_N_COLS], int n, int N, int vector[MAX_N], int result[MAX_N]) {
    for (int i = 0; i < n; i++) {
        result[i] = 0;
        for (int j = 0; j < N; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
}

static void generate_pk(int lattice[MAX_N][MAX_N_COLS], int n, int N, int secret_key[MAX_N], int uniform_vector[MAX_N], int public_key[MAX_N][MAX_N_COLS + 1]) {
    int mv_result[MAX_N];
    matrix_vector_multiply(lattice, n, N, secret_key, mv_result);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < N; j++) {
            public_key[i][j] = lattice[i][j];
        }
        public_key[i][N] = mv_result[i] + uniform_vector[i];  
    }
}

// Writes the keys to shared memory
static void write_to_shared_memory(int secret_key[MAX_N], int public_key[MAX_N][MAX_N_COLS + 1]) {
    int *shared_mem = (int *)shared_mem_vaddr;

    for (int i = 0; i < MAX_N; i++) {
        shared_mem[i] = secret_key[i];
    }

    int offset = MAX_N;
    for (int i = 0; i < MAX_N; i++) {
        for (int j = 0; j < MAX_N_COLS + 1; j++) {
            shared_mem[offset++] = public_key[i][j];
        }
    }
}

void init(void) {
    microkit_dbg_puts("KEY GENERATION SERVER: Ready to receive notifications...\n");
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        int lattice[MAX_N][MAX_N_COLS];
        int secret_key[MAX_N];
        int error_vector[MAX_N];
        int public_key[MAX_N][MAX_N_COLS + 1];

        generate_lattice(lattice, MAX_N, MAX_N_COLS, MOD_Q_VALUE);
        generate_sk(secret_key, MAX_N);
        generate_pk(lattice, MAX_N, MAX_N_COLS, secret_key, error_vector, public_key);

        write_to_shared_memory(secret_key, public_key);

        // Notify the consumer PD that the keys are ready
        microkit_notify(CONSUMER_CHANNEL);
    }
}
