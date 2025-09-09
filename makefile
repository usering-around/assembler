CC := gcc
CFLAGS := -Wall -Wextra -ansi -pedantic -Iinclude
SRC_DIR := src
OBJ_DIR := obj
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# link all the object files together
assembler: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# create object directory if not present
$(OBJ_DIR):
	mkdir -p $@

# compile all the source files to the object directory
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f -r $(OBJ_DIR) assembler

# debug build to use with gdb or any other debugger
.PHONY: dbg
dbg: CFLAGS += -g
dbg: assembler

# optimized build 
.PHONY: opt
opt: CFLAGS += -O3 -flto
opt: assembler


