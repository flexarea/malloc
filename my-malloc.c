#include <unistd.h>
#include <string.h>

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
        top_of_heap = ((char *) bottom) + (round_size + sizeof(heap_record) + extra_bytes);
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
    if ((void *) ((char *)current_record + round_size) >= (void *) top_of_heap){
        // We have reached the end of the heap, so we must extend it.
        if (brk((char *) top_of_heap + extra_bytes) == -1){
            // Hqandle the error, but i dont really know what we'd do here. 
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
    // check if size is greater than previous allocation chunk
    heap_record *record_to_reallocate = (void *)((char *) ptr - sizeof(heap_record));

    if(record_to_reallocate->section_size < size) {

        heap_record *current_record = bottom;
        while (current_record->next_record != current_record) {
            if (current_record->free == 1){
                // Check to see if the freed chunck is the right size

                if (size <= current_record->section_size) {
                    //copy initialized data from record to reallocate
                    memcpy(current_record, record_to_reallocate, sizeof(heap_record) + record_to_reallocate->section_size);

                    int old_record_section_size = current_record->section_size;
                    current_record->section_size = size;
                    current_record->free = 0;

                    if(current_record->next_record != current_record) {
                        // try to see if we can fit a memory block between the reallocated block and the one after
                        int space_remaining = (char *)current_record->next_record - ((char *)current_record + sizeof(heap_record)*2 + current_record->section_size);
                        //check if a new block can be allocated within remaining space
                        if(space_remaining >= 16) {
                            //create new block within remaining space
                            int new_size = space_remaining-(space_remaining%16); //for example if the space is 65 bytes we allocate 64
                            current_record->next_record = (char *)current_record + sizeof(heap_record) + current_record->section_size;
                            heap_record *new_record = current_record->next_record;
                            new_record->next_record = (char *)current_record + sizeof(heap_record) + old_record_section_size;
                            new_record->section_size = new_size;
                            return (char *)current_record + sizeof(heap_record);
                        }
                    }
                }
            }
            current_record = current_record->next_record;
        }

        if (current_record->free == 1){
            // Check to see if the (last?) freed chunck is the right size
            if (size <= current_record->section_size) {
                //copy data from previous block to new block
                memcpy(current_record, record_to_reallocate, sizeof(heap_record) + record_to_reallocate->section_size);
                current_record->section_size = size;
                current_record->free = 0;
                record_to_reallocate->free=1;
                return (char *)current_record + sizeof(heap_record);
            }
        }

        // At this point, we have checked all existing records and have not found a free one that's big enough.
        if ((void *) ((char *)current_record + calculate_nearest_size(size)) >= (void *) top_of_heap){
            // We have reached the end of the heap, so we must extend it.
            if (brk((char *) top_of_heap + extra_bytes) == -1){
                return NULL;
            }
        }
        // initialize fresh block and duplicate memory
        void *new_record = malloc(size);
        memcpy(new_record, record_to_reallocate, sizeof(heap_record) + record_to_reallocate->section_size);
        //free previous block
        record_to_reallocate->free = 1;
        return new_record;
    } else {
        // The block to reallocate has enough space
        int old_record_section_size = record_to_reallocate->section_size;
        record_to_reallocate->section_size = size;

        if(record_to_reallocate->next_record != record_to_reallocate) {
            // try to see if we can fit a memory block between the reallocated block and the one after
            int space_remaining = (char *)record_to_reallocate->next_record - ((char *)record_to_reallocate + sizeof(heap_record)*2 + record_to_reallocate->section_size);
            //check if a new block can be allocated within remaining space
            if(space_remaining >= 16) {
                //create new block within remaining space
                int new_size = space_remaining-(space_remaining%16); //for example if the space is 65 bytes we allocate 64
                record_to_reallocate->next_record = (char *)record_to_reallocate + sizeof(heap_record) + record_to_reallocate->section_size;
                heap_record *new_record = record_to_reallocate->next_record;
                new_record->next_record = (char *)record_to_reallocate + sizeof(heap_record) + old_record_section_size;
                new_record->section_size = new_size;
                return (char *)record_to_reallocate + sizeof(heap_record);
            }
        }
        return (char *)record_to_reallocate + sizeof(heap_record);
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
