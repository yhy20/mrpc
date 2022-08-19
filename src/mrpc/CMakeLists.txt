aux_source_directory(../base BASE_LIST)
aux_source_directory(. MRPC_LIST)
add_library(mrpc ${MRPC_LIST} ${BASE_LIST})
target_link_libraries(mrpc libmuduo_net.a libmuduo_base.a protobuf pthread)
