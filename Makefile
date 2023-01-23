main:
	@make --no-print-directory build

build: FORCE
	cmake -S . -B build
	mv ./build/compile_commands.json .
	cmake --build build --target all

FORCE: ;
