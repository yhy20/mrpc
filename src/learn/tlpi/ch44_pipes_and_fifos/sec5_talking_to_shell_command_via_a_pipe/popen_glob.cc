#include <ctype.h>
#include <limits.h>

#include "../../header.h"

/// ls -d '*' 2> /dev/null
#define POPEN_FMT "ls -d %s 2> /dev/null"
#define PAT_SIZE 50
#define PCMD_BUF_SIZE (sizeof(POPEN_FMT) + PAT_SIZE)

int main(void)
{
    char pat[PAT_SIZE];
    char popenCmd[PCMD_BUF_SIZE];
    FILE *fp;
    bool badPattern;
    int status, fileCnt, j;
    char pathname[PATH_MAX];

    while(true)
    {
        printf("pattern: ");
        fflush(stdout);
        /**
         * fgets(3) 
         * char *fgets(char *s, int size, FILE *stream);
         * 
         * fgets() reads in at most one less than size
         * characters from stream and stores them into the
         * buffer pointed to by s. Reading stops after an
         * EOF or a newline. If a newline is read, it is
         * stored into the buffer. A terminating numm byte
         * ('\0') is stored after the last character in the 
         * buffer.
         */
        if(nullptr == fgets(pat, PAT_SIZE, stdin))
            break;  // EOF
        
        int len = strlen(pat);
        
        if(len <= 1) continue;  // Empty line, only '\n'.

        if(pat[len - 1] == '\n')
            pat[len - 1] = '\0';  // Strip trailing newline.

        // printf("len = %d\n", len);

        for(j = 0, badPattern = false; j < len && !badPattern; ++j)
        {
            /**
             * isalnum(3)
             * checks for an an alphanumeric character;
             * it is equivalent to (isalpha(c) || isdigit(c))
             * 
             * strchr(3)
             * char *strchr(const char *s, int c);
             * The strchr() function returns a pointer to the
             * first occurrence of the character c in the string s.
             */
            if(!isalnum((unsigned char) pat[j]) && 
               strchr("_*?[^-].", pat[j]) == nullptr)
               badPattern = true;
        }
        if(badPattern)
        {
            printf("Bad pattern character: %c\n", pat[j - 1]);
            continue;
        }

        /// Build and execute command to glob 'pat'

        snprintf(popenCmd, PCMD_BUF_SIZE, POPEN_FMT, pat);
        popenCmd[PCMD_BUF_SIZE - 1] = '\0';

        fp = popen(popenCmd, "r");
        if(nullptr == fp)
        {
            LOG_ERROR("popen() failed!\n");
            continue;
        } 

        /// Read resulting list of pathnames until EOF

        fileCnt = 0;
        while(nullptr != fgets(pathname, PATH_MAX, fp))
        {
            printf("%s", pathname);
            fileCnt++;
        }

        /// Close pipe, fetch and display termination status.
        status = pclose(fp);
        printf("    %d matching file%s\n", fileCnt, (fileCnt != 1) ? "s" : "");
        printf("    pclose() status == %#x\n", (unsigned int) status);

        if(-1 == status)
        {
            
        }
       
    }
     exit(EXIT_SUCCESS);
}