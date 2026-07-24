using System;
using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.error_reporting</c> — XError callback configuration.</summary>
public sealed class GdkErrorReporting : GdkServiceBase
{
    internal GdkErrorReporting(GodotObject o) : base(o)
    {
        _o.Connect("error_reported", Callable.From((Variant a0) =>
            ErrorReported?.Invoke(GdkResult.From(a0.AsGodotObject()))));
    }

    public event Action<GdkResult> ErrorReported;

    public GdkResult ConfigureOptions(int debuggerPresentOptions, int debuggerNotPresentOptions) =>
        GdkResult.From(Call("configure_options", debuggerPresentOptions, debuggerNotPresentOptions).AsGodotObject());

    public GdkResult SetCallbackEnabled(bool enabled) =>
        GdkResult.From(Call("set_callback_enabled", enabled).AsGodotObject());

    public bool IsCallbackEnabled() => Call("is_callback_enabled").AsBool();
}
