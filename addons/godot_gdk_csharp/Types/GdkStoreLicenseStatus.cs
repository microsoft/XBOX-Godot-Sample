using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>License status for an XStore product/SKU.</summary>
public sealed class GdkStoreLicenseStatus : GdkObject
{
    internal GdkStoreLicenseStatus(GodotObject o) : base(o) { }
    public static GdkStoreLicenseStatus From(GodotObject o) => o == null ? null : new GdkStoreLicenseStatus(o);

    public string StoreId => GetString("store_id");
    public string LicensableSku => GetString("licensable_sku");
    public int Status => GetInt32("status");
}
