using Xunit;

namespace FacadeParity.Tests;

/// <summary>
/// Confirms <c>Result.From(null)</c> produces a diagnosable synthetic failure
/// (stable <c>Code</c> + non-empty <c>Message</c>) rather than an
/// <c>Ok == false</c> result with blank fields, so a native call that returns
/// <c>Nil</c> (e.g. method/signature drift) is branchable by callers. These
/// run headless: <c>From(null)</c> never touches the native object.
/// </summary>
public class ResultSyntheticFailureTests
{
    [Fact]
    public void GdkResultFromNullIsDiagnosableFailure()
    {
        GodotGdk.GdkResult r = GodotGdk.GdkResult.From(null);
        Assert.False(r.Ok);
        Assert.Equal("null_native_result", r.Code);
        Assert.False(string.IsNullOrEmpty(r.Message));
    }

    [Fact]
    public void PlayFabResultFromNullIsDiagnosableFailure()
    {
        GodotPlayFab.PlayFabResult r = GodotPlayFab.PlayFabResult.From(null);
        Assert.False(r.Ok);
        Assert.Equal("null_native_result", r.Code);
        Assert.False(string.IsNullOrEmpty(r.Message));
    }
}
