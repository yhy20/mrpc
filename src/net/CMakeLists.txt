include(CheckFunctionExists)

check_function_exists(accept4 HAVE_ACCEPT4)
if(NOT HAVE_ACCEPT4)
    set_source_files_properties(SocketsOps.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

set(net_SRCS
    InetAddress.cc
    SocketsOps.cc
    Buffer.cc
    Timer.cc
    TimerQueue.cc
    Poller.cc
    Socket.cc
    Channel.cc
    EventLoop.cc
    EventLoopThread.cc
    EventLoopThreadPool.cc
    TcpConnection.cc
    Acceptor.cc
    TcpServer.cc
    Connector.cc
    TcpClient.cc
    ./poller/DefaultPoller.cc
    ./poller/EPollPoller.cc)

#add_library(mrpc_base ${base_SRCS})
add_library(mrpc_net SHARED ${net_SRCS})
target_link_libraries(mrpc_net mrpc_base pthread)

add_subdirectory(tests)