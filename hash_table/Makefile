#Default Cflags used for compilation
CFLAGS := -Wall -Werror -Wextra -O2 -MMD -g3

# Directory where are located header files
INCLUDE_DIR := include

# Directory where are located some other necessary headers file
INCLUDE_PATH_HEADER := ..

# Variable that will store flags command to include headers
INCLUDES := -I$(INCLUDE_DIR) -I$(INCLUDE_PATH_HEADER)

# Directory where are source files
SRC_DIR := src

# All source files
SRC := $(shell find $(SRC_DIR) -name '*.c')

# Directory where are object directory
OBJ_DIR := objs

# All object files
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# The dependency files that will be used in order to add header dependencies
DEPEND = $(OBJS:.o=.d)

# Directory where will the builded library will be stored
LIB_FOLDER := lib

# Library name
LIB_NAME := libhash_table.a

# Library path
LIB := $(LIB_FOLDER)/$(LIB_NAME)

all : $(LIB)

$(LIB) : $(OBJS)
		@mkdir -p lib
		ar rcs $@ $^

# Rule to generate all object file and create OBJ_DIR if not exist
$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | $(OBJ_DIR)
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Header dependencies. Adds the rules in the .d files, if they exists, in order to
# add headers as dependencies of obj files (see .d files).
# This rules will be merged with the previous rules.
-include $(DEPEND)

$(OBJ_DIR): ; @mkdir -p $@

# Removes all the build directories (objs, deps), executable and library and recreate them
.PHONY : re
re : fclean $(LIB)

# Removes all the build directories (objs, deps), executable and library
.PHONY : fclean
fclean : clean
		rm -rf $(LIB_FOLDER) $(TEST_OBJ_DIR) $(TARGET)

# Removes the obj directory
.PHONY : clean
clean :
		rm -rf $(OBJ_DIR)
