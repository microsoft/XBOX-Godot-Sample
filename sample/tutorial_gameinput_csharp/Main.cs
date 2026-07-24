using Godot;
using GodotGameInput;

namespace TutorialGameInputCSharp;

/// <summary>
/// GameInput action-bridge tutorial (C# port of <c>sample/tutorial_gameinput/main.gd</c>).
///
/// Builds a <see cref="GameInputActionMap"/> programmatically, attaches a
/// <see cref="GameInputMapper"/> that polls every frame and drives Godot's
/// <c>InputMap</c>, and renders live action state plus device hot-plug events.
/// Independent of GDK / PlayFab — no sign-in flow.
/// </summary>
public partial class Main : Control
{
    private const float PlayerSpeed = 240.0f;
    private const float PlayerJumpVelocity = -480.0f;
    private const float PlayerGravity = 1200.0f;
    private const float PlayerFloorY = 320.0f;

    private Label _runtimeStatus;
    private Label _deviceCount;
    private Label _devices;
    private Label _actionState;
    private RichTextLabel _hotplugLog;
    private ColorRect _player;

    private float _playerVelocityY;
    private GameInputMapper _mapper;

    public override void _Ready()
    {
        _runtimeStatus = GetNode<Label>("Root/RuntimeStatus");
        _deviceCount = GetNode<Label>("Root/DeviceCount");
        _devices = GetNode<Label>("Root/Devices");
        _actionState = GetNode<Label>("Root/ActionState");
        _hotplugLog = GetNode<RichTextLabel>("Root/HotplugLog");
        _player = GetNode<ColorRect>("Player");

        if (!GameInput.IsAvailable)
        {
            _runtimeStatus.Text = "GameInput singleton missing. Build the addon (cmake --build build --preset debug).";
            _deviceCount.Text = "";
            _devices.Text = "";
            _actionState.Text = "";
            return;
        }

        if (!GameInput.IsInitialized)
        {
            GD.PushWarning("[Pad] GameInput runtime not available — gamepad input disabled.");
            _runtimeStatus.Text = "GameInput runtime NOT initialized (set game_input/runtime/initialize_on_startup=true).";
        }
        else
        {
            _runtimeStatus.Text = "GameInput runtime initialized.";
        }

        _mapper = GameInputMapper.Create();
        _mapper.Node.Name = "GamepadMapper";
        _mapper.ActionMap = BuildDefaultMap();
        AddChild(_mapper.Node);

        GameInput.DeviceConnected += OnDeviceConnected;
        GameInput.DeviceDisconnected += OnDeviceDisconnected;

        RefreshDevices();
        AppendHotplug($"Seeded with {GameInput.ConnectedDeviceCount} device(s) at startup");

        _player.Position = new Vector2(GetViewportRect().Size.X * 0.5f, PlayerFloorY);
    }

    private static GameInputActionMap BuildDefaultMap()
    {
        GameInputActionMap map = GameInputActionMap.Create();

        GameInputBinding accept = GameInputBinding.Create();
        accept.Action = "ui_accept";
        accept.Source = GameInputDevice.Source.BtnA;
        map.AddBinding(accept);

        GameInputBinding jump = GameInputBinding.Create();
        jump.Action = "jump";
        jump.Source = GameInputDevice.Source.BtnA;
        map.AddBinding(jump);

        GameInputBinding left = GameInputBinding.Create();
        left.Action = "move_left";
        left.Source = GameInputDevice.Source.AxisLeftX;
        left.IsAxis = true;
        left.AxisInvert = true;
        map.AddBinding(left);

        GameInputBinding right = GameInputBinding.Create();
        right.Action = "move_right";
        right.Source = GameInputDevice.Source.AxisLeftX;
        right.IsAxis = true;
        map.AddBinding(right);

        return map;
    }

    public override void _PhysicsProcess(double delta)
    {
        if (!GameInput.IsAvailable)
        {
            return;
        }

        float direction = Input.GetActionStrength("move_right") - Input.GetActionStrength("move_left");
        Vector2 position = _player.Position;
        position.X += direction * PlayerSpeed * (float)delta;

        if (Input.IsActionJustPressed("jump") && position.Y >= PlayerFloorY)
        {
            _playerVelocityY = PlayerJumpVelocity;
        }

        _playerVelocityY += PlayerGravity * (float)delta;
        position.Y += _playerVelocityY * (float)delta;
        if (position.Y >= PlayerFloorY)
        {
            position.Y = PlayerFloorY;
            _playerVelocityY = 0.0f;
        }

        float viewportWidth = GetViewportRect().Size.X;
        position.X = Mathf.Clamp(position.X, 0.0f, viewportWidth - _player.Size.X);
        _player.Position = position;
    }

    public override void _Process(double delta)
    {
        if (!GameInput.IsAvailable)
        {
            return;
        }

        _actionState.Text =
            $"move_left={Input.GetActionStrength("move_left"):0.00}  " +
            $"move_right={Input.GetActionStrength("move_right"):0.00}  " +
            $"jump={Input.IsActionPressed("jump")}  " +
            $"ui_accept={Input.IsActionPressed("ui_accept")}";
    }

    private void OnDeviceConnected(GameInputDevice device)
    {
        AppendHotplug($"connected: id={device.DeviceId} ({device.DisplayName})");
        RefreshDevices();
    }

    private void OnDeviceDisconnected(long deviceId)
    {
        AppendHotplug($"disconnected: id={deviceId}");
        RefreshDevices();
    }

    private void RefreshDevices()
    {
        _deviceCount.Text = $"Connected gamepads: {GameInput.ConnectedDeviceCount}";

        var lines = new System.Collections.Generic.List<string>();
        foreach (GameInputDevice device in GameInput.GetDevices(GameInput.DeviceKind.Gamepad))
        {
            lines.Add($"- id={device.DeviceId} {device.DisplayName}");
        }

        if (lines.Count == 0)
        {
            lines.Add("- (none — plug in a gamepad)");
        }

        _devices.Text = string.Join("\n", lines);
    }

    private void AppendHotplug(string line) => _hotplugLog.AppendText(line + "\n");
}
