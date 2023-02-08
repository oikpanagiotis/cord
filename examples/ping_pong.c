#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cord.h>

static char *get_message_content(cord_message_t *message) {
	return cord_strbuf_to_cstring(message->content);
}

static bool string_equals(const char *s1, const char *s2) {
	return strcmp(s1, s2) == 0;
}

void on_message(cord_t *ctx, cord_message_t *message) {
	printf("%.*s\n", message->content->length, message->content->data);
	if (string_equals(get_message_content(message), "ping")) {
		cord_send_text(ctx, "Pong!");
	}
}

int main(void) {
	const char *url = "wss://gateway.discord.gg";
	cord_t *context = cord_create();

	cord_on_message(context, on_message);

	cord_connect(context, url);
	cord_destroy(context);
	return 0;
}
