#include <stdint.h>
#include <microkit.h>
#include "printf.h"  // Use microkit_dbg_puts for debug printing

#define NOTIFY_CHANNEL 1
#define MAX_N 4  // Define fixed matrix/vector size
#define MAX_N_COLS 4
#define MOD_Q_VALUE 1024

// Seed for the random number generator
static unsigned int rand_seed = 42;  // Fixed seed for reproducibility

// Simple pseudo-random number generator (Linear Congruential Generator - LCG)
static int simple_rand() {
    rand_seed = (rand_seed * 1103515245 + 12345) & 0x7FFFFFFF; // LCG formula
    return rand_seed;
}

// Function to generate a random integer uniformly sampled from [-s, s]
static int randomUniformInt(int s) {
    int range = 2 * s + 1;
    return (simple_rand() % range) - s;
}

// Function to generate a random integer modulo q, centered at 0
static int mod_q(int q) {
    int r = simple_rand() % q;
    if (2 * r >= q) {
        r -= q;
    }
    return r;
}

// Generates an n x N lattice and prints it
static void generate_lattice(int lattice[MAX_N][MAX_N_COLS], int n, int N, int q) {
    microkit_dbg_puts("Generated lattice:\n");
    for (int i = 0; i < n; i++) {
        char buf[128] = {0};  // Increase buffer size to fit all elements
        int offset = 0;       // Keeps track of the current buffer position
        for (int j = 0; j < N; j++) {
            lattice[i][j] = mod_q(q);
            offset += snprintf(buf + offset, sizeof(buf) - offset, "%7d ", lattice[i][j]);
        }
        microkit_dbg_puts(buf);
        microkit_dbg_puts("\n");
    }
}


// Generates a secret key vector of size N, prints it
static void generate_sk(int secret_key[MAX_N], int N) {
    microkit_dbg_puts("Secret Key: ");
    for (int i = 0; i < N; i++) {
        secret_key[i] = simple_rand() % 2;  // Generates either 0 or 1
        char buf[4];
        snprintf(buf, sizeof(buf), "%d ", secret_key[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

// Performs matrix-vector multiplication and prints the result
static void matrix_vector_multiply(int matrix[MAX_N][MAX_N_COLS], int n, int N, int vector[MAX_N], int result[MAX_N]) {
    microkit_dbg_puts("Matrix-Vector Multiplication Result:\n");
    for (int i = 0; i < n; i++) {
        result[i] = 0;
        for (int j = 0; j < N; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
        char buf[16];
        snprintf(buf, sizeof(buf), "%d ", result[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

// Generates a public key matrix by concatenating the matrix-vector multiplication result
static void generate_pk(int lattice[MAX_N][MAX_N_COLS], int n, int N, int secret_key[MAX_N], int uniform_vector[MAX_N]) {
    int mv_result[MAX_N];  // Stores matrix-vector multiplication results
    matrix_vector_multiply(lattice, n, N, secret_key, mv_result);

    microkit_dbg_puts("Public Key Matrix:\n");
    for (int i = 0; i < n; i++) {
        char buf[128] = {0};  // Increase buffer size
        int offset = 0;
        for (int j = 0; j < N; j++) {
            offset += snprintf(buf + offset, sizeof(buf) - offset, "%7d ", lattice[i][j]);
        }
        // Append the extra column (result + uniform_vector)
        int pk_value = mv_result[i] + uniform_vector[i];
        snprintf(buf + offset, sizeof(buf) - offset, "%7d", pk_value);
        microkit_dbg_puts(buf);
        microkit_dbg_puts("\n");
    }
}


// Samples an error vector and prints it
static void error_sampling(int error_vector[MAX_N], int n) {
    microkit_dbg_puts("Error Vector: ");
    for (int i = 0; i < n; i++) {
        error_vector[i] = randomUniformInt(10);  // Example range: [-10,10]
        char buf[4];
        snprintf(buf, sizeof(buf), "%d ", error_vector[i]);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

// Function that generates and prints 5 random numbers
static void generate_random_numbers() {
    microkit_dbg_puts("Random numbers: ");
    for (int i = 0; i < 5; i++) {
        char buf[16];
        int random_value = randomUniformInt(5);  // Example: Generate numbers in [-5, 5]
        snprintf(buf, sizeof(buf), "%d ", random_value);
        microkit_dbg_puts(buf);
    }
    microkit_dbg_puts("\n");
}

void init(void) {
    // Server initialization (if needed)
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        generate_random_numbers();

        // Declare fixed-size lattice, secret key, and uniform vector
        int lattice[MAX_N][MAX_N_COLS];
        int secret_key[MAX_N];
        int error_vector[MAX_N];

        // Generate and print lattice, secret key, and uniform vector
        generate_lattice(lattice, MAX_N, MAX_N_COLS, MOD_Q_VALUE);
        generate_sk(secret_key, MAX_N);
        error_sampling(error_vector, MAX_N);

        // Generate and print public key
        generate_pk(lattice, MAX_N, MAX_N_COLS, secret_key, error_vector);
    }
}

// Dummy protected procedure (required if pp="true" in the system description)
microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    return microkit_msginfo_new(0, 0);
}
