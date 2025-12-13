# vcpkg-clang-mingw.cmake
# Points vcpkg to your LLVM-MinGW installation

# 1. Set the compiler to Clang
set(CMAKE_C_COMPILER C:/LLVM-MinGW/bin/clang.exe)
set(CMAKE_CXX_COMPILER C:/LLVM-MinGW/bin/clang++.exe)

# 2. Add the LLVM-MinGW directory to the PATH for vcpkg's build process
list(APPEND CMAKE_PROGRAM_PATH "C:/LLVM-MinGW/bin")

# 3. Use the LLVM linker (lld) and archiver (llvm-ar)
set(CMAKE_LINKER C:/LLVM-MinGW/bin/lld.exe)
set(CMAKE_AR C:/LLVM-MinGW/bin/llvm-ar.exe)

# Optional but recommended: Tell vcpkg that the compiler supports C++17
set(VCPKG_CMAKE_SYSTEM_NAME "Windows") 
set(VCPKG_C_FLAGS "-fuse-ld=lld")
set(VCPKG_CXX_FLAGS "-fuse-ld=lld")