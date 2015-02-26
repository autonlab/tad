cmake_minimum_required(VERSION 2.8)
include(ExternalProject)

project(srl)

# Do not allow in-source builds:
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fPIC")
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Build options.
option(BUILD_SHARED     "Build as a shared library."            OFF)
option(BUILD_SAMPLES    "Build the C++ sample applications."    ON)
option(INSTALL_SAMPLES  "Install the sample application"        ON)

if (UNIX)
    set(SAMPLE_INSTALL_PATH "/usr/local/bin")
    if (X86_64 OR CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(LIB_INSTALL_PATH "/usr/local/lib64")
    else()
        set(LIB_INSTALL_PATH "/usr/local/lib")
    endif()
    set(HEADER_INSTALL_PATH "/usr/local/include")
else(WIN32)
    set(SAMPLE_INSTALL_PATH "C:/Program Files/${PROJECT_NAME}/samples")
    set(LIB_INSTALL_PATH "C:/Program Files/${PROJECT_NAME}/lib")
    set(HEADER_INSTALL_PATH "C:/Program Files/${PROJECT_NAME}/include")
endif()

# Third party targets.
ExternalProject_Add(jsoncpp_git
    GIT_REPOSITORY https://github.com/open-source-parsers/jsoncpp.git
    GIT_TAG 1.4.4
    CMAKE_ARGS -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF -DJSONCPP_WITH_PKGCONFIG_SUPPORT=OFF -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_SOURCE_DIR}/thirdparty/jsoncpp
    UPDATE_COMMAND ""
    INSTALL_DIR ${CMAKE_SOURCE_DIR}/thirdparty/jsoncpp)

ExternalProject_Get_Property(jsoncpp_git INSTALL_DIR)
set(JSONCPP_INSTALL_DIR ${INSTALL_DIR})
include_directories(${JSONCPP_INSTALL_DIR}/include)
link_directories(${JSONCPP_INSTALL_DIR}/lib)

# Library target.
include_directories(include)
file(GLOB_RECURSE LIB_SRCS src/*.cpp)
if (BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${LIB_SRCS})
else()
    add_library(${PROJECT_NAME} STATIC ${LIB_SRCS})
endif()
add_dependencies(${PROJECT_NAME} jsoncpp_lib_static)
target_link_libraries(${PROJECT_NAME} jsoncpp pthread rt)
install(TARGETS ${PROJECT_NAME} DESTINATION ${LIB_INSTALL_PATH})
install(DIRECTORY include/ DESTINATION ${HEADER_INSTALL_PATH})

if (BUILD_SAMPLES)
    add_subdirectory(samples)
endif()
