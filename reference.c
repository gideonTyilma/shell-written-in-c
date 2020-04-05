#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <sysexits.h>
#include <unistd.h>
#define MAX_LEN 1024

/* Implement your shell here */
int main() {
    pid_t result;

    int ex = 1, status;
    const char *exit1 = "hastalavista", *exit2 = "exit";
    char *argv[3], command[MAX_LEN];
    char buffer[MAX_LEN];

    while (ex) {
        argv[0] = malloc(1024);
        argv[1] = malloc(1024);
        argv[2] = NULL;
        printf("shell_jr: ");
        fgets(buffer, MAX_LEN, stdin);

        if (feof(stdin)) {
            exit(0);
        }
        if (strstr(buffer, exit1) || strstr(buffer, exit2)) {
            printf("See you\n");
            ex = 0;
        }
        if (strstr(buffer, "cd")) {
            sscanf(buffer, "%s %s", command, argv[1]);
            if (chdir(argv[1]) != 0) {
                printf("Cannot change directory to %s\n", argv[1]);
                fflush(stdout);
            }
        }

        sscanf(buffer, "%s %s", command, argv[1]);
        strcpy(argv[0], command);
        fflush(stdout);
        if (strcmp(command, "cd") != 0 && strcmp(command, exit1) != 0 &&
            strcmp(command, exit2) != 0) {
            if ((result = fork()) < 0) {
                exit(1);
            } else if (result == 0) {
                if (execvp(command, argv) < 0) {
                    printf("Failed to execute %s\n", command);
                    fflush(stdout);
                    exit(EX_OSERR);
                }
            }
        }
        free(argv[1]);
        free(argv[0]);
        wait(&status);
    }

    exit(0);
}