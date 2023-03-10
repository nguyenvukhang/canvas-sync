MAKEFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MAKEFILE_DIR  := $(dir $(MAKEFILE_PATH))

# QT_STATIC_DIR ?= $(MAKEFILE_DIR)/Qt/static
QT_STATIC_DIR ?= $(HOME)/Qt/6.2.4/macos
QT_SRC_DIR ?= $(MAKEFILE_DIR)/Qt/src

QT_CFG_ARGS := -opensource -release -static -nomake examples -nomake tests -skip qt3d -skip qttranslations -skip qt5compat -skip qtactiveqt -skip qtcharts -skip qtcoap -skip qtconnectivity -skip qtdatavis3d -skip qtdeclarative -skip qtdoc -skip qthttpserver -skip qtlanguageserver -skip qtlottie -skip qtmqtt -skip qtmultimedia -skip qtnetworkauth -skip qtopcua -skip qtpositioning -skip qtquick3d -skip qtquick3dphysics -skip qtquicktimeline -skip qtremoteobjects -skip qtscxml -skip qtsensors -skip qtserialbus -skip qtserialport -skip qtshadertools -skip qtspeech -skip qtsvg -skip qttools -skip qtwebsockets
QT_INIT_ARGS := --module-subset=qtbase
QT_VERSION ?= 6.2.4

# current: FORCE
# 	make main
# 	make test

install: FORCE
	make main
	rm -rf '/Applications/Canvas Sync.app'
	cp -a './build/Canvas Sync.app' '/Applications/Canvas Sync.app'

main:
	@make --no-print-directory build
	mv ./build/compile_commands.json .

test: FORCE
	$(MAKEFILE_DIR)/build/test/CanvasSyncTest

build: FORCE
	@echo "-- USING QT_STATIC_DIR: $(QT_STATIC_DIR)"
	cmake -DQT_STATIC_DIR=$(QT_STATIC_DIR) -DCMAKE_BUILD_TYPE=Release \
		-S . -B build
	cmake --build build --parallel # alternatively: cd build && make

build-win: FORCE
	cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -G Ninja
	cmake --build build

qt-setup:
	make clone-qt
	make init-qt

clone-qt:
	git clone https://code.qt.io/qt/qt5.git -b v$(QT_VERSION) $(QT_SRC_DIR)

init-qt:
	cd $(QT_SRC_DIR) && perl init-repository $(QT_INIT_ARGS)

config-qt:
	cd $(QT_SRC_DIR) && ./configure -prefix $(QT_STATIC_DIR) $(QT_CFG_ARGS)

clean-qt:
	cd $(QT_SRC_DIR) && ninja clean

build-qt:
	cd $(QT_SRC_DIR) && ninja -j6 && ninja install

open:
	open 'build/Canvas Sync.app'

reset:
	rm -rf ~/files/test/*/*

clean:
	rm -rf $(MAKEFILE_DIR)/.cache
	rm -rf $(MAKEFILE_DIR)/build

FORCE: ;
