#ifndef EVENTS_H
#define EVENTS_H

#include "client.h"
#include <jansson.h>

typedef enum gateway_event_t {
    GATEWAY_EVENT_CHANNEL_CREATE,
    GATEWAY_EVENT_CHANNEL_UPDATE,
    GATEWAY_EVENT_CHANNEL_DELETE,
    GATEWAY_EVENT_CHANNEL_PINS_UPDATE,
    GATEWAY_EVENT_GUILD_CREATE,
    GATEWAY_EVENT_GUILD_UPDATE,
    GATEWAY_EVENT_GUILD_DELETE,
    GATEWAY_EVENT_GUILD_BAN_ADD,
    GATEWAY_EVENT_GUILD_BAN_REMOVE,
    GATEWAY_EVENT_GUILD_EMOJIS_UPDATE,
    GATEWAY_EVENT_GUILD_MEMBER_ADD,
    GATEWAY_EVENT_GUILD_MEMBER_UPDATE,
    GATEWAY_EVENT_GUILD_MEMBERS_CHUNK,
    GATEWAY_EVENT_GUILD_ROLE_CREATE,
    GATEWAY_EVENT_GUILD_ROLE_UPDATE,
    GATEWAY_EVENT_GUILD_ROLE_DELETE,
    GATEWAY_EVENT_INVITE_CREATE,
    GATEWAY_EVENT_INVITE_DELETE,
    GATEWAY_EVENT_MESSAGE_CREATE,
    GATEWAY_EVENT_MESSAGE_UPDATE,
    GATEWAY_EVENT_MESSAGE_DELETE_BULK,
    GATEWAY_EVENT_MESSAGE_REACTION_ADD,
    GATEWAY_EVENT_MESSAGE_REACTION_REMOVE,
    GATEWAY_EVENT_MESSAGE_REACTION_REMOVE_ALL,
    GATEWAY_EVENT_MESSAGE_REACTION_REMOVE_EMOJI,
    GATEWAY_EVENT_PRESENCE_UPDATE,

    GATEWAY_EVENT_COUNT
} gateway_event_t;

typedef void (*event_handler)(cord_client_t *client, json_t *data, char *event);

#define MAX_EVENT_NAME_LEN 64
typedef struct cord_gateway_event_t {
    char name[MAX_EVENT_NAME_LEN];
    event_handler handler;
} cord_gateway_event_t;

void on_channel_create(cord_client_t *client, json_t *data, char *event);
void on_channel_update(cord_client_t *client, json_t *data, char *event);
void on_channel_delete(cord_client_t *client, json_t *data, char *event);
void on_channel_pins_update(cord_client_t *client, json_t *data, char *event);
void on_guild_create(cord_client_t *client, json_t *data, char *event);
void on_guild_update(cord_client_t *client, json_t *data, char *event);
void on_guild_delete(cord_client_t *client, json_t *data, char *event);
void on_guild_ban_add(cord_client_t *client, json_t *data, char *event);
void on_guild_ban_remove(cord_client_t *client, json_t *data, char *event);
void on_guild_emojis_update(cord_client_t *client, json_t *data, char *event);
void on_guild_member_add(cord_client_t *client, json_t *data, char *event);
void on_guild_member_update(cord_client_t *client, json_t *data, char *event);
void on_guild_members_chunk(cord_client_t *client, json_t *data, char *event);
void on_guild_role_create(cord_client_t *client, json_t *data, char *event);
void on_guild_role_update(cord_client_t *client, json_t *data, char *event);
void on_guild_role_delete(cord_client_t *client, json_t *data, char *event);
void on_invite_create(cord_client_t *client, json_t *data, char *event);
void on_invite_delete(cord_client_t *client, json_t *data, char *event);
void on_message_create(cord_client_t *client, json_t *data, char *event);
void on_message_update(cord_client_t *client, json_t *data, char *event);
void on_message_delete_bulk(cord_client_t *client, json_t *data, char *event);
void on_message_reaction_add(cord_client_t *client, json_t *data, char *event);
void on_message_reaction_remove(cord_client_t *client, json_t *data,
                                char *event);
void on_message_reaction_remove_all(cord_client_t *client, json_t *data,
                                    char *event);
void on_message_reaction_remove_emoji(cord_client_t *client, json_t *data,
                                      char *event);
void on_presence_update(cord_client_t *client, json_t *data, char *event);

bool cord_gateway_event_has_handler(cord_gateway_event_t *event);
cord_gateway_event_t *get_gateway_event_from_cstring(char *event_name);
cord_gateway_event_t *get_gateway_event(gateway_event_t event);

#endif
