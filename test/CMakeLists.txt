enable_testing()

add_executable(mc-tests
    test_main.cpp
    t_nbt.h
)

target_link_libraries(mc-tests GTest::gtest_main SFW::SFW)
target_include_directories(mc-tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

install(TARGETS mc-tests DESTINATION ${CMAKE_INSTALL_PREFIX}/tests)