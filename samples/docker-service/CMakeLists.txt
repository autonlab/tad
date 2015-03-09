project(docker-service)

find_program(DOCKER docker)

if (DOCKER)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    file(GLOB DEPS *)
    get_property(DEPS_CPP_SERVER TARGET cpp-server PROPERTY LOCATION)
    file(GLOB DEPS_PY_SRL ${CMAKE_SOURCE_DIR}/python/SRL/dist/*.tar.gz)
    file(GLOB DEPS_PY_SERVICE ${CMAKE_CURRENT_SOURCE_DIR}/../python-eventdetector-service/package/dist/*.tar.gz)
    add_custom_command(
        OUTPUT docker-service
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} sh build-docker
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT_DIR}/docker-service
        DEPENDS ${DEPS} ${DEPS_CPP_SERVER} ${DEPS_PY_SRL} ${DEPS_PY_SERVICE}
        )

    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT_DIR}/docker-service)
else()
    message("Docker not found. Container will not be built.")
endif()
