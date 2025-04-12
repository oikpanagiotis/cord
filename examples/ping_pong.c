#include <cord.h>

void on_message(cord_t *cord, cord_bump_t *bump, cord_message_t *message) {
    cord_str_t content = cord_message_get_str(message);

    if (cord_str_equals(content, cstr("ping"))) {
        cord_send_text(cord, message->channel_id, "Pong!");
    }

    if (cord_str_equals(content, cstr("me"))) {
        cord_user_t *user = cord_get_current_user(cord, bump);
        char *id = cord_strbuf_to_cstring(*user->id);
        char *username = cord_strbuf_to_cstring(*user->username);
        logger_info("User{id: %s, name: %s}", id, username);
    }
}

int main(void) {
    cord_t *cord = cord_create();
    cord_on_message(cord, on_message);

    cord_connect(cord);
    cord_destroy(cord);
    return 0;
}
