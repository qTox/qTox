# atomic builtins are required for threading support.

INCLUDE(CheckCXXSourceCompiles)
INCLUDE(CheckLibraryExists)

# Sometimes linking against libatomic is required for atomic ops, if
# the platform doesn't support lock-free atomics.

function(check_working_cxx_atomics varname)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11")
  CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
std::atomic<int> x;
int main() {
  return x;
}
" ${varname})
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_working_cxx_atomics)

function(check_working_cxx_atomics64 varname)
  set(OLD_CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS})
  set(CMAKE_REQUIRED_FLAGS "-std=c++11 ${CMAKE_REQUIRED_FLAGS}")
  CHECK_CXX_SOURCE_COMPILES("
#include <atomic>
#include <cstdint>
std::atomic<uint64_t> x (0);
int main() {
  uint64_t i = x.load(std::memory_order_relaxed);
  return 0;
}
" ${varname})
  set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQUIRED_FLAGS})
endfunction(check_working_cxx_atomics64)

# Determine if the compiler has GCC-compatible command-line syntax.

if(NOT DEFINED COMPILER_IS_GCC_COMPATIBLE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(COMPILER_IS_GCC_COMPATIBLE ON)
  elseif( MSVC )
    set(COMPILER_IS_GCC_COMPATIBLE OFF)
  elseif( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
    set(COMPILER_IS_GCC_COMPATIBLE ON)
  elseif( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Intel" )
    set(COMPILER_IS_GCC_COMPATIBLE ON)
  else()
    message(STATUS "Warning: Unknown compiler, assume GCC compatible")
    set(COMPILER_IS_GCC_COMPATIBLE ON)
  endif()
endif()

# This isn't necessary on MSVC, so avoid command-line switch annoyance
# by only running on GCC-like hosts.
if (COMPILER_IS_GCC_COMPATIBLE)
  # First check if atomics work without the library.
  check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITHOUT_LIB)
  # If not, check if the library exists, and atomics work with it.
  if(NOT HAVE_CXX_ATOMICS_WITHOUT_LIB)
    check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
    if( HAVE_LIBATOMIC )
      list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
      check_working_cxx_atomics(HAVE_CXX_ATOMICS_WITH_LIB)
      if (NOT HAVE_CXX_ATOMICS_WITH_LIB)
        message(FATAL_ERROR "Host compiler must support std::atomic!")
      endif()
    else()
      message(FATAL_ERROR "Host compiler appears to require libatomic, but cannot find it.")
    endif()
  endif()
endif()

# Check for 64 bit atomic operations.
if(MSVC)
  set(HAVE_CXX_ATOMICS64_WITHOUT_LIB True)
else()
  check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITHOUT_LIB)
endif()

# If not, check if the library exists, and atomics work with it.
if(NOT HAVE_CXX_ATOMICS64_WITHOUT_LIB)
  check_library_exists(atomic __atomic_load_8 "" HAVE_CXX_LIBATOMICS64)
  if(HAVE_CXX_LIBATOMICS64)
    list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
    check_working_cxx_atomics64(HAVE_CXX_ATOMICS64_WITH_LIB)
    if (NOT HAVE_CXX_ATOMICS64_WITH_LIB)
      message(FATAL_ERROR "Host compiler must support std::atomic!")
    endif()
  else()
    message(FATAL_ERROR "Host compiler appears to require libatomic, but cannot find it.")
  endif()
endif()
