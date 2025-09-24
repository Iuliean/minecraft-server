cmake_minimum_required(VERSION 3.27.7)

project(mc-server)

include(FetchContent)
FetchContent_Declare(ZStrGitRepo
  GIT_REPOSITORY    "https://github.com/mateidavid/zstr" # can also be a local filesystem path!
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

set_property(TARGET SFW PROPERTY CXX_STANDARD 20)
