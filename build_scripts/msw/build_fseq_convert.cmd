cd ..
cd ..
cd fseq_convert

cmake -S. -Bcmake_vs -G"Visual Studio 17 2022"
cmake --build cmake_vs --config Release
