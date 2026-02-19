#Default Cflags used for compilation
CFLAGS := -Wall -Wextra -MMD -g3

# Directory where are located dynamic_array header files
DSTRING_INCLUDE_DIR := ../string/includes

# Directory where are located dynamic_array header files
DYNAMIC_ARR_INCLUDE_DIR := ../dynamic_array/include

# Directory where are located header files
INCLUDE_DIR := include

# Directory where are located some other necessary headers file
HEADER_ROOT_DIR := ..

# Variable that will store flags command to include headers
INCLUDES := -I$(INCLUDE_DIR) -I$(HEADER_ROOT_DIR) -I$(DYNAMIC_ARR_INCLUDE_DIR) -I$(DSTRING_INCLUDE_DIR)

OBJ_DIR := objs

SRCS_DIRS := src ../dynamic_array/src ../string/src

SRCS := $(wildcard src/*.c) $(wildcard ../dynamic_array/src/*.c) $(wildcard ../string/src/*.c)
OBJS := $(addprefix $(OBJ_DIR)/,$(addsuffix .o,$(notdir $(basename $(SRCS)))))
VPATH := $(SRCS_DIRS)
# The dependency files that will be used in order to add header dependencies
DEPEND = $(OBJS:.o=.d)

# Directory where will the builded library will be stored
LIB_FOLDER := lib

# Library name
LIB_NAME := libgeneral_lib.a

# Library path
LIB := $(LIB_FOLDER)/$(LIB_NAME)

all : $(LIB)

$(LIB) : $(OBJS)
		@mkdir -p lib
		ar rcs $@ $^

# Rule to generate all object file and create OBJ_DIR if not exist
$(OBJ_DIR)/%.o : %.c | $(OBJ_DIR)
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
		rm -rf $(LIB_FOLDER) $(OBJ_DIR)

# Removes the obj directory
.PHONY : clean
clean :
		rm -rf $(OBJ_DIR)
