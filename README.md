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
#include <stdlib.h>
#include <string.h>
#include <cord.h>

void on_message(discord_t *client, cord_message_t *msg) {
	if (strcmp(msg->content, "ping") == 0) {
		// Allocate a response
		cord_message_t *response = malloc(sizeof(cord_message_t));
		response->channel_id = calloc(1, 256);
		strcpy(response->channel_id, msg->channel_id);

		// Write & send the message
		discord_message_set_content(response, "Pong!");
		discord_send_message(client, response);

		// Free the message's memory
		cord_message_free(response);
	}
}

int main(void) {
	const char *url = "wss://gateway.discord.gg/?v=6&encoding=json";
	cord_t *context = cord_create();

	// Provide callback
	cord_on_message(context, on_message);

	cord_connect(context, url);
	cord_destroy(context);
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

## Documentation
To be written

## Roadmap
To be written  

