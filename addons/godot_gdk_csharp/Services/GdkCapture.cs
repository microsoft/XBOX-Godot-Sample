using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.capture</c> — game DVR capture and diagnostic clips/screenshots.</summary>
public sealed class GdkCapture : GdkServiceBase
{
    internal GdkCapture(GodotObject o) : base(o) { }

    public GdkResult EnableCapture() => GdkResult.From(Call("enable_capture").AsGodotObject());

    public GdkResult DisableCapture() => GdkResult.From(Call("disable_capture").AsGodotObject());

    public Task<GdkResult> RecordDiagnosticClipAsync(double duration) =>
        CallResultAsync("record_diagnostic_clip_async", duration);

    public Task<GdkResult> TakeDiagnosticScreenshotAsync(string pathHint) =>
        CallResultAsync("take_diagnostic_screenshot_async", pathHint);

    public GdkCaptureMetaData CreateMetadata(int reservedBytes) =>
        GdkCaptureMetaData.From(Call("create_metadata", reservedBytes).AsGodotObject());
}
