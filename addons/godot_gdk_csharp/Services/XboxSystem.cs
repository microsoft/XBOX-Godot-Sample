using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary><c>GDK.system</c> — title/sandbox/service-config metadata.</summary>
public sealed class XboxSystem : XboxServiceBase
{
    internal XboxSystem(GodotObject o) : base(o) { }

    public XboxResult GetTitleId() => XboxResult.From(Call("get_title_id").AsGodotObject());
    public XboxResult GetTitleIdHex() => XboxResult.From(Call("get_title_id_hex").AsGodotObject());
    public XboxResult GetSandboxId() => XboxResult.From(Call("get_sandbox_id").AsGodotObject());
    public XboxResult GetServiceConfigurationId() => XboxResult.From(Call("get_service_configuration_id").AsGodotObject());
    public bool IsXboxServicesInitialized() => Call("is_xbox_services_initialized").AsBool();

    public bool IsFeatureAvailable(string featureName) =>
        Call("is_feature_available", featureName).AsBool();
}
