#include <stdio.h>
#include <stdlib.h>
int main() {
    execlp("echo","echo","hello",NULL);

    return 0;
}