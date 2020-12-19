#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#include "str/sds.h"

// https://discord.com/developers/docs/resources/user#user-object
typedef struct cord_user_t {
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
} cord_user_t;

int cord_user_init(cord_user_t *user);
int cord_user_serialize(cord_user_t *author, json_t *data);
void cord_user_free(cord_user_t *user);

// https://discord.com/developers/docs/resources/guild#guild-member-object
typedef struct cord_guild_member_t {
    cord_user_t *user;
    sds nick;
    sds *roles; // array
    sds joined_at;
    sds premium_since;
    bool deaf;
    bool mute;
    bool pending;    
} cord_guild_member_t;

int cord_guild_member_init(cord_guild_member_t *member);
int cord_guild_member_serialize(cord_guild_member_t *member, json_t *data);
void cord_guild_member_free(cord_guild_member_t *member);

// https://discord.com/developers/docs/topics/permissions#role-object
typedef struct cord_role_t {
    sds id;
    sds name;
    int color;
    bool hoist;
    int position;
    sds permissions;
    bool managed;
    bool mentionable;
    //cord_role_tags_t tag;
} cord_role_t;

int cord_role_init(cord_role_t *role);
int cord_role_serialize(cord_role_t *role, json_t *data);
void cord_role_free(cord_role_t *role);

// https://discord.com/developers/docs/resources/channel#channel-mention-object
typedef struct cord_channel_mention_t {
    sds id;
    sds guild_id;
    int type;
    sds name;
} cord_channel_mention_t;

int cord_channel_mention_init(cord_channel_mention_t *mention);
int cord_channel_mention_serialize(cord_channel_mention_t *mention, json_t *data);
void cord_channel_mention_free(cord_channel_mention_t *mention);

// https://discord.com/developers/docs/resources/channel#attachment-object
typedef struct cord_attachment_t {
	sds id;
	sds filename;
	int size;
	sds url;
	sds proxy_url;
	int height;
	int width;
} cord_attachment_t;

int cord_attachment_init(cord_attachment_t *at);
int cord_attachment_serialize(cord_attachment_t *at, json_t *data);
void cord_attachment_free(cord_attachment_t *at);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-footer-structure
typedef struct cord_embed_footer_t {
	sds text;
	sds icon_url;
	sds proxy_icon_url;
} cord_embed_footer_t;

int cord_embed_footer_init(cord_embed_footer_t *ft);
int cord_embed_footer_serialize(cord_embed_footer_t *ft, json_t *data);
void cord_embed_footer_free(cord_embed_footer_t *ft);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-image-structure
typedef struct cord_embed_image_t {
	sds url;
	sds proxy_url;
	int height;
	int width;
} cord_embed_image_t;

int cord_embed_image_init(cord_embed_image_t *img);
int cord_embed_image_serialize(cord_embed_image_t *img, json_t *data);
void cord_embed_image_free(cord_embed_image_t *img);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-thumbnail-structure
typedef struct cord_embed_thumbnail_t {
	sds url;
	sds proxy_url;
	int height;
	int width;
} cord_embed_thumbnail_t;

int cord_embed_thumbnail_init(cord_embed_thumbnail_t *tn);
int cord_embed_thumbnail_serialize(cord_embed_thumbnail_t *tn, json_t *data);
void cord_embed_thumbnail_free(cord_embed_thumbnail_t *tn);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-video-structure
typedef struct cord_embed_video_t {
	sds url;
	int height;
	int width;
} cord_embed_video_t;

int cord_embed_video_init(cord_embed_video_t *evid);
int cord_embed_video_serialize(cord_embed_video_t *evid, json_t *data);
void cord_embed_video_free(cord_embed_video_t *evid);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-provider-structure
typedef struct cord_embed_provider_t {
	sds name;
	sds url;
} cord_embed_provider_t;

int cord_embed_provider_init(cord_embed_provider_t *epr);
int cord_embed_provider_serialize(cord_embed_provider_t *epr, json_t *data);
void cord_embed_provider_free(cord_embed_provider_t *epr);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-author-structure
typedef struct cord_embed_author_t {
	sds name;
	sds url;
	sds icon_url;
	sds proxy_icon_url;
} cord_embed_author_t;

int cord_embed_author_init(cord_embed_author_t *eauth);
int cord_embed_author_serialize(cord_embed_author_t *eauth, json_t *data);
void cord_embed_author_free(cord_embed_author_t *eauth);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-field-structure
typedef struct cord_embed_field_t {
	sds name;
	sds value;
	bool inline_;
} cord_embed_field_t;

int cord_embed_field_init(cord_embed_field_t *efield);
int cord_embed_field_serialize(cord_embed_field_t *efield, json_t *data);
void cord_embed_field_free(cord_embed_field_t *efield);

// https://discord.com/developers/docs/resources/channel#embed-object
typedef struct cord_embed_t {
	sds title;
	sds type;
	sds description;
	sds url;
	sds timestamp;
	int color;
	cord_embed_footer_t *footer;
	cord_embed_image_t *image;
	cord_embed_thumbnail_t *thumbnail;
	cord_embed_video_t *video;
	cord_embed_provider_t *provider;
	cord_embed_author_t *author;

	cord_embed_field_t *fields; // TODO: array
} cord_embed_t;

int cord_embed_init(cord_embed_t *emb);
int cord_embed_serialize(cord_embed_t *emb, json_t *data);
void cord_embed_free(cord_embed_t *emb);

// https://discord.com/developers/docs/resources/channel#message-object
typedef struct cord_message_t {
	sds id;
	sds channel_id;
	sds guild_id;
	cord_user_t *author;
	cord_guild_member_t *member;
	sds content;
	sds timestamp;
	sds edited_timestamp;
	bool tts;
	bool mention_everyone;
	cord_guild_member_t *mentions;
	cord_role_t *mention_roles;
	cord_channel_mention_t *mention_channels;
	cord_attachment_t *attachments;
	cord_embed_t *embeds;
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
	//cord_message_t *referenced_message;
} cord_message_t;

int cord_message_init(cord_message_t *msg);
int cord_message_serialize(cord_message_t *msg, json_t *data);
void cord_message_free(cord_message_t *msg);

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