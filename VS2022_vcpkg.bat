rmdir /s /q cmake_vs 2>nul
cmake.exe -S. -Bcmake_vs -G"Visual Studio 18 2026" -DwxWidgets_ROOT_DIR="D:/software/wxWidgets" -DXLIGHTS_USE_VCPKG=ON -DCMAKE_TOOLCHAIN_FILE="D:/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows-static-md
pause
