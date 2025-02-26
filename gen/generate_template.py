#!/usr/bin/env python3
import os

# Define file templates
client_c = r'''#include <stdint.h>
#include <microkit.h>

#define NOTIFY_CHANNEL 1

void init(void) {
    microkit_notify(NOTIFY_CHANNEL);
}

void notified(microkit_channel channel) {
    // PD_consume does nothing on notification in this simple example.
}
'''

server_c = r'''#include <stdint.h>
#include <microkit.h>
#include "printf.h"

#define NOTIFY_CHANNEL 1

void init(void) {
    // Server initialization (if needed)
}

void notified(microkit_channel channel) {
    if (channel == NOTIFY_CHANNEL) {
        microkit_dbg_puts("hello");
    }
}

microkit_msginfo protected(microkit_channel channel, microkit_msginfo msginfo) {
    return microkit_msginfo_new(0, 0);
}
'''

keygen_system = r'''<?xml version="1.0" encoding="UTF-8"?>
<system>
    <!-- Server: PD_KeyGen -->
    <protection_domain name="server" priority="254" pp="true">
        <program_image path="server.elf" />
    </protection_domain>

    <!-- Client: PD_consume -->
    <protection_domain name="client" priority="253" pp="false">
        <program_image path="client.elf" />
    </protection_domain>

    <!-- Channel connecting the client and server -->
    <channel>
        <end pd="client" id="1" />
        <end pd="server" id="1" />
    </channel>
</system>
'''

makefile_template = r'''ifndef MICROKIT_SDK
	MICROKIT_SDK := ../microkit-sdk-1.4.1
endif

ifndef TOOLCHAIN
	TOOLCHAIN_AARCH64_LINUX_GNU := $(shell command -v aarch64-linux-gnu-gcc 2> /dev/null)
	TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU := $(shell command -v aarch64-unknown-linux-gnu-gcc 2> /dev/null)
	ifdef TOOLCHAIN_AARCH64_LINUX_GNU
		TOOLCHAIN := aarch64-linux-gnu
	else ifdef TOOLCHAIN_AARCH64_UNKNOWN_LINUX_GNU
		TOOLCHAIN := aarch64-unknown-linux-gnu
	else
		$(error "Could not find an AArch64 cross-compiler")
	endif
endif

BOARD := qemu_virt_aarch64
MICROKIT_CONFIG := debug
BUILD_DIR := build
CPU := cortex-a53

CC := $(TOOLCHAIN)-gcc
LD := $(TOOLCHAIN)-ld
AS := $(TOOLCHAIN)-as
MICROKIT_TOOL ?= $(MICROKIT_SDK)/bin/microkit

PRINTF_OBJS := printf.o util.o

SERVER_OBJS := $(PRINTF_OBJS) server.o
CLIENT_OBJS := $(PRINTF_OBJS) client.o

BOARD_DIR := $(MICROKIT_SDK)/board/$(BOARD)/$(MICROKIT_CONFIG)

IMAGES := server.elf client.elf
SYSTEM_FILE := keygen.system

CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g -Wall \
		  -I$(BOARD_DIR)/include -Iinclude -DBOARD_$(BOARD)
LDFLAGS := -L$(BOARD_DIR)/lib
LIBS := -lmicrokit -Tmicrokit.ld

IMAGE_FILE := $(BUILD_DIR)/loader.img
REPORT_FILE := $(BUILD_DIR)/report.txt

all: directories $(IMAGE_FILE)

directories:
	@mkdir -p $(BUILD_DIR)

run: $(IMAGE_FILE)
	qemu-system-aarch64 -machine virt,virtualization=on \
		-cpu $(CPU) \
		-serial mon:stdio \
		-device loader,file=$(IMAGE_FILE),addr=0x70000000,cpu-num=0 \
		-m size=2G -nographic

$(BUILD_DIR)/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/server.elf: $(addprefix $(BUILD_DIR)/, $(SERVER_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/client.elf: $(addprefix $(BUILD_DIR)/, $(CLIENT_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE): $(addprefix $(BUILD_DIR)/, $(IMAGES)) $(SYSTEM_FILE)
	$(MICROKIT_TOOL) $(SYSTEM_FILE) --search-path $(BUILD_DIR) --board $(BOARD) \
		--config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)
'''

# Function to write file if it doesn't already exist (or always write if desired)
def write_file(filename, content):
    with open(filename, 'w') as f:
        f.write(content)
    print(f"Created {filename}")

# Write out the files
write_file("client.c", client_c)
write_file("server.c", server_c)
write_file("keygen.system", keygen_system)
write_file("Makefile", makefile_template)
