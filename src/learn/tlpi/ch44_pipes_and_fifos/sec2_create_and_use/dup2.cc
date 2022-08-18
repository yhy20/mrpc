#include "../../header.h"

int main()
{
    int pipefd[2];
    if(-1 == pipe(pipefd))
        LOG_FATAL("pipe() error!");
    
    LOG_INFO("pipefd[0] = %d, pipefd[1] = %d",
              pipefd[0], pipefd[1]);

    dup2(pipefd[1], STDOUT_FILENO);
    LOG_INFO("pipefd[0] = %d, pipefd[1] = %d",
              pipefd[0], pipefd[1]);

    dup2(pipefd[0], STDIN_FILENO);
    LOG_INFO("pipefd[0] = %d, pipefd[1] = %d",
              pipefd[0], pipefd[1]);

    char msg[] = "Hello!";
    // write(pipefd[1], msg, sizeof(msg));
    // write(STDOUT_FILENO, msg, sizeof(msg));
    printf(msg);
    fflush(stdout);

    close(pipefd[1]);
    close(STDOUT_FILENO);


    char buf[100];
    // read(pipefd[0], buf, sizeof(buf));
    // read(STDIN_FILENO, buf, sizeof(buf));
    scanf("%s", buf);
    fprintf(stderr, "%s\n", buf);

    close(pipefd[0]);
    close(STDIN_FILENO);

    exit(EXIT_SUCCESS);
}