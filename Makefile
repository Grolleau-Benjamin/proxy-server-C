CC = gcc
CFLAGS = -Wall -Wextra -Iincludes
CFLAGS_DEBUG = -DDEBUG -g

SRCS = main.c src/server.c src/http_helper.c src/logger.c src/rules.c src/config.c src/server_helper.c src/dns_helper.c

OBJS = $(SRCS:src/%.c=obj/%.o)
OBJS := $(OBJS:main.c=obj/main.o)  

DEBUG_OBJS = $(OBJS:.o=_debug.o)

DEBUG_SRCS = $(filter-out main.c, $(SRCS))
OBJS_TEST = $(DEBUG_SRCS:src/%.c=obj/%.o)

TARGET = proxy
DEBUG_TARGET = proxy_debug

TEST_SRCS = test/test_http_helper.c test/test_server.c test/test_logger.c test/test_config.c test/test_rules.c test/test_server_helper.c test/test_dns_helper.c
TEST_OBJS = $(TEST_SRCS:test/test_%.c=obj/test_%.o)
TEST_TARGETS = $(TEST_SRCS:test/%.c=test/%)

.PHONY: all debug clean test docs

all: $(TARGET)
debug: $(DEBUG_TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(DEBUG_TARGET): $(DEBUG_OBJS) obj/main_debug.o
	$(CC) $(CFLAGS_DEBUG) -o $@ $^

obj/%.o: src/%.c | obj
	$(CC) $(CFLAGS) -c $< -o $@
obj/main.o: main.c | obj
	$(CC) $(CFLAGS) -c $< -o $@

obj/%_debug.o: src/%.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@
obj/main_debug.o: main.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

obj/%.o: test/%.c | obj
	$(CC) $(CFLAGS_DEBUG) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -f obj/*.o obj/*_debug.o $(TARGET) $(DEBUG_TARGET) $(TEST_TARGETS)
	rm -rf docs
	echo > logs/proxy.log

docs:
	mkdir -p docs
	rm -rf docs/*
	doxygen
	command -v firefox >/dev/null 2>&1 && firefox docs/html/index.html >/dev/null 2>&1


$(TEST_TARGETS): test/%: obj/%.o $(OBJS_TEST)
	$(CC) $(CFLAGS_DEBUG) -o $@ $^

test: $(TEST_TARGETS)
	@for test in $(TEST_TARGETS); do \
		echo "Running $$test..."; \
		./$$test; \
	done
