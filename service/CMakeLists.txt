project(python-srl-event-detector)

if (PYTHON_EXECUTABLE)
    set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    file(GLOB DEPS package/*.py)
    add_custom_command(
        OUTPUT python-srl-event-detector
        COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_SOURCE_DIR}/package ${PYTHON_EXECUTABLE} setup.py sdist
        COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT_DIR}/python-srl-event-detector
        DEPENDS ${DEPS}
        )

    add_custom_target(${PROJECT_NAME} ALL DEPENDS ${OUTPUT_DIR}/python-srl-event-detector)
else()
    message("python not found. srl_event_detector package will not be built.")
endif()
