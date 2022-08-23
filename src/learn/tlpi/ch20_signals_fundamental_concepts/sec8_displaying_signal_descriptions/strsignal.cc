#include "../../header.h"

/**
 * #define _BSD_SOURCE
 * #include <signal.h>
 * 
 * extern const char* const sys_siglist[];
 * 
 * #define _GNU_SOURCE
 * #include <string.h>
 * 
 * char* strsignal(int sig);
 */

int main(void)
{
    /**
     * Each signal has an associated printable description. These
     * descriptions are listed in the array sys_siglist. For example, 
     * we can refer to sys_siglist[SIGPIPE] to get the description for 
     * SIGPIPE(broken pipe). However, rather than using the sys_siglist
     * array directly, the strsignal() function is preferable.
     */
    printf("%s\n", sys_siglist[SIGINT]);
    printf("%s\n", sys_siglist[SIGPIPE]);
    printf("%s\n", sys_siglist[SIGHUP]);
    printf("%s\n", sys_siglist[SIGCONT]);

    /**
     * The strsignal() function performs bounds checking on the sig argument, 
     * and then returns a pointer to a printable description of the signal, or
     * a pointer to an error string if the signal number was invalid.
     */
    printf("<--------strsignal-------->\n");
    printf("%s\n", strsignal(SIGINT));
    printf("%s\n", strsignal(SIGPIPE));
    printf("%s\n", strsignal(SIGHUP));
    printf("%s\n", strsignal(SIGCONT));
    printf("<--------error-------->\n");
    char* str = strsignal(100000);
    /**
     * On some other UNIX implementations, strsignal() returns NULL if sig is invalid.
     * Aside from bounds checking, another advantage of strsignal() over the direct use
     * of sys_siglist is that strsignal() is locale-sensitive, so that signal descriptions
     * will be displayed in the local language.
     */
    if(NULL == str) printf("error signal!");
    else printf("%s\n", str);
    exit(EXIT_SUCCESS);
}