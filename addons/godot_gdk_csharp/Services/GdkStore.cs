using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.store</c> — XStore license status, entitlements, and purchase UI.</summary>
public sealed class GdkStore : GdkServiceBase
{
    internal GdkStore(GodotObject o) : base(o) { }

    public Task<GdkResult> QueryLicenseStatusAsync(GdkUser user, string storeId) =>
        CallResultAsync("query_license_status_async", user?.Raw, storeId);

    public Task<GdkResult> RefreshEntitlementsAsync(GdkUser user, string storeId) =>
        CallResultAsync("refresh_entitlements_async", user?.Raw, storeId);

    public Task<GdkResult> ShowPurchaseUiAsync(GdkUser user, string storeId) =>
        CallResultAsync("show_purchase_ui_async", user?.Raw, storeId);

    public GdkStoreLicenseStatus GetCachedLicenseStatus(string storeId) =>
        GdkStoreLicenseStatus.From(Call("get_cached_license_status", storeId).AsGodotObject());

    public GdkResult CheckCachedLicenseStatus(string storeId) =>
        GdkResult.From(Call("check_cached_license_status", storeId).AsGodotObject());
}
