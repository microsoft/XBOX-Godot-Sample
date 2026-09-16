using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Xml.Linq;
using Xunit;

namespace FacadeParity.Tests;

public class PlayFabParityTests
{
    private static readonly Assembly Facade = typeof(GodotPlayFab.PlayFab).Assembly;

    private static void AssertParameter(
        ParameterInfo parameter,
        string expectedName,
        Type expectedType,
        bool expectedOptional,
        object expectedDefaultValue = null)
    {
        Assert.Equal(expectedName, parameter.Name);
        Assert.Equal(expectedType, parameter.ParameterType);
        Assert.Equal(expectedOptional, parameter.IsOptional);
        if (expectedOptional)
        {
            Assert.Equal(expectedDefaultValue, parameter.DefaultValue);
        }
    }

    public static IEnumerable<object[]> Classes() =>
        Directory.GetFiles(RepoPaths.DocClasses("godot_playfab"), "*.xml")
            .Select(f => new object[] { Path.GetFileNameWithoutExtension(f) });

    [Theory]
    [MemberData(nameof(Classes))]
    public void NativeClassHasManagedWrapper(string nativeClass)
    {
        System.Type csharpType = ParityChecker.ResolveType(Facade, nativeClass, "PlayFab", typeof(GodotPlayFab.PlayFab));
        Assert.True(csharpType != null, $"No C# facade type found for native class '{nativeClass}'.");

        string xml = Path.Combine(RepoPaths.DocClasses("godot_playfab"), nativeClass + ".xml");
        List<string> missing = ParityChecker.FindMissingMembers(xml, csharpType);

        Assert.True(missing.Count == 0,
            $"{csharpType.FullName} is missing wrappers for native members: {string.Join(", ", missing)}");
    }

    [Fact]
    public void FriendLeaderboardCompatibilityMethodKeepsItsSignature()
    {
        MethodInfo method = typeof(GodotPlayFab.Services.PlayFabLeaderboards)
            .GetMethod(nameof(GodotPlayFab.Services.PlayFabLeaderboards.GetFriendLeaderboardAsync));

        Assert.NotNull(method);
        Assert.Equal(typeof(Task<GodotPlayFab.PlayFabResult>), method.ReturnType);

        ParameterInfo[] parameters = method.GetParameters();
        Assert.Collection(
            parameters,
            parameter => AssertParameter(
                parameter, "user", typeof(GodotPlayFab.Types.PlayFabUser), false),
            parameter => AssertParameter(
                parameter, "leaderboard_name", typeof(string), false),
            parameter => AssertParameter(
                parameter, "include_xbox_friends", typeof(bool), true, true),
            parameter => AssertParameter(
                parameter, "version", typeof(int), true, -1));
    }

    [Fact]
    public void FriendLeaderboardSourceMethodAndEnumMatchNativeContract()
    {
        Type facadeType = typeof(GodotPlayFab.Services.PlayFabLeaderboards);
        Type enumType = typeof(GodotPlayFab.Services.PlayFabLeaderboards.FriendSources);
        MethodInfo method = facadeType.GetMethod(
            nameof(GodotPlayFab.Services.PlayFabLeaderboards.GetFriendLeaderboardWithSourcesAsync));

        Assert.NotNull(method);
        Assert.Equal(typeof(Task<GodotPlayFab.PlayFabResult>), method.ReturnType);

        ParameterInfo[] parameters = method.GetParameters();
        Assert.Collection(
            parameters,
            parameter => AssertParameter(
                parameter, "user", typeof(GodotPlayFab.Types.PlayFabUser), false),
            parameter => AssertParameter(
                parameter, "leaderboard_name", typeof(string), false),
            parameter => AssertParameter(
                parameter, "friend_sources", enumType, false),
            parameter => AssertParameter(
                parameter, "version", typeof(int), true, -1));

        Assert.NotNull(enumType.GetCustomAttribute<FlagsAttribute>());
        Assert.Equal(typeof(long), Enum.GetUnderlyingType(enumType));

        var values = new (string NativeName, string ManagedName, long Value)[]
        {
            ("FRIEND_SOURCE_NONE", "None", 0),
            ("FRIEND_SOURCE_STEAM", "Steam", 1),
            ("FRIEND_SOURCE_FACEBOOK", "Facebook", 2),
            ("FRIEND_SOURCE_XBOX", "Xbox", 4),
            ("FRIEND_SOURCE_PSN", "Psn", 8),
            ("FRIEND_SOURCE_ALL", "All", 16),
        };
        foreach ((string _, string managedName, long value) in values)
        {
            Assert.Equal(value, Convert.ToInt64(Enum.Parse(enumType, managedName)));
        }

        var combined =
            GodotPlayFab.Services.PlayFabLeaderboards.FriendSources.Steam |
            GodotPlayFab.Services.PlayFabLeaderboards.FriendSources.Xbox;
        Assert.Equal(5L, (long)combined);

        XDocument nativeDocs = XDocument.Load(
            Path.Combine(RepoPaths.DocClasses("godot_playfab"), "PlayFabLeaderboards.xml"));
        Dictionary<string, long> nativeValues = nativeDocs.Root
            .Element("constants")
            .Elements("constant")
            .Where(constant => (string)constant.Attribute("enum") == "FriendSources")
            .ToDictionary(
                constant => (string)constant.Attribute("name"),
                constant => long.Parse((string)constant.Attribute("value")));

        foreach ((string nativeName, string managedName, long _) in values)
        {
            Assert.True(nativeValues.TryGetValue(nativeName, out long nativeValue));
            Assert.Equal(
                Convert.ToInt64(Enum.Parse(enumType, managedName)),
                nativeValue);
        }
    }
}
