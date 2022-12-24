#include "discord.h"
#include "core/error.h"
#include "events.h"
#include "types.h"
#include "core/log.h"
#include "core/util.h"
#include "core/typedefs.h"
#include "sds.h"

#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#include <ev.h>
#include <assert.h>
#include <uwsc/config.h>

static string_ptr os_name = "Linux";

static void load_identification_info(identification *id) {
	id->token = getenv("CORD_APPLICATION_TOKEN");
	assert(id->token);
	if (!id->token) {
		log_error("Bot token not found. Please set CORD_APPLICATION_TOKEN enviroment variable.");
		exit(1);
	}

	id->os = (char *)os_name;
	id->device = LIBRARY_NAME;
	id->library = LIBRARY_NAME;
}

// Assumes that sequence is initialized to -1
static bool is_valid_sequence(int s) {
	return s >= 0;
}

static f64 heartbeat_to_double(i32 interval) {
	i32 decimal = interval / 1000;
	i32 floating_ms = interval % 1000;

	f64 decimal_d = (f64)decimal;
	f64 floating_d = (f64)floating_ms * 0.001;
	return decimal_d + floating_d;
}

typedef struct json_payload_t {
	char *json_string;
	size_t length;
} json_payload_t;

static bool is_valid_json_payload(json_payload_t payload) {
	return payload.json_string && payload.length > 0;
}

static json_payload_t json_payload_from_json(json_t *json) {
	char *json_string = json_dumps(json, 0);
	if (!json_string) {
		return (json_payload_t){ "", 0 };
	}

	size_t length = strlen(json_string);
	return (json_payload_t){ json_string, length };
}

static void json_payload_destroy(json_payload_t payload) {
	if (payload.json_string) {
		free(payload.json_string);
		payload.json_string = NULL;
	}
}

static void send_json_payload(discord_t *client, json_payload_t payload) {
	client->ws_client->send(client->ws_client, payload.json_string, payload.length, UWSC_OP_TEXT);
}

static json_t *heartbeat_json_object_create(i32 sequence) {
	json_t *heartbeat_object = json_object();
	if (!heartbeat_object) {
		return NULL;
	}
	json_object_set_new(heartbeat_object, PAYLOAD_KEY_OPCODE, json_integer(1));

	// Send sequence number unless we havent received one
	json_t *sequence_object = is_valid_sequence(sequence) ? json_integer(sequence) : json_null();
	json_object_set_new(heartbeat_object, PAYLOAD_KEY_DATA, sequence_object);
	return heartbeat_object;
}

static void send_heartbeat(discord_t *client) {
	json_t *heartbeat_object = heartbeat_json_object_create(client->sequence);
	if (!heartbeat_object) {
		log_error("Failed to create heartbeat json object");
		return;
	}

	json_payload_t heartbeat = json_payload_from_json(heartbeat_object);
	if (!is_valid_json_payload(heartbeat)) {
		log_error("Failed to create json payload for heartbeat");
		return;
	}
	send_json_payload(client, heartbeat);
	client->heartbeat_acknowledged = false;

	json_payload_destroy(heartbeat);
	json_decref(heartbeat_object);
}

static void heartbeat_cb(struct ev_loop *loop, ev_timer *timer, int revents) {
	(void)revents;
	
	struct uwsc_client *ws_client = timer->data;
	discord_t *client = ws_client->ext;

	send_heartbeat(client);
	ev_timer_again(loop, timer);
}

static void sigint_cb(struct ev_loop *loop, ev_signal *w, int revents) {
	(void)revents;

	if (w->signum == SIGINT) {
		ev_break(loop, EVBREAK_ALL);
		// Should we clean up here or call another func after we exit	
		log_debug("Exiting");	
	}
}

static void on_open(struct uwsc_client *ws_client) {
	(void)ws_client;	
	log_debug("Connected");
}

static void on_heartbeat(struct uwsc_client *ws_client, json_t *data) {
	discord_t *client = ws_client->ext;

	json_t *hb_interval_json = json_object_get(data, "heartbeat_interval");
	if (!hb_interval_json) {
		log_error("Failed to get heartbeat object");
		return;
	}
	client->hb_interval = (int)json_integer_value(hb_interval_json);

	// Send heartbeat
	send_heartbeat(client);

	if (!client->sent_initial_heartbeat) {
		log_debug("Sending initial heartbeat");
		client->sent_initial_heartbeat = true;
		
		struct ev_timer *timer_watcher = malloc(sizeof(struct ev_timer));
		// Keep a reference to the watcher so we can free it later
		client->hb_watcher = timer_watcher;
		
		ev_init(timer_watcher, heartbeat_cb);
		timer_watcher->data = ws_client;
		timer_watcher->repeat = heartbeat_to_double(client->hb_interval);
		ev_timer_again(ws_client->loop, timer_watcher);
	}
}

static int send_to_gateway(discord_t *client, json_t *payload) {
	char *buf = json_dumps(payload, 0);
	int len = strlen(buf);
	int rc = client->ws_client->send(client->ws_client, buf, len, UWSC_OP_TEXT);
	free(buf);
	json_decref(payload);
	return rc;
}

static void send_identify(struct uwsc_client *ws_client) {
	discord_t *client = ws_client->ext;

	log_debug("Identifying");
	json_t *payload = json_object();
	json_object_set_new(payload, PAYLOAD_KEY_OPCODE, json_integer(2));
	
	json_t *d = json_object();
	json_object_set_new(d, "token", json_string(client->id.token));
	json_t *properties = json_object();
	json_object_set_new(d, "properties", properties);

	json_object_set_new(properties, "$os", json_string(client->id.os));
	json_object_set_new(properties, "$browser", json_string(client->id.library));
	json_object_set_new(properties, "$device", json_string(client->id.device));

	json_object_set_new(payload, PAYLOAD_KEY_DATA, d);

	send_to_gateway(client, payload);
	log_debug("Sent identify payload");
}

typedef struct gateway_payload {
	int op;		// opcode
	int s;		// sequence
	sds t;		// event
	json_t *d;	// json data
} gateway_payload;

void gateway_payload_init(gateway_payload *payload) {
	payload->op = -1;
	payload->s = -1;
	payload->t = NULL;
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
		payload->t = sdsnew("");
	} else {
		json_t *t = json_object_get(raw_payload, PAYLOAD_KEY_EVENT);
		if (!t) {
			log_error("Failed to get \"t\"");
			return -1;
		}
		payload->t = json_string_value(t) ? sdsnew(json_string_value(t)) : sdsnew("");
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
	int template_len = strlen(json_content_template);

	int len = strlen(content);
	msg->content = calloc(1, len + template_len + 1);
	sprintf(msg->content, json_content_template, content);
}


char *DISCORD_API_URL = "https://discordapp.com/api";
int discord_send_message(discord_t *disc, cord_message_t *msg) {
	assert(msg);
	// TODO: Prefix the channel and ids based on our cache
	const int URL_LEN = 1024;
	char *final_url = calloc(1, sizeof(char) * URL_LEN);
	if (!final_url) {
		log_error("Failed to allocate final url");
		return -1;
	}

	snprintf(final_url, URL_LEN, "%s/channels/%s/messages", DISCORD_API_URL, msg->channel_id);
	http_request_t *req = http_request_create(HTTP_POST, final_url, discord_api_header(disc->http), msg->content);
	http_client_perform_request(disc->http, req);
	return 0;
}						

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
			map_property(msg, id, "id", key, value_copy);
			map_property(msg, content, "content", key, value_copy);
			map_property(msg, channel_id, "channel_id", key, value_copy);
			map_property(msg, guild_id, "guild_id", key, value_copy);
			map_property(msg, timestamp, "timestamp", key, value_copy);
			map_property(msg, nonce, "nonce", key, value_copy);

			// Serialize author object
			if (string_is_equal(key, "author")) {

			}


		} else if (json_is_boolean(value)) {
			bool val = json_boolean_value(value);
			map_property(msg, tts, "tts", key, val);
			map_property(msg, pinned, "pinned", key, val);
			map_property(msg, mention_everyone, "mention_everyone", key, val);

		} else if (json_is_array(value)) {
			// todo
		} else if (json_is_object(value)) {
			// check this out
			// how do we traverse the tree
		} else if (json_is_integer(value)) {
			json_int_t val = json_integer_value(value);
			int num = (int)val; // long long -> int
			map_property(msg, type, "type", key, num);
		} else if (json_is_null(value)) {

		}
	}

	return msg;
}

void discord_message_destroy(cord_message_t *msg) {
	if (msg) {
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
				log_debug("Received Event: %s", event);
				
				cord_gateway_event_t *all_events = get_all_gateway_events();
				cord_gateway_event_t *ev = get_gateway_event(all_events, event);
				if (event_has_handler(ev)) {
					ev->handler(disc, payload->d, event);
				} else {
					log_warning("No handler for event(%s)", event);
				}
			}	
			break;
		case OP_HEARTBEAT:
			log_debug("Server is requesting Hearbeat");
			on_heartbeat(ws_client, payload->d);
			break;
		case OP_RECONNECT:
			log_debug("Server is requesting Reconnect");
			disc->must_reconnect = true;
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
			disc->heartbeat_acknowledged = true;
			break;
		default: // fallthrough
			log_error("Default switch case sentinel");
			break;
	}
	
	if (disc->sent_initial_heartbeat) {
		if (disc->heartbeat_acknowledged) {
			log_debug("heartbeat acknowledged");
		} else {
			log_debug("Zombie");
		}
	}
	
	// json_decref(payload->d);
	free(payload);
}

static void on_error(struct uwsc_client *ws_client, int err, const char *msg) {
	(void)ws_client;

	log_error("Websocket error(%d): %s", err, msg);
}

static void on_close(struct uwsc_client *ws_client, int code, const char *reason) {
	(void)ws_client;
	(void)code;
	(void)reason;

	log_debug("Closing connection");
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
	disc->must_reconnect = false;

	// TODO: Refactor. Move this somewhere else
	// Assign callbacks
	cord_gateway_event_t *all_events = get_all_gateway_events();
	cord_gateway_event_t *on_message_event = get_gateway_event(all_events, "MESSAGE_CREATE");
	on_message_event->handler = on_message_create;

	return disc;
}

static const int ping_interval = 5;

static void client_init(discord_t *client, const char *url) {
	client->ws_client = uwsc_new(client->loop, url, ping_interval, NULL);
	if (!client->ws_client) {
		log_error("Failed to initialize websocket client");
		exit(1);
	}

	client->ws_client->onopen = on_open;
	client->ws_client->onmessage = on_message;
	client->ws_client->onerror = on_error;
	client->ws_client->onclose = on_close;
	// Store the wrapper client pointer to be accessed in the callbacks
	client->ws_client->ext = client;
}

static void client_reconnect_to(discord_t *client, const char *url) {
	// Turn off timers
	ev_timer_stop(client->loop, client->hb_watcher);
	free(client->hb_watcher);
	free(client->ws_client);
	
	client_init(client, url);
	client->sent_initial_heartbeat = false;
	client->must_reconnect = false;

	log_debug("Reconnecting");
	ev_run(client->loop, 0);
}

static void check_reconnect_cb(struct ev_loop *loop, ev_check *w, int revents) {
	(void)loop;
	(void)revents;
	discord_t *client = (discord_t *)w->data;
	assert(client);

	if (client) {
		if (client->must_reconnect) {
			client_reconnect_to(client, "wss://gateway.discord.gg/?v=6&encoding=json");
		}
	}
}

int discord_connect(discord_t *client, const char *url) {
	client->loop = EV_DEFAULT;
	client_init(client, url);
	client->must_reconnect = false;

	log_debug("Connecting");
	struct ev_check *reconnect_watcher = malloc(sizeof(struct ev_check));
	reconnect_watcher->data = client;
	ev_check_init(reconnect_watcher, check_reconnect_cb);
	ev_check_start(client->loop, reconnect_watcher);
	client->reconnect_watcher = reconnect_watcher;

	struct ev_signal *sigint_watcher = malloc(sizeof(struct ev_signal));
	ev_signal_init(sigint_watcher, sigint_cb, SIGINT);
	ev_signal_start(client->loop, sigint_watcher);
	client->sigint_watcher = sigint_watcher;
	ev_run(client->loop, 0);
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

