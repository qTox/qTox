# the name of the target operating system
set(CMAKE_SYSTEM_NAME Windows)

set(COMPILER_PREFIX "x86_64-w64-mingw32")

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc )
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres )
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++ )

set(CMAKE_C_FLAGS " -static -g -std=gnu99 -DAL_LIBTYPE_STATIC")
set(CMAKE_CXX_FLAGS " -static-libgcc -static -O3 -s -std=gnu99 -w -DAL_LIBTYPE_STATIC")
set(CMAKE_STATIC_LINKER_FLAGS "")

set(INCLUDE_DIRECTORIES SYSTEM /usr/share/mingw-w64/include/)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(WIN32 TRUE)
set(UNIX FALSE)
