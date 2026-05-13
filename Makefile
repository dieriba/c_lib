include config.mk



SRC_DIRS := src
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))


TEST_DIRS = tests
SRCS_TEST := $(shell find $(TEST_DIRS) -name '*.c')
OBJS_TESTS := $(addprefix $(TARGET_DIR)/, $(SRCS_TEST:.c=.o))
BIN_TESTS := $(OBJS_TESTS:.o=)

DEPS := $(OBJS:.o=.d) $(OBJS_TESTS:.o=.d)

INC_DIRS := ./includes
INC_DIRS += $(shell find $(SRC_DIRS) -type d)

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP -Wall -Werror -Wextra

all: $(LIB_PATH)
.PHONY: all

$(LIB_PATH): $(OBJS)
	mkdir -p $(dir $@)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests: $(BIN_TESTS) 
.PHONY: tests

$(BIN_TESTS): %: %.o $(LIB_PATH)
	mkdir -p $(dir $@)
	$(CC) $< $(LIB_PATH) -o $@

$(TARGET_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) -c $< -o $@

clean:
	rm -rf $(TARGET_DIR)
.PHONY: clean

re: clean all
.PHONY: re

-include $(DEPS)