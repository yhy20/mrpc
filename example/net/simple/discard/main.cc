#include <unistd.h>

#include "discard.h"
#include "Logging.h"
#include "EventLoop.h"

int main()
{
  LOG_INFO << "pid = " << getpid();
  mrpc::net::EventLoop loop;
  mrpc::net::InetAddress listenAddr(2009);
  DiscardServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}
