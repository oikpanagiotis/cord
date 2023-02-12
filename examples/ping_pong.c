#include <cord.h>
#include <stdlib.h>
#include <string.h>

// Function to get content of cord_message_t as a C string
char *get_message_content(cord_t *cord, cord_message_t *message) {
    cord_str_t content = cord_strbuf_to_str(*message->content);
    char *memory = calloc(1, content.length + 1);
    return memcpy(memory, content.data, content.length);
}

void on_message(cord_t *cord, cord_message_t *message) {
    char *content = get_message_content(cord, message);

    if (strcmp(content, "ping") == 0) {
        cord_send_text(cord, message->channel_id, "Pong!");
    }

    free(content);
}

int main(void) {
    cord_t *cord = cord_create();
    cord_on_message(cord, on_message);

    cord_connect(cord, "wss://gateway.discord.gg");
    cord_destroy(cord);
    return 0;
}
