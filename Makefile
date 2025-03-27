ifndef MICROKIT_SDK
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

# Common objects (e.g. printf support)
PRINTF_OBJS := printf.o util.o

# Server (PD_KeyGen) objects, client (PD_requeste)
# and consumer (PD_consumer) objects
KEYGEN_OBJS := $(PRINTF_OBJS) keygen.o
ENCRYPTION_OBJS := $(PRINTF_OBJS) encryption.o
DECRYPTION_OBJS := $(PRINTF_OBJS) decryption.o
LWE_OPS_OBJS := $(PRINTF_OBJS) LWE_operations.o

BOARD_DIR := $(MICROKIT_SDK)/board/$(BOARD)/$(MICROKIT_CONFIG)

# IMAGES for protection domains
IMAGES := keygen.elf client.elf encryption.elf decryption.elf LWE_operations.elf
# The system description file for this key generation example is named keygen.system.
SYSTEM_FILE := LWE.system

CFLAGS := -mcpu=$(CPU) -mstrict-align -nostdlib -ffreestanding -g -Wall -Wno-unused-function \
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

$(BUILD_DIR)/keygen.elf: $(addprefix $(BUILD_DIR)/, $(KEYGEN_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/client.elf: $(addprefix $(BUILD_DIR)/, $(CLIENT_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/encryption.elf: $(addprefix $(BUILD_DIR)/, $(ENCRYPTION_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/decryption.elf: $(addprefix $(BUILD_DIR)/, $(DECRYPTION_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(BUILD_DIR)/LWE_operations.elf: $(addprefix $(BUILD_DIR)/, $(LWE_OPS_OBJS))
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

$(IMAGE_FILE): $(addprefix $(BUILD_DIR)/, $(IMAGES)) $(SYSTEM_FILE)
	$(MICROKIT_TOOL) $(SYSTEM_FILE) --search-path $(BUILD_DIR) --board $(BOARD) \
		--config $(MICROKIT_CONFIG) -o $(IMAGE_FILE) -r $(REPORT_FILE)
