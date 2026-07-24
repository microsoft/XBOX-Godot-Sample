using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabCatalog : PlayFabServiceBase
{
    internal PlayFabCatalog(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> CreateDraftItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("create_draft_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> CreateUploadUrlsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("create_upload_urls_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteEntityItemReviewsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_entity_item_reviews_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetCatalogConfigAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_catalog_config_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetDraftItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_draft_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetDraftItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_draft_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetEntityDraftItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_entity_draft_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetEntityItemReviewAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_entity_item_review_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemContainersAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_containers_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemModerationStateAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_moderation_state_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemPublishStatusAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_publish_status_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemReviewsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_reviews_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemReviewSummaryAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_item_review_summary_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> PublishDraftItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("publish_draft_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ReportItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("report_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ReportItemReviewAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("report_item_review_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ReviewItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("review_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SearchItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("search_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetItemModerationStateAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_item_moderation_state_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SubmitItemReviewVoteAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("submit_item_review_vote_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> TakedownItemReviewsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("takedown_item_reviews_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateCatalogConfigAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_catalog_config_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateDraftItemAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_draft_item_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
