

BUILD_DIR := build
BUILD_LIB := $(BUILD_DIR)/lib/d_lib.a

SRC_DIRS := src
SRCS := $(shell find $(SRC_DIRS) -name '*.c')
OBJS_DIR := $(BUILD_DIR)/objs
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

DEPS := $(OBJS:.o=.d)

INC_DIRS := ./includes
INC_DIRS += $(SRC_DIRS)/container

INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP


$(BUILD_LIB): $(OBJS)
	mkdir -p $(dir $@)
	ar rcs $@ $^

$(BUILD_DIR)/%.c.o: %.c
	echo $^
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)