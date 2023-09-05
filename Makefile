# CC = gcc
# CFLAGS =
# LDFLAGS = -pthread

# SRC = ex3.c
# OBJ = $(SRC:.c=.o)

# OUTPUT = ex3.out

# all: $(OUTPUT)

# $(OUTPUT): $(OBJ)
# 	$(CC) $(LDFLAGS) $^ -o $@

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJ) $(OUTPUT)



# Compiler options
CC := gcc
CFLAGS := -w -pthread

# Directories
SRC_DIR := .
OBJ_DIR := obj
BOUNDED_BUFFER_OBJ_DIR := $(OBJ_DIR)/BoundedQueue

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS += $(wildcard $(SRC_DIR)/BoundedBuffer/*.c)
SRCS += $(wildcard $(SRC_DIR)/CoEditor/*.c)
SRCS += $(wildcard $(SRC_DIR)/Producer/*.c)
SRCS += $(wildcard $(SRC_DIR)/UnboundedBuffer/*.c)
SRCS += $(wildcard $(SRC_DIR)/Dispatcher/*.c)
SRCS += $(wildcard $(SRC_DIR)/ScreenManager/*.c)

OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: a.out

# Rule to compile object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Rule to link the executable
a.out: $(OBJS)
	@$(CC) $(CFLAGS) $^ -o $@
	@rm -rf $(OBJ_DIR)

# Target to run the executable with conf.txt as argument
run: a.out
	@./a.out conf.txt

# Cleanup
clean:
	@rm -f a.out
	@rm -rf $(OBJ_DIR)

.PHONY: all run clean