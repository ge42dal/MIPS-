cmake_minimum_required(VERSION 3.10)
project(MIPS_Interpreter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add compiler flags for debugging and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g")

# include directories
include_directories(src)

# core library sources
set(CORE_SOURCES
    src/mips_core.cpp
    src/cpu_instructions.cpp
    src/assembler.cpp
    src/debugger.cpp
)

# create static library for the core functionality
add_library(mips_core STATIC ${CORE_SOURCES})

# create executables
add_executable(mips-assemble src/mips_assemble.cpp)
target_link_libraries(mips-assemble mips_core)

add_executable(mips-interpret src/mips_interpret.cpp)
target_link_libraries(mips-interpret mips_core)

add_executable(mips-execute src/mips_execute.cpp)
target_link_libraries(mips-execute mips_core)

# create debugger executable
add_executable(mips-debug src/debug_main.cpp)
target_link_libraries(mips-debug mips_core)

# create a test executable
add_executable(mips-test src/main.cpp)
target_link_libraries(mips-test mips_core)
