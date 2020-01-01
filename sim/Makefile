# Basic UART-only simulator for Amethyst, using simavr
# (https://github.com/buserror/simavr)
# Starts a picocom session hooked up to USART1 on the simulated AVR, allowing
# interaction via serial. Works well with the `forth_serial` app.
PROGRAM_NAME = amsim
SRC_OBJ = uart_pty.o sim.o

OBJDIR = build
INCLUDE_DIRS = /usr/local/include/simavr ./include
LIB_DIRS =
RPATH =
LIBS = elf simavr

OPTIMIZE_FLAGS += -Os
DEBUG_FLAGS += -g3 -fno-omit-frame-pointer
CPPFLAGS += -DDEBUG=1

CFLAGS_COMMON = $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS) -Wall -Wno-missing-braces -MMD -MP
CFLAGS += $(CFLAGS_COMMON) -std=c11
CXXFLAGS += $(CFLAGS_COMMON) -std=c++17

CPPFLAGS += $(foreach includedir,$(INCLUDE_DIRS),-I$(includedir))

LDFLAGS += $(foreach libdir,$(LIB_DIRS),-L$(libdir))
LDFLAGS += $(foreach lib,$(LIBS),-l$(lib))
LDFLAGS += $(RPATH)
LDFLAGS += $(DEBUG_FLAGS)

OBJ = $(addprefix $(OBJDIR)/,$(SRC_OBJ))
DEPS = $(OBJ:.o=.d)

.PHONY: all clean

all: $(PROGRAM_NAME)

$(PROGRAM_NAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(PROGRAM_NAME)

$(OBJ): | $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -f $(PROGRAM_NAME)
	rm -rf $(OBJDIR)
	rm -rf *.dSYM

-include $(DEPS)



