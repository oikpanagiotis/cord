#include "cord.h"
#include "../core/errors.h"
#include "../core/log.h"
#include "../discord/client.h"

#include <jansson.h>
#include <uwsc/uwsc.h>

cord_t *cord_create(void) {
    // TODO: Move this to a global context initialization routine
    global_logger_init();

    cord_t *cord = malloc(sizeof(cord_t));
    cord->gateway_client = discord_create();
    return cord;
}

void cord_connect(cord_t *cord, const char *url) {
    discord_connect(cord->gateway_client, url);
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
                     void (*on_message_cb)(cord_message_t *message)) {
    cord->gateway_client->event_callbacks.on_message_cb = on_message_cb;
}