QT_VERSION := 6.4.2

QT_SRC_DIR ?= ~/_Qt_/git/src
QT_OUTPUT_DIR ?= $(QT_SRC_DIR)/../output

ifneq (,$(OPENSSL_ROOT_DIR))
CMAKE_FLAGS := -DOPENSSL_ROOT_DIR=$(OPENSSL_ROOT_DIR)
endif

ifneq (,$(QT_OUTPUT_DIR))
CMAKE_FLAGS := $(CMAKE_FLAGS) -DQT_OUTPUT_DIR=$(QT_OUTPUT_DIR)
endif

QT_CONFIG_FLAGS := -opensource -release -static \
		-nomake examples \
		-skip qt3d \
		-skip qt5compat \
		-skip qtactiveqt \
		-skip qtcharts \
		-skip qtcoap \
		-skip qtconnectivity \
		-skip qtdatavis3d \
		-skip qtdeclarative \
		-skip qtdoc \
		-skip qthttpserver \
		-skip qtlanguageserver \
		-skip qtlottie \
		-skip qtmqtt \
		-skip qtmultimedia \
		-skip qtnetworkauth \
		-skip qtopcua \
		-skip qtpositioning \
		-skip qtquick3d \
		-skip qtquick3dphysics \
		-skip qtquicktimeline \
		-skip qtremoteobjects \
		-skip qtscxml \
		-skip qtsensors \
		-skip qtserialbus \
		-skip qtserialport \
		-skip qtshadertools \
		-skip qtspeech \
		-skip qtsvg \
		-skip qttools \
		-skip qtwebsockets

ifeq ($(OS),Windows_NT)
QT_CONFIG_FLAGS += -no-webkit -no-script -no-scripttools
endif

main:
	@make --no-print-directory build
	mv ./build/compile_commands.json .

ci:
	make get-qt
	make build-qt
	make build

build: FORCE
	cmake $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Release \
		-S . -B build
	cmake --build build

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
	cd $(QT_SRC_DIR) && ./configure $(QT_CONFIG_FLAGS) -prefix $(QT_OUTPUT_DIR)
	cd $(QT_SRC_DIR)/qtbase && ./configure $(QTBASE_CONFIG_FLAGS) \
		-nomake examples \
		-nomake tests

clean-qt:
	cd $(QT_SRC_DIR) && ninja clean

build-qt:
	cd $(QT_SRC_DIR) && ninja -j6 && ninja install

FORCE: ;
