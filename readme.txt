conan install .. --output-folder=../conan --build=missing --profile=release -o boost/*:shared=False
cmake .. -DCMAKE_TOOLCHAIN_FILE=../conan/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=release && makej

clear && cmake .. -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=../conan_provider.cmake -DCMAKE_BUILD_TYPE=debug && makej