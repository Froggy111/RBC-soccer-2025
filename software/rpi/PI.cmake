# Enable verbose output for debugging
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set the target system details
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Set the path to the compilation toolchain
# WARNING: Change this path to match your toolchain location
set(tools ${CMAKE_CURRENT_LIST_DIR}/cross-pi-gcc-14.2.0-64)

# Set the target architecture
set(CMAKE_LIBRARY_ARCHITECTURE aarch64-linux-gnu)

# Configure flags for linking and compiling
set(common_flags "-fPIC -Wl,-rpath-link,/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE} -L/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}")
# Add static linking of libstdc++ and libgcc
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${common_flags} -static-libstdc++ -static-libgcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${common_flags}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${common_flags}")

# Set the prefix for the compiler binaries
set(BIN_PREFIX ${tools}/bin/aarch64-linux-gnu)
# Set the version for the compiler binaries
# WARNING: Change this version to match your toolchain
set(BIN_VERSION "14.2.0")

# Configure the compiler tools
set(CMAKE_C_COMPILER ${BIN_PREFIX}-gcc-${BIN_VERSION})
set(CMAKE_CXX_COMPILER ${BIN_PREFIX}-g++)
set(CMAKE_Fortran_COMPILER ${BIN_PREFIX}-gfortran-${BIN_VERSION})

# Configure additional compiler tools
set(CMAKE_LINKER ${BIN_PREFIX}-ld-${BIN_VERSION} CACHE STRING "Set the compiler tool LD" FORCE)
set(CMAKE_AR ${BIN_PREFIX}-ar-${BIN_VERSION} CACHE STRING "Set the compiler tool AR" FORCE)
set(CMAKE_NM ${BIN_PREFIX}-nm-${BIN_VERSION} CACHE STRING "Set the compiler tool NM" FORCE)
set(CMAKE_OBJCOPY ${BIN_PREFIX}-objcopy-${BIN_VERSION} CACHE STRING "Set the compiler tool OBJCOPY" FORCE)
set(CMAKE_OBJDUMP ${BIN_PREFIX}-objdump-${BIN_VERSION} CACHE STRING "Set the compiler tool OBJDUMP" FORCE)
set(CMAKE_RANLIB ${BIN_PREFIX}-ranlib-${BIN_VERSION} CACHE STRING "Set the compiler tool RANLIB" FORCE)
set(CMAKE_STRIP ${BIN_PREFIX}-strip-${BIN_VERSION} CACHE STRING "Set the compiler tool STRIP" FORCE)

# Configure the behavior for finding programs, libraries, and include files
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)