/*
This basic server is to demonstate a protection domain (PD) in
its simplest form.  This PD has a channel to another PD, which
we call the client.  The server PD and the client PD are connected
via a channel.  The server PD has a notified() entry point to
output a null-terminated string onto the console.
*/

#include <stdint.h>
#include <microkit.h>
#include "printf.h"  // Use microkit_dbg_puts for debug printing

#define NOTIFY_CHANNEL 1

void init(void) {
    // Server initialization (if needed)
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        microkit_dbg_puts("hello from the server");
    }
}

// Dummy protected procedure (required if pp="true" in the system description)
microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    return microkit_msginfo_new(0, 0);
}
