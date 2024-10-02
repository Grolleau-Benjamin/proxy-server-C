CC = gcc
CFLAGS = -Wall -Wextra -Iincludes
DEBUG_FLAGS = -DDEBUG

SRCS = main.c src/utils.c
OBJS = obj/main.o obj/utils.o

TARGET = proxy

.PHONY: all debug clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

obj/%.o: %.c | obj
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: src/%.c | obj
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o $@

obj:
	mkdir -p obj

debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

clean:
	rm -f $(OBJS) $(TARGET)
