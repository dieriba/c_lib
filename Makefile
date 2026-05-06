TARGET_DIR := target

BUILD_DIR := $(TARGET_DIR)/build
BUILD_LIB := $(BUILD_DIR)/lib/d_lib.a

SRC_DIRS := src
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

DEPS := $(OBJS:.o=.d)

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


.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)