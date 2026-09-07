HOST_CC ?= clang
.PHONY: test build
test:
	mkdir -p build
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined test/scene.c src/c/scene.c -o build/scene-test
	./build/scene-test
build: test
	pebble build --sdk 4.33.1
