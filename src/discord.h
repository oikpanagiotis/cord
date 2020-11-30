#ifndef DISCORD_H
#define DISCORD_H

#include <stdbool.h>
#include <stdlib.h>
#include <uwsc/uwsc.h>
#include <ev.h>

#include "http.h"

#define PAYLOAD_KEY_OPCODE "op"
#define PAYLOAD_KEY_DATA "d"
#define PAYLOAD_KEY_SEQUENCE "s"
#define PAYLOAD_KEY_EVENT "t"

#define OP_DISPATCH 0
#define OP_HEARTBEAT 1
#define OP_IDENTIFY 2
#define OP_PRESENCE_UPDATE 3
#define OP_VOICE_STATUS_UPDATE 4
#define OP_RESUME 6
#define OP_RECONNECT 7
#define OP_REQUEST_GUILD_MEMBERS 8
#define OP_INVALID_SESSION 9
#define OP_HELLO 10
#define OP_HEARTBEAT_ACK 11

typedef struct discord_author_t {
	char *name;
	char *id;
	char *discriminator;
	char *avatar;
	int public_flags;
} discord_author_t;

typedef struct discord_message_t {
	int type;
	bool tts;
	char *id;
	char *content;
	char *channel_id;
	char *guild_id;
	char *timestamp;
	void *edited_timestamp;
	void *referenced_message;
	bool pinned;
	char *nonce;
	void *mentions;
	void *mention_roles;
	bool mention_everyone;
	int flags;
	void *embeds;
	void *attachments;
} discord_message_t;

typedef struct discord_guild_t {
	char *id; // snowflake
	char *name;
	char *icon; // string/null
	char *splash; //string/null
	char *discovery_splash; // string/null
} discord_guild_t;

// Load these from enviroment variables
typedef struct identification {
	char *token;
	char *os;
	char *library;
	char *device;
} identification;

enum {
	ACTION_NONE,
	ACTION_SEND_MESSAGE,

	ACTION_COUNT
};

typedef struct discord_t discord_t;
typedef void (*on_msg)(discord_t *disc, discord_message_t *msg);

typedef struct discord_event_t {
	int type;
	char *token;
	int token_length;

} discord_event_t;

typedef struct discord_t {
	// should be the first field in the struct	
	struct uwsc_client *ws_client;
	struct ev_loop *loop;
	struct ev_timer *hb_watcher;
	struct ev_signal *sigint_watcher;

	int hb_interval;
	int sequence;
	bool sent_initial_heartbeat;

	identification id;
	http_client_t *http;

	on_msg on_message_callback;
} discord_t;

discord_t *discord_create(void);
// do these need to be in a header file?
int discord_connect(discord_t *disc, const char *url);
void discord_destroy(discord_t *disc);

int discord_send_message(discord_t *disc, discord_message_t *msg);
void discord_message_set_content(discord_message_t *msg, char *content);

void discord_message_destroy(discord_message_t *msg);
// Events
enum {
	CHANNEL_CREATE,
	MESSAGE_CREATE,

	EVENT_COUNT
};

#endif
