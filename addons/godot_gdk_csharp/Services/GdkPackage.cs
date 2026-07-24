using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.package</c> — package metadata, mounts, and resource packs (DLC).</summary>
public sealed class GdkPackage : GdkServiceBase
{
    internal GdkPackage(GodotObject o) : base(o) { }

    public GdkResult EnumeratePackages(int packageKind, int scope) =>
        GdkResult.From(Call("enumerate_packages", packageKind, scope).AsGodotObject());

    public GdkResult FindPackageByIdentifier(string packageIdentifier, int packageKind, int scope) =>
        GdkResult.From(Call("find_package_by_identifier", packageIdentifier, packageKind, scope).AsGodotObject());

    public GdkResult GetCurrentProcessPackageIdentifier() =>
        GdkResult.From(Call("get_current_process_package_identifier").AsGodotObject());

    public Task<GdkResult> MountPackageAsync(string packageIdentifier) =>
        CallResultAsync("mount_package_async", packageIdentifier);

    public Task<GdkResult> LoadResourcePackAsync(string packageIdentifier, string packRelativePath, bool replaceFiles, long offset) =>
        CallResultAsync("load_resource_pack_async", packageIdentifier, packRelativePath, replaceFiles, offset);

    public Godot.Collections.Array GetLoadedResourcePacks() =>
        Call("get_loaded_resource_packs").AsGodotArray();

    public GdkResult GetInstallProgress(string packageIdentifier) =>
        GdkResult.From(Call("get_install_progress", packageIdentifier).AsGodotObject());
}
