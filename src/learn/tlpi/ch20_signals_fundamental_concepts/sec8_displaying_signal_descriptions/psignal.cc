#include "../../header.h"

void sighandler(int sig)
{
    if(sig == SIGPIPE)
    {
        /**
         * The psignal() function display(on standard error) the string
         * given in its argument msg, followed by a colon, and then the
         * signal description corresponding to sig. Like strsignal(), 
         * psignal() is locale-sensitive.
         */
        psignal(sig, "PIPE error");
    }    
}

int main(void)
{
    int pipefd[2];
    if(-1 == pipe(pipefd))
        LOG_FATAL("pipe error!");
    
    signal(SIGPIPE, sighandler);

    close(pipefd[0]);
    char msg[] = "hello!";
    write(pipefd[1], msg, strlen(msg));
    exit(EXIT_SUCCESS);
}