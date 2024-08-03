#Default Cflags used for compilation
CFLAGS := -Wall -Wextra -MMD -g3

# Directory where are located header files
DSTRING_INCLUDE_DIR := ../../string/includes

# Directory where are located header files
GENERAL_LIB_INCLUDE_DIR := ../../general_lib/include

# Directory where are located header files
DYNAMIC_ARR_INCLUDE_DIR := ../include

# Directory where are located some other necessary headers file
HEADER_ROOT_DIR := ../..

# Directory where are source files
SRC_DIR := src

# All source files
SRC := $(shell find $(SRC_DIR) -name '*.c')

# Directory where are object directory
OBJ_DIR := objs

# All object files
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

# Variable that will store flags command to include headers
INCLUDES := -I$(GENERAL_LIB_INCLUDE_DIR) -I$(HEADER_ROOT_DIR) -I$(DYNAMIC_ARR_INCLUDE_DIR) -I$(DSTRING_INCLUDE_DIR)

# Directory where will the builded library will be stored
LIB_FOLDER := ../lib

# The dependency files that will be used in order to add header dependencies
DEPEND = $(OBJS:.o=.d)

# Library name
LIB_NAME := libdynamic_array.a

# Dynamic Arr Lib
DYNAMIC_ARR_LIB := $(LIB_FOLDER)/$(LIB_NAME)

# General lil
GENERAL_LIB := ../../general_lib/lib/libgeneral_lib.a

# Executable name
TARGET := test

.PHONY: $(TARGET) 
$(TARGET): $(OBJS) $(DYNAMIC_ARR_LIB) $(GENERAL_LIB)
			$(CC) $^ -o $(TARGET)

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c  | $(OBJ_DIR)
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(DYNAMIC_ARR_LIB):
		$(MAKE) -C ..

$(GENERAL_LIB):
		$(MAKE) -C ../../general_lib

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
		rm -rf $(TARGET) $(OBJ_DIR)

# Removes the obj directory
.PHONY : clean
clean :
		rm -rf *.d