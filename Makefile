CC = gcc
CFLAGS_RELEASE = -Wall -Wextra -Iincludes
CFLAGS_DEBUG = -Wall -Wextra -Iincludes -DDEBUG -g

SRCS = main.c src/utils.c

RELEASE_OBJS = obj/main.o obj/utils.o

DEBUG_OBJS = obj/main_debug.o obj/utils_debug.o

TARGET = proxy
DEBUG_TARGET = proxy_debug

.PHONY: all debug clean

all: $(TARGET)
debug: $(DEBUG_TARGET)

$(TARGET): $(RELEASE_OBJS)
	$(CC) $(CFLAGS_RELEASE) -o $@ $^

$(DEBUG_TARGET): $(DEBUG_OBJS)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^

obj/main.o: main.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/utils.o: src/utils.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/main_debug.o: main.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/utils_debug.o: src/utils.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj:
	mkdir -p obj

clean:
	rm -f obj/*.o obj/*.o_debug $(TARGET) $(DEBUG_TARGET)
