rmdir /s /q cmake_vs 2>nul
cmake.exe -S. -Bcmake_vs -G"Visual Studio 17 2022" -DwxWidgets_ROOT_DIR="C:/software/wxWidgets" -DXLIGHTS_USE_VCPKG=ON -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
pause
