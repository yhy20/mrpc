# aux_source_directory(. MRPC_LIST)

set(mrpc_SRCS
    Mrpc.cc
    MrpcChannel.cc
    MrpcController.cc
    RpcHeader.pb.cc
    RpcProvider.cc)

# add_library(mrpc_base ${base_SRCS})
add_library(mrpc SHARED ${mrpc_SRCS})
target_link_libraries(mrpc mrpc_net mrpc_base protobuf pthread)
