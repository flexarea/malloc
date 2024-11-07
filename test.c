#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    void * p1 = malloc(10);
    void * p2 = malloc(10);
    free(p1);
    free(p2);
    void * p3 = malloc(5);
    void * p4 = malloc(5);

    return 1;
}