using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.capture</c> — game DVR capture and diagnostic clips/screenshots.</summary>
public sealed class XboxCapture : XboxServiceBase
{
    internal XboxCapture(GodotObject o) : base(o) { }

    public XboxResult EnableCapture() => XboxResult.From(Call("enable_capture").AsGodotObject());

    public XboxResult DisableCapture() => XboxResult.From(Call("disable_capture").AsGodotObject());

    public Task<XboxResult> RecordDiagnosticClipAsync(double duration) =>
        CallResultAsync("record_diagnostic_clip_async", duration);

    public Task<XboxResult> TakeDiagnosticScreenshotAsync(string pathHint) =>
        CallResultAsync("take_diagnostic_screenshot_async", pathHint);

    public XboxCaptureMetaData CreateMetadata(int reservedBytes) =>
        XboxCaptureMetaData.From(Call("create_metadata", reservedBytes).AsGodotObject());
}
