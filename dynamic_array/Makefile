SHELL = /bin/sh


CFLAGS := -Wall -Werror -Wextra -O2 -MMD
INCLUDE_DIR := include
INCLUDE_PATH_HEADER := ..
INCLUDES := -I$(INCLUDE_DIR) -I$(INCLUDE_PATH_HEADER)

SRC_DIR := src
SRC := $(shell find $(SRC_DIR) -name '*.c')

OBJ_DIR := objs
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))

DEPS_OBJS := $(patsubst $(OBJ_DIR)/%.o, $(OBJ_DIR)/%.d, $(OBJS))

LIB_FOLDER := lib
LIB_NAME := libdynamic_array.a
LIB := $(LIB_FOLDER)/$(LIB_NAME)

TEST_DIR := test
TEST_SRC := $(shell find $(TEST_DIR) -name 'test.c')
TEST_OBJ_DIR := test/objs
TEST_OBJS := $(patsubst $(TEST_DIR)/test.c, $(TEST_OBJ_DIR)/test.o, $(TEST_SRC))
TARGET = test/test


.PHONY : clean fclean test re all

all : $(LIB)

$(LIB) : $(OBJS)
		@mkdir -p lib
		ar rcs $@ $^

-include: $(DEPS_OBJS)

$(OBJS) : $(SRC)
		@mkdir -p $(OBJ_DIR)
		cc $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TARGET)

$(TARGET):   $(TEST_OBJS) $(LIB)
		cc $< -L $(LIB_FOLDER) -ldynamic_array -o $(TARGET)


$(TEST_OBJS): $(TEST_SRC)
		@mkdir -p $(TEST_OBJ_DIR)
		cc $(CFLAGS) $(INCLUDES) -c $< -o $@

re : $(LIB)

fclean : clean
		rm -rf $(LIB_FOLDER) $(TEST_OBJ_DIR) $(TARGET)
clean :
		rm -rf $(OBJ_DIR)
