PROGNAME = Sandtris
OFILES := $(shell find . -name '*.c' | sed 's/\.c/\.o/g')

.PHONY: all
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

LINKSCRIPT_DIR = ./cs2110-tools

CROSS ?= arm-none-eabi-
AS := $(CROSS)as
CC := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

# Resolve toolchain paths from the selected compiler so builds work across
# Homebrew/macOS and distro/Linux layouts.
ARMINC := $(shell $(CC) -print-file-name=include)
ARMLIB := $(dir $(shell $(CC) -print-file-name=libc.a))
LIBGCC := $(shell $(CC) -print-libgcc-file-name)

CRTBEGIN := $(shell $(CC) -print-file-name=crtbegin.o)
CRTEND := $(shell $(CC) -print-file-name=crtend.o)
CRTI := $(shell $(CC) -print-file-name=crti.o)
CRTN := $(shell $(CC) -print-file-name=crtn.o)

CRELEASE = -O2
LDRELEASE = -s

MODEL   = -mthumb-interwork -mthumb
CFLAGS  = -Wall -Werror -std=c99 -pedantic -Wextra -fno-common $(MODEL) -mlong-calls -I $(ARMINC) -g
LDFLAGS = -nostartfiles -lc -lgcc -L $(ARMLIB) \
	  -L $(dir $(LIBGCC)) \
	  -T $(LINKSCRIPT_DIR)/arm-gba.ld

crt0.o : $(LINKSCRIPT_DIR)/crt0.s
	@$(AS) $(MODEL) $^ -o crt0.o

LDFLAGS += --specs=nosys.specs
ifneq ("$(wildcard $(ARMLIB)thumb)","")
LDFLAGS += -L $(ARMLIB)thumb
endif

# Adjust default compiler warnings and errors
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Werror=vla
CFLAGS += -O2

$(PROGNAME).gba: clean $(PROGNAME).elf
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(CRTBEGIN) $(CRTEND) $(CRTI) $(CRTN) $(OFILES)
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	rm -f *.o *.elf *.gba *.log */*.o *.sav */*/*.sav client
