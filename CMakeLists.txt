# 1. Boost this to at least 3.28 (3.30+ preferred for C++23/26 std modules)
cmake_minimum_required(VERSION 3.30)

#set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "d0edc3af-4c50-42ea-a356-e2862fe7a444")

# Set compilers BEFORE the project() call
set(CMAKE_C_COMPILER "$ENV{HOME}/gcc-trunk/bin/gcc")
set(CMAKE_CXX_COMPILER "$ENV{HOME}/gcc-trunk/bin/g++")

project(CppJsonMagic VERSION 0.1.0 LANGUAGES CXX)

# 2. Use formal CMake variables for the standard
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 3. Enable standard library modules
#set(CMAKE_CXX_MODULE_STD ON)

# 4. Global compiler flags (ONLY flags here)
add_compile_options(-freflection -Wall -Wextra)

set(CMAKE_BUILD_TYPE Debug)

# 5. Define your executable
add_executable(CppJsonMagic main.cpp
    db_classes.cpp 
    json_magic.cpp
)

target_link_libraries(CppJsonMagic PRIVATE
    pthread
    dl
)