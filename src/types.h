#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "str/sds.h"

// https://discord.com/developers/docs/resources/user#user-object
typedef struct discord_user_t {
	sds id;
	sds username;
	sds discriminator;
	sds avatar;	
	bool bot;
	bool system_; // _ to make identified unique
	bool mfa_enabled;
	sds locale;
	bool verified;
	sds email;
	int flags;
	int premium_type;
	int public_flags;	
} discord_user_t;

int serialize_user(discord_user_t *author, json_t *data);

typedef struct discord_guild_member_t {
    discord_user_t *user;
    sds nick;
    sds *roles; // array
    sds joined_at;
    sds premium_since;
    bool deaf;
    bool mute;
    bool pending;    
} discord_guild_member_t;

// https://discord.com/developers/docs/resources/channel#message-object
typedef struct discord_message_t {
	sds id;
	sds channel_id;
	sds guild_id;
	discord_author_t *author;
	discord_guild_member_t *member;
	sds content;
	sds timestamp;
	sds edited_timestamp;
	bool tts;
	bool mention_everyone;
	//discord_guild_member_t *mentions;
	//discord_role_t *mention_roles;
	//discord_channel_mention *mention_channels;
	//discord_attachment *attachments;
	//discord_embed_t *embeds;
	//discord_reaction_t *reactions;
	int nonce; // or string?
    bool pinned;
	sds webhook_id;
	int type;
	//discord_message_activity_t *activity;
	//discord_message_application_t *application;
	//discord_message_reference_t *message_reference;
	int flags; // combined as a bitfield(check bitwise operators on how to check the fieldset)
	//discord_sticker_t *stickers;
	//discord_message_t *referenced_message;
} discord_message_t;

int serialize_message(discord_message_t *msg, json_t *data);

// TODO: Implement these
int serialize_message_activity(void *ptr, json_t *data);
int serialize_message_application(void *ptr, json_t *data);
int serialize_message_reference(void *ptr, json_t *data);
int serialize_referenced_message(void *ptr, json_t *value);

typedef struct discord_guild_t {
	sds id;
	sds name;
	sds icon;
	sds splash;
	sds discovery_splash;
} discord_guild_t;

#endif