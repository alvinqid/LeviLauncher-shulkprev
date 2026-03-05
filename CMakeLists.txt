cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0079 NEW)

project(shulke)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_VISIBILITY_INLINES_HIDDEN OFF)

include(FetchContent)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(
    preloader_android
    GIT_REPOSITORY https://github.com/LiteLDev/preloader-android.git
    GIT_TAG main
)
FetchContent_GetProperties(preloader_android)
if(NOT preloader_android_POPULATED)
    FetchContent_Populate(preloader_android)
    
    file(READ "${preloader_android_SOURCE_DIR}/src/pl/Logger.h" LOGGER_CONTENT)
    string(REPLACE "#include <format>" "#include <fmt/format.h>" LOGGER_CONTENT "${LOGGER_CONTENT}")
    string(REPLACE "std::vformat" "fmt::vformat" LOGGER_CONTENT "${LOGGER_CONTENT}")
    string(REPLACE "std::make_format_args" "fmt::make_format_args" LOGGER_CONTENT "${LOGGER_CONTENT}")
    file(WRITE "${preloader_android_SOURCE_DIR}/src/pl/Logger.h" "${LOGGER_CONTENT}")
    
    add_subdirectory(${preloader_android_SOURCE_DIR} ${preloader_android_BINARY_DIR})
endif()

target_link_libraries(preloader PUBLIC fmt::fmt)

FetchContent_Declare(zydis
    GIT_REPOSITORY https://github.com/zyantific/zydis.git)

FetchContent_Declare(properties-parser
    GIT_REPOSITORY https://github.com/minecraft-linux/properties-parser.git
    GIT_TAG 79f5360f889100bd4a1e069f4d9b86e94c902633
    EXCLUDE_FROM_ALL
)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8)

FetchContent_MakeAvailable(
    zydis
    properties-parser
    glm
)

add_library(shulke SHARED
    src/main.cpp
    src/item/itemstackbase.cpp
    src/nbt/nbt.cpp
    src/ui/minecraftuirendercontext.cpp
    src/ui/nineslicehelper.cpp
    src/shulkerenderer/shulkerrenderer.cpp
    src/util/scache.cpp
)

target_include_directories(shulke PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(shulke PUBLIC
    glm::glm
    log
    android
    preloader
    properties-parser
    Zydis::Zydis
)

target_link_options(shulke PRIVATE
    -static-libstdc++
)

set_target_properties(shulke PROPERTIES
    OUTPUT_NAME "shulke"
)

install(TARGETS shulke LIBRARY DESTINATION mods)
