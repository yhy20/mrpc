set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/tests/base)

# Macro for add test target
macro(add_mrpc_base_test TEST_TARGET)
    string(TOLOWER ${TEST_TARGET} EXECUTABLE_FILE_NAME)
    add_executable(${EXECUTABLE_FILE_NAME} "${TEST_TARGET}.cc")
    # target_link_libraries(${EXECUTABLE_FILE_NAME} mrpc_base boost_unit_test_framework pthread)
    target_link_libraries(${EXECUTABLE_FILE_NAME} mrpc_base pthread)
endmacro(add_mrpc_base_test)

add_mrpc_base_test(Atomic_test)

add_mrpc_base_test(Mutex_test)

add_mrpc_base_test(Condition_test)

add_mrpc_base_test(CountDownLatch_test)

add_mrpc_base_test(CurrentThread_test)

add_mrpc_base_test(Thread_test)

add_mrpc_base_test(ThreadPool_test)

add_mrpc_base_test(StringPiece_test)

add_mrpc_base_test(Date_test)

add_mrpc_base_test(TimeStamp_test)

add_mrpc_base_test(TimeZone_test)

add_mrpc_base_test(LogStream_bench)

# add_mrpc_base_test(LogStream_test)

add_mrpc_base_test(Logging_test)

add_mrpc_base_test(AsyncLogging_test)

