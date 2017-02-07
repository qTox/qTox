################################################################################
#
# :: Testing
#
################################################################################

set(TEST_NAME unit_test)

set(TEST_SOURCES
  test/main.cpp
  test/core/toxpk_test.cpp
  test/core/toxid_test.cpp
)

if (NOT ENABLE_UNIT_TESTS)
  set(ENABLE_UNIT_TESTS True)
endif()

if (${ENABLE_UNIT_TESTS})
  search_dependency(GTEST PACKAGE GTest OPTIONAL)
  if (NOT GTEST_FOUND)
    search_dependency(GTEST PACKAGE gtest OPTIONAL)
    if (NOT GTEST_FOUND)
      search_dependency(GTEST PACKAGE googletest OPTIONAL)
      if (NOT GTEST_FOUND)
        search_dependency(GTEST PACKAGE google-test OPTIONAL)
        if (NOT GTEST_FOUND)
          message("GTest not found.")
        endif()
      endif()
    endif()
  endif()

  search_dependency(GMOCK PACKAGE GMock OPTIONAL)
  if (NOT GMOCK_FOUND)
    search_dependency(GMOCK PACKAGE gmock OPTIONAL)
    if (NOT GMOCK_FOUND)
      message("GMock not found.")
    endif()
  endif()

  if (GTEST_FOUND AND GMOCK_FOUND)
    add_executable(${TEST_NAME}
      ${TEST_SOURCES}
    )

    target_link_libraries(${TEST_NAME}
      ${PROJECT_NAME}_static
      ${ALL_LIBRARIES}
      gtest gmock
    )
  else()
    message("Can't build tests")
  endif()
endif()
