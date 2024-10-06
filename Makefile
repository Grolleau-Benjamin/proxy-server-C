CC = gcc
CFLAGS_RELEASE = -Wall -Wextra -Iincludes
CFLAGS_DEBUG = -Wall -Wextra -Iincludes -DDEBUG -g

SRCS = main.c src/utils.c src/server.c src/http_helper.c src/logger.c src/rules.c src/config.c

RELEASE_OBJS = obj/main.o obj/utils.o obj/server.o obj/http_helper.o obj/logger.o obj/rules.o obj/config.o

DEBUG_OBJS = obj/main_debug.o obj/utils_debug.o obj/server_debug.o obj/http_helper_debug.o obj/logger_debug.o obj/rules_debug.o obj/config_debug.o

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
obj/server.o: src/server.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/http_helper.o: src/http_helper.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/logger.o: src/logger.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/rules.o: src/rules.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@
obj/config.o: src/config.c | obj
	$(CC) $(CFLAGS_RELEASE) -c $< -o $@

obj/main_debug.o: main.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/utils_debug.o: src/utils.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/server_debug.o: src/server.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/http_helper_debug.o: src/http_helper.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/logger_debug.o: src/logger.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/rules_debug.o: src/rules.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/config_debug.o: src/config.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj:
	mkdir -p obj

clean:
	rm -f obj/*.o obj/*.o_debug $(TARGET) $(DEBUG_TARGET)
