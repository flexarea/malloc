#include <unistd.h>
#include <string.h>

static void *bottom;
static int extra = 10000;


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
        if (current_record->free != 0){
            // Check to see if the freed chunck is the right size
            if (current_record->section_size <= req_size) {
                current_record->free = 0;
                //cast to char pointer for intuitive arithmatic
                return (char *) current_record + sizeof(heap_record);
            }
        }
        current_record = current_record->next_record;
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
    return NULL;
}

void *realloc(void *ptr, size_t size) {
    return NULL;
}

void free(void *ptr) {
    heap_record *record_to_free = (void *)((char *) ptr - sizeof(heap_record));
    record_to_free->free = 1;
}