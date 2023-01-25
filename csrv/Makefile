MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR  := $(dir $(MAKEFILE_PATH))

CMAKE_PRG ?= $(shell (command -v cmake3 || echo cmake))
CMAKE_BUILD_TYPE ?= Release # as opposed to Debug
CMAKE_FLAGS := -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

include ../.env

develop:
	@make --no-print-directory build
	@make run_tests
	@make save_config
	@cd debug && make build
	@echo '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~'
	@CANVAS_TOKEN=$(CANVAS_TOKEN) ./debug/build/canvas_debug

build: FORCE
	# configure the build
	@$(CMAKE_PRG) \
		-B $(MAKEFILE_DIR)/build -S $(MAKEFILE_DIR) $(CMAKE_FLAGS)
	# run the build
	@$(CMAKE_PRG) --build $(MAKEFILE_DIR)/build

# save the build command for LSP use
save_config:
	@mv $(MAKEFILE_DIR)/build/compile_commands.json .

# run the tests binary
run_tests:
	@cd $(MAKEFILE_DIR)/build && GTEST_COLOR=1 ctest --output-on-failure

FORCE: ;
