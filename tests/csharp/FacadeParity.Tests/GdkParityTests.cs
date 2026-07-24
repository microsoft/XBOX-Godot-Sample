using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using Xunit;

namespace FacadeParity.Tests;

public class GdkParityTests
{
    private static readonly Assembly Facade = typeof(GodotGdk.Gdk).Assembly;

    public static IEnumerable<object[]> Classes() =>
        Directory.GetFiles(RepoPaths.DocClasses("godot_gdk"), "*.xml")
            .Select(f => new object[] { Path.GetFileNameWithoutExtension(f) });

    [Theory]
    [MemberData(nameof(Classes))]
    public void NativeClassHasManagedWrapper(string nativeClass)
    {
        System.Type csharpType = ParityChecker.ResolveType(Facade, nativeClass, "GDK", typeof(GodotGdk.Gdk));
        Assert.True(csharpType != null, $"No C# facade type found for native class '{nativeClass}'.");

        string xml = Path.Combine(RepoPaths.DocClasses("godot_gdk"), nativeClass + ".xml");
        List<string> missing = ParityChecker.FindMissingMembers(xml, csharpType);

        Assert.True(missing.Count == 0,
            $"{csharpType.FullName} is missing wrappers for native members: {string.Join(", ", missing)}");
    }
}
