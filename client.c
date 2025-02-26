/*
This basic client is to demonstate a protection domain (PD) in
its simplest form.  This PD has a channel to another PD, which
we call the server.  The client PD and the server PD are connected
via a channel.  The client PD outputs a null-terminated string 
onto the console using the init() entry point before using the
notify() asynchronous notification to the server PD.
*/

#include <stdint.h>
#include <microkit.h>

#define NOTIFY_CHANNEL 1

void init(void) {
    // Immediately request the server to print "hello"
    microkit_dbg_puts("INITIATING KEY GENERATION...");
    microkit_dbg_puts("\n");
    microkit_dbg_puts("NOTIFYING SERVER...");
    microkit_notify(NOTIFY_CHANNEL);
}

void notified(microkit_channel channel) {
    // No notifications expected on the client.
}
