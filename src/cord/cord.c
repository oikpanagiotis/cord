#include "cord.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../discord/client.h"

#include <assert.h>
#include <jansson.h>
#include <uwsc/uwsc.h>

cord_t *cord_create(void) {
    // TODO: Move this to a global context initialization routine
    global_logger_init();

    cord_t *cord = malloc(sizeof(cord_t));
    cord->gateway_client = discord_create();
    cord->gateway_client->user_data = cord;
    return cord;
}

void cord_connect(cord_t *cord, const char *url) {
    cord_client_connect(cord->gateway_client, url);
}

void cord_destroy(cord_t *cord) {
    if (cord) {
        if (cord->gateway_client) {
            discord_destroy(cord->gateway_client);
        }
        free(cord);
    }
    global_logger_destroy();
}

void cord_on_message(cord_t *cord,
                     void (*on_message_cb)(cord_t *ctx,
                                           cord_message_t *message)) {
    cord->gateway_client->event_callbacks.on_message_cb = on_message_cb;
}

void cord_send_text(cord_t *cord, char *message) {
    (void)cord;
    (void)message;
    assert(NULL && "Implement this");
}

void cord_send_message(cord_t *cord, cord_message_t *message) {
    (void)cord;
    (void)message;
    assert(NULL && "Implement this");
}
