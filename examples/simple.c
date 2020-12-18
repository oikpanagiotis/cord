#include <stdlib.h>
#include <string.h>
#include <cord.h>

void on_message(discord_t *disc, discord_message_t *msg) {
	if (strcmp(msg->content, "ping") == 0) {
		// Allocate a response
		discord_message_t *response = malloc(sizeof(discord_message_t));
		response->channel_id = calloc(1, 256);
		strcpy(response->channel_id, msg->channel_id);

		// Write & send the message
		discord_message_set_content(response, "Pong!");
		discord_send_message(disc, response);

		// Free the message's memory
		discord_message_destroy(response);
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
