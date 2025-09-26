cmake_minimum_required(VERSION 3.27.7)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(FLAGS -Wall -Werror -Wpedantic -Wno-deprecated-literal-operator)

project(mc-server)

include(FetchContent)
FetchContent_Declare(ZStrGitRepo
  GIT_REPOSITORY    "https://github.com/mateidavid/zstr" # can also be a local "data/registries/blocks.json" "data/registries/items.json" "data/registries/registries.json" "registry.json"ystem path!
  GIT_TAG           "master"
)
FetchContent_MakeAvailable(ZStrGitRepo)

add_subdirectory(dependencies/SFW)
add_subdirectory(dependencies/nlohmann-json)

add_subdirectory(src)

target_include_directories(${PROJECT_NAME}
                        PRIVATE include/
                        PRIVATE dependencies/nlohmann-json/single_include)

target_link_libraries(${PROJECT_NAME} PRIVATE SFW::SFW PRIVATE zstr::zstr)

target_compile_options(${PROJECT_NAME} PRIVATE ${FLAGS})


install(DIRECTORY "data/registries/packets" DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY "data/registries" DESTINATION ${CMAKE_INSTALL_PREFIX})
