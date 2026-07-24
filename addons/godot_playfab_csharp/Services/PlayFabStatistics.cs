using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabStatistics : PlayFabServiceBase
{
    internal PlayFabStatistics(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> CreateStatisticDefinitionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("create_statistic_definition_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteStatisticDefinitionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_statistic_definition_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteStatisticsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_statistics_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetStatisticDefinitionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_statistic_definition_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetStatisticsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_statistics_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetStatisticsForEntitiesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_statistics_for_entities_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> IncrementStatisticVersionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("increment_statistic_version_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListStatisticDefinitionsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_statistic_definitions_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateStatisticDefinitionAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_statistic_definition_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateStatisticsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_statistics_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
