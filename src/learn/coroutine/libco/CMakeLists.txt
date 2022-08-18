# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/learn/libco)

# # Macro for add example target
# macro(add_libco_test EXAMPLE_TARGET)
#     string(TOLOWER "${EXAMPLE_TARGET}_libco_test" EXECUTABLE_NAME)
#     add_executable(${EXECUTABLE_NAME} "${EXAMPLE_TARGET}.cc")
#     target_link_libraries(${EXECUTABLE_NAME} colib_static pthread dl)
# endmacro(add_libco_test)

# add_libco_test(EchoClient)
# add_libco_test(EchoServer)
# add_libco_test(Thread)



# # Macro for add example target
# macro(add_example_target EXAMPLE_TARGET)
#     add_executable("example_${EXAMPLE_TARGET}" "example_${EXAMPLE_TARGET}.cpp")
#     target_link_libraries("example_${EXAMPLE_TARGET}" colib_static pthread dl)
# endmacro(add_example_target)

# add_example_target(closure)
# add_example_target(cond)
# add_example_target(copystack)
# add_example_target(echocli)
# add_example_target(echosvr)
# add_example_target(poll)
# add_example_target(setenv)
# add_example_target(specific)
# add_example_target(thread)


