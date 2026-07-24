using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabInventory : PlayFabServiceBase
{
    internal PlayFabInventory(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AddInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("add_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteInventoryCollectionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_inventory_collection_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ExecuteInventoryOperationsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("execute_inventory_operations_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ExecuteTransferOperationsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("execute_transfer_operations_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetInventoryCollectionIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_inventory_collection_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetInventoryOperationStatusAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_inventory_operation_status_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetTransactionHistoryAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_transaction_history_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> PurchaseInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("purchase_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RedeemGooglePlayInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("redeem_google_play_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RedeemMicrosoftStoreInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("redeem_microsoft_store_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RedeemPlayStationStoreInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("redeem_play_station_store_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RedeemSteamInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("redeem_steam_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SubtractInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("subtract_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> TransferInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("transfer_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateInventoryItemsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_inventory_items_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
