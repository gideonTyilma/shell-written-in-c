/*
Gediyon Yilma
113314131

*/

#include "executor.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <sysexits.h>
#include <unistd.h>
#include "command.h"

static void print_tree(struct tree *t);

int exec_aux(struct tree *t, int par_in_fd, int par_out_fd) {
    int pid, fd, fd1, status;
    int pipefd[2];
    char buf;
    if (t->conjunction == NONE) {
        if (strstr(t->argv[0], "exit")) {
            exit(0);
        }

        if (strstr(t->argv[0], "cd")) {
            if (t->argv[1]) {
                if (chdir(t->argv[1]) != 0) {
                    perror("cd");
                } else {
                    return 0;
                }
            } else {
                if (chdir(getenv("HOME")) != 0) {
                    perror("chidir");
                } else {
                    return 0;
                }
            }
        }

        if (!(t->input) && !(t->output)) {
            if (strcmp(t->argv[0], "exit") != 0 &&
                strcmp(t->argv[0], "cd") != 0) {
                if ((pid = fork()) < 0) {
                    perror("FORK");
                    exit(1);
                } else if (pid == 0) {
                    dup2(par_in_fd, STDIN_FILENO);
                    dup2(par_out_fd, STDOUT_FILENO);
                    if (execvp(t->argv[0], t->argv) < 0) {
                        fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
                        exit(EX_OSERR);
                    }
                } else {
                    wait(&status);
                    if (WIFEXITED(status)) {
                        return WEXITSTATUS(status);
                    } else {
                        return 1;
                    }
                }
            }
        } else if (!(t->input) && (t->output)) { /*bla > bla*/

            if ((pid = fork()) < 0) {
                perror("FORK");
                exit(1);
            } else if (pid == 0) {
                fd = open(t->output, O_WRONLY | O_TRUNC | O_CREAT, 0664);
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup");
                }
                if (execvp(t->argv[0], t->argv) < 0) {
                    fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
                    exit(EX_OSERR);
                }
            } else {
                wait(&status);
                if (WIFEXITED(status)) {
                    return WEXITSTATUS(status);

                } else {
                    return 1;
                }
            }

        } else if ((t->input) && !(t->output)) { /*bla < bla*/
            if ((pid = fork()) < 0) {
                perror("FORK");
                exit(1);
            } else if (pid == 0) {
                fd = open(t->input, O_RDONLY);
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup");
                }
                if (execvp(t->argv[0], t->argv) < 0) {
                    fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
                    exit(EX_OSERR);
                }
            } else {
                wait(&status);
                if (WIFEXITED(status)) {
                    return WEXITSTATUS(status);

                } else {
                    return 1;
                }
            }

        } else if ((t->input) && (t->output)) {
            if ((pid = fork()) < 0) {
                perror("FORK");
                exit(1);
            } else if (pid == 0) {
                fd = open(t->input, O_RDONLY);
                fd1 = open(t->output, O_WRONLY | O_TRUNC | O_CREAT, 0664);
                if (dup2(fd, STDIN_FILENO) < 0) {
                    perror("dup");
                }
                if (dup2(fd1, STDOUT_FILENO) < 0) {
                    perror("dup");
                }

                if (execvp(t->argv[0], t->argv) < 0) {
                    fprintf(stderr, "Failed to execute %s\n", t->argv[0]);
                    exit(EX_OSERR);
                }
            } else {
                wait(&status);
                if (WIFEXITED(status)) {
                    return WEXITSTATUS(status);
                } else {
                    return 1;
                }
            }
        }
    } else if (t->conjunction == AND) {
        if (exec_aux(t->left, STDIN_FILENO, STDOUT_FILENO) == 0) {
            exec_aux(t->right, STDIN_FILENO, STDOUT_FILENO);
        }
    } else if (t->conjunction == SUBSHELL) {
        if (t->output) {
            fd1 = open(t->output, O_WRONLY | O_TRUNC | O_CREAT, 0664);
        } else {
            fd1 = STDOUT_FILENO;
        }
        if (t->input) {
            fd = open(t->input, O_RDONLY);
        } else {
            fd = STDIN_FILENO;
        }
        pid = fork();
        if (pid == 0) {
            printf("%d %d\n", fd, fd1);
            exec_aux(t->left, fd, fd1);
        } else {
            wait(&status);
            exit(0);
        }

    }
    /*PIPE CODE*/
    else if (t->conjunction == PIPE) {
        if (pipe(pipefd) == -1) {
            exit(EXIT_FAILURE);
        }
        if ((pid = fork()) < 0) {
            perror("FORK");
            exit(1);
        } else if (pid == 0) { /*left child writes to right stdin*/
            close(pipefd[1]);
            if (t->output) {
                fd1 = open(t->output, O_WRONLY | O_TRUNC | O_CREAT, 0664);
            }
            if (t->right->input != NULL) {
                printf("Ambiguous input redirect.\n");
            } else {
                exec_aux(t->right, pipefd[0], fd1);
            }

            if (t->output) {
                close(fd1);
            }

            close(pipefd[0]);
            exit(0);

        } else { /*right child reads from left stdout*/
            close(pipefd[0]);
            if (t->input) {
                fd = open(t->input, O_RDONLY);
            }
            if (t->left->output != NULL) {
                printf("Ambiguous output redirect.\n");

            } else {
                exec_aux(t->left, fd, pipefd[1]);
            }

            if (t->input) {
                close(fd);
            }

            close(pipefd[1]);

            wait(&status);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);

            } else {
                return 1;
            }
        }
    }
    /*PIPE CODE*/
    return 1;
}

int execute(struct tree *t) {
    /*  print_tree(t);*/

    return exec_aux(t, STDIN_FILENO, STDOUT_FILENO);
}

static void print_tree(struct tree *t) {
    if (t != NULL) {
        print_tree(t->left);

        if (t->conjunction == NONE) {
            printf("NONE: %s, ", t->argv[0]);
        } else {
            printf("%s, ", conj[t->conjunction]);
        }
        printf("IR: %s, ", t->input);
        printf("OR: %s\n", t->output);

        print_tree(t->right);
    }
}
