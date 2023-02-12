# Cord
Cord is a library for interfacing with the [Discord API](https://discord.com/developers/docs/intro) using HTTP and Websockets.
The goal of the project is to provide an easy to use library for developing
discord applications such as bots using C. The library depends on [Jansson](https://github.com/akheron/jansson)
for JSON serialization/desirialization, [libuwsc](https://github.com/zhaojh329/libuwsc), for websocket communication
and [libcurl](https://github.com/curl/curl) to perform HTTP requests.

NOTE: The library is still in early development stage and many features are missing  

## Example
The following example bot uses cord to create a simple echo bot  
```
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

```
## Building
Build and install [libuwsc](https://github.com/zhaojh329/libuwsc)

Install the dependencies  
`sudo apt install libcurl4-openssl-dev libjansson-dev`

Build and install the library
```
git clone https://github.com/oikpanagiotis/cord  
cd cord && mkdir build && cd build
cmake ..
make
sudo make install
```

Before using the library in your program make sure to set the following enviroment variable
```
CORD_APPLICATION_TOKEN
```

