ifneq (,$(OPENSSL_ROOT_DIR))
CMAKE_FLAGS := "-DOPENSSL_ROOT_DIR=$(OPENSSL_ROOT_DIR)"
endif

main:
	@make --no-print-directory build

ci:
	cmake $(CMAKE_FLAGS) \
		-DCMAKE_BUILD_TYPE=Release \
		-S . -B build
	cmake --build build

build: FORCE
	make ci
	mv ./build/compile_commands.json .

inspect:
	otool -L ./build/CanvasSync.app/Contents/MacOS/CanvasSync

open:
	open ./build/CanvasSync.app

docs:
	git clone git://code.qt.io/qt/qt5.git qt6
	./init-repository

static:
	cd ~/Qt/6.4.2/Src && ./configure -commercial -release -static \
			-nomake tests -nomake examples \
			-skip qtwebengine \
			-skip qtimageformats \
      -skip qtlanguageserver \
      -skip qtshadertools \
      -skip qtsvg \
      -skip qtdeclarative \
      -skip qtquicktimeline \
      -skip qtquick3d \
      -skip qtmultimedia \
      -skip qt3d \
      -skip qt5compat \
      -skip qtactiveqt \
      -skip qtcharts \
      -skip qtcoap \
      -skip qtconnectivity \
      -skip qtdatavis3d \
      -skip qtwebsockets \
      -skip qthttpserver \
      -skip qttools \
      -skip qtdoc \
      -skip qtlottie \
      -skip qtmqtt \
      -skip qtnetworkauth \
      -skip qtopcua \
      -skip qtserialport \
      -skip qtpositioning \
      -skip qtquick3dphysics \
      -skip qtremoteobjects \
      -skip qtscxml \
      -skip qtsensors \
      -skip qtserialbus \
      -skip qtspeech \
      -skip qttranslations \
      -skip qtvirtualkeyboard \
      -skip qtwayland \
      -skip qtwebchannel \
      -skip qtwebview

FORCE: ;
