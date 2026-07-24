using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using Xunit;

namespace FacadeParity.Tests;

/// <summary>
/// Guards the hand-maintained <c>ResetSingletonState()</c> cache-invalidation
/// methods on the <c>Gdk</c> and <c>PlayFab</c> facades. When the native
/// singleton is re-resolved (extension reload / editor restart), these methods
/// must null every cached service-wrapper field; otherwise callers receive
/// wrappers still bound to the freed native instance. This test fails if a new
/// wrapper cache field is added without also being reset, turning the "keep in
/// sync" comment into an enforced invariant.
/// </summary>
public class SingletonResetTests
{
    public static IEnumerable<object[]> Facades() => new[]
    {
        new object[] { typeof(GodotGdk.Gdk) },
        new object[] { typeof(GodotPlayFab.PlayFab) },
    };

    [Theory]
    [MemberData(nameof(Facades))]
    public void ResetSingletonStateNullsEveryCachedWrapper(Type facade)
    {
        MethodInfo reset = facade.GetMethod(
            "ResetSingletonState", BindingFlags.NonPublic | BindingFlags.Static);
        Assert.True(reset != null,
            $"{facade.FullName} has no private static ResetSingletonState() method.");

        // A cached service wrapper is a private static field whose type is a
        // class defined in the facade assembly itself. This deliberately
        // excludes _singleton (GodotObject, GodotSharp assembly) and
        // _signalsConnected (bool), while capturing every wrapper cache.
        FieldInfo[] wrapperFields = facade
            .GetFields(BindingFlags.NonPublic | BindingFlags.Static)
            .Where(f => f.FieldType.IsClass && f.FieldType.Assembly == facade.Assembly)
            .ToArray();

        Assert.True(wrapperFields.Length > 0,
            $"Expected {facade.FullName} to expose cached service-wrapper fields.");

        // Populate each field with a non-null sentinel constructed without
        // running the wrapper constructor (which would require a live native
        // object), then invoke the reset and confirm every field is cleared.
        foreach (FieldInfo f in wrapperFields)
        {
            f.SetValue(null, RuntimeHelpers.GetUninitializedObject(f.FieldType));
        }

        reset.Invoke(null, null);

        string[] notReset = wrapperFields
            .Where(f => f.GetValue(null) != null)
            .Select(f => f.Name)
            .ToArray();

        Assert.True(notReset.Length == 0,
            $"{facade.Name}.ResetSingletonState() did not null cached wrapper field(s): "
            + $"{string.Join(", ", notReset)}. Add them to ResetSingletonState().");
    }
}
