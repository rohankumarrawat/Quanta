cmake_minimum_required(VERSION 3.10)
project(Quanta)

# 1. Standard Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    # 2. Paths
    set(LLVM_PREFIX "/opt/homebrew/opt/llvm")
    set(HOMEBREW_PREFIX "/opt/homebrew") # Standard location on Apple Silicon

    # 3. Add Include and Library paths
    include_directories(${LLVM_PREFIX}/include)

    # Tell linker where to find zstd and other Homebrew libs
    link_directories(${LLVM_PREFIX}/lib ${HOMEBREW_PREFIX}/lib)
    
    set(LLVM_CONFIG_EXE "${LLVM_PREFIX}/bin/llvm-config")
else()
    # Assume llvm-config is in the PATH
    set(LLVM_CONFIG_EXE "llvm-config")
endif()

# 4. Get Compile Flags
execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LLVM_CXXFLAGS}")

if (WIN32)
    # Statically link runtime libraries on Windows so users don't need MinGW DLLs
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

# 5. Define Executable
add_executable(quanta 
    src/main.cpp 
    src/lexer.cpp 
    src/parser.cpp 
    src/codegen.cpp
)

# 6. Get Library List
execute_process(
    COMMAND ${LLVM_CONFIG_EXE} --libs --system-libs --link-static
    OUTPUT_VARIABLE LLVM_LIBS_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Clean up output
string(REPLACE "\n" " " LLVM_LIBS_CLEAN "${LLVM_LIBS_OUTPUT}")
string(REPLACE " " ";" LLVM_LIBS_LIST "${LLVM_LIBS_CLEAN}")

# 7. Link
if (APPLE)
    target_link_libraries(quanta PRIVATE ${LLVM_LIBS_LIST} z ncurses zstd)
elseif (WIN32)
    target_link_libraries(quanta PRIVATE ${LLVM_LIBS_LIST} z zstd)
else()
    target_link_libraries(quanta PRIVATE ${LLVM_LIBS_LIST} z ncurses zstd)
endif()