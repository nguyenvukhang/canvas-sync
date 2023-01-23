build: FORCE
	cmake -S . -B build
	mv ./build/compile_commands.json .
	QMAKE_EXECUTABLE=/Users/khang/Qt/6.4.2/macos/bin/qmake \
		cmake --build build --target all

FORCE: ;
