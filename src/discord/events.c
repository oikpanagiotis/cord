#include "events.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "client.h"
#include "serialization.h"

#include <assert.h>
#include <jansson.h>

/*
 *  Dictionary of all the possible discord gateway events
 */
static cord_gateway_event_t gateway_events[] = {
    {"CHANNEL_CREATE", NULL},
    {"CHANNEL_UPDATE", NULL},
    {"CHANNEL_DELETE", NULL},
    {"CHANNEL_PINS_UPDATE", NULL},
    {"GUILD_CREATE", NULL},
    {"GUILD_UPDATE", NULL},
    {"GUILD_DELETE", NULL},
    {"GUILD_BAN_ADD", NULL},
    {"GUILD_BAN_REMOVE", NULL},
    {"GUILD_EMOJIS_UPDATE", NULL},
    {"GUILD_MEMBER_ADD", NULL},
    {"GUILD_MEMBER_UPDATE", NULL},
    {"GUILD_MEMBERS_CHUNK", NULL},
    {"GUILD_ROLE_CREATE", NULL},
    {"GUILD_ROLE_UPDATE", NULL},
    {"GUILD_ROLE_DELETE", NULL},
    {"INVITE_CREATE", NULL},
    {"INVITE_DELETE", NULL},
    {"MESSAGE_CREATE", NULL},
    {"MESSAGE_UPDATE", NULL},
    {"MESSAGE_DELETE_BULK", NULL},
    {"MESSAGE_REACTION_ADD", NULL},
    {"MESSAGE_REACTION_REMOVE", NULL},
    {"MESSAGE_REACTION_REMOVE_ALL", NULL},
    {"MESSAGE_REACTION_REMOVE_EMOJI", NULL},
    {"PRESENCE_UPDATE", NULL},

    {"", NULL}};

bool cord_gateway_event_has_handler(cord_gateway_event_t *event) {
    return event && event->handler;
}

static inline bool is_last_event(cord_gateway_event_t *event) {
    return cstring_is_empty(event->name) && event->handler == NULL;
}

cord_gateway_event_t *get_gateway_event_from_cstring(char *event) {
    cord_gateway_event_t *event_iterator = gateway_events;
    while (!is_last_event(event_iterator)) {
        if (cstring_is_equal(event_iterator->name, event)) {
            return event_iterator;
        }
        event_iterator++;
    }

    return NULL;
}

cord_gateway_event_t *get_gateway_event(gateway_event_t event) {
    i32 event_table_index = (i32)event;
    assert((event_table_index >= 0) &&
           (event_table_index < (i32)GATEWAY_EVENT_COUNT));

    return &gateway_events[event_table_index];
}

static void clear_message_lifecycle_allocator(cord_client_t *client) {
    cord_bump_clear(client->message_lifecycle_allocator);
}

static void log_event(char *event) {
    logger_info("Received event: [%s]", event);
}

void on_channel_create(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_channel_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_channel_delete(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_channel_pins_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_create(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_delete(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_ban_add(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_ban_remove(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_emojis_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_member_add(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_member_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_members_chunk(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_role_create(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_role_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_guild_role_delete(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_invite_create(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_invite_delete(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_create(cord_client_t *client, json_t *data, char *event) {
    cord_t *cord = (cord_t *)client->user_data;
    log_event(event);

    cord_serialize_result_t message =
        cord_message_serialize(data, client->message_lifecycle_allocator);

    if (message.error) {
        logger_error("Failed to serialize message: %s",
                     cord_error(message.error));
        return;
    }

    client->event_callbacks.on_message_cb(cord, message.obj);
    clear_message_lifecycle_allocator(client);
    json_decref(data);
}

void on_message_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_delete_bulk(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_reaction_add(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_reaction_remove(cord_client_t *client,
                                json_t *data,
                                char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_reaction_remove_all(cord_client_t *client,
                                    json_t *data,
                                    char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_message_reaction_remove_emoji(cord_client_t *client,
                                      json_t *data,
                                      char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}

void on_presence_update(cord_client_t *client, json_t *data, char *event) {
    assert(false && "Not Implemented");
    (void)data;
    (void)client;
    (void)event;
}
