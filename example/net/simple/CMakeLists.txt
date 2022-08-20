set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/examples/net)

add_executable(simple_echo echo/echo.cc ./echo/main.cc)
target_link_libraries(simple_echo mrpc_net)

add_executable(simple_discard discard/discard.cc ./discard/main.cc)
target_link_libraries(simple_discard mrpc_net)