#include <stdint.h>
#include <microkit.h>
#include "printf.h"  // Use microkit_dbg_puts for debug printing

#define NOTIFY_CHANNEL 1

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

void init(void) {
    // Server initialization (if needed)
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        // Generate and print 5 random numbers using randomUniformInt()
        microkit_dbg_puts("Random numbers: ");
        for (int i = 0; i < 5; i++) {
            char buf[16];
            int random_value = randomUniformInt(5);  // Example: Generate numbers in [-5, 5]
            snprintf(buf, sizeof(buf), "%d ", random_value);
            microkit_dbg_puts(buf);
        }
        microkit_dbg_puts("\n");
    }
}

// Dummy protected procedure (required if pp="true" in the system description)
microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    return microkit_msginfo_new(0, 0);
}
