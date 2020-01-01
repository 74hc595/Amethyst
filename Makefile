# If 1, flash the device over USB. The Optiboot bootloader must already be
#   programmed onto the chip. No special programming device is required (just a
#   USB cable) but it is slow.
# If 0, flash the device using an AVR-ISP programming device. Much faster.
#   This will erase the bootloader if one has been programmed.
OPTIBOOT ?= 0
USB_SERIAL_DEVICE ?= $(firstword $(wildcard /dev/cu.usbserial-*))

# Override to build for a specific hardware revision (see include/defs.h)
HW_VERSION ?= AMETHYST_HW_LATEST

# List of "apps" to include in the ROM image.
# The first item in the list will be started at boot.
# The default is "launcher".
APPS = launcher forth_serial forth plasma slideshow \
  test_bitmap_modes test_colors test_keys	test_mode_alignment test_patterns \
	test_raster_interrupts test_text_modes

# ---------- Shouldn't have to touch anything below this point ----------
DEVICE  = atmega1284
F_CPU   = 14318182	# 4x colorburst
FUSE_L  = 0xBF
FUSE_H  = 0xD1
FUSE_E  = 0xFF

# Change this line to suit your programmer if you aren't using an AVRISP mkII
AVRDUDE_ISP = avrdude -c avrispmkII -P usb -p $(DEVICE)

ifeq ($(OPTIBOOT),1)
	AVRDUDE = avrdude -c arduino -p $(DEVICE) -P $(USB_SERIAL_DEVICE) -b 57600
else
	AVRDUDE = $(AVRDUDE_ISP)
endif

OUT  = colorvideo

STARTUP_APP = $(firstword $(APPS))

APP_DIRS = $(addprefix apps/,$(APPS))
SRC_DIRS = system assets $(APP_DIRS)

BUILD_DIRS   = $(addprefix build/,$(SRC_DIRS))
INCLUDE_DIRS = include

SRC = $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.c))
ASM = $(foreach sdir,$(SRC_DIRS),$(wildcard $(sdir)/*.S))
OBJ = $(patsubst %.c,build/%.o,$(SRC)) $(patsubst %.S,build/%.o,$(ASM))
DEPS = $(OBJ:.o=.d)
vpath %.c $(SRC_DIRS)
vpath %.S $(SRC_DIRS)

CC      = avr-gcc
OBJDUMP = avr-objdump
OBJCOPY = avr-objcopy
SIZE    = avr-size
GDB     = avr-gdb

SIMAVR  = ./sim/amsim

CFLAGS  = -g -Os -std=gnu11 -Wundef -Wall -Wextra -Wno-unused-parameter -Wno-missing-braces -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fdata-sections -ffunction-sections -mrelax -Wa,--no-pad-sections -MMD -MP -DF_CPU=$(F_CPU) -DSTARTUP_APP=$(STARTUP_APP) -DHW_VERSION=$(HW_VERSION) -include isr_reserved_registers.h
LDFLAGS = -g -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 -Wl,-Map,$(OUT).map -Wl,--gc-sections -Wl,--relax -Wl,@banned_stdlib_functions -T atmega1284_amethyst.ld
COMPILE = $(CC) $(CFLAGS) -mmcu=$(DEVICE)

CFLAGS += $(addprefix -I,$(INCLUDE_DIRS))

LD_SCRIPT_FRAGMENTS = build/app_overlays.ld

.PHONY: all checkdirs clean flash flash-bootloader fuse getty getty-stop program sim

all: checkdirs $(OUT).hex

program: flash fuse

checkdirs: $(BUILD_DIRS)

$(BUILD_DIRS):
	mkdir -p $@

build/app_overlays.ld:
	./scripts/generate-app-linker-script-overlay.sh .noinit $(APPS) > $@

stackusage.txt: $(OUT).elf
	cd build
	find . -name "*.su" -exec cat {} \; | sort -n -k2 > ./stackusage.txt

$(OUT).elf: $(OBJ) $(LD_SCRIPT_FRAGMENTS)
	$(COMPILE) $(LDFLAGS) -o $@ $(filter %.o,$^)
	$(OBJDUMP) -D $(OUT).elf > $(OUT).lst
	$(SIZE) $@

$(OUT).hex: $(OUT).elf
	rm -f $(OUT).hex $(OUT).eep.hex
	$(OBJCOPY) -j .text -j .data -j .app_*_data -O ihex $(OUT).elf $(OUT).hex


clean:
	rm -f $(OUT).hex $(OUT).lst $(OUT).obj $(OUT).cof $(OUT).list $(OUT).map $(OUT).eep.hex $(OUT).elf stackusage.txt
	rm -rf build

fuse:
	@[ "$(FUSE_H)" != "" -a "$(FUSE_L)" != "" ] || \
		{ echo "*** Edit Makefile and choose values for FUSE_L and FUSE_H!"; exit 1; }
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m -U efuse:w:$(FUSE_E):m

flash: $(OUT).hex
	$(AVRDUDE) -U flash:w:$(OUT).hex:i

# Burn bootloader (requires an AVR-ISP programming device)
flash-bootloader:
	# override bootloader bits in hfuse
	$(eval FUSE_H=$(shell printf '0x%X\n' "$$(( $(FUSE_H) & 0xF8 | 0x06 ))"))
	$(AVRDUDE_ISP) -e -u -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m -U efuse:w:$(FUSE_E):m -U lock:w:0xff:m
	$(AVRDUDE_ISP) -U flash:w:bootloader/optiboot_amethyst.hex -U lock:w:0xcf:m

sim: $(OUT).hex
	$(MAKE) -C sim
	$(SIMAVR) $(OUT).hex

debugsim:
	$(GDB) --eval-command="target remote :1234" $(OUT).elf

# macOS: start a serial console at boot, allowing you to use the Amethyst as
# a serial terminal.
# (See http://www.club.cc.cmu.edu/~mdille3/doc/mac_osx_serial_console.html for
# information on creating the "serialconsole.plist" file.
getty:
	launchctl bootstrap system /Library/LaunchDaemons/serialconsole.plist

getty-stop:
	launchctl bootout system /Library/LaunchDaemons/serialconsole.plist

# --------------------
define make-goal
$1/%.o: %.S
	$(COMPILE) -x assembler-with-cpp -c $$< -o $$@

$1/%.o: %.c
	$(COMPILE) -c $$< -o $$@
endef
# --------------------

$(foreach bdir,$(BUILD_DIRS),$(eval $(call make-goal,$(bdir))))
-include $(DEPS)
