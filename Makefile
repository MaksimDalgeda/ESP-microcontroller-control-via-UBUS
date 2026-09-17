.PHONY: all clean run

CC ?= gcc

CFLAGS ?= -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L

CPPFLAGS := \
	-Iinclude

SRC := $(wildcard src/*.c)

OBJ := $(patsubst src/%.c,build/%.o,$(SRC))

TARGET := build/esp-ubus

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
		$(LDLIBS) \
		-o $@

run: all
	./$(TARGET)

clean:
	rm -rf build