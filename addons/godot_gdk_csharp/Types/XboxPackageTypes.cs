using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>A mounted package; resolve in-package paths and close when done.</summary>
public sealed class XboxPackageMount : XboxObject
{
    internal XboxPackageMount(GodotObject o) : base(o) { }
    public static XboxPackageMount From(GodotObject o) => o == null ? null : new XboxPackageMount(o);

    public string PackageIdentifier => GetString("package_identifier");
    public string MountPath => GetString("mount_path");
    public Godot.Collections.Dictionary PackageDetails => GetDict("package_details");
    public bool IsValid => GetBool("valid");

    public XboxResult ResolvePath(string relativePath) =>
        XboxResult.From(Call("resolve_path", relativePath).AsGodotObject());

    public XboxResult Close() => XboxResult.From(Call("close").AsGodotObject());
}

/// <summary>A loaded resource pack from a package.</summary>
public sealed class XboxPackageResourcePack : XboxObject
{
    internal XboxPackageResourcePack(GodotObject o) : base(o) { }
    public static XboxPackageResourcePack From(GodotObject o) => o == null ? null : new XboxPackageResourcePack(o);

    public string PackageIdentifier => GetString("package_identifier");
    public string MountPath => GetString("mount_path");
    public string PackRelativePath => GetString("pack_relative_path");
    public string PackPath => GetString("pack_path");
    public Godot.Collections.Dictionary PackageDetails => GetDict("package_details");
    public bool ReplaceFiles => GetBool("replace_files");
    public long Offset => GetInt("offset");
}
