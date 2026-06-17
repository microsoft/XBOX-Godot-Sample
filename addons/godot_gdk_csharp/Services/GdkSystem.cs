using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.system</c> — title/sandbox/service-config metadata.</summary>
public sealed class GdkSystem : GdkServiceBase
{
    internal GdkSystem(GodotObject o) : base(o) { }

    public GdkResult GetTitleId() => GdkResult.From(Call("get_title_id").AsGodotObject());
    public GdkResult GetTitleIdHex() => GdkResult.From(Call("get_title_id_hex").AsGodotObject());
    public GdkResult GetSandboxId() => GdkResult.From(Call("get_sandbox_id").AsGodotObject());
    public GdkResult GetServiceConfigurationId() => GdkResult.From(Call("get_service_configuration_id").AsGodotObject());
    public bool IsXboxServicesInitialized() => Call("is_xbox_services_initialized").AsBool();
}
