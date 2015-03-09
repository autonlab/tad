project(python-srl)

if (PYTHON_EXECUTABLE)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    file(GLOB DEPS *.py)
    add_custom_command(
        OUTPUT python-srl
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_EXECUTABLE} setup.py sdist
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT_DIR}/python-srl
        DEPENDS ${DEPS}
        )

    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT_DIR}/python-srl)
else()
    message("python not found. srl package will not be built.")
endif()
