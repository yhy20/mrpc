#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>



void SetFd1()
{

}

void SetFd2()
{

}

int main(int argc, char* argv[])
{
    fd_set readfds, writefds;
    int nfds, fd, numRead, j;
    struct timeval timeout;
    struct timeval *pto;
    char buf[10];


    nfds = 0;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);


    /// Ignore exceptional events.
    int ready = select(nfds, &readfds, &writefds, NULL, pto);
    if(-1 == ready)
    {

    }
    else
    {

    }

    // exit(EXIT_SUCCESS);
}
