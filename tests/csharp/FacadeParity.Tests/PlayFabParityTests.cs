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
            parameter =>
            {
                Assert.Equal("user", parameter.Name);
                Assert.Equal(typeof(GodotPlayFab.Types.PlayFabUser), parameter.ParameterType);
                Assert.False(parameter.IsOptional);
            },
            parameter =>
            {
                Assert.Equal("leaderboard_name", parameter.Name);
                Assert.Equal(typeof(string), parameter.ParameterType);
                Assert.False(parameter.IsOptional);
            },
            parameter =>
            {
                Assert.Equal("include_xbox_friends", parameter.Name);
                Assert.Equal(typeof(bool), parameter.ParameterType);
                Assert.True(parameter.IsOptional);
                Assert.Equal(true, parameter.DefaultValue);
            },
            parameter =>
            {
                Assert.Equal("version", parameter.Name);
                Assert.Equal(typeof(int), parameter.ParameterType);
                Assert.True(parameter.IsOptional);
                Assert.Equal(-1, parameter.DefaultValue);
            });
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
            parameter =>
            {
                Assert.Equal("user", parameter.Name);
                Assert.Equal(typeof(GodotPlayFab.Types.PlayFabUser), parameter.ParameterType);
                Assert.False(parameter.IsOptional);
            },
            parameter =>
            {
                Assert.Equal("leaderboard_name", parameter.Name);
                Assert.Equal(typeof(string), parameter.ParameterType);
                Assert.False(parameter.IsOptional);
            },
            parameter =>
            {
                Assert.Equal("friend_sources", parameter.Name);
                Assert.Equal(enumType, parameter.ParameterType);
                Assert.False(parameter.IsOptional);
            },
            parameter =>
            {
                Assert.Equal("version", parameter.Name);
                Assert.Equal(typeof(int), parameter.ParameterType);
                Assert.True(parameter.IsOptional);
                Assert.Equal(-1, parameter.DefaultValue);
            });

        Assert.NotNull(enumType.GetCustomAttribute<FlagsAttribute>());
        Assert.Equal(typeof(long), Enum.GetUnderlyingType(enumType));

        var expectedValues = new Dictionary<string, long>
        {
            ["None"] = 0,
            ["Steam"] = 1,
            ["Facebook"] = 2,
            ["Xbox"] = 4,
            ["Psn"] = 8,
            ["All"] = 16,
        };
        foreach ((string name, long expectedValue) in expectedValues)
        {
            Assert.Equal(expectedValue, Convert.ToInt64(Enum.Parse(enumType, name)));
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

        var nativeToManaged = new Dictionary<string, string>
        {
            ["FRIEND_SOURCE_NONE"] = "None",
            ["FRIEND_SOURCE_STEAM"] = "Steam",
            ["FRIEND_SOURCE_FACEBOOK"] = "Facebook",
            ["FRIEND_SOURCE_XBOX"] = "Xbox",
            ["FRIEND_SOURCE_PSN"] = "Psn",
            ["FRIEND_SOURCE_ALL"] = "All",
        };
        foreach ((string nativeName, string managedName) in nativeToManaged)
        {
            Assert.True(nativeValues.TryGetValue(nativeName, out long nativeValue));
            Assert.Equal(
                Convert.ToInt64(Enum.Parse(enumType, managedName)),
                nativeValue);
        }
    }
}
