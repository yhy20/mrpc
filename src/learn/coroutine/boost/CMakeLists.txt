set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/learn/coroutine/boost/)

# Macro for add example target
macro(add_boost_example EXAMPLE_TARGET)
    string(TOLOWER "${EXAMPLE_TARGET}_example" EXECUTABLE_NAME)
    add_executable(${EXECUTABLE_NAME} "${EXAMPLE_TARGET}.cc")
    target_link_libraries(${EXECUTABLE_NAME} boost_coroutine boost_context pthread)
endmacro(add_boost_example)

add_boost_example(01_Basic)
add_boost_example(03_Push)
