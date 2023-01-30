QT_CFG_ARGS := -opensource -release -static -nomake examples -nomake tests -skip qt3d -skip qttranslations -skip qt5compat -skip qtactiveqt -skip qtcharts -skip qtcoap -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc -skip qthttpserver -skip qtlanguageserver -skip qtlottie -skip qtmqtt -skip qtmultimedia -skip qtnetworkauth -skip qtopcua -skip qtpositioning -skip qtquick3d -skip qtquick3dphysics -skip qtquicktimeline -skip qtremoteobjects -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtshadertools -skip qtspeech -skip qtsvg -skip qttools -skip qtwebsockets
QT_STATIC_DIR ?= ~/_Qt_/git/output
QT_SRC_DIR ?= ~/_Qt_/git/src

main:
	@make --no-print-directory build
	mv ./build/compile_commands.json .

build: FORCE
	@echo "-- USING QT_STATIC_DIR: $(QT_STATIC_DIR)"
	cmake -DQT_STATIC_DIR=$(QT_STATIC_DIR) -DCMAKE_BUILD_TYPE=Release \
		-S . -B build
	cmake --build build # alternatively: cd build && make

inspect:
	otool -L ./build/CanvasSync.app/Contents/MacOS/CanvasSync

open:
	open ./build/CanvasSync.app

get-qt:
	mkdir -p $(QT_SRC_DIR)
	@if [ ! -f $(QT_SRC_DIR)/configure ]; then \
		git clone https://code.qt.io/qt/qt5.git -b v$(QT_VERSION) $(QT_SRC_DIR); \
		cd $(QT_SRC_DIR) && ./init-repository -f --module-subset=qtbase; \
	fi

config-qt:
	cd $(QT_SRC_DIR) && ./configure $(QT_CFG_ARGS) -prefix $(QT_STATIC_DIR)

clean-qt:
	cd $(QT_SRC_DIR) && ninja clean

build-qt:
	cd $(QT_SRC_DIR) && ninja -j6 && ninja install

FORCE: ;
