#include <sys/wait.h>
#include "../../header.h"

int main(int argc, char* argv[])
{
    int pipefd[2];
    
    if(-1 == pipe(pipefd))
    {
        LOG_FATAL("pipe() error!");
    }
    
    pid_t pid = fork();
    if(-1 == pid) LOG_FATAL("fork() error!");
    else if(0 == pid) // child reads from pipe
    {
        close(pipefd[1]); // close unused write end

        char buf[100] = { 0 };
        ssize_t len = 0;
        while((len = read(pipefd[0], &buf, sizeof(buf))) > 0)
        {
            write(STDOUT_FILENO, &buf, len);
        }
        if(-1 == len) LOG_FATAL("read() error!");
        close(pipefd[0]);

        /// ps -ef | grep -E 'UID*|pipe_example' 
        /// wait to see process group status
        sleep(3);
        _exit(EXIT_SUCCESS);
    }
    else // parent writes msg to pipe
    {   
        close(pipefd[0]); // close unused read end

        char msg[] = "Hello!\n";
        write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]); // reader will see EOF

        /// TODO: man 2 wait waitpid waitid test
        /// wait for a child to die
        int status;
        pid_t child_pid = wait(&status);
        if(-1 == child_pid) LOG_FATAL("wait() error!");
        printf("pid = %ld, child_pid = %ld\n", (long)pid, (long)child_pid);

        /// returns true if the child terminated normally, that is, by calling
        /// exit(3) or _exit(2), or by returning from main()
        if(WIFEXITED(status)) printf("child process terminated normally\n");

        exit(EXIT_SUCCESS);
    }

    return 0;
}