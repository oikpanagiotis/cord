#ifndef CORD_H
#define CORD_H

#include "../core/log.h"
#include "../core/memory.h"

#include "../discord/client.h"

#define MAX_USER_ALLOCATORS 6

typedef i32 cord_allocator_id_t;

typedef struct cord_t {
    cord_client_t *client;
    cord_logger_t *logger;
    cord_bump_t *user_allocators[MAX_USER_ALLOCATORS];
    i32 allocator_count;
    cord_bump_t *permanent_allocator;
    void *user_data;
} cord_t;

cord_t *cord_create(void);
void cord_connect(cord_t *cord);
void cord_destroy(cord_t *cord);

cord_allocator_id_t cord_get_allocator(cord_t *cord);
void *cord_alloc(cord_t *cord, cord_allocator_id_t id, size_t size);
void cord_pop_allocator(cord_t *cord, cord_allocator_id_t id, size_t size);
void cord_clear_allocator(cord_t *cord, cord_allocator_id_t id);
void cord_destroy_allocator(cord_t *cord, cord_allocator_id_t id);

void cord_on_message(cord_t *cord,
                     void (*on_message_cb)(cord_t *ctx,
                                           cord_bump_t *bump,
                                           cord_message_t *message));

void cord_send_text(cord_t *cord, cord_strbuf_t *guild_id, char *message);
void cord_send_message(cord_t *cord, cord_message_t *message);

cord_user_t *cord_get_current_user(cord_t *cord, cord_bump_t *bump);
cord_str_t cord_message_get_str(cord_message_t *message);

#endif
