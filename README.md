# Cord
Cord is a library for interfacing with the [Discord API](https://discord.com/developers/docs/intro) using HTTP and Websockets.
The goal of the project is to provide an easy to use library for developing
discord applications such as bots using C. The library depends on [Jansson](https://github.com/akheron/jansson)
for JSON serialization/desirialization, [libuwsc](https://github.com/zhaojh329/libuwsc), for websocket communication
and [libcurl](https://github.com/curl/curl) to perform HTTP requests.

NOTE: The library is still in early development stage and many features are missing  

## Example
The following example bot uses cord to create a simple echo bot  
`
void on_message(discord_t *disc, discord_message_t *msg) {    
    if (strcmp(msg->content, "ping") == 0) {  
        discord_message_t *response = malloc(sizeof(discord_message_t));  
        response->channel_id = calloc(1, 256);  
        strcpy(response->channel_id, msg->channel_id);  
        
        discord_message_set_content(response, "Pong!");  
        discord_send_message(disc, response);  
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
`
## Documentation
To be written

## Building
Build and install [libuwsc](https://github.com/zhaojh329/libuwsc)

Install the dependencies  
`sudo apt install libcurl4-openssl-dev libjansson-dev`

Build and install the library  
`
git clone https://github.com/oikpanagiotis/cord  
cd cord && mkdir bin build
make
`

Install the library  
`sudo make install`

Before using make sure to set the following enviroment variables  
`
DISCORD_APPLICATION_TOKEN  
DISCORD_APPLICATION_DEVICE  
DISCORD_APPLICATION_OS  
DISCORD_APPLICATION_LIBRARY  
`

## Roadmap
To be written  

