#ifndef TYPES_H
#define TYPES_H

#include "../core/errors.h"
#include "../core/array.h"
#include "../core/strings.h"

#include <stdbool.h>
#include <jansson.h>


typedef enum cord_discord_channel_type_t {
	DISCORD_CHANNEL_GUILD_TEXT,
	DISCORD_CHANNEL_DM,
	DISCORD_CHANNEL_GUILD_VOICE,
	DISCORD_CHANNEL_GROUP_DM,
	DISCORD_CHANNEL_GUILD_CATEGORY,
	DISCORD_CHANNEL_GUILD_ANNOUNCEMENT,
	DISCORD_CHANNEL_ANNOUNCEMENT_THREAD,
	DISCORD_CHANNEL_PUBLIC_THREAD,
	DISCORD_CHANNEL_PRIVATE_THREAD,
	DISCORD_CHANNEL_GUILD_STATE_VOICE,
	DISCORD_CHANNEL_GUILD_DIRECTORY,
	DISCORD_CHANNEL_GUILD_FORUM
} cord_discord_channel_type_t;

typedef enum cord_discord_video_quality_mode_t {
	DISCORD_VIDEO_QUALITY_AUTO = 1,
	DISCORD_VIDEO_QUALITY_FULL = 2
} cord_discord_video_quality_mode_t;

typedef enum cord_discord_channel_flag_t {
	DISCORD_CHANNEL_FLAGS_PINNED = (1 << 1),
	DISCORD_CHANNEL_FLAGS_REQUIRE_TAG = (1 << 4)
} cord_discord_channel_flag_t;

typedef enum cord_discord_sort_order_type_t {
	DISCORD_SORT_ORDER_LATEST_ACTIVITY,
	DISCORD_SORT_ORDER_CREATION_DATE
} cord_discord_sort_order_type_t;

typedef enum cord_discord_forum_layout_type_t {
	DISCORD_FORUM_LAYOUT_NOT_SET,
	DISCORD_FORUM_LAYOUT_LIST_VIEW,
	DISCORD_FORUM_LAYOUT_GALLERY_VIEW
} cord_discord_forum_layout_type_t;

typedef enum cord_object_field_type_t {
	CORD_OBJECT_FIELD_TYPE_NUMBER,
	CORD_OBJECT_FIELD_TYPE_STRING,
	CORD_OBJECT_FIELD_TYPE_BOOL,
} cord_object_field_type_t;

typedef struct cord_serialize_result_t {
	void *obj;
	cord_error_t error;
} cord_serialize_result_t;

typedef struct cord_object_field_t {
	cord_object_field_type_t type;
	bool is_optional;
	bool is_valid;
	union {
		cord_strbuf_t *_string;
		f64 _float;
		u64 _integer;
		bool _bool;
		cord_array_t *_array;
	};
} cord_object_field_t;

cord_object_field_t cord_object_field_from_string(cord_bump_t *allocator, const char *cstring);
cord_object_field_t cord_object_field_from_number(cord_bump_t *allocator, i64 value);
cord_object_field_t cord_object_field_from_bool(cord_bump_t *allocator, bool value);

// https://discord.com/developers/docs/resources/user#user-object
typedef struct cord_user_t {
	cord_object_field_t id;
	cord_object_field_t username;
	cord_object_field_t discriminator;
	cord_object_field_t avatar;	
	cord_object_field_t bot;
	cord_object_field_t system_;
	cord_object_field_t mfa_enabled;
	cord_object_field_t locale;
	cord_object_field_t verified;
	cord_object_field_t email;
	cord_object_field_t flags;
	cord_object_field_t premium_type;
	cord_object_field_t public_flags;

	cord_bump_t *allocator;
} cord_user_t;

void cord_user_init(cord_user_t *user, cord_bump_t *allocator);
cord_serialize_result_t cord_user_serialize(json_t *data, cord_bump_t *allocator);

// https://discord.com/developers/docs/topics/permissions#role-object
typedef struct cord_role_t {
    cord_object_field_t id;
    cord_object_field_t name;
    cord_object_field_t color;
    cord_object_field_t hoist;
    cord_object_field_t position;
    cord_object_field_t permissions;
    cord_object_field_t managed;
    cord_object_field_t mentionable;
    //cord_role_tags_t tag;

	cord_bump_t *allocator;
} cord_role_t;

void cord_role_init(cord_role_t *role, cord_bump_t *allocator);
cord_role_t *cord_role_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/guild#guild-member-object
typedef struct cord_guild_member_t {
    cord_user_t *user;
    cord_object_field_t nick;
    cord_role_t *roles[64];
	int _roles_count;
	cord_object_field_t joined_at;
    cord_object_field_t premium_since;
    cord_object_field_t deaf;
    cord_object_field_t mute;
    cord_object_field_t pending;

	cord_bump_t *allocator;
} cord_guild_member_t;

void cord_guild_member_init(cord_guild_member_t *member, cord_bump_t *allocator);
cord_serialize_result_t cord_guild_member_serialize(json_t *data, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#channel-mention-object
typedef struct cord_channel_mention_t {
    cord_strbuf_t id;
    cord_strbuf_t guild_id;
    int type;
    cord_strbuf_t name;
} cord_channel_mention_t;

void cord_channel_mention_init(cord_channel_mention_t *mention, cord_bump_t *allocator);
cord_channel_mention_t *cord_channel_mention_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#attachment-object
typedef struct cord_attachment_t {
	cord_strbuf_t id;
	cord_strbuf_t filename;
	int size;
	cord_strbuf_t url;
	cord_strbuf_t proxy_url;
	int height;
	int width;
} cord_attachment_t;

void cord_attachment_init(cord_attachment_t *attachment, cord_bump_t *allocator);
cord_attachment_t *cord_attachment_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-footer-structure
typedef struct cord_embed_footer_t {
	cord_strbuf_t text;
	cord_strbuf_t icon_url;
	cord_strbuf_t proxy_icon_url;
} cord_embed_footer_t;

void cord_embed_footer_init(cord_embed_footer_t *efooter, cord_bump_t *allocator);
cord_embed_footer_t *cord_embed_footer_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-image-structure
typedef struct cord_embed_image_t {
	cord_strbuf_t url;
	cord_strbuf_t proxy_url;
	int height;
	int width;
} cord_embed_image_t;

void cord_embed_image_init(cord_embed_image_t *eimage, cord_bump_t *allocator);
cord_embed_image_t *cord_embed_image_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-thumbnail-structure
typedef struct cord_embed_thumbnail_t {
	cord_strbuf_t url;
	cord_strbuf_t proxy_url;
	int height;
	int width;
} cord_embed_thumbnail_t;

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *ethumbnail, cord_bump_t *allocator);
cord_embed_thumbnail_t *cord_embed_thumbnail_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-video-structure
typedef struct cord_embed_video_t {
	cord_strbuf_t url;
	int height;
	int width;
} cord_embed_video_t;

void cord_embed_video_init(cord_embed_video_t *evideo, cord_bump_t *allocator);
cord_embed_video_t *cord_embed_video_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-provider-structure
typedef struct cord_embed_provider_t {
	cord_strbuf_t name;
	cord_strbuf_t url;
} cord_embed_provider_t;

void cord_embed_provider_init(cord_embed_provider_t *eprovider, cord_bump_t *allocator);
cord_embed_provider_t *cord_embed_provider_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-author-structure
typedef struct cord_embed_author_t {
	cord_strbuf_t name;
	cord_strbuf_t url;
	cord_strbuf_t icon_url;
	cord_strbuf_t proxy_icon_url;
} cord_embed_author_t;

void cord_embed_author_init(cord_embed_author_t *eauthor, cord_bump_t *allocator);
cord_embed_author_t *cord_embed_author_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-field-structure
typedef struct cord_embed_field_t {
	cord_strbuf_t name;
	cord_strbuf_t value;
	bool inline_;
} cord_embed_field_t;

void cord_embed_field_init(cord_embed_field_t *efield, cord_bump_t *allocator);
cord_embed_field_t *cord_embed_field_serialize(json_t *data, cord_error_t *err);

// https://discord.com/developers/docs/resources/channel#embed-object
typedef struct cord_embed_t {
	cord_strbuf_t title;
	cord_strbuf_t type;
	cord_strbuf_t description;
	cord_strbuf_t url;
	cord_strbuf_t timestamp;
	int color;
	cord_embed_footer_t *footer;
	cord_embed_image_t *image;
	cord_embed_thumbnail_t *thumbnail;
	cord_embed_video_t *video;
	cord_embed_provider_t *provider;
	cord_embed_author_t *author;

	cord_array_t *fields; // cord_embed_fields[]
} cord_embed_t;

int cord_embed_init(cord_embed_t *emb, cord_bump_t *allocator);
int cord_embed_serialize(json_t *emb, json_t *data);

// (Emoji) - https://discord.com/developers/docs/resources/emoji#emoji-object
typedef struct cord_emoji_t {
	cord_strbuf_t id;
	cord_strbuf_t name;
	cord_array_t *roles; // cord_role_t[]
	cord_user_t *user;
	bool require_colons;
	bool managed;
	bool animated;
	bool available;
} cord_emoji_t;

void cord_emoji_init(cord_emoji_t *emoji, cord_bump_t *allocator);
cord_emoji_t *cord_emoji_serialize(json_t *data, cord_error_t *err);

typedef struct cord_reaction_t {
	int count;
	bool me;
	cord_emoji_t *emoji;
} cord_reaction_t;

void cord_reaction_init(cord_reaction_t *reactio, cord_bump_t *allocator);
cord_reaction_t *cord_reaction_serialize(json_t *data, cord_error_t *err);

// (Message Activity) - https://discord.com/developers/docs/resources/channel#message-object-message-activity-structure
typedef struct cord_message_activity_t {
	int type;
	cord_strbuf_t party_id;
} cord_message_activity_t;

void cord_message_activity_init(cord_message_activity_t *mactivity, cord_bump_t *allocator);
cord_message_activity_t *cord_message_activity_serialize(json_t *data, cord_error_t *err);

// (Message Application) - https://discord.com/developers/docs/resources/channel#message-object-message-application-structure
typedef struct cord_message_application_t {
	cord_strbuf_t id;
	cord_strbuf_t cover_image;
	cord_strbuf_t description;
	cord_strbuf_t icon;
	cord_strbuf_t name;
} cord_message_application_t;

void cord_message_application_init(cord_message_application_t *mapplication, cord_bump_t *allocator);
cord_message_application_t *cord_message_application_serialize(json_t *data, cord_error_t *err);

// (Message Reference) - https://discord.com/developers/docs/resources/channel#message-object-message-reference-structure
typedef struct cord_message_reference_t {
	cord_strbuf_t message_id;
	cord_strbuf_t channel_id;
	cord_strbuf_t guild_id;
} cord_message_reference_t;

void cord_message_reference_init(cord_message_reference_t *mreference, cord_bump_t *allocator);
cord_message_reference_t *cord_message_reference_serialize(json_t *data, cord_error_t *err);

// (Message Sticker) - https://discord.com/developers/docs/resources/channel#message-object-message-sticker-structure
typedef struct cord_message_sticker_t {
	cord_strbuf_t id;
	cord_strbuf_t pack_id;
	cord_strbuf_t name;
	cord_strbuf_t description;
	cord_strbuf_t tags;
	cord_strbuf_t asset;
	cord_strbuf_t preview_asset;
	int format_type;
} cord_message_sticker_t;

void cord_message_sticker_init(cord_message_sticker_t *msticker, cord_bump_t *allocator);
cord_message_sticker_t *cord_message_sticker_serialize(json_t *data, cord_error_t *err);

// (Message) - https://discord.com/developers/docs/resources/channel#message-object
typedef struct cord_message_t {
	cord_strbuf_t id;
	cord_strbuf_t channel_id;
	cord_strbuf_t guild_id;
	cord_user_t *author;
	cord_guild_member_t *member;
	cord_strbuf_t content;
	cord_strbuf_t timestamp;
	cord_strbuf_t edited_timestamp;
	bool tts;
	bool mention_everyone;

	cord_array_t *mentions;
	cord_array_t *mention_roles;
	cord_array_t *mention_channels;
	cord_array_t *attachments;
	cord_array_t *embeds;
	cord_array_t *reactions;	

	cord_strbuf_t nonce;
    bool pinned;
	cord_strbuf_t webhook_id;
	int type;
	cord_message_activity_t *activity;
	cord_message_application_t *application;
	cord_message_reference_t *message_reference;
	int flags; // combined as a bitfield(check bitwise operators on how to check the fieldset)

	cord_array_t *stickers; // cord_message_sticker_t[]

	/*
	This field is only returned for messages with a type of 19 (REPLY).
	If the message is a reply but the referenced_message field is not present,
	the backend did not attempt to fetch the message that was being replied to,
	so its state is unknown. If the field exists but is null, the referenced message was deleted.
	*/
	struct cord_message_t *referenced_message;
} cord_message_t;

void cord_message_init(cord_message_t *message, cord_bump_t *allocator);
cord_serialize_result_t cord_message_serialize(json_t *data, cord_bump_t *allocator);

typedef struct cord_guild_t {
	cord_strbuf_t id;
	cord_strbuf_t name;
	cord_strbuf_t icon;
	cord_strbuf_t splash;
	cord_strbuf_t discovery_splash;
} cord_guild_t;

int cord_guild_init(cord_guild_t *guild, cord_bump_t *allocator);
int cord_guild_serialize(cord_guild_t *g, json_t *data);

#endif
