#!/bin/bash
set -e

if [ ! -d "./vcpkg" ]; then
  git clone https://github.com/microsoft/vcpkg.git
fi

cd vcpkg
./bootstrap-vcpkg.sh
cd ..
./vcpkg/vcpkg install
