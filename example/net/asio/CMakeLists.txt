set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/examples/net)

add_executable(asio_chat_client client.cc)
target_link_libraries(asio_chat_client mrpc_net)

add_executable(asio_chat_server server.cc)
target_link_libraries(asio_chat_server mrpc_net)