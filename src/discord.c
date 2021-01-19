#include "discord.h"
#include "events.h"
#include "types.h"
#include "log.h"
#include "util.h"
#include "constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <ev.h>
#include <assert.h>

static char *os_name = "Linux";

static void load_identification_info(identification *id) {
	id->token = getenv("CORD_APPLICATION_TOKEN");
	assert(id->token);
	if (!id->token) {
		log_error("Bot token not found. Please set CORD_APPLICATION_TOKEN enviroment variable.");
		exit(1);
	}

	// TODO: Implement os detection
	id->os = os_name;
	id->device = LIB_NAME;
	id->library = LIB_NAME;
}

// Assumes that sequence is initialized to -1
static bool is_valid_sequence(int s) {
	return (s >= 0) ? true : false;
}

static void free_buf_and_json(char *buf, json_t *json) {
	free(buf);
	json_decref(json);
}

static double heartbeat_to_double(int interval) {
	int decimal = interval / 1000;
	int floating_ms = interval % 1000;

	double decimal_d = (double)decimal;
	double floating_d = (double)floating_ms * 0.001;
	return decimal_d + floating_d;
}

static void heartbeat_cb(struct ev_loop *loop, ev_timer *w, int revents) {
	(void)revents;
	
	struct uwsc_client *ws_client = w->data;
	discord_t *disc = ws_client->ext;

	json_t *hb_json = json_object();
	json_object_set_new(hb_json, PAYLOAD_KEY_OPCODE, json_integer(1));
	if (is_valid_sequence(disc->sequence)) {
		json_object_set_new(hb_json, PAYLOAD_KEY_DATA, json_integer(disc->sequence));
	} else {
		json_object_set_new(hb_json, PAYLOAD_KEY_DATA, json_null());
	}	
	char *buf = json_dumps(hb_json, 0);
	int buflen = strlen(buf);
	ws_client->send(ws_client, buf, buflen, UWSC_OP_TEXT);
	ev_timer_again(loop, w);
	
	free_buf_and_json(buf, hb_json);
}

static void sigint_cb(struct ev_loop *loop, ev_signal *w, int revents) {
	(void)revents;

	if (w->signum == SIGINT) {
		ev_break(loop, EVBREAK_ALL);
		// Should we clean up here or call another func after we exit	
		log_info("Exiting");	
	}
}

static void on_open(struct uwsc_client *ws_client) {
	(void)ws_client;	
	log_info("Connected");
}

// TODO: Should we pass the buffer directly instead of the payload?
static void on_heartbeat(struct uwsc_client *ws_client, json_t *d_json) {
	discord_t *disc = ws_client->ext;

	json_t *hb_interval_json = json_object_get(d_json, "heartbeat_interval");
	if (!hb_interval_json) {
		log_error("Failed to get heartbeat object");
		return;
	}
	disc->hb_interval = (int)json_integer_value(hb_interval_json);

	json_t *hb_json = json_object();
	json_object_set_new(hb_json, PAYLOAD_KEY_OPCODE, json_integer(1));
	// Send sequence number unless we havent received one
	if (is_valid_sequence(disc->sequence)) {
		json_object_set_new(hb_json, PAYLOAD_KEY_DATA, json_integer(disc->sequence));
	} else {
		json_object_set_new(hb_json, PAYLOAD_KEY_DATA, json_null());
	}	

	char *buf = json_dumps(hb_json, 0);
	int buflen = strlen(buf);
	// Send heartbeat
	ws_client->send(ws_client, buf, buflen, UWSC_OP_TEXT);
	free_buf_and_json(buf, hb_json);

	if (!disc->sent_initial_heartbeat) {
		log_info("Sending initial heartbeat");
		disc->sent_initial_heartbeat = true;
		
		struct ev_timer *timer_watcher = malloc(sizeof(struct ev_timer));
		// Keep a reference to the watcher so we can free it later
		disc->hb_watcher = timer_watcher;
		
		ev_init(timer_watcher, heartbeat_cb);
		timer_watcher->data = ws_client;
		timer_watcher->repeat = heartbeat_to_double(disc->hb_interval);
		ev_timer_again(ws_client->loop, timer_watcher);
	}
}

static void send_identify(struct uwsc_client *ws_client) {
	discord_t *disc = ws_client->ext;

	log_info("Identifying");
	json_t *payload = json_object();
	json_object_set_new(payload, PAYLOAD_KEY_OPCODE, json_integer(2));
	
	json_t *d = json_object();
	json_object_set_new(d, "token", json_string(disc->id.token));
	json_t *properties = json_object();
	json_object_set_new(d, "properties", properties);

	json_object_set_new(properties, "$os", json_string(disc->id.os));
	json_object_set_new(properties, "$browser", json_string(disc->id.library));
	json_object_set_new(properties, "$device", json_string(disc->id.device));

	json_object_set_new(payload, PAYLOAD_KEY_DATA, d);

	char *buf = json_dumps(payload, 0);
	int len = strlen(buf);
	ws_client->send(ws_client, buf, len, UWSC_OP_TEXT);
	free_buf_and_json(buf, payload);
	log_info("Sent identify payload");
}

typedef struct gateway_payload {
	int op;		// opcode
	int s;		// sequence
	char t[64];	// event
	json_t *d;	// json data
} gateway_payload;

void gateway_payload_init(gateway_payload *payload) {
	payload->op = -1;
	payload->s = -1;
	memset(payload->t, 0, 64);	
	payload->d = NULL;
}

// parse_gatway_payload() makes a copy of the payload data field
// so the user is safe to free the original json payload
int parse_gatway_payload(json_t *raw_payload, gateway_payload *payload) {
	json_t *opcode = json_object_get(raw_payload, PAYLOAD_KEY_OPCODE);
	if (!opcode) {
		log_error("Failed to get \"op\"");
		return -1;
	}

	json_int_t num_opcode = json_integer_value(opcode);
	if (num_opcode != OP_DISPATCH) {
		strcpy(payload->t, "");
	} else {
		json_t *t = json_object_get(raw_payload, PAYLOAD_KEY_EVENT);
		if (!t) {
			log_error("Failed to get \"t\"");
			return -1;
		}
		strcpy(payload->t, (json_string_value(t)) ? json_string_value(t) : "");
	}
	payload->op = (int)num_opcode;

	json_t *s = json_object_get(raw_payload, PAYLOAD_KEY_SEQUENCE);
	if (s) {
		payload->s = json_integer_value(s);
	}
	
	json_t *d = json_object_get(raw_payload, PAYLOAD_KEY_DATA);
	if (!d) {
		log_error("Failed to get \"d\"");
		return -1;	
	}
	payload->d = json_deep_copy(d);
	return 0;
}

void discord_message_set_content(cord_message_t *msg, char *content) {
	char *json_content_template = "{\"content\": \"%s\"}";

	int len = strlen(content);
	msg->content = calloc(1, len + 1);
	sprintf(msg->content, json_content_template, content);
}


char *DISCORD_API_URL = "https://discordapp.com/api";
int discord_send_message(discord_t *disc, cord_message_t *msg) {
	// TODO: Prefix the channel and ids based on our cache
	const int URL_LEN = 512;
	char *final_url = calloc(1, URL_LEN);
	if (!final_url) {
		log_error("Failed to allocate final url");
		return -1;
	}
	snprintf(final_url, URL_LEN, "%s/channels/%s/messages", DISCORD_API_URL, msg->channel_id);
	http_request_t *req = http_request_create(HTTP_POST, final_url, discord_api_header(disc->http), msg->content);
	http_client_perform_request(disc->http, req);
	return 0;
}

#define M_msg_prop(msg, prop, prop_str, key, val) \
	do { \
		if (string_is_equal(key, prop_str)) { \
			msg->prop = val; \
		} \
} while (0)								

// Used from events module
cord_message_t *message_from_json(json_t *data) {
	cord_message_t *msg = malloc(sizeof(cord_message_t));
	if (!msg) {
		log_error("Failed to allocate discord message");
		return NULL;
	}

	const char *key = NULL;
	json_t *value = NULL;

	json_object_foreach(data, key, value) {
		if (json_is_string(value)) {
			const char *str = json_string_value(value);
			
			char *value_copy = strdup(str);
			M_msg_prop(msg, id, "id", key, value_copy);
			M_msg_prop(msg, content, "content", key, value_copy);
			M_msg_prop(msg, channel_id, "channel_id", key, value_copy);
			M_msg_prop(msg, guild_id, "guild_id", key, value_copy);
			M_msg_prop(msg, timestamp, "timestamp", key, value_copy);
			M_msg_prop(msg, nonce, "nonce", key, value_copy);

			// Serialize author object
			if (string_is_equal(key, "author")) {

			}


		} else if (json_is_boolean(value)) {
			bool val = json_boolean_value(value);
			M_msg_prop(msg, tts, "tts", key, val);
			M_msg_prop(msg, pinned, "pinned", key, val);
			M_msg_prop(msg, mention_everyone, "mention_everyone", key, val);

		} else if (json_is_array(value)) {
			// todo
		} else if (json_is_object(value)) {
			// check this out
			// how do we traverse the tree
		} else if (json_is_integer(value)) {
			json_int_t val = json_integer_value(value);
			int num = (int)val; // long long -> int
			M_msg_prop(msg, type, "type", key, num);
		} else if (json_is_null(value)) {

		}
	}

	return msg;
}

void discord_message_destroy(cord_message_t *msg) {
	if (msg) {
		if (msg->id) free(msg->id);
		if (msg->content) free(msg->content);

		free(msg);
	}
}

static void on_message(struct uwsc_client *ws_client, void *data, size_t len, bool binary) {
	(void)binary;

	discord_t *disc = ws_client->ext;

	json_error_t err = {0};	
	// Serialize payload
	json_t *raw_payload = json_loadb(data, len, 0, &err);
	if (!raw_payload) {
		log_error("Failed to serialize payload: %s", err.text);
		return;
	}
	gateway_payload *payload = malloc(sizeof(gateway_payload));
	gateway_payload_init(payload);	

	int rc = parse_gatway_payload(raw_payload, payload);
	if (rc < 0) {
		log_error("Failed to parse gateway payload");
	}
	// When the reference count of a json object reaches decreases
	// by one, the reference count of all it's children is also
	// decreased by 1, so the whole object tree can be cleaned up
	json_decref(raw_payload);

	char *event = payload->t;
	switch (payload->op) {
		// Dispatch is returned if we've received an event from the gateway	
		case OP_DISPATCH:
			if (!string_is_empty(event)) {
				log_info("Received Event: %s", event);
				
				receiving_event *all_events = get_all_receicing_events();
				receiving_event *ev = get_receiving_event(all_events, event);
				if (event_has_handler(ev)) {
					ev->handler(disc, payload->d, event);
				} else {
					log_warning("Invalid handler for event(%s)", event);
				}
			}	
			break;
		case OP_HEARTBEAT:
			log_info("Server is requesting Hearbeat");
			break;
		case OP_RECONNECT:
			log_info("Server is requesting Reconnect");
			break;
		case OP_INVALID_SESSION:
			log_warning("Invalid Session");
			break;
		case OP_HELLO:
			if (!disc->sent_initial_heartbeat) {
				on_heartbeat(ws_client, payload->d);
				send_identify(ws_client);
			} else {
				on_heartbeat(ws_client, payload->d);
			}
			break;
		case OP_HEARTBEAT_ACK:
			break;
		default: // fallthrough
			log_error("Default switch case sentinel");
			break;
	}
	
	// json_decref(payload->d);
	free(payload);
}

static void on_error(struct uwsc_client *ws_client, int err, const char *msg) {
	(void)ws_client;
	(void)err;
	(void)msg;

	log_error("Connection error");
}

static void on_close(struct uwsc_client *ws_client, int code, const char *reason) {
	(void)ws_client;
	(void)code;
	(void)reason;

	log_info("Closing connection");
}

discord_t *discord_create(void) {
	identification id = {0};
	load_identification_info(&id);

	discord_t *disc = malloc(sizeof(discord_t));
	if (!disc) {
		log_error("Failed to allocate discord context");
		return NULL;
	}

	disc->http = http_client_create(id.token);
	if (!disc->http->bot_token) {
		log_error("Failed to create bot token");
		free(disc);
		return NULL;
	}

	if (!disc->http) {
		log_error("Failed to create http client");
		free(disc);
		return NULL;
	}
	disc->id = id;
	disc->hb_interval = -1;
	disc->sequence = -1;
	disc->sent_initial_heartbeat = false;
	disc->loop = NULL;

	// TODO: Move this somewhere else
	// Assign callbacks
	receiving_event *all_events = get_all_receicing_events();
	receiving_event *on_message_event = get_receiving_event(all_events, "MESSAGE_CREATE");
	on_message_event->handler = on_message_create;

	return disc;
}

int discord_connect(discord_t *disc, const char *url) {
	const int ping_interval = 5;
	disc->loop = EV_DEFAULT;
	disc->ws_client = uwsc_new(disc->loop, url, ping_interval, NULL);

	disc->ws_client->onopen = on_open;
	disc->ws_client->onmessage = on_message;
	disc->ws_client->onerror = on_error;
	disc->ws_client->onclose = on_close;
	disc->ws_client->ext = disc;

	log_info("Connecting");
	struct ev_signal *sigint_watcher = malloc(sizeof(struct ev_signal));
	ev_signal_init(sigint_watcher, sigint_cb, SIGINT);
	ev_signal_start(disc->loop, sigint_watcher);
	disc->sigint_watcher = sigint_watcher;
	ev_run(disc->loop, 0);
	return 0;
}

void discord_destroy(discord_t *disc) {
	if (disc) {
		if (disc->ws_client) {
			free(disc->ws_client);
		}
		if (disc->hb_watcher) {
			free(disc->hb_watcher);
		}
		if (disc->sigint_watcher) {
			free(disc->sigint_watcher);
		}
		if (disc->http) {
			http_client_destroy(disc->http);
		}
		free(disc);
	}
}

