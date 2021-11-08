/* Compile:
 * $ gcc -Wall -O6 -ansi -std=c99 -o bbq bbq.c
 */

#include <stdio.h>
#include <string.h>

int lineBufferSize = 4096;

int main (int argc, char ** argv) {
    FILE *stream;
    extern FILE *popen(); extern int pclose();
    char* prefix = NULL;
    int prefixLen = 0;
    char* cmd = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp (argv[i], "--prefix") == 0 ||
            strcmp (argv[i], "-p") == 0) {
            prefix = argv[i + 1];
            prefixLen = strlen(prefix);
            if (prefixLen <= 0) {
                fprintf(stderr, "%s: %s must not be empty.\n", argv[0], argv[i]);
                return 1;
            }
            i++;
        } else if (argv[i][0] == '-') { // else other option
            printf ("\007Usage: %s [option ...] [cmd string]\n", argv[0]);
            printf ("Valid options are:\n");
            printf ("  -p --prefix            Add prefix for perline\n");
            printf ("  -h --help --usage      Output this help.\n");
            return 0;
        } else {
            cmd = argv[i];
            if (cmd == NULL) {
                fprintf(stderr, "%s: No cmd string argument specified.\n", argv[0]);
                return 1;
            }
        }
    }
    if (!(stream = popen(cmd, "r"))) {
        fprintf(stderr, "%s: Could not open subpipe for writing.\n", argv[0]);
        return 1;
    }

    char buffer[lineBufferSize + 1];
    int i = 0;
    char ch;
    while ((ch = getc(stream)) != EOF) {
        buffer[i++] = ch;
        if (i >= lineBufferSize) {
            buffer[i++] = '\n';
            ch = '\n';
        }
        if (ch == '\n') {
            if (prefixLen > 0) {
                fwrite(prefix, 1, prefixLen, stderr);
            }
            fwrite(buffer, 1, i, stderr);
            i = 0;
        }
    }

    if (fclose(stdout)) {
        fprintf(stderr, "%s: Could not close standard output.\n",argv[0]);
    }
    if (pclose(stream)) {
        fprintf(stderr, "%s: Could not close subpipe.\n", argv[0]);
    }
    return(0);
}
