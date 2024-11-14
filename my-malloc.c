#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//b my-malloc.c:72

static void *bottom;
static int extra_bytes = 10000;
static void *top_of_heap;

typedef struct my_struct {
    void *next_record;
    unsigned int section_size;
    int free;
} heap_record;

size_t calculate_nearest_size(size_t size){
    if (size % 16 == 0) {
        return size;
    }
    return size + (16 - (size % 16));
}
void *malloc(size_t req_size) {
    size_t round_size = calculate_nearest_size(req_size);
    if(bottom == NULL){
        bottom = sbrk(round_size + sizeof(heap_record) + extra_bytes);
        top_of_heap = sbrk(0);
        heap_record *head = bottom;
        head->next_record = bottom;
        head->section_size = round_size;
        head->free = 0;
        return (char *) bottom + sizeof(heap_record);
    }
    heap_record *current_record = bottom;
    while (current_record->next_record != current_record) {
        if (current_record->free == 1){
            // Check to see if the freed chunck is the right size
            if (req_size <= current_record->section_size) {
                current_record->free = 0;
                //cast to char pointer for intuitive arithmatic
                return (char *) current_record + sizeof(heap_record);
            }
        }
        current_record = current_record->next_record;
    }

    if (current_record->free == 1){
        // Check to see if the freed chunck is the right size
        if (req_size <= current_record->section_size) {
            current_record->free = 0;
            //cast to char pointer for intuitive arithmatic
            return (char *) current_record + sizeof(heap_record);
        }
    }
    // At this point, we have checked all existing records and have not found a free one that's big enough.
    if ((void *) ((char *)current_record + (2*sizeof(heap_record) + current_record->section_size + round_size) ) >= (void *) top_of_heap){
        // We have reached the end of the heap, so we must extend it.
        if ((top_of_heap = sbrk(round_size + sizeof(current_record) + extra_bytes)) == (void *)-1){
            // Handle the error, but i dont really know what we'd do here.
            return NULL; 
        }
    }
    current_record->next_record = (char *) current_record + sizeof(heap_record) + current_record->section_size;
    heap_record *new_record = current_record->next_record;
    new_record->next_record = new_record;
    new_record->section_size = round_size;
    new_record->free = 0;
    return (char *) new_record + sizeof(heap_record);
}

void *calloc(size_t nmemb, size_t size) {
    void *val = malloc((nmemb*size));
    return memset(val, 0, calculate_nearest_size(nmemb*size));
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL){
        void * val = malloc(size);
        return val;
    }
    if (size == 0) {
        free(ptr);
    }
    // check if size is greater than previous allocation chunk
    heap_record *record_to_reallocate = (void *)((char *) ptr - sizeof(heap_record));
    size_t round_size = calculate_nearest_size(size);
    if (record_to_reallocate->section_size >= round_size){
        return ptr;
    }else{
        void * new_chunk = malloc(size);
        memcpy(new_chunk,ptr,record_to_reallocate->section_size);
        free(ptr);
        return new_chunk;
    }
}

size_t malloc_usable_size(void *ptr){
    if (ptr == NULL) {
        return 0;
    }
    heap_record *hr = (void*) ((char *) ptr - sizeof(heap_record));
    return hr->section_size;
}


void free(void *ptr) {
    if(ptr != NULL){
        heap_record *record_to_free = (void *)((char *) ptr - sizeof(heap_record));
        record_to_free->free = 1;
    }
}
