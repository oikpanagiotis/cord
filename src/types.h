#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <jansson.h>

#include "error.h"
#include "array.h"
#include "sds.h"

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

void cord_user_init(cord_user_t *user);
cord_user_t *cord_user_serialize(json_t *data, cord_err *err);
void cord_user_free(cord_user_t *user);

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

void cord_role_init(cord_role_t *role);
cord_role_t *cord_role_serialize(json_t *data, cord_err *err);
void cord_role_free(cord_role_t *role);

// https://discord.com/developers/docs/resources/guild#guild-member-object
typedef struct cord_guild_member_t {
    cord_user_t *user;
    sds nick;
    cord_role_t *roles[64];
	int _roles_count;
	sds joined_at;
    sds premium_since;
    bool deaf;
    bool mute;
    bool pending;    
} cord_guild_member_t;

void cord_guild_member_init(cord_guild_member_t *member);
cord_guild_member_t *cord_guild_member_serialize(json_t *data, cord_err *err);
void cord_guild_member_free(cord_guild_member_t *member);

// https://discord.com/developers/docs/resources/channel#channel-mention-object
typedef struct cord_channel_mention_t {
    sds id;
    sds guild_id;
    int type;
    sds name;
} cord_channel_mention_t;

void cord_channel_mention_init(cord_channel_mention_t *mention);
cord_channel_mention_t *cord_channel_mention_serialize(json_t *data, cord_err *err);
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

void cord_attachment_init(cord_attachment_t *at);
cord_attachment_t *cord_attachment_serialize(json_t *data, cord_err *err);
void cord_attachment_free(cord_attachment_t *at);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-footer-structure
typedef struct cord_embed_footer_t {
	sds text;
	sds icon_url;
	sds proxy_icon_url;
} cord_embed_footer_t;

void cord_embed_footer_init(cord_embed_footer_t *ft);
cord_embed_footer_t *cord_embed_footer_serialize(json_t *data, cord_err *err);
void cord_embed_footer_free(cord_embed_footer_t *ft);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-image-structure
typedef struct cord_embed_image_t {
	sds url;
	sds proxy_url;
	int height;
	int width;
} cord_embed_image_t;

void cord_embed_image_init(cord_embed_image_t *img);
cord_embed_image_t *cord_embed_image_serialize(json_t *data, cord_err *err);
void cord_embed_image_free(cord_embed_image_t *img);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-thumbnail-structure
typedef struct cord_embed_thumbnail_t {
	sds url;
	sds proxy_url;
	int height;
	int width;
} cord_embed_thumbnail_t;

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *tn);
cord_embed_thumbnail_t *cord_embed_thumbnail_serialize(json_t *data, cord_err *err);
void cord_embed_thumbnail_free(cord_embed_thumbnail_t *tn);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-video-structure
typedef struct cord_embed_video_t {
	sds url;
	int height;
	int width;
} cord_embed_video_t;

void cord_embed_video_init(cord_embed_video_t *evid);
cord_embed_video_t *cord_embed_video_serialize(json_t *data, cord_err *err);
void cord_embed_video_free(cord_embed_video_t *evid);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-provider-structure
typedef struct cord_embed_provider_t {
	sds name;
	sds url;
} cord_embed_provider_t;

void cord_embed_provider_init(cord_embed_provider_t *epr);
cord_embed_provider_t *cord_embed_provider_serialize(json_t *data, cord_err *err);
void cord_embed_provider_free(cord_embed_provider_t *epr);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-author-structure
typedef struct cord_embed_author_t {
	sds name;
	sds url;
	sds icon_url;
	sds proxy_icon_url;
} cord_embed_author_t;

void cord_embed_author_init(cord_embed_author_t *eauth);
cord_embed_author_t *cord_embed_author_serialize(json_t *data, cord_err *err);
void cord_embed_author_free(cord_embed_author_t *eauth);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-field-structure
typedef struct cord_embed_field_t {
	sds name;
	sds value;
	bool inline_;
} cord_embed_field_t;

void cord_embed_field_init(cord_embed_field_t *efield);
cord_embed_field_t *cord_embed_field_serialize(json_t *data, cord_err *err);
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

// (Emoji) - https://discord.com/developers/docs/resources/emoji#emoji-object
typedef struct cord_emoji_t {
	sds id;
	sds name;
	cord_role_t *roles; // array
	cord_user_t *user;
	bool require_colons;
	bool managed;
	bool animated;
	bool available;
} cord_emoji_t;

void cord_emoji_init(cord_emoji_t *emj);
cord_emoji_t *cord_emoji_serialize(json_t *data, cord_err *err);
void cord_emoji_free(cord_emoji_t *emj);

typedef struct cord_reaction_t {
	int count;
	bool me;
	cord_emoji_t *emoji;
} cord_reaction_t;

void cord_reaction_init(cord_reaction_t *react);
cord_reaction_t *cord_reaction_serialize(json_t *data, cord_err *err);
void cord_reaction_free(cord_reaction_t *react);

// (Message Activity) - https://discord.com/developers/docs/resources/channel#message-object-message-activity-structure
typedef struct cord_message_activity_t {
	int type;
	sds party_id;
} cord_message_activity_t;

void cord_message_activity_init(cord_message_activity_t *ma);
cord_message_activity_t *cord_message_activity_serialize(json_t *data, cord_err *err);
void cord_message_activity_free(cord_message_activity_t *ma);

// (Message Application) - https://discord.com/developers/docs/resources/channel#message-object-message-application-structure
typedef struct cord_message_application_t {
	sds id;
	sds cover_image;
	sds description;
	sds icon;
	sds name;
} cord_message_application_t;

void cord_message_application_init(cord_message_application_t *app);
cord_message_application_t *cord_message_application_serialize(json_t *data, cord_err *err);
void cord_message_application_free(cord_message_application_t *app);

// (Message Reference) - https://discord.com/developers/docs/resources/channel#message-object-message-reference-structure
typedef struct cord_message_reference_t {
	sds message_id;
	sds channel_id;
	sds guild_id;
} cord_message_reference_t;

void cord_message_reference_init(cord_message_reference_t *mr);
cord_message_reference_t *cord_message_reference_serialize(json_t *data, cord_err *err);
void cord_message_reference_free(cord_message_reference_t *mr);

// (Message Sticker) - https://discord.com/developers/docs/resources/channel#message-object-message-sticker-structure
typedef struct cord_message_sticker_t {
	sds id;
	sds pack_id;
	sds name;
	sds description;
	sds tags;
	sds asset;
	sds preview_asset;
	int format_type;
} cord_message_sticker_t;

void cord_message_sticker_init(cord_message_sticker_t *ms);
cord_message_sticker_t *cord_message_sticker_serialize(json_t *data, cord_err *err);
void cord_message_sticker_free(cord_message_sticker_t *ms);

#define MAX_ARRAY 4
// (Message) - https://discord.com/developers/docs/resources/channel#message-object
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

	cord_array_t *mentions;
	cord_array_t *mention_roles;
	cord_array_t *mention_channels;
	cord_array_t *attachments;
	cord_array_t *embeds;
	cord_array_t *reactions;	

	sds nonce;
    bool pinned;
	sds webhook_id;
	int type;
	cord_message_activity_t *activity;
	cord_message_application_t *application;
	cord_message_reference_t *message_reference;
	int flags; // combined as a bitfield(check bitwise operators on how to check the fieldset)

	cord_message_sticker_t *stickers[MAX_ARRAY];
	int _stickers_count;

	/*
	This field is only returned for messages with a type of 19 (REPLY).
	If the message is a reply but the referenced_message field is not present,
	the backend did not attempt to fetch the message that was being replied to,
	so its state is unknown. If the field exists but is null, the referenced message was deleted.
	*/
	struct cord_message_t *referenced_message;
} cord_message_t;

void cord_message_init(cord_message_t *msg);
cord_message_t *cord_message_serialize(json_t *data, cord_err *err);
void cord_message_free(cord_message_t *msg);

typedef struct cord_guild_t {
	sds id;
	sds name;
	sds icon;
	sds splash;
	sds discovery_splash;
} cord_guild_t;

int cord_guild_init(cord_guild_t *g);
int cord_guild_serialize(cord_guild_t *g, json_t *data);
void cord_guild_free(cord_guild_t *g);

#endif
