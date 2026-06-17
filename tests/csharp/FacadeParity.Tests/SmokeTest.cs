using Xunit;

namespace FacadeParity.Tests;

/// <summary>Smoke test: confirms the facade assemblies load and reflect.</summary>
public class SmokeTest
{
    [Fact]
    public void GdkFacadeAssemblyLoads()
    {
        System.Type gdk = typeof(GodotGdk.Gdk);
        Assert.NotNull(gdk);
    }

    [Fact]
    public void PlayFabFacadeAssemblyLoads()
    {
        System.Type playfab = typeof(GodotPlayFab.PlayFab);
        Assert.NotNull(playfab);
    }

    [Fact]
    public void GameInputFacadeAssemblyLoads()
    {
        System.Type gameInput = typeof(GodotGameInput.GameInput);
        Assert.NotNull(gameInput);
    }
}
