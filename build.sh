# configure
# cmake -DCMAKE_INSTALL_PREFIX=$PWD/install -S . -B ./build
mkdir -p build
cmake --build build --parallel `nproc`
