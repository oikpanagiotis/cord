#include "entities.h"
#include "../core/log.h"

#include <assert.h>

void cord_user_init(cord_user_t *user, cord_bump_t *allocator) {
    user->id = NULL;
    user->username = NULL;
    user->discriminator = NULL;
    user->avatar = NULL;
    user->bot = false;
    user->system_ = false;
    user->mfa_enabled = false;
    user->locale = NULL;
    user->verified = false;
    user->email = NULL;
    user->flags = 0;
    user->premium_type = 0;
    user->public_flags = 0;

    assert(allocator && "Allocator passed to user init must not be null");
    user->allocator = allocator;
}

void cord_guild_member_init(cord_guild_member_t *member,
                            cord_bump_t *allocator) {
    cord_user_init(member->user, allocator);
    member->nick = NULL;
    member->roles = NULL;
    member->joined_at = NULL;
    member->premium_since = NULL;
    member->deaf = false;
    member->mute = false;
    member->pending = false;

    assert(allocator && "Allocator for cord_guild_member_t can not be null");
    member->allocator = allocator;
}

void cord_role_init(cord_role_t *role, cord_bump_t *allocator) {
    role->id = NULL;
    role->name = NULL;
    role->color = 0;
    role->hoist = false;
    role->position = 0;
    role->permissions = NULL;
    role->managed = false;
    role->mentionable = false;

    assert(allocator && "Allocator passed to role init must not be null");
    role->allocator = allocator;
}

void cord_role_tag_init(cord_role_tag_t *role_tag, cord_bump_t *allocator) {
    role_tag->available_for_purchase = false;
    role_tag->bot_id = NULL;
    role_tag->guild_connections = false;
    role_tag->integration_id = NULL;
    role_tag->premium_subscriber = false;
    role_tag->subscription_listing_id = NULL;
    role_tag->allocator = allocator;
}

void cord_channel_mention_init(cord_channel_mention_t *mention,
                               cord_bump_t *allocator) {
    mention->id = NULL;
    mention->guild_id = NULL;
    mention->type = 0;
    mention->name = NULL;
    mention->allocator = allocator;
}

void cord_attachment_init(cord_attachment_t *attachment,
                          cord_bump_t *allocator) {
    attachment->id = NULL;
    attachment->filename = NULL;
    attachment->size = 0;
    attachment->url = NULL;
    attachment->proxy_url = NULL;
    attachment->height = 0;
    attachment->width = 0;

    attachment->allocator = allocator;
}

void cord_embed_footer_init(cord_embed_footer_t *embed_footer,
                            cord_bump_t *allocator) {
    embed_footer->text = NULL;
    embed_footer->icon_url = NULL;
    embed_footer->proxy_icon_url = NULL;

    embed_footer->allocator = allocator;
}

void cord_embed_image_init(cord_embed_image_t *embed_image,
                           cord_bump_t *allocator) {
    embed_image->url = NULL;
    embed_image->proxy_url = NULL;
    embed_image->height = 0;
    embed_image->width = 0;
    embed_image->allocator = allocator;
}

void cord_embed_thumbnail_init(cord_embed_thumbnail_t *embed_thumbnail,
                               cord_bump_t *allocator) {
    embed_thumbnail->url = NULL;
    embed_thumbnail->proxy_url = NULL;
    embed_thumbnail->height = 0;
    embed_thumbnail->width = 0;
    embed_thumbnail->allocator = allocator;
}

void cord_embed_video_init(cord_embed_video_t *embed_video,
                           cord_bump_t *allocator) {
    embed_video->url = NULL;
    embed_video->height = 0;
    embed_video->width = 0;
    embed_video->allocator = allocator;
}

void cord_embed_provider_init(cord_embed_provider_t *embed_provider,
                              cord_bump_t *allocator) {
    embed_provider->name = NULL;
    embed_provider->url = NULL;

    embed_provider->allocator = allocator;
}

void cord_embed_author_init(cord_embed_author_t *embed_author,
                            cord_bump_t *allocator) {
    embed_author->name = NULL;
    embed_author->url = NULL;
    embed_author->icon_url = NULL;
    embed_author->proxy_icon_url = NULL;
    embed_author->allocator = allocator;
}

void cord_embed_field_init(cord_embed_field_t *embed_field,
                           cord_bump_t *allocator) {
    embed_field->name = NULL;
    embed_field->value = NULL;
    embed_field->inline_ = false;
    embed_field->allocator = allocator;
}

void cord_embed_init(cord_embed_t *embed, cord_bump_t *allocator) {
    embed->title = NULL;
    embed->type = NULL;
    embed->description = NULL;
    embed->url = NULL;
    embed->timestamp = NULL;
    embed->color = 0;
    embed->footer = NULL;
    embed->image = NULL;
    embed->thumbnail = NULL;
    embed->video = NULL;
    embed->provider = NULL;
    embed->author = NULL;
    embed->fields = NULL;
    embed->allocator = allocator;
}

void cord_emoji_init(cord_emoji_t *emoji, cord_bump_t *allocator) {
    emoji->id = NULL;
    emoji->name = NULL;
    emoji->roles = NULL;
    emoji->user = NULL;
    emoji->require_colons = false;
    emoji->managed = false;
    emoji->animated = false;
    emoji->available = false;
    emoji->allocator = allocator;
}

void cord_reaction_init(cord_reaction_t *reaction, cord_bump_t *allocator) {
    reaction->count = 0;
    reaction->me = false;
    reaction->emoji = NULL;
    reaction->allocator = allocator;
}

void cord_message_activity_init(cord_message_activity_t *message_activity,
                                cord_bump_t *allocator) {
    message_activity->type = 0;
    message_activity->party_id = NULL;
    message_activity->allocator = allocator;
}

void cord_message_application_init(cord_message_application_t *app,
                                   cord_bump_t *allocator) {
    app->id = NULL;
    app->cover_image = NULL;
    app->description = NULL;
    app->icon = NULL;
    app->name = NULL;
    app->allocator = allocator;
}

void cord_message_sticker_init(cord_message_sticker_t *message_sticker,
                               cord_bump_t *allocator) {
    message_sticker->id = NULL;
    message_sticker->pack_id = NULL;
    message_sticker->name = NULL;
    message_sticker->description = NULL;
    message_sticker->tags = NULL;
    message_sticker->asset = NULL;
    message_sticker->preview_asset = NULL;
    // https://discord.com/developers/docs/resources/channel#message-object-message-sticker-format-types
    message_sticker->format_type = 0;
    message_sticker->allocator = allocator;
}

void cord_message_set_content(cord_message_t *message, const char *content) {
    cord_strbuf_t *builder = cord_strbuf_create();
    if (!builder) {
        logger_error("Failed to allocate memory for message content");
        return;
    }
    cord_strbuf_append(builder, cstr(content));
    message->content = builder;
}

void cord_guild_init(cord_guild_t *guild, cord_bump_t *allocator) {
    guild->id = NULL;
    guild->name = NULL;
    guild->icon = NULL;
    guild->splash = NULL;
    guild->discovery_splash = NULL;
    guild->allocator = allocator;
}
