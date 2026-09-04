CC ?= cc
CFLAGS ?= -std=c17 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -O2
CPPFLAGS = -Iinclude
SRC = src/main.c src/config.c src/monitor.c src/process.c src/registry.c src/logger.c src/controller.c
OBJ = $(SRC:.c=.o)

all: autonicer demo/cpu_hog

autonicer: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@

demo/cpu_hog: demo/cpu_hog.c
	$(CC) $(CFLAGS) $< -o $@

test: autonicer
	./tests/run_tests.sh

desktop-install:
	$(MAKE) autonicer
	npm install --prefix desktop

desktop-build: desktop-install
	npm run build --prefix desktop

desktop-dev: desktop-install
	npm run tauri dev --prefix desktop

clean:
	rm -f autonicer demo/cpu_hog $(OBJ) tests/test_autonicer

.PHONY: all test clean
