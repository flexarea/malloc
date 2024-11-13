#include <stdio.h>
#include <stdlib.h>

void realloc_test();
size_t malloc_usable_size(void *ptr);
int main(int argc, char *argv[]){
            
    char str[] = "Esdras";
    void * p1 = malloc(10);
    void * p2 = malloc(10);
    free(p1);
    free(p2);
    void * p3 = malloc(40);
    size_t t = malloc_usable_size(p3);
    char * p4 = (char *) malloc(8);

    for (int i=0; i<6; i++) {
        *(p4+i) = str[i];    
    }
    char *p5 = realloc(p4, 10);
    free(p5);

    char * p6 = (char *) calloc(8, sizeof(char));
    free(p6);

    return 1;
}

