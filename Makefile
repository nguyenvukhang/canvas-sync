ifneq (,$(OPENSSL_ROOT_DIR))
CMAKE_FLAGS := "-DOPENSSL_ROOT_DIR=$(OPENSSL_ROOT_DIR)"
endif

main:
	make --no-print-directory build

ci:
	cmake $(CMAKE_FLAGS) -S . -B build
	cmake --build build

build: FORCE
	make ci
	mv ./build/compile_commands.json .

FORCE: ;
