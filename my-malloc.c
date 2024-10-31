#include <unistd.h>
#include <string.h>

static void *bottom;
static int extra = 10000;


typedef struct my_struct {
    void *next_record;
    int section_size;
    int free;
} record;

void *malloc(size_t size) {
    if(bottom == NULL){
        int round_size = size + (size % 16);
        bottom = sbrk(round_size + sizeof(record) + extra);
        record new;
        new.next_record = bottom;
        new.section_size = round_size;
        new.free = 0;
        memcpy(bottom,&new,sizeof(record));
    }
    return NULL;
}

void *calloc(size_t nmemb, size_t size) {
    return NULL;
}

void *realloc(void *ptr, size_t size) {
    return NULL;
}

void *free(void *ptr) {
    return NULL;
}