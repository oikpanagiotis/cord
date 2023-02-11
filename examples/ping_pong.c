#include <cord.h>
#include <string.h>

// Function to get content of cord_message_t as a C string
char *get_message_content(cord_t *cord, cord_message_t *message) {
    int allocator_id = *(int *)cord->user_data;

    cord_str_t content = cord_strbuf_to_str(*message->content);
    char *memory = cord_alloc(cord, allocator_id, content.length + 1);
    memset(memory, 0, content.length);
    return memcpy(memory, content.data, content.length);
}

void on_message(cord_t *cord, cord_message_t *message) {
    int allocator_id = *(int *)cord->user_data;

    char *content = get_message_content(cord, message);
    if (strcmp(content, "ping") == 0) {
        cord_send_text(cord, message->guild_id, "Pong!");
    }

    // At the end of each call we clear the allocated memory with a single call
    cord_clear_allocator(cord, allocator_id);
}

int main(void) {
    cord_t *cord = cord_create();
    int allocator_id = cord_get_allocator(cord);

    // Pass allocator_id to user_data in order to access it in on_message
    cord->user_data = &allocator_id;
    cord_on_message(cord, on_message);

    cord_connect(cord, "wss://gateway.discord.gg");
    cord_destroy(cord);
    return 0;
}
