# installing qt from scratch

## requirements

git, ninja

## get the repo

[instructions](https://wiki.qt.io/Building_Qt_6_from_Git)
```
git clone https://code.qt.io/qt/qt5.git qt6
git switch v6.4.2
./init-repository --module-subset=qtbase,qtshadertools,qtdeclarative
```

## build

```sh
mkdir qt6-build
cd qt6-build
../qt6/configure -prefix /path/to/install
cmake --build . --parallel 4
cmake --install .
```

in particular, I ran `../qt6/configure -static`

to get static libraries
