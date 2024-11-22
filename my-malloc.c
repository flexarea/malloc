#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static void *heap_bottom;
static void *top_of_heap;
#define HEAP_SIZE 1000

typedef struct {
    void *next_record;
    uint32_t size;
    int32_t free;
} heap_record;

size_t calc_alligned_address(size_t size) {
    if (size % 16 == 0) {
        return size;
    }
    return size + (16 - (size % 16));
}

void *malloc(size_t req_size) {
    size_t round_size = calc_alligned_address(req_size);
    if (heap_bottom == NULL) {
        if ((heap_bottom = sbrk(round_size + sizeof(heap_record) + HEAP_SIZE)) == (void *) -1) {
            return NULL;
        }
        top_of_heap = (char *)heap_bottom + (round_size + sizeof(heap_record) + HEAP_SIZE);
        heap_record *head = heap_bottom;
        head->next_record = heap_bottom;
        head->size = round_size;
        head->free = 0;
        return (char *) heap_bottom + sizeof(heap_record);
    }

    heap_record *cur_rec = heap_bottom;
    while (cur_rec->next_record != cur_rec) {
        if (cur_rec->free == 1) {
            // Check to see if the freed chunk is the right size
            if (round_size <= cur_rec->size) {
                if ((char *) cur_rec->next_record - ((char *) cur_rec + round_size) >= 32) {
                    cur_rec->size = round_size;
                    heap_record *new_record = (void *) ((char *) cur_rec + sizeof(heap_record) + cur_rec->size);                    
                    new_record->next_record = cur_rec->next_record;
                    new_record->size = (char *) new_record->next_record - (char *) new_record - sizeof(heap_record); 
                    new_record->free = 1;
                    cur_rec->next_record = new_record;
                }
                cur_rec->free = 0;
                // Cast to char pointer for intuitive arithmatic
                return (char *) cur_rec + sizeof(heap_record);
            }
        }
        cur_rec = cur_rec->next_record;
    }

    if (cur_rec->free == 1) {
        // Check to see if the freed chunck is the right size
        if (round_size <= cur_rec->size) {
            cur_rec->size = round_size;
            cur_rec->free = 0;
            // Cast to char pointer for intuitive arithmatic
            return (char *) cur_rec + sizeof(heap_record);
        }
    }
    // At this point, we have checked all existing records and have not found a free one that's big enough.
    if ((void *) ((char *)cur_rec + (2*sizeof(heap_record) + cur_rec->size + round_size) ) >= (void *) top_of_heap) {
        // We have reached the end of the heap, so we must extend it.
        if ((top_of_heap = sbrk(round_size + sizeof(cur_rec) + HEAP_SIZE)) == (void *)-1) {
            return NULL; 
        }
    }
    cur_rec->next_record = (char *) cur_rec + sizeof(heap_record) + cur_rec->size;
    heap_record *new_record = cur_rec->next_record;
    new_record->next_record = new_record;
    new_record->size = round_size;
    new_record->free = 0;
    return (char *) new_record + sizeof(heap_record);
}

void *calloc(size_t nmemb, size_t size) {
    void *val;
    if ((val = malloc(nmemb*size)) == NULL) {
        return NULL;
    }
    return memset(val, 0, calc_alligned_address(nmemb*size));
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
    }
    // Check if size is greater than previous chunk
    heap_record *record_to_reallocate = (void *)((char *) ptr - sizeof(heap_record));
    size_t round_size = calc_alligned_address(size);
    if (record_to_reallocate->size >= round_size){
        if (record_to_reallocate->next_record != record_to_reallocate) {
            if ((char *)record_to_reallocate->next_record - ((char *) record_to_reallocate + round_size) >= 32) {
                record_to_reallocate->size = round_size;
                heap_record *new_record = (void *)((char *) record_to_reallocate + sizeof(heap_record) + record_to_reallocate->size);
                new_record->next_record = record_to_reallocate->next_record;
                new_record->size = (char *)new_record->next_record - (char *)new_record - sizeof(heap_record); 
                new_record->free = 1;
                record_to_reallocate->next_record = new_record;
            }
        } else {
            // It is the last memory chunk in the linked-list so readjust the size
            record_to_reallocate->size = round_size;
        }
        return ptr;
    }
    // Malloc's optimization should take care of readjusting sizes and inserting new blocks
    void * new_chunk;
    if ( (new_chunk = malloc(size)) == NULL) {
        return NULL;
    }
    memcpy(new_chunk, ptr, record_to_reallocate->size);
    free(ptr);
    return new_chunk;
}

size_t malloc_usable_size(void *ptr){
    if (ptr == NULL) {
        return 0;
    }
    heap_record *hr = (void*) ((char *) ptr - sizeof(heap_record));
    return hr->size;
}

void free(void *ptr) {
    if(ptr != NULL){
        // Check if the next struct is also free. If so, combine.
        heap_record *record_to_free = (void *)((char *) ptr - sizeof(heap_record));
        if (record_to_free->next_record != record_to_free) {
            // There is a next Struct
            heap_record *second_record = record_to_free->next_record;
            if (second_record->free) {
                record_to_free->next_record = second_record->next_record;
                record_to_free->size += second_record->size + sizeof(heap_record);
            }
        }
        record_to_free->free = 1;
    }
}
