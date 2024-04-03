################################################################################
# These are variables for the GBA toolchain build
# You can add others if you wish to
# ***** Eric Li *****
################################################################################

# TA-TODO: Put your application name here.
# This should be a just a name i.e MyFirstGBAProgram
# No SPACES AFTER THE NAME.
PROGNAME = Sandtris

# TA-TODO: Add the C files you want compiled here (replace extension with .o)
# OFILES should contain all the object files you want to compile into your
# program. However, feel free to comment the first line out and define the object files
# yourself. For example if you have main.c and myLib.c then in the following
# line you would put main.o and myLib.o.
OFILES := $(shell find . -name '*.c' ! -name 'client.c' | sed 's/\.c/\.o/g')
# OFILES = gba.o font.o main.o images/garbage.o

################################################################################
# These are various settings used to make the GBA toolchain work
# DO NOT EDIT BELOW.
################################################################################

.PHONY: all
all: CFLAGS += $(CDEBUG) -I../shared
all: LDFLAGS += $(LDDEBUG)
all: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

include /opt/cs2110-tools/GBAVariables.mak

LDFLAGS += --specs=nosys.specs

# Adjust default compiler warnings and errors
CFLAGS += -Wstrict-prototypes -Wold-style-definition -Werror=vla
CFLAGS += -O2

.PHONY: debug
debug: CFLAGS += $(CDEBUG) -I../shared
debug: LDFLAGS += $(LDDEBUG)
debug: $(PROGNAME).gba
	@echo "[FINISH] Created $(PROGNAME).gba"

$(PROGNAME).gba: clean $(PROGNAME).elf client
	@echo "[LINK] Linking objects together to create $(PROGNAME).gba"
	@$(OBJCOPY) -O binary $(PROGNAME).elf $(PROGNAME).gba

$(PROGNAME).elf: crt0.o $(GCCLIB)/crtbegin.o $(GCCLIB)/crtend.o $(GCCLIB)/crti.o $(GCCLIB)/crtn.o $(OFILES) libc_sbrk.o
	$(CC) -o $(PROGNAME).elf $^ $(LDFLAGS)

.PHONY: client
client: client.c
	@gcc -o client client.c

.PHONY: mgba
mgba: debug
	@./client ${PROGNAME}.gba
	@echo "[EXECUTE] Running emulator mGBA"

.PHONY: gdb
gdb: debug
	@./client ${PROGNAME}.gba gdb
	@sleep 1
	@echo "[EXECUTE] Attempting to connect to GDB server."
	@echo "          Make sure mGBA is already running on the host machine!"
	@gdb-multiarch -ex 'file $(PROGNAME).elf' -ex 'target remote host.docker.internal:2345'

.PHONY: submit
submit: clean
	@rm -f submission.tar.gz
	@tar --exclude="examples" -czvf submission.tar.gz *

.PHONY: maze
maze: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/maze/Maze.gba

.PHONY: mario
mario: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/mario/Mario.gba

.PHONY: pong
pong: client
	@echo "[EXECUTE] Running emulator mGBA"
	@echo "          Please see emulator.log if this fails"
	@./client examples/pong/Pong.gba

.PHONY: clean
clean:
	@echo "[CLEAN] Removing all compiled files"
	# rm -f *.o *.elf *.gba *.log */*.o *.sav */*/*.sav client
