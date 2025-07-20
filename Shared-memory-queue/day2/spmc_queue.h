#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define SPMC_QUEUE_VERSION 0

enum spmc_mode {
    spmc_mode_reader = 0,
    spmc_mode_writer
};

typedef struct spmc_header {
    int fd;
    char *path;
    enum spmc_mode mode;
    size_t shared_size;
} spmc_header_t;

typedef struct spmc_shared {
    uint8_t version;
    size_t element_capacity;
    size_t element_size;
    bool initialized;
    bool client_connected;
    int64_t writer_idx;
    int64_t reader_idx;
    uint8_t data[];
} spmc_shared_t;

typedef struct spmc_queue {
    spmc_header_t header;
    spmc_shared_t *shared;
} spmc_queue_t;


spmc_queue_t *spmc_queue_create(const char *path, size_t element_size, size_t element_capacity, enum spmc_mode mode);
void spmc_queue_destroy(spmc_queue_t *queue);
bool spmc_queue_enqueue(spmc_queue_t *queue, uint8_t *src_data);
bool spmc_queue_dequeue(spmc_queue_t *queue, uint8_t *dst_data);
