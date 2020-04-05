#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <sysexits.h>
#include <unistd.h>

int main() {
    int fd;
    char writ[] = "to be or not to be\n";
    char buf[10];
    read(0, buf, 10);
    write(1, buf, 10);
ope
    return fd;
}