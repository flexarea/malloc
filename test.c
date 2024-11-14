#include <stdio.h>
#include <stdlib.h>

void realloc_test();
size_t malloc_usable_size(void *ptr);
int main(int argc, char *argv[]){
            
    char str[] = "Esdras";
    void * p1 = malloc(10);
    void * p2 = malloc(20);
    void * p3 = malloc(40);
    char * p4 = (char *) malloc(81);

    for (int i=0; i<6; i++) {
        *(p4+i) = str[i];    
    }
    char *p5 = realloc(p4, 16);
    free(p5);

    char * p6 = (char *) calloc(8, sizeof(char));
    free(p6);

    return 1;
}

