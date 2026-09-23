.PHONY: all clean run

CC ?= gcc

CFLAGS ?= \
	-Wall \
	-Wextra \
	-g \
	-std=c11 \
	-D_POSIX_C_SOURCE=200809L \

CPPFLAGS := \
	-I. \
	-Icommon/include \
	-Idevices/include \
	-Iesp/include \
	-Iubus/include

SRC := \
	main.c \
	$(wildcard common/src/*.c) \
	$(wildcard devices/src/*.c) \
	$(wildcard esp/src/*.c) \
	$(wildcard ubus/src/*.c)

OBJ := $(patsubst %.c,build/%.o,$(SRC))

TARGET := build/esp-ubus

LDLIBS := \
	-lubus \
	-lubox \
	-lblobmsg_json \
	-lserialport \
	-lpthread

all: $(TARGET)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(OBJ) -o $@ $(LDLIBS)

run: all
	./$(TARGET)

clean:
	rm -rf build