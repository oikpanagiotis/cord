#include <string.h>
#include <stdbool.h>
#include <cord.h>

// Example code for a bot application that supports polls and votes
// The error handling code has been removed for simplicity purposes

#define URL "wss://gateway.discord.gg/?v=6&encoding=json"
enum {
    POLL,
    RESULTS,
    VOTE,
};

typedef struct poll {
    int id;
    char first_choice[64];
    char second_choice[64];
    int first_choice_vote_count;
    int second_choice_vote_count;
} poll;

typedef struct simple_bot {
    cord_t *cord_context;
    poll *active_poll;

    void (*on_message)(struct simple_bot *self, char *msg, char *channel);
} simple_bot;

static bool string_equals(const char *s1, const char *s2) {
    if (strcmp(s1, s2) == 0) {
        return true;
    } else {
        return false;
    }
}

static bool is_command(char *str, int *type) {
    char keyword[32] = {0};
    for (int i = 0; i < 32; i++) {
        if (str[i] == ' ') {
            break;
        }

        keyword[i] = str[i];
    }

    printf("Keyword: %s\n\n", keyword);

    if (string_equals(keyword, "poll")) {
        *type = POLL;
        printf("POLL\n");
        return true;
    } else if (string_equals(keyword, "result")) {
        printf("RESULT\n");
        *type = RESULTS;
        return true;
    } else if (string_equals(keyword, "1") || string_equals(keyword, "2")) {
        printf("VOTE\n");
        *type = VOTE;
        return true;
    }

    return false;
}

static void parse_poll(char *str, poll *out) {
    (void)out;
    
    poll *new_poll = malloc(sizeof(poll));

    char *a = NULL;
    char *b = NULL;

    for (int i = 0; i < 160; i++) {
        if (str[i] == ' ' && a == NULL) {
            a = str + i + 1;
        }

        if (str[i] == ' ' && a != NULL) {
            b = str + i + 1;
            break;
        }
    }

    char *iter = a;
    int i = 0;
    while (iter[i] != ' ') {
        new_poll->first_choice[i] = iter[i];
        i++;
    }

    iter = b;
    i = 0;
    while (iter[i] != ' ') {
        new_poll->second_choice[i] = iter[i];
        i++;
    }

    new_poll->first_choice_vote_count = 0;
    new_poll->second_choice_vote_count = 0;
    out = new_poll;
}

static void show_results(simple_bot *bot, char *channel) {
    cord_message_t *results = malloc(sizeof(cord_message_t));
	results->channel_id = calloc(1, sizeof(char) * 512);
	strcpy(results->channel_id, channel);
    char *buf = calloc(1, sizeof(char) * 1024);
    poll *p = bot->active_poll;
    snprintf(buf, 1024, "Poll Results: %s: %d    %s: %d",
            p->first_choice, p->first_choice_vote_count,
            p->second_choice, p->second_choice_vote_count);

	discord_message_set_content(results, buf);
	discord_send_message(bot->cord_context->client, results);
    discord_message_destroy(results);
}

static void update_vote(simple_bot *bot, char *vote) {
    if (string_equals(vote, "1")) {
        bot->active_poll->first_choice_vote_count++;
    } else if (string_equals(vote, "2")) {
        bot->active_poll->second_choice_vote_count++;
    }
}

void simple_bot_on_message(simple_bot *bot, char *msg, char *channel) {
    int type = 0;
    if (is_command(msg, &type)) {
        if (type == POLL) {
            free(bot->active_poll);
            printf("Parsing poll\n");
            parse_poll(msg, bot->active_poll);
        } else if (type == RESULTS) {
            printf("result\n");
            show_results(bot, channel);
        } else if (type == VOTE) {
            printf("Voting\n");
            update_vote(bot, msg);
        }
    }
}

void on_message(discord_t *client, cord_message_t *msg) {
    // We can cast discord_t/cord_t to our custom type
    // as long as we put it at the top of the struct
    simple_bot *bot = (simple_bot *)client->user_data;

    // Wrapper to seperate our application's callback
    // from the library's callback
    simple_bot_on_message(bot, msg->content, msg->channel_id);
}

void simple_bot_configure(simple_bot *bot) {
    bot->on_message = simple_bot_on_message;
    bot->cord_context->client->user_data = bot;
    cord_on_message(bot->cord_context, on_message);
}

void simple_bot_start(simple_bot *bot) {
    cord_connect(bot->cord_context, URL);
}

int simple_bot_cleanup(simple_bot *bot) {
    cord_destroy(bot->cord_context);
    free(bot->active_poll);
    return 0;
}

int main(void) {
    // Initialize the application
    simple_bot bot = {
        .cord_context = cord_create(),
        .active_poll = malloc(sizeof(poll))
    };

    // Configure the application and connect to the server
    simple_bot_configure(&bot);
    simple_bot_start(&bot);
    
	return simple_bot_cleanup(&bot);
}
