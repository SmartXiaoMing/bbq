/* Compile:
 * $ gcc -Wall -O6 -ansi -std=c99 -pthread -o bbq bbq.c
 */

#include <stdio.h>
#include <string.h>
#include <pthread.h>

int lineBufferSize = 4096;

struct Command {
    char* prefix;
    char* cmd;
    FILE* file;
    pthread_t tid;
};

void* handleStream(void* data) {
    struct Command* command = (struct Command*) data;
    int prefixLen = command->prefix == NULL ? 0 : strlen(command->prefix);
    char buffer[lineBufferSize + 1];
    int i = 0;
    char ch;
    while ((ch = getc(command->file)) != EOF) {
        buffer[i++] = ch;
        if (i >= lineBufferSize) {
            buffer[i++] = '\n';
            ch = '\n';
        }
        if (ch == '\n') {
            if (prefixLen > 0) {
                fwrite(command->prefix, 1, prefixLen, stderr);
            }
            fwrite(buffer, 1, i, stderr);
            i = 0;
        }
    }
    pclose(command->file);
    return NULL;
}

int main (int argc, char ** argv) {
    struct Command commands[argc];
    int commandSize = 0;
    commands[commandSize].prefix = NULL;
    commands[commandSize].cmd = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp (argv[i], "--prefix") == 0 ||
            strcmp (argv[i], "-p") == 0) {
            commands[commandSize].prefix = argv[i + 1];
            i += 1;
        } else if (argv[i][0] == '-') { // else other option
            printf ("\007Usage: %s [-p prefix] cmd1 [-p prefix] cmd2 ...\n", argv[0]);
            printf ("Valid options are:\n");
            printf ("  -p --prefix      Add prefix for perline\n");
            printf ("  -h --help --usage    Output this help.\n");
            return 0;
        } else {
            commands[commandSize].cmd = argv[i];
            commandSize++;
            commands[commandSize].prefix = NULL;
            commands[commandSize].cmd = NULL;
        }
    }
    if (commandSize == 0) {
        fprintf(stderr, "%s: cmd can not be empty.\n", argv[0]);
        printf ("\007Usage: %s [-p prefix] cmd1 [-p prefix] cmd2 ...\n", argv[0]);
        return 1;
    }
    for (int i = 0; i < commandSize; ++i) {
        commands[i].file = popen(commands[i].cmd, "r");
        if (!commands[i].file) {
            fprintf(stderr, "%s: Could not open subpipe for writing.\n", argv[0]);
            return 1;
        }
    }
    for (int i = 1; i < commandSize; ++i) {
        pthread_create(&commands[i].tid, NULL, handleStream, &commands[i]);
    }
    handleStream(&commands[0]);
    for (int i = 1; i < commandSize; ++i) {
        pthread_join(commands[i].tid, NULL);
    }
    if (fclose(stdout)) {
        fprintf(stderr, "%s: Could not close standard output.\n", argv[0]);
    }
    return(0);
}