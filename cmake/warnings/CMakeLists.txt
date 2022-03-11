project(qtox_warnings)

add_library(${PROJECT_NAME} INTERFACE)
add_library(qtox::warnings ALIAS ${PROJECT_NAME})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
        set(CLANGCL ON)
    else()
        set(CLANG ON)
    endif()
endif()

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-Wweak-vtables COMPILER_SUPPORTS_WARNING_WEAK_VTABLES)
CHECK_CXX_COMPILER_FLAG(-Wdate-time COMPILER_SUPPORTS_WDATE_TIME)

target_compile_options(${PROJECT_NAME} INTERFACE
    $<$<OR:$<BOOL:${CLANG}>,$<CXX_COMPILER_ID:GNU>>:
        -fno-common;
        -fstrict-overflow;
        -ftrapv;
        -pedantic-errors;
        -Wall;
        -Wcast-align;
        -Wdouble-promotion;
        -Wextra;
        -Wformat=2;
        -Wmissing-declarations;
        -Wnon-virtual-dtor;
        -Wnull-dereference;
        -Wold-style-cast;
        -Woverloaded-virtual;
        -Wshadow;
        -Wsign-compare;
        -Wundef;
    >
    $<$<CXX_COMPILER_ID:GNU>:
        -Wduplicated-cond;
        -Wlogical-op;
    >
    $<$<BOOL:${CLANG}>:
        -Wmissing-variable-declarations;
        -Wno-gnu-zero-variadic-macro-arguments; # Required for gtest 1.10.
    >
    $<$<OR:$<BOOL:${CLANGCL}>,$<CXX_COMPILER_ID:MSVC>>:
        /permissive-;
        /W4;
        /w14254; # 'operator': conversion from 'type1:field_bits' to
                 # 'type2:field_bits', possible loss of data
        /w14263; # 'function': member function does not override any base class
                 # virtual member function
        /w14265; # 'classname': class has virtual functions, but destructor is not
                 # virtual instances of this class may not be destructed correctly
        /w14287; # 'operator': unsigned/negative constant mismatch
        /w14289; # nonstandard extension used: 'variable': loop control variable
                 # declared in the for-loop is used outside the for-loop scope
        /w14296; # 'operator': expression is always 'boolean_value'
        /w14311; # 'variable': pointer truncation from 'type1' to 'type2'
        /w14545; # expression before comma evaluates to a function which is missing
                 # an argument list
        /w14546; # function call before comma missing argument list
        /w14547; # 'operator': operator before comma has no effect; expected
                 # operator with side-effect
        /w14549; # 'operator': operator before comma has no effect; did you intend
                 # 'operator'?
        /w14555; # expression has no effect; expected expression with side- effect
        /w14619; # pragma warning: there is no warning number 'number'
        /w14640; # Enable warning on thread un-safe static member initialization
        /w14826; # Conversion from 'type1' to 'type_2' is sign-extended. This may
                 # cause unexpected runtime behavior.
        /w14928; # illegal copy-initialization; more than one user-defined
                 # conversion has been implicitly applied
        /wd4244; # 'argument': conversion from 'int' to 'unsigned char', possible
                 # loss of data # This one is sort of required for gtest.
        /WX;
    >
    $<$<BOOL:${STRICT_OPTIONS}>:
        -Werror;
    >
    $<$<BOOL:${COMPILER_SUPPORTS_WARNING_WEAK_VTABLES}>:
        -Wweak-vtables; # https://llvm.org/docs/CodingStandards.html#provide-a-virtual-method-anchor-for-classes-in-headers
    >
    $<$<BOOL:${COMPILER_SUPPORTS_WDATE_TIME}>:
        -Wdate-time; # avoid timestamps in binary for reproducible builds, not added until GCC 4.9
    >
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<NOT:$<BOOL:${HAIKU}>>>:
        -Wstack-protector;
    >
)
