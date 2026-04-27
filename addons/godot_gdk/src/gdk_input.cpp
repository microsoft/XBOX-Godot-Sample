#include "gdk_input.h"

#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <GameInput.h>

namespace godot {

GDKInput *GDKInput::singleton = nullptr;

GDKInput *GDKInput::get_singleton() {
    return singleton;
}

GDKInput::GDKInput() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
    memset(m_devices, 0, sizeof(m_devices));
}

GDKInput::~GDKInput() {
    if (m_initialized) {
        shutdown();
    }
    singleton = nullptr;
}

void GDKInput::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &GDKInput::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &GDKInput::shutdown);
    ClassDB::bind_method(D_METHOD("process"), &GDKInput::process);
    ClassDB::bind_method(D_METHOD("is_initialized"), &GDKInput::is_initialized);
    ClassDB::bind_method(D_METHOD("get_connected_device_count"), &GDKInput::get_connected_device_count);
    ClassDB::bind_method(D_METHOD("set_rumble", "joy_id", "low_frequency", "high_frequency", "left_trigger", "right_trigger"),
                         &GDKInput::set_rumble);
    ClassDB::bind_method(D_METHOD("stop_rumble", "joy_id"), &GDKInput::stop_rumble);

    ADD_SIGNAL(MethodInfo("device_connected", PropertyInfo(Variant::INT, "joy_id")));
    ADD_SIGNAL(MethodInfo("device_disconnected", PropertyInfo(Variant::INT, "joy_id")));
}

int GDKInput::find_device_index(IGameInputDevice *device) const {
    for (int i = 0; i < m_device_count; i++) {
        if (m_devices[i].device == device) {
            return i;
        }
    }
    return -1;
}

int GDKInput::find_device_index_by_joy_id(int joy_id) const {
    for (int i = 0; i < m_device_count; i++) {
        if (m_devices[i].godot_joy_id == joy_id) {
            return i;
        }
    }
    return -1;
}

int GDKInput::allocate_joy_id() {
    bool used[MAX_DEVICES] = {};
    for (int i = 0; i < m_device_count; i++) {
        if (m_devices[i].godot_joy_id >= 0 && m_devices[i].godot_joy_id < MAX_DEVICES) {
            used[m_devices[i].godot_joy_id] = true;
        }
    }
    for (int i = 0; i < MAX_DEVICES; i++) {
        if (!used[i]) return i;
    }
    return -1;
}

static void CALLBACK game_input_device_callback(
    GameInputCallbackToken callback_token,
    void *context,
    IGameInputDevice *device,
    uint64_t timestamp,
    GameInputDeviceStatus current_status,
    GameInputDeviceStatus previous_status
) {
    auto *self = static_cast<GDKInput *>(context);

    bool was_connected = (previous_status & GameInputDeviceConnected) != 0;
    bool is_connected = (current_status & GameInputDeviceConnected) != 0;

    if (is_connected && !was_connected) {
        self->on_device_connected(device);
    } else if (!is_connected && was_connected) {
        self->on_device_disconnected(device);
    }
}

void GDKInput::on_device_connected(IGameInputDevice *device) {
    if (m_device_count >= MAX_DEVICES) return;

    const GameInputDeviceInfo *info = device->GetDeviceInfo();
    if (!(info->supportedInput & GameInputKindGamepad)) {
        return;
    }

    int joy_id = allocate_joy_id();
    if (joy_id < 0) return;

    m_devices[m_device_count].device = device;
    m_devices[m_device_count].godot_joy_id = joy_id;
    m_device_count++;

    UtilityFunctions::print("GameInput: Controller connected as joy ", joy_id);
    call_deferred("emit_signal", "device_connected", joy_id);
}

void GDKInput::on_device_disconnected(IGameInputDevice *device) {
    int idx = find_device_index(device);
    if (idx < 0) return;

    int joy_id = m_devices[idx].godot_joy_id;

    for (int i = idx; i < m_device_count - 1; i++) {
        m_devices[i] = m_devices[i + 1];
    }
    m_device_count--;

    UtilityFunctions::print("GameInput: Controller disconnected, joy ", joy_id);
    call_deferred("emit_signal", "device_disconnected", joy_id);
}

Error GDKInput::initialize() {
    if (m_initialized) return ERR_ALREADY_EXISTS;

    HRESULT hr = GameInputCreate(&m_game_input);
    if (FAILED(hr)) {
        UtilityFunctions::push_error("Failed to create GameInput interface");
        return ERR_CANT_CREATE;
    }

    hr = m_game_input->RegisterDeviceCallback(
        nullptr,
        GameInputKindGamepad,
        GameInputDeviceConnected,
        GameInputBlockingEnumeration,
        this,
        game_input_device_callback,
        &m_device_callback_token
    );

    if (FAILED(hr)) {
        UtilityFunctions::push_error("Failed to register GameInput device callback");
        m_game_input->Release();
        m_game_input = nullptr;
        return ERR_CANT_CREATE;
    }

    m_initialized = true;
    UtilityFunctions::print("GameInput initialized, ", m_device_count, " device(s) found");
    return OK;
}

void GDKInput::shutdown() {
    if (!m_initialized) return;

    if (m_game_input && m_device_callback_token) {
        m_game_input->UnregisterCallback(m_device_callback_token, 5000);
        m_device_callback_token = 0;
    }

    m_device_count = 0;

    if (m_game_input) {
        m_game_input->Release();
        m_game_input = nullptr;
    }

    m_initialized = false;
}

// Helper to send a joypad button event through Godot's input system
static void send_joy_button(int device, JoyButton button, bool pressed) {
    Ref<InputEventJoypadButton> ev;
    ev.instantiate();
    ev->set_device(device);
    ev->set_button_index(button);
    ev->set_pressed(pressed);
    ev->set_pressure(pressed ? 1.0f : 0.0f);
    Input::get_singleton()->parse_input_event(ev);
}

// Helper to send a joypad axis event through Godot's input system
static void send_joy_axis(int device, JoyAxis axis, float value) {
    Ref<InputEventJoypadMotion> ev;
    ev.instantiate();
    ev->set_device(device);
    ev->set_axis(axis);
    ev->set_axis_value(value);
    Input::get_singleton()->parse_input_event(ev);
}

void GDKInput::poll_gamepad(int index) {
    IGameInputDevice *device = m_devices[index].device;
    int joy_id = m_devices[index].godot_joy_id;

    IGameInputReading *reading = nullptr;
    HRESULT hr = m_game_input->GetCurrentReading(GameInputKindGamepad, device, &reading);
    if (FAILED(hr) || !reading) return;

    GameInputGamepadState state = {};
    if (reading->GetGamepadState(&state)) {
        GameInputGamepadButtons prev = m_devices[index].prev_buttons;
        GameInputGamepadButtons cur  = state.buttons;

        // Helper: only send button events on state change
        auto edge_button = [&](JoyButton btn, GameInputGamepadButtons mask) {
            bool was = (prev & mask) != 0;
            bool now = (cur  & mask) != 0;
            if (was != now) {
                send_joy_button(joy_id, btn, now);
            }
        };

        edge_button(JOY_BUTTON_A,              GameInputGamepadA);
        edge_button(JOY_BUTTON_B,              GameInputGamepadB);
        edge_button(JOY_BUTTON_X,              GameInputGamepadX);
        edge_button(JOY_BUTTON_Y,              GameInputGamepadY);
        edge_button(JOY_BUTTON_LEFT_SHOULDER,  GameInputGamepadLeftShoulder);
        edge_button(JOY_BUTTON_RIGHT_SHOULDER, GameInputGamepadRightShoulder);
        edge_button(JOY_BUTTON_LEFT_STICK,     GameInputGamepadLeftThumbstick);
        edge_button(JOY_BUTTON_RIGHT_STICK,    GameInputGamepadRightThumbstick);
        edge_button(JOY_BUTTON_BACK,           GameInputGamepadView);
        edge_button(JOY_BUTTON_START,          GameInputGamepadMenu);
        edge_button(JOY_BUTTON_DPAD_UP,        GameInputGamepadDPadUp);
        edge_button(JOY_BUTTON_DPAD_DOWN,      GameInputGamepadDPadDown);
        edge_button(JOY_BUTTON_DPAD_LEFT,      GameInputGamepadDPadLeft);
        edge_button(JOY_BUTTON_DPAD_RIGHT,     GameInputGamepadDPadRight);

        m_devices[index].prev_buttons = cur;

        // Axes (continuous values, always send)
        send_joy_axis(joy_id, JOY_AXIS_LEFT_X,       state.leftThumbstickX);
        send_joy_axis(joy_id, JOY_AXIS_LEFT_Y,      -state.leftThumbstickY);   // Y inverted
        send_joy_axis(joy_id, JOY_AXIS_RIGHT_X,      state.rightThumbstickX);
        send_joy_axis(joy_id, JOY_AXIS_RIGHT_Y,     -state.rightThumbstickY);  // Y inverted
        send_joy_axis(joy_id, JOY_AXIS_TRIGGER_LEFT,  state.leftTrigger);
        send_joy_axis(joy_id, JOY_AXIS_TRIGGER_RIGHT, state.rightTrigger);
    }

    reading->Release();
}

void GDKInput::process() {
    if (!m_initialized) return;

    for (int i = 0; i < m_device_count; i++) {
        poll_gamepad(i);
    }
}

bool GDKInput::is_initialized() const { return m_initialized; }
int GDKInput::get_connected_device_count() const { return m_device_count; }

void GDKInput::set_rumble(int joy_id, float low_frequency, float high_frequency,
                          float left_trigger, float right_trigger) {
    if (!m_initialized) return;

    int idx = find_device_index_by_joy_id(joy_id);
    if (idx < 0) return;

    IGameInputDevice *device = m_devices[idx].device;

    // Verify the device supports rumble
    const GameInputDeviceInfo *info = device->GetDeviceInfo();
    if (info->supportedRumbleMotors == GameInputRumbleNone) return;

    GameInputRumbleParams params = {};
    params.lowFrequency  = CLAMP(low_frequency, 0.0f, 1.0f);
    params.highFrequency = CLAMP(high_frequency, 0.0f, 1.0f);
    params.leftTrigger   = CLAMP(left_trigger, 0.0f, 1.0f);
    params.rightTrigger  = CLAMP(right_trigger, 0.0f, 1.0f);

    device->SetRumbleState(&params);
}

void GDKInput::stop_rumble(int joy_id) {
    set_rumble(joy_id, 0.0f, 0.0f, 0.0f, 0.0f);
}

} // namespace godot
