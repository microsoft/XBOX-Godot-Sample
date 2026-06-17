using Godot;
using GodotGdk;
using GodotGdk.Services;
using GodotGdk.Types;

/// <summary>
/// Integrated tech demo — GDK Game Chat panel.
///
/// Demonstrates GDK-native voice + text chat with single-process loopback: the
/// title-owned transport is represented by feeding outgoing frames straight
/// back into <see cref="GdkGameChat.ProcessIncomingDataFrame"/>.
/// </summary>
public partial class PanelGameChat : VBoxContainer
{
    private const string LoopbackXuid = "2814639011419087";
    private const int LoopbackEndpoint = 1;

    private Label _status;
    private RichTextLabel _chatLog;
    private LineEdit _chatInput;
    private Button _send;
    private CheckButton _muteMic;
    private Auth _auth;
    private GdkGameChat _chat;
    private string _localXuid = string.Empty;
    private bool _initialized;

    public override async void _Ready()
    {
        _status = GetNode<Label>("Status");
        _chatLog = GetNode<RichTextLabel>("ChatLog");
        _chatInput = GetNode<LineEdit>("ChatInput");
        _send = GetNode<Button>("Send");
        _muteMic = GetNode<CheckButton>("MuteMic");

        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null)
        {
            _status.Text = "[ERR] Auth autoload missing";
            return;
        }
        if (!Gdk.IsAvailable)
        {
            _status.Text = "[ERR] godot_gdk extension not loaded";
            return;
        }

        _auth.StateChanged += OnAuthStateChanged;
        if (_auth.IsSignedIn())
        {
            InitializeAfterSignIn();
            return;
        }

        await _auth.SignInAsync();
        if (IsInsideTree() && _auth.IsSignedIn()) InitializeAfterSignIn();
    }

    public override void _ExitTree()
    {
        if (_auth != null) _auth.StateChanged -= OnAuthStateChanged;
        if (!_initialized || _chat == null) return;

        _chat.OutgoingDataFrame -= OnOutgoingDataFrame;
        _chat.TextChatReceived -= OnTextChatReceived;
        _chat.TranscribedChatReceived -= OnTranscribedChatReceived;
        _chat.Cleanup();
    }

    private void OnAuthStateChanged(Auth.State state)
    {
        if (!_initialized && _auth != null && _auth.IsSignedIn()) InitializeAfterSignIn();
    }

    private void InitializeAfterSignIn()
    {
        if (_initialized) return;
        _initialized = true;
        _send.Pressed += OnSendPressed;
        _muteMic.Toggled += OnMuteMicToggled;

        _chat = Gdk.GameChat;
        GdkResult init = _chat.Initialize(16, GdkGameChat.RELATIONSHIPSENDANDRECEIVEALL);
        if (!init.Ok)
        {
            _status.Text = $"game_chat.initialize() failed: {init.Message} ({init.Code})";
            return;
        }

        _chat.OutgoingDataFrame += OnOutgoingDataFrame;
        _chat.TextChatReceived += OnTextChatReceived;
        _chat.TranscribedChatReceived += OnTranscribedChatReceived;

        GdkResult addLocal = _chat.AddLocalUser(_auth.XboxUser);
        if (!addLocal.Ok)
        {
            _status.Text = $"add_local_user() failed: {addLocal.Message} ({addLocal.Code})";
            return;
        }
        _localXuid = TutorialSupport.DictString(addLocal.Data.AsGodotDictionary(), "xuid");

        GdkResult addRemote = _chat.AddRemoteUser(LoopbackXuid, LoopbackEndpoint);
        if (!addRemote.Ok)
        {
            _status.Text = $"add_remote_user() failed: {addRemote.Message}";
            return;
        }

        string shortXuid = _localXuid.Length > 8 ? _localXuid[..8] : _localXuid;
        _status.Text = $"Game Chat ready — local {shortXuid} + loopback peer on endpoint {LoopbackEndpoint}";
        _chatLog.AppendText("[i]No transport built: outgoing frames are looped back to the local manager.[/i]\n");
    }

    private void OnSendPressed()
    {
        string text = _chatInput.Text.StripEdges();
        if (string.IsNullOrEmpty(text) || _chat == null || string.IsNullOrEmpty(_localXuid)) return;

        GdkResult result = _chat.SendText(_localXuid, text);
        if (result.Ok)
        {
            _chatLog.AppendText($"[me] {text}\n");
            _chatInput.Text = string.Empty;
        }
        else
        {
            _chatLog.AppendText($"[i]send_text failed: {result.Message} ({result.Code})[/i]\n");
        }
    }

    private void OnMuteMicToggled(bool buttonPressed)
    {
        if (_chat == null || string.IsNullOrEmpty(_localXuid)) return;
        GdkResult result = _chat.SetMicrophoneMuted(_localXuid, buttonPressed);
        if (result.Ok)
        {
            _chatLog.AppendText($"[i]microphone {(buttonPressed ? "muted" : "unmuted")} (text chat still works)[/i]\n");
        }
        else
        {
            _chatLog.AppendText($"[i]set_microphone_muted failed: {result.Message}[/i]\n");
        }
    }

    private void OnOutgoingDataFrame(long[] targetEndpointIds, byte[] bytes, int transportRequirement)
    {
        if (_chat == null) return;
        foreach (long endpoint in targetEndpointIds)
        {
            GdkResult result = _chat.ProcessIncomingDataFrame((int)endpoint, bytes);
            if (!result.Ok) _chatLog.AppendText($"[i]process_incoming_data_frame failed: {result.Message}[/i]\n");
        }
        _chatLog.AppendText($"[frame] looped {bytes.Length} byte(s) back to {targetEndpointIds.Length} endpoint(s)\n");
    }

    private void OnTextChatReceived(string senderXuid, string message)
    {
        string label = senderXuid.Length > 8 ? senderXuid[..8] : senderXuid;
        _chatLog.AppendText($"[{label}] {message}\n");
    }

    private void OnTranscribedChatReceived(string speakerXuid, string message)
    {
        string label = speakerXuid.Length > 8 ? speakerXuid[..8] : speakerXuid;
        _chatLog.AppendText($"[{label} ~voice] {message}\n");
    }
}
