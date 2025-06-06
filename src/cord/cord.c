#include "cord.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../core/memory.h"
#include "../discord/client.h"
#include "../discord/serialization.h"
#include "../http/rest.h"

#include <assert.h>
#include <jansson.h>
#include <uwsc/log.h>
#include <uwsc/uwsc.h>

cord_t *cord_create(void) {
    global_logger_init();

    cord_bump_t *application_allocator = cord_bump_create_with_size(MB(8));

    cord_t *cord = balloc(application_allocator, sizeof(cord_t));
    cord->permanent_allocator = application_allocator;
    cord->client = cord_client_create(cord->permanent_allocator);
    cord->client->user_data = cord;

    cord->allocator_count = 0;
    for (i32 i = 0; i < MAX_USER_ALLOCATORS; i++) {
        cord->user_allocators[i] = NULL;
    }
    return cord;
}

void cord_connect(cord_t *cord) {
    cord_client_connect(cord->client);
}

void cord_destroy(cord_t *cord) {
    if (cord) {
        for (i32 i = 0; i < cord->allocator_count; i++) {
            cord_bump_destroy(cord->user_allocators[i]);
        }

        cord_client_destroy(cord->client);
        free(cord);
    }
    global_logger_destroy();

    // destroy permanent allocator last
    cord_bump_destroy(cord->permanent_allocator);
}

static bool is_valid_allocator_id(cord_t *cord, i32 allocator_id) {
    bool is_empty = false;
    bool is_valid_index =
        !(allocator_id < 0 || allocator_id >= MAX_USER_ALLOCATORS);

    if (is_valid_index) {
        is_empty = cord->user_allocators[allocator_id] != NULL;
    }
    return is_valid_index && is_empty;
}

i32 cord_get_allocator(cord_t *cord) {
    if (cord->allocator_count == MAX_USER_ALLOCATORS) {
        logger_error("Failed to create user allocator");
        return -1;
    }

    cord_bump_t *allocator = cord_bump_create_with_size(KB(4));
    if (!allocator) {
        logger_error("Can not create more than %d user allocators",
                     MAX_USER_ALLOCATORS);
        return -1;
    }

    const i32 allocator_id = cord->allocator_count;
    cord->user_allocators[allocator_id] = allocator;
    cord->allocator_count++;
    return allocator_id;
}

void *cord_alloc(cord_t *cord, i32 allocator_id, size_t size) {
    if (!is_valid_allocator_id(cord, allocator_id)) {
        logger_error("allocator with id %d does not exist", allocator_id);
        return NULL;
    }

    cord_bump_t *allocator = cord->user_allocators[allocator_id];
    void *memory = balloc(allocator, size);
    if (!memory) {
        logger_error("allocator with id %d does not have enough free space",
                     allocator_id);
        return NULL;
    }

    return memory;
}

void cord_pop_allocator(cord_t *cord, i32 allocator_id, size_t size) {
    if (!is_valid_allocator_id(cord, allocator_id)) {
        logger_error("allocator with id %d does not exist", allocator_id);
        return;
    }

    cord_bump_t *allocator = cord->user_allocators[allocator_id];
    cord_bump_pop(allocator, size);
}

void cord_clear_allocator(cord_t *cord, i32 allocator_id) {
    if (!is_valid_allocator_id(cord, allocator_id)) {
        logger_error("allocator with id %d does not exist", allocator_id);
        return;
    }

    cord_bump_t *allocator = cord->user_allocators[allocator_id];
    if (!allocator) {
        logger_error("Failed to clear user allocator with id %d", allocator_id);
        return;
    }

    cord_bump_clear(allocator);
}

void cord_destroy_allocator(cord_t *cord, i32 allocator_id) {
    if (!is_valid_allocator_id(cord, allocator_id)) {
        logger_error("allocator with id %d does not exist", allocator_id);
        return;
    }

    i32 allocator_index = allocator_id;
    cord_bump_destroy(cord->user_allocators[allocator_id]);

    i32 allocator_reorder_count = cord->allocator_count - allocator_index - 1;
    assert(allocator_reorder_count >= 0);

    if (allocator_reorder_count > 0) {
        for (u32 i = 0; i < (u32)allocator_reorder_count; i++) {
            u32 empty_slot_index = allocator_id + i;
            u32 current_index = allocator_id + i + 1;
            cord->user_allocators[empty_slot_index] =
                cord->user_allocators[current_index];
        }
        cord->allocator_count--;
    }
}

void cord_on_message(cord_t *cord,
                     void (*on_message_cb)(cord_t *ctx,
                                           cord_bump_t *bump,
                                           cord_message_t *message)) {
    cord->client->event_callbacks.on_message_cb = on_message_cb;
}

void cord_send_text(cord_t *cord, cord_strbuf_t *channel_id, char *message) {
    cord_bump_t *bump = cord_bump_create();
    cord_message_t *msg = balloc(bump, sizeof(cord_message_t));
    cord_strbuf_t *content = cord_strbuf_create();
    cord_strbuf_append(content, cstr(message));
    msg->content = content;
    msg->channel_id = channel_id;
    cord_client_send_message(cord->client, msg);
    cord_bump_destroy(bump);
}

void cord_send_message(cord_t *cord, cord_message_t *message) {
    cord_client_send_message(cord->client, message);
}

cord_user_t *cord_get_current_user(cord_t *cord, cord_bump_t *bump) {
    return cord_api_get_current_user(cord->client->http, bump);
}

cord_str_t cord_message_get_str(cord_message_t *message) {
    return cord_strbuf_to_str(*message->content);
}
