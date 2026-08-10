using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>License status for an XStore product/SKU.</summary>
public sealed class XboxStoreLicenseStatus : XboxObject
{
    internal XboxStoreLicenseStatus(GodotObject o) : base(o) { }
    public static XboxStoreLicenseStatus From(GodotObject o) => o == null ? null : new XboxStoreLicenseStatus(o);

    public string StoreId => GetString("store_id");
    public string LicensableSku => GetString("licensable_sku");
    public int Status => GetInt32("status");
}
