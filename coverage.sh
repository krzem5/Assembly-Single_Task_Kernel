sudo apt install -y nasm gcc qemu lcov gcovr
rm -rf build
./build.sh --coverage --run
cat build/coverage_info.gcda|gcov-tool merge-stream
lcov -c -d build/objects -o build/coverage.info
