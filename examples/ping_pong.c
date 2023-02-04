#include <stdlib.h>
#include <string.h>
#include <cord.h>

static char *get_message_content(cord_message_t *message) {
	return cord_strbuf_to_cstring(message->content);
}

static bool string_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}

void on_message(cord_message_t *message) {
	if (string_equals(get_message_content(message), "ping")) {
		// Allocate a response
		// cord_message_t *response = malloc(sizeof(cord_message_t));
		// response->channel_id = calloc(1, 256);
		// strcpy((char*)response->channel_id, (char*)msg->channel_id);

		// Write & send the message
		// discord_message_set_content(response, "Pong!");
		// discord_send_message(disc, response);
		// discord_message_destroy(response);
	}
}

int main(void) {
	const char *url = "wss://gateway.discord.gg/?v=6&encoding=json";
	cord_t *context = cord_create();

	cord_on_message(context, on_message);

	cord_connect(context, url);
	cord_destroy(context);
	return 0;
}
