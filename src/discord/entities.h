#ifndef ENTITIES_H
#define ENTITIES_H

#include "../core/array.h"
#include "../core/memory.h"
#include "../core/strings.h"

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

// https://discord.com/developers/docs/resources/user#user-object
typedef struct cord_user_t {
    cord_strbuf_t *id;
    cord_strbuf_t *username;
    cord_strbuf_t *discriminator;
    cord_strbuf_t *avatar;
    bool bot;
    bool system_;
    bool mfa_enabled;
    cord_strbuf_t *locale;
    bool verified;
    cord_strbuf_t *email;
    i32 flags;
    i32 premium_type;
    i32 public_flags;

    cord_bump_t *allocator;
} cord_user_t;

void cord_user_init(cord_user_t *user, cord_bump_t *allocator);

// https://discord.com/developers/docs/topics/permissions#role-object
typedef struct cord_role_t {
    cord_strbuf_t *id;
    cord_strbuf_t *name;
    i32 color;
    bool hoist;
    cord_strbuf_t *icon;
    cord_strbuf_t *unicode_emoji;
    i32 position;
    cord_strbuf_t *permissions;
    bool managed;
    bool mentionable;
    cord_array_t *tags;

    cord_bump_t *allocator;
} cord_role_t;

void cord_role_init(cord_role_t *role, cord_bump_t *allocator);

// https://discord.com/developers/docs/topics/permissions#role-object-role-tags-structure
typedef struct cord_role_tag_t {
    cord_strbuf_t *bot_id;
    cord_strbuf_t *integration_id;
    bool premium_subscriber; // The docs mark this as of type null. Check if
                             // it's used at all
    cord_strbuf_t *subscription_listing_id;
    bool available_for_purchase; // null?
    bool guild_connections;      // null?

    cord_bump_t *allocator;
} cord_role_tag_t;

void cord_role_tag_init(cord_role_tag_t *role_tag, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/guild#guild-member-object
typedef struct cord_guild_member_t {
    cord_user_t *user;
    cord_strbuf_t *nick;
    cord_array_t *roles; // cord_role_t[]
    cord_strbuf_t *joined_at;
    cord_strbuf_t *premium_since;
    bool deaf;
    bool mute;
    bool pending;

    cord_bump_t *allocator;
} cord_guild_member_t;

void cord_guild_member_init(cord_guild_member_t *member, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#channel-mention-object
typedef struct cord_channel_mention_t {
    cord_strbuf_t *id;
    cord_strbuf_t *guild_id;
    i32 type;
    cord_strbuf_t *name;

    cord_bump_t *allocator;
} cord_channel_mention_t;

void cord_channel_mention_init(cord_channel_mention_t *mention, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#attachment-object
typedef struct cord_attachment_t {
    cord_strbuf_t *id;
    cord_strbuf_t *filename;
    i32 size;
    cord_strbuf_t *url;
    cord_strbuf_t *proxy_url;
    i32 height;
    i32 width;

    cord_bump_t *allocator;
} cord_attachment_t;

void cord_attachment_init(cord_attachment_t *attachment, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-footer-structure
typedef struct cord_embed_footer_t {
    cord_strbuf_t *text;
    cord_strbuf_t *icon_url;
    cord_strbuf_t *proxy_icon_url;

    cord_bump_t *allocator;
} cord_embed_footer_t;

void cord_embed_footer_init(cord_embed_footer_t *efooter, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-image-structure
typedef struct cord_embed_image_t {
    cord_strbuf_t *url;
    cord_strbuf_t *proxy_url;
    i32 height;
    i32 width;

    cord_bump_t *allocator;
} cord_embed_image_t;

void cord_embed_image_init(cord_embed_image_t *eimage, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-thumbnail-structure
typedef struct cord_embed_thumbnail_t {
    cord_strbuf_t *url;
    cord_strbuf_t *proxy_url;
    i32 height;
    i32 width;

    cord_bump_t *allocator;
} cord_embed_thumbnail_t;

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *ethumbnail,
                               cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-video-structure
typedef struct cord_embed_video_t {
    cord_strbuf_t *url;
    i32 height;
    i32 width;

    cord_bump_t *allocator;
} cord_embed_video_t;

void cord_embed_video_init(cord_embed_video_t *evideo, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-provider-structure
typedef struct cord_embed_provider_t {
    cord_strbuf_t *name;
    cord_strbuf_t *url;

    cord_bump_t *allocator;
} cord_embed_provider_t;

void cord_embed_provider_init(cord_embed_provider_t *eprovider, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-author-structure
typedef struct cord_embed_author_t {
    cord_strbuf_t *name;
    cord_strbuf_t *url;
    cord_strbuf_t *icon_url;
    cord_strbuf_t *proxy_icon_url;

    cord_bump_t *allocator;
} cord_embed_author_t;

void cord_embed_author_init(cord_embed_author_t *eauthor, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object-embed-field-structure
typedef struct cord_embed_field_t {
    cord_strbuf_t *name;
    cord_strbuf_t *value;
    bool inline_;

    cord_bump_t *allocator;
} cord_embed_field_t;

void cord_embed_field_init(cord_embed_field_t *efield, cord_bump_t *allocator);

// https://discord.com/developers/docs/resources/channel#embed-object
typedef struct cord_embed_t {
    cord_strbuf_t *title;
    cord_strbuf_t *type;
    cord_strbuf_t *description;
    cord_strbuf_t *url;
    cord_strbuf_t *timestamp;
    i32 color;
    cord_embed_footer_t *footer;
    cord_embed_image_t *image;
    cord_embed_thumbnail_t *thumbnail;
    cord_embed_video_t *video;
    cord_embed_provider_t *provider;
    cord_embed_author_t *author;
    cord_array_t *fields; // cord_embed_field_t[]

    cord_bump_t *allocator;
} cord_embed_t;

void cord_embed_init(cord_embed_t *emb, cord_bump_t *allocator);

// (Emoji) - https://discord.com/developers/docs/resources/emoji#emoji-object
typedef struct cord_emoji_t {
    cord_strbuf_t *id;
    cord_strbuf_t *name;
    cord_array_t *roles; // cord_role_t[]
    cord_user_t *user;
    bool require_colons;
    bool managed;
    bool animated;
    bool available;

    cord_bump_t *allocator;
} cord_emoji_t;

void cord_emoji_init(cord_emoji_t *emoji, cord_bump_t *allocator);

typedef struct cord_reaction_t {
    i32 count;
    bool me;
    cord_emoji_t *emoji;

    cord_bump_t *allocator;
} cord_reaction_t;

void cord_reaction_init(cord_reaction_t *reactio, cord_bump_t *allocator);

// (Message Activity) -
// https://discord.com/developers/docs/resources/channel#message-object-message-activity-structure
typedef struct cord_message_activity_t {
    i32 type;
    cord_strbuf_t *party_id;

    cord_bump_t *allocator;
} cord_message_activity_t;

void cord_message_activity_init(cord_message_activity_t *mactivity,
                                cord_bump_t *allocator);

// (Message Application) -
// https://discord.com/developers/docs/resources/channel#message-object-message-application-structure
typedef struct cord_message_application_t {
    cord_strbuf_t *id;
    cord_strbuf_t *cover_image;
    cord_strbuf_t *description;
    cord_strbuf_t *icon;
    cord_strbuf_t *name;

    cord_bump_t *allocator;
} cord_message_application_t;

void cord_message_application_init(cord_message_application_t *mapplication,
                                   cord_bump_t *allocator);

// (Message Reference) -
// https://discord.com/developers/docs/resources/channel#message-object-message-reference-structure
typedef struct cord_message_reference_t {
    cord_strbuf_t *message_id;
    cord_strbuf_t *channel_id;
    cord_strbuf_t *guild_id;
} cord_message_reference_t;

void cord_message_reference_init(cord_message_reference_t *mreference,
                                 cord_bump_t *allocator);

// (Message Sticker) -
// https://discord.com/developers/docs/resources/channel#message-object-message-sticker-structure
typedef struct cord_message_sticker_t {
    cord_strbuf_t *id;
    cord_strbuf_t *pack_id;
    cord_strbuf_t *name;
    cord_strbuf_t *description;
    cord_strbuf_t *tags;
    cord_strbuf_t *asset;
    cord_strbuf_t *preview_asset;
    i32 format_type;

    cord_bump_t *allocator;
} cord_message_sticker_t;

void cord_message_sticker_init(cord_message_sticker_t *msticker, cord_bump_t *allocator);

// (Message) -
// https://discord.com/developers/docs/resources/channel#message-object
typedef struct cord_message_t {
    cord_strbuf_t *id;
    cord_strbuf_t *channel_id;
    cord_strbuf_t *guild_id;
    cord_user_t *author;
    cord_guild_member_t *member;
    cord_strbuf_t *content;
    cord_strbuf_t *timestamp;
    cord_strbuf_t *edited_timestamp;
    bool *tts;
    bool *mention_everyone;

    cord_array_t *mentions;
    cord_array_t *mention_roles;
    cord_array_t *mention_channels;
    cord_array_t *attachments;
    cord_array_t *embeds;
    cord_array_t *reactions;

    cord_strbuf_t *nonce;
    bool *pinned;
    cord_strbuf_t *webhook_id;
    i32 *type;
    cord_message_activity_t *activity;
    cord_message_application_t *application;
    cord_message_reference_t *message_reference;
    i32 *flags;              // combined as a bitfield
    cord_array_t *stickers; // cord_message_sticker_t[]

    /*
    This field is only returned for messages with a type of 19 (REPLY).
    If the message is a reply but the referenced_message field is not present,
    the backend did not attempt to fetch the message that was being replied to,
    so its state is unknown. If the field exists but is null, the referenced
    message was deleted.
    */
    struct cord_message_t *referenced_message;

    cord_bump_t *allocator;
} cord_message_t;

void cord_message_set_content(cord_message_t *message, const char *content);

typedef struct cord_guild_t {
    cord_strbuf_t *id;
    cord_strbuf_t *name;
    cord_strbuf_t *icon;
    cord_strbuf_t *splash;
    cord_strbuf_t *discovery_splash;

    cord_bump_t *allocator;
} cord_guild_t;

void cord_guild_init(cord_guild_t *guild, cord_bump_t *allocator);

#endif
