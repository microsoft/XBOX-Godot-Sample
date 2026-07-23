using System;
using System.Collections.Generic;
using Godot;

namespace GodotGameInput;

/// <summary>
/// Static entry point to the Microsoft GameInput runtime, mirroring the native
/// <c>GameInput</c> engine singleton. Unlike the GDK / PlayFab facades this API
/// is fully synchronous (poll-based) and integrates with Godot's
/// <c>Input</c>/<c>InputMap</c> flow rather than exposing an async result model.
/// </summary>
public static class GameInput
{
    private static GodotObject _singleton;
    private static bool _signalsConnected;

    /// <summary>Device-kind bit flags, matching native <c>GameInput.DeviceKind</c>.</summary>
    [Flags]
    public enum DeviceKind
    {
        Unknown = 0,
        Gamepad = 1,
        Keyboard = 2,
        Mouse = 4,
        All = 7,
    }

    /// <summary>True when the <c>godot_gameinput</c> GDExtension is loaded.</summary>
    public static bool IsAvailable => Engine.HasSingleton("GameInput");

    internal static GodotObject Singleton
    {
        get
        {
            if (_singleton != null && GodotObject.IsInstanceValid(_singleton))
            {
                return _singleton;
            }

            _singleton = Engine.HasSingleton("GameInput") ? Engine.GetSingleton("GameInput") : null;
            EnsureSignalsConnected();
            return _singleton;
        }
    }

    private static GodotObject Require()
    {
        return Singleton
            ?? throw new InvalidOperationException(
                "GameInput singleton is not registered. Is the godot_gameinput GDExtension built and loaded?");
    }

    // --- Lifecycle ---
    public static bool Initialize() => Require().Call("initialize").AsBool();

    public static void Shutdown() => Singleton?.Call("shutdown");

    public static bool IsInitialized => Singleton != null && Singleton.Call("is_initialized").AsBool();

    public static void Poll() => Singleton?.Call("poll");

    // --- Devices ---
    public static IReadOnlyList<GameInputDevice> GetDevices(DeviceKind kindMask = DeviceKind.Gamepad)
    {
        var result = new List<GameInputDevice>();
        if (Singleton == null)
        {
            return result;
        }

        Godot.Collections.Array devices = Singleton.Call("get_devices", (int)kindMask).AsGodotArray();
        foreach (Variant device in devices)
        {
            GameInputDevice wrapped = GameInputDevice.From(device.AsGodotObject());
            if (wrapped != null)
            {
                result.Add(wrapped);
            }
        }

        return result;
    }

    public static GameInputDevice GetPrimaryDevice(DeviceKind kindMask = DeviceKind.Gamepad) =>
        Singleton == null
            ? null
            : GameInputDevice.From(Singleton.Call("get_primary_device", (int)kindMask).AsGodotObject());

    public static GameInputReading GetCurrentReading(GameInputDevice device) =>
        Singleton == null || device == null
            ? null
            : GameInputReading.From(Singleton.Call("get_current_reading", device.Raw).AsGodotObject());

    public static int ConnectedDeviceCount =>
        Singleton == null ? 0 : Singleton.Call("get_connected_device_count").AsInt32();

    // --- Haptics ---
    public static bool SetVibration(GameInputDevice device, float lowFreq, float highFreq,
        float leftTrigger = 0.0f, float rightTrigger = 0.0f)
    {
        if (Singleton == null || device == null)
        {
            return false;
        }

        return Singleton.Call("set_vibration", device.Raw, lowFreq, highFreq, leftTrigger, rightTrigger).AsBool();
    }

    public static void StopHaptics(GameInputDevice device)
    {
        if (Singleton != null && device != null)
        {
            Singleton.Call("stop_haptics", device.Raw);
        }
    }

    // --- Signals (main-thread, connected once on first singleton resolution) ---
    public static event Action<GameInputDevice> DeviceConnected;
    public static event Action<long> DeviceDisconnected;

    private static void EnsureSignalsConnected()
    {
        if (_signalsConnected || _singleton == null)
        {
            return;
        }

        _signalsConnected = true;
        _singleton.Connect("device_connected",
            Callable.From((GodotObject device) => DeviceConnected?.Invoke(GameInputDevice.From(device))));
        _singleton.Connect("device_disconnected",
            Callable.From((long deviceId) => DeviceDisconnected?.Invoke(deviceId)));
    }
}
