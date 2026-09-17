.PHONY: all clean run

CC ?= gcc

STAGING_DIR ?= $(STAGING_DIR)

CFLAGS ?= -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L

CPPFLAGS := \
	-Iinclude \
	-I$(STAGING_DIR)/usr/include

SRC := $(wildcard src/*.c)

OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

TARGET := build/esp-ubus

LDFLAGS := \
	-L$(STAGING_DIR)/usr/lib

LDLIBS := \
	-lubus \
	-lubox \
	-lblobmsg_json \
	-lserialport \
	-lpthread

all: $(TARGET)

build/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) \
		$(LDFLAGS) \
		$(LDLIBS) \
		-o $@

run: all
	LD_LIBRARY_PATH=$(STAGING_DIR)/usr/lib:$$LD_LIBRARY_PATH \
	./$(TARGET)

clean:
	rm -rf build