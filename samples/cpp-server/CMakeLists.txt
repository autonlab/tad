project(cpp-server)

include_directories(${RTMC_INCLUDE_PATH})
file(GLOB_RECURSE SRCS *.cpp)

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME}
    srl)

install(TARGETS ${PROJECT_NAME} DESTINATION ${SAMPLE_INSTALL_PATH})
