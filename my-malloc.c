#include <unistd.h>
#include <string.h>

static void *bottom;
static int extra = 2;


typedef struct my_struct {
    void *next_record;
    int section_size;
    int free;
} heap_record;

void *malloc(size_t req_size) {
    int round_size = req_size + (req_size % 16);
    if(bottom == NULL){
        bottom = sbrk(round_size + sizeof(heap_record) + extra);
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
    current_record->next_record = (char *) current_record + sizeof(heap_record) + current_record->section_size;
    heap_record *new_record = current_record->next_record;
    new_record->next_record = new_record;
    new_record->section_size = round_size;
    new_record->free = 0;
    return (char *) new_record + sizeof(heap_record);
}

void *calloc(size_t nmemb, size_t size) {

    int round_size = (nmemb*size) + ((nmemb*size) % 16);
    if(bottom == NULL){
        bottom = sbrk(round_size + sizeof(heap_record) + extra);
        heap_record *head = bottom;
        head->next_record = bottom;
        head->section_size = round_size;
        //fill the requested chunk
        void *addr = (char *) head + sizeof(heap_record);
        memset(addr, 0, round_size);
        head->free = 0;
        return (char *) bottom + sizeof(heap_record);
    }
    heap_record *current_record = bottom;
    void *addr = (char *) current_record + sizeof(heap_record);
    while (current_record->next_record != current_record) {
        if (current_record->free == 1){
            // Check to see if the freed chunck is the right size

            if (round_size <= current_record->section_size) {
                current_record->free = 0;
                memset(addr, 0, round_size);
                return (char *) current_record + sizeof(heap_record);
            }
        }
        current_record = current_record->next_record;
    }

    if (current_record->free == 1){
        // Check to see if the freed chunck is the right size

        if (round_size <= current_record->section_size) {
            current_record->free = 0;
            //cast to char pointer for intuitive arithmatic
            memset(addr, 0, round_size);
            return (char *) current_record + sizeof(heap_record);
        }
    }
    // At this point, we have checked all existing records and have not found a free one that's big enough.
    current_record->next_record = (char *) current_record + sizeof(heap_record) + current_record->section_size;
    heap_record *new_record = current_record->next_record;
    new_record->next_record = new_record;
    new_record->section_size = round_size;
    new_record->free = 0;
    memset(addr, 0, round_size);
    return (char *) new_record + sizeof(heap_record);
}

void *realloc(void *ptr, size_t size) {
    // check if size is greater than previous allocation chunk
    heap_record *record_to_reallocate = (void *)((char *) ptr - sizeof(heap_record));

    if(record_to_reallocate->section_size < size) {
        return NULL;
    }
    // reallocate requested chunk
    record_to_reallocate->free = 0;
    record_to_reallocate->section_size = size + (size % 16);
    return (char *) record_to_reallocate + sizeof(heap_record);

}

void free(void *ptr) {
    if(ptr != NULL){
        heap_record *record_to_free = (void *)((char *) ptr - sizeof(heap_record));
        record_to_free->free = 1;
    }
}
