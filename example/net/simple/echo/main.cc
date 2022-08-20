#include "echo.h"
#include "Logging.h"
#include "EventLoop.h"

#include <unistd.h>

int main(void)
{
    LOG_INFO << "pid = " << getpid();
    mrpc::net::EventLoop loop;
    mrpc::net::InetAddress listenAddr(2007);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();

    return 0;
}
