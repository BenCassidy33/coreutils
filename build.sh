set -xe

mkdir -p build
gcc -o build/ls ls.c 
build/ls .
