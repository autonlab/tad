project(srl-samples)

find_package(Boost
    COMPONENTS
        filesystem
        program_options
    REQUIRED)

include_directories(${RTMC_INCLUDE_PATH})
file(GLOB_RECURSE SRCS *.cpp)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME}
    ${Boost_LIBRARIES}
    srl)

install(TARGETS ${PROJECT_NAME} DESTINATION ${SAMPLE_INSTALL_PATH})
