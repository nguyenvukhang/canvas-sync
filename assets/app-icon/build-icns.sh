#!/usr/bin/env bash

rm -rf win mac win-tmp mac-tmp pp

cp -r export win-tmp
cp -r export mac-tmp

# build for mac
ICNS_DIR=canvas-sync.iconset
mkdir $ICNS_DIR
mv mac-tmp/**/*.png $ICNS_DIR
rm -rf mac-tmp
iconutil -c icns $ICNS_DIR
rm -rf $ICNS_DIR

send() {
  mv win-tmp/**/*${1}x${1}.png pp
  mv pp/* win/icon-${1}.png
}

# build for windows
mkdir win pp
send 16
send 32
send 128
send 256
send 512
cd win
convert *.png appicon.ico
mv appicon.ico ../../..

cd ..
rm -rf win mac win-tmp mac-tmp pp
