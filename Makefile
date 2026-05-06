TARGET_DIR := target

BUILD_DIR := $(TARGET_DIR)/build
LIB_NAME := libd_lib.a
BUILD_LIB := $(BUILD_DIR)/lib/$(LIB_NAME)

SRC_DIRS := src
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))


TEST_DIRS = tests
SRCS_TEST := $(shell find $(TEST_DIRS) -name '*.c')
OBJS_TESTS := $(addprefix $(TARGET_DIR)/, $(SRCS_TEST:.c=.o))
BIN_TESTS := $(OBJS_TESTS:.o=)



DEPS := $(OBJS:.o=.d) $(OBJS_TESTS:.o=.d)

INC_DIRS := ./includes
INC_DIRS += $(SRC_DIRS)/container

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP


$(BUILD_LIB): $(OBJS)
	mkdir -p $(dir $@)
	ar rcs $@ $^

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests: $(BUILD_LIB) $(BIN_TESTS) 
.PHONY: tests

$(BIN_TESTS): %: %.o
	mkdir -p $(dir $@)
	$(CC) $< $(BUILD_LIB) -o $@

$(TARGET_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INC_FLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET_DIR)

-include $(DEPS)