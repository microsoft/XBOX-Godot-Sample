using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.store</c> — XStore license status, entitlements, and purchase UI.</summary>
public sealed class XboxStore : XboxServiceBase
{
    internal XboxStore(GodotObject o) : base(o) { }

    public Task<XboxResult> QueryLicenseStatusAsync(XboxUser user, string storeId) =>
        CallResultAsync("query_license_status_async", user?.Raw, storeId);

    public Task<XboxResult> RefreshEntitlementsAsync(XboxUser user, string storeId) =>
        CallResultAsync("refresh_entitlements_async", user?.Raw, storeId);

    public Task<XboxResult> ShowPurchaseUiAsync(XboxUser user, string storeId) =>
        CallResultAsync("show_purchase_ui_async", user?.Raw, storeId);

    public XboxStoreLicenseStatus GetCachedLicenseStatus(string storeId) =>
        XboxStoreLicenseStatus.From(Call("get_cached_license_status", storeId).AsGodotObject());

    public XboxResult CheckCachedLicenseStatus(string storeId) =>
        XboxResult.From(Call("check_cached_license_status", storeId).AsGodotObject());

    public Task<XboxResult> ShowProductPageUiAsync(XboxUser user, string storeId) =>
        CallResultAsync("show_product_page_ui_async", user?.Raw, storeId);

    public Task<XboxResult> ShowAssociatedProductsUiAsync(XboxUser user, string storeId, string productKinds) =>
        CallResultAsync("show_associated_products_ui_async", user?.Raw, storeId, productKinds);

    public Task<XboxResult> ShowRateAndReviewUiAsync(XboxUser user) =>
        CallResultAsync("show_rate_and_review_ui_async", user?.Raw);

    public Task<XboxResult> ShowRedeemTokenUiAsync(
        XboxUser user, string token, string[] allowedStoreIds, bool disallowCsvRedemption) =>
        CallResultAsync("show_redeem_token_ui_async", user?.Raw, token, allowedStoreIds, disallowCsvRedemption);

    public Task<XboxResult> ShowGiftingUiAsync(XboxUser user, string storeId, string name, string extendedJson) =>
        CallResultAsync("show_gifting_ui_async", user?.Raw, storeId, name, extendedJson);
}
