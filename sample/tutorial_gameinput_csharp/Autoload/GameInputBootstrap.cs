using GodotGameInput.Runtime;

namespace TutorialGameInputCSharp;

/// <summary>
/// Project-local autoload entry point. Subclasses the facade's
/// <see cref="GameInputRuntime"/> so Godot can register it as an autoload by
/// script path (autoloads resolve a script in the project tree, while the
/// runtime logic lives in the referenced <c>GodotGameInputCSharp</c> assembly).
/// </summary>
public partial class GameInputBootstrap : GameInputRuntime
{
}
