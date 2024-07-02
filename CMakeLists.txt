cmake_minimum_required(VERSION 3.27.7)

project(mc-server)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/$<CONFIG>/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out/$<CONFIG>/)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_subdirectory(dependencies/SFW)
add_subdirectory(dependencies/nlohmann-json)

add_subdirectory(src)

target_include_directories(${PROJECT_NAME}
                        PRIVATE include/
                        PRIVATE dependencies/nlohmann-json/single_include)

target_link_libraries(${PROJECT_NAME} PRIVATE SFW::SFW)

set_property(TARGET SFW PROPERTY CXX_STANDARD 20)
