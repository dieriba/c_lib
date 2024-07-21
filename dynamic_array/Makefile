# Directories
SRC_DIR = src
OBJ_DIR = objs
DEPS_DIR = .deps
INCLUDE_DIR = include
ROOT_DIR = ..
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -Wextra -g3 -I./$(INCLUDE_DIR) -I$(ROOT_DIR)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPS_DIR)/$*.d

# Source files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Output library
LIB = libdynamic_array.a

# Dependency files
DEPS = $(patsubst $(SRC_DIR)/%.c, $(DEPS_DIR)/%.d, $(SRC))

.PHONY: all clean

all: $(LIB)

# Create the object directory and dependency directory if they don't exist
$(OBJ_DIR) $(DEPS_DIR):
	@mkdir -p $@

# Compile source files to object files with header dependencies
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEPS_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Create the static library from object files
$(LIB): $(OBJ)
	ar rcs $@ $^

# Include the dependency files
-include $(DEPS)

clean:
	rm -f $(OBJ) $(LIB)
	rm -rf $(DEPS_DIR)
	rm -rf $(OBJ_DIR)