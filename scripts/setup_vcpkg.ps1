if (-Not (Test-Path "./vcpkg")) {
    git clone https://github.com/microsoft/vcpkg.git
}

cd vcpkg
.\bootstrap-vcpkg.bat
cd ..
.\vcpkg\vcpkg.exe install
