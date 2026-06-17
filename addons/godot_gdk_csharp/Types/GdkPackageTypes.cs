using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>A mounted package; resolve in-package paths and close when done.</summary>
public sealed class GdkPackageMount : GdkObject
{
    internal GdkPackageMount(GodotObject o) : base(o) { }
    public static GdkPackageMount From(GodotObject o) => o == null ? null : new GdkPackageMount(o);

    public string PackageIdentifier => GetString("package_identifier");
    public string MountPath => GetString("mount_path");
    public Godot.Collections.Dictionary PackageDetails => GetDict("package_details");
    public bool IsValid => GetBool("valid");

    public GdkResult ResolvePath(string relativePath) =>
        GdkResult.From(Call("resolve_path", relativePath).AsGodotObject());

    public GdkResult Close() => GdkResult.From(Call("close").AsGodotObject());
}

/// <summary>A loaded resource pack from a package.</summary>
public sealed class GdkPackageResourcePack : GdkObject
{
    internal GdkPackageResourcePack(GodotObject o) : base(o) { }
    public static GdkPackageResourcePack From(GodotObject o) => o == null ? null : new GdkPackageResourcePack(o);

    public string PackageIdentifier => GetString("package_identifier");
    public string MountPath => GetString("mount_path");
    public string PackRelativePath => GetString("pack_relative_path");
    public string PackPath => GetString("pack_path");
    public Godot.Collections.Dictionary PackageDetails => GetDict("package_details");
    public bool ReplaceFiles => GetBool("replace_files");
    public long Offset => GetInt("offset");
}
