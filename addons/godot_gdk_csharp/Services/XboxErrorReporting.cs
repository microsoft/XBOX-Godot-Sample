using System;
using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary><c>GDK.error_reporting</c> — XError callback configuration.</summary>
public sealed class XboxErrorReporting : XboxServiceBase
{
    internal XboxErrorReporting(GodotObject o) : base(o)
    {
        _o.Connect("error_reported", Callable.From((Variant a0) =>
            ErrorReported?.Invoke(XboxResult.From(a0.AsGodotObject()))));
    }

    public event Action<XboxResult> ErrorReported;

    public XboxResult ConfigureOptions(int debuggerPresentOptions, int debuggerNotPresentOptions) =>
        XboxResult.From(Call("configure_options", debuggerPresentOptions, debuggerNotPresentOptions).AsGodotObject());

    public XboxResult SetCallbackEnabled(bool enabled) =>
        XboxResult.From(Call("set_callback_enabled", enabled).AsGodotObject());

    public bool IsCallbackEnabled() => Call("is_callback_enabled").AsBool();
}
