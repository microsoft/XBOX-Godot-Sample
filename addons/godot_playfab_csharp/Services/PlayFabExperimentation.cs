using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabExperimentation : PlayFabServiceBase
{
    internal PlayFabExperimentation(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> GetTreatmentAssignmentAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_treatment_assignment_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
