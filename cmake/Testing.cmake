################################################################################
#
# :: Testing
#
################################################################################

include(CTest)

enable_testing()

function(auto_test subsystem module)
  add_executable(test_${module}
    test/common.h
    test/${subsystem}/${module}_test.cpp)
  target_link_libraries(test_${module}
    ${PROJECT_NAME}_static
    ${CHECK_LIBRARIES}
    Qt5::Test)
  add_test(
    NAME test_${module}
    COMMAND test_${module})
endfunction()

search_dependency(CHECK PACKAGE check OPTIONAL)
if (CHECK_FOUND)
  auto_test(core toxpk)
  auto_test(core toxid)
  auto_test(chatlog textformatter)
endif()
