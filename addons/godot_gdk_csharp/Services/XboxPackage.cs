using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary><c>GDK.package</c> — package metadata, mounts, and resource packs (DLC).</summary>
public sealed class XboxPackage : XboxServiceBase
{
    internal XboxPackage(GodotObject o) : base(o) { }

    public XboxResult EnumeratePackages(int packageKind, int scope) =>
        XboxResult.From(Call("enumerate_packages", packageKind, scope).AsGodotObject());

    public XboxResult FindPackageByIdentifier(string packageIdentifier, int packageKind, int scope) =>
        XboxResult.From(Call("find_package_by_identifier", packageIdentifier, packageKind, scope).AsGodotObject());

    public XboxResult GetCurrentProcessPackageIdentifier() =>
        XboxResult.From(Call("get_current_process_package_identifier").AsGodotObject());

    public Task<XboxResult> MountPackageAsync(string packageIdentifier) =>
        CallResultAsync("mount_package_async", packageIdentifier);

    public Task<XboxResult> LoadResourcePackAsync(string packageIdentifier, string packRelativePath, bool replaceFiles, long offset) =>
        CallResultAsync("load_resource_pack_async", packageIdentifier, packRelativePath, replaceFiles, offset);

    public Godot.Collections.Array GetLoadedResourcePacks() =>
        Call("get_loaded_resource_packs").AsGodotArray();

    public XboxResult GetInstallProgress(string packageIdentifier) =>
        XboxResult.From(Call("get_install_progress", packageIdentifier).AsGodotObject());
}
