set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/tests/net)

# Macro for add test target
macro(add_mrpc_net_test TEST_TARGET)
    string(TOLOWER ${TEST_TARGET} EXECUTABLE_FILE_NAME)
    add_executable(${EXECUTABLE_FILE_NAME} "${TEST_TARGET}.cc")
    # target_link_libraries(${EXECUTABLE_FILE_NAME} mrpc_net mrpc_base boost_unit_test_framework pthread)
    target_link_libraries(${EXECUTABLE_FILE_NAME} mrpc_net mrpc_base pthread)
endmacro(add_mrpc_net_test)

# add_mrpc_net_test(Buffer_test)
# add_mrpc_net_test(InetAddress_test)
add_mrpc_net_test(TimerQueue_test)
add_mrpc_net_test(Channel_test)
add_mrpc_net_test(EchoServer_test)
add_mrpc_net_test(EchoClient_test)
