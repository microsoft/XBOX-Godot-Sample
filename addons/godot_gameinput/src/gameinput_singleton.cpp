#include "gameinput_singleton.h"

#include "gameinput_device.h"
#include "gameinput_reading.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <type_traits>

namespace godot {

// vcpkg's `gameinput` port ships the GameInput v3 redistributable, which
// returns device info via an HRESULT + out-param instead of the v1 direct
// pointer return. This helper bridges the v3 shape back to the v1-style
// `nullptr-on-failure` ergonomics used throughout this file.
static inline const GameInputDeviceInfo *_get_device_info(IGameInputDevice *device) {
    if (!device) return nullptr;
    const GameInputDeviceInfo *info = nullptr;
    HRESULT hr = device->GetDeviceInfo(&info);
    return SUCCEEDED(hr) ? info : nullptr;
}

template <typename DeviceT>
static inline HRESULT _set_rumble_state_checked_impl(DeviceT *device,
                                                     const GameInputRumbleParams *params) {
    using ReturnT = decltype(device->SetRumbleState(params));
    if constexpr (std::is_same_v<ReturnT, void>) {
        device->SetRumbleState(params);
        return S_OK;
    } else {
        ReturnT result = device->SetRumbleState(params);
        if constexpr (std::is_same_v<ReturnT, HRESULT>) {
            return result;
        } else if constexpr (std::is_same_v<ReturnT, bool>) {
            return result ? S_OK : E_FAIL;
        } else {
            return (HRESULT)result;
        }
    }
}

static inline HRESULT _set_rumble_state_checked(IGameInputDevice *device,
                                                const GameInputRumbleParams *params) {
    if (!device || !params) {
        return E_POINTER;
    }
    return _set_rumble_state_checked_impl(device, params);
}

GameInput *GameInput::singleton = nullptr;

GameInput *GameInput::get_singleton() {
    return singleton;
}

GameInput::GameInput() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

GameInput::~GameInput() {
    if (m_initialized) {
        shutdown();
    }
    if (singleton == this) {
        singleton = nullptr;
    }
}

void GameInput::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &GameInput::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &GameInput::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &GameInput::is_initialized);
    ClassDB::bind_method(D_METHOD("poll"), &GameInput::poll);
    ClassDB::bind_method(D_METHOD("get_devices", "kind_mask"),
                         &GameInput::get_devices, DEFVAL((int)DEVICE_GAMEPAD));
    ClassDB::bind_method(D_METHOD("get_primary_device", "kind_mask"),
                         &GameInput::get_primary_device, DEFVAL((int)DEVICE_GAMEPAD));
    ClassDB::bind_method(D_METHOD("get_current_reading", "device"),
                         &GameInput::get_current_reading);
    ClassDB::bind_method(D_METHOD("set_vibration", "device", "low_freq", "high_freq",
                                  "left_trigger", "right_trigger"),
                         &GameInput::set_vibration, DEFVAL(0.0f), DEFVAL(0.0f));
    ClassDB::bind_method(D_METHOD("stop_haptics", "device"), &GameInput::stop_haptics);
    ClassDB::bind_method(D_METHOD("get_connected_device_count"),
                         &GameInput::get_connected_device_count);

    ADD_SIGNAL(MethodInfo("device_connected",
                          PropertyInfo(Variant::OBJECT, "device", PROPERTY_HINT_RESOURCE_TYPE,
                                       "GameInputDevice")));
    ADD_SIGNAL(MethodInfo("device_disconnected",
                          PropertyInfo(Variant::INT, "device_id")));

    BIND_ENUM_CONSTANT(DEVICE_UNKNOWN);
    BIND_ENUM_CONSTANT(DEVICE_GAMEPAD);
    BIND_ENUM_CONSTANT(DEVICE_KEYBOARD);
    BIND_ENUM_CONSTANT(DEVICE_MOUSE);
    BIND_ENUM_CONSTANT(DEVICE_ALL);
}

bool GameInput::_ensure_initialized() {
    if (m_initialized && m_game_input != nullptr) {
        return true;
    }
    if (!m_warned_uninitialized) {
        UtilityFunctions::push_warning(
            "GameInput: method called before initialize() succeeded — returning safe default. "
            "Call GameInput.initialize() once at startup.");
        m_warned_uninitialized = true;
    }
    return false;
}

int GameInput::_kind_mask_from_supported(GameInputKind k) const {
    int m = DEVICE_UNKNOWN;
    if (k & GameInputKindGamepad)  m |= DEVICE_GAMEPAD;
    if (k & GameInputKindKeyboard) m |= DEVICE_KEYBOARD;
    if (k & GameInputKindMouse)    m |= DEVICE_MOUSE;
    return m;
}

int GameInput::_find_index_by_id(int64_t id) const {
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].id == id) return i;
    }
    return -1;
}

int GameInput::_find_index_by_native(IGameInputDevice *native) const {
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].native == native) return i;
    }
    return -1;
}

Ref<GameInputDevice> GameInput::_make_wrapper(int64_t id) {
    Ref<GameInputDevice> w;
    w.instantiate();
    w->_set_device_id(id);
    return w;
}

void CALLBACK GameInput::_on_device_callback(
        GameInputCallbackToken /*token*/, void *context,
        IGameInputDevice *device, uint64_t /*timestamp*/,
        GameInputDeviceStatus current_status,
        GameInputDeviceStatus previous_status) {
    auto *self = static_cast<GameInput *>(context);
    if (!self || !device || !self->m_accepting_callbacks.load(std::memory_order_acquire)) {
        return;
    }

    bool was_connected = (previous_status & GameInputDeviceConnected) != 0;
    bool is_connected  = (current_status  & GameInputDeviceConnected) != 0;

    if (is_connected == was_connected) {
        return;
    }

    PendingDeviceEvent ev;
    ev.kind = is_connected ? PendingEventKind::Connected : PendingEventKind::Disconnected;
    ev.native_device = device;
    // AddRef so the pointer is valid until the main thread processes it.
    device->AddRef();

    {
        std::lock_guard<std::mutex> lock(self->m_event_mutex);
        if (!self->m_accepting_callbacks.load(std::memory_order_acquire)) {
            device->Release();
            return;
        }
        self->m_pending_events.push_back(ev);
    }
}

void GameInput::_release_pending_events_locked() {
    for (uint32_t i = 0; i < m_pending_events.size(); ++i) {
        if (m_pending_events[i].native_device) {
            m_pending_events[i].native_device->Release();
        }
    }
    m_pending_events.clear();
}

void GameInput::_clear_pending_events() {
    std::lock_guard<std::mutex> lock(m_event_mutex);
    _release_pending_events_locked();
}

void GameInput::_drain_pending_events() {
    LocalVector<PendingDeviceEvent> drained;
    {
        std::lock_guard<std::mutex> lock(m_event_mutex);
        drained = m_pending_events;
        m_pending_events.clear();
    }

    for (uint32_t i = 0; i < drained.size(); ++i) {
        PendingDeviceEvent &ev = drained[i];

        if (ev.kind == PendingEventKind::Connected) {
            if (_find_index_by_native(ev.native_device) >= 0) {
                // Duplicate — release our extra ref and skip.
                ev.native_device->Release();
                continue;
            }

            const GameInputDeviceInfo *info = _get_device_info(ev.native_device);
            int kind_mask = info ? _kind_mask_from_supported(info->supportedInput) : DEVICE_UNKNOWN;

            DeviceEntry entry;
            entry.id = m_next_device_id.fetch_add(1);
            entry.native = ev.native_device; // takes ownership of the AddRef
            entry.wrapper = _make_wrapper(entry.id);
            entry.kind_mask = kind_mask;
            m_devices.push_back(entry);

            UtilityFunctions::print(
                "GameInput: device connected id=", entry.id,
                " kind_mask=", entry.kind_mask,
                " name=", device_get_display_name(entry.id));

            emit_signal("device_connected", entry.wrapper);
        } else { // Disconnected
            int idx = _find_index_by_native(ev.native_device);
            if (idx < 0) {
                ev.native_device->Release();
                continue;
            }
            int64_t id = m_devices[idx].id;
            m_devices.write[idx].native->Release();
            m_devices.remove_at(idx);
            // Also release the AddRef from the callback (the entry held one ref;
            // the callback added a second one).
            ev.native_device->Release();

            UtilityFunctions::print("GameInput: device disconnected id=", id);
            emit_signal("device_disconnected", (int64_t)id);
        }
    }
}

void GameInput::_real_poll() {
    if (!m_game_input) return;

    for (int i = 0; i < m_devices.size(); ++i) {
        DeviceEntry &entry = m_devices.write[i];
        if (!(entry.kind_mask & DEVICE_GAMEPAD)) {
            continue;
        }

        IGameInputReading *reading = nullptr;
        HRESULT hr = m_game_input->GetCurrentReading(GameInputKindGamepad, entry.native, &reading);
        if (FAILED(hr) || !reading) {
            continue;
        }

        GameInputGamepadState s{};
        if (reading->GetGamepadState(&s)) {
            if (entry.has_state) {
                entry.prev_state = entry.cur_state;
                entry.has_prev_state = true;
            } else {
                entry.prev_state = s;
                entry.has_prev_state = false;
            }
            entry.cur_state = s;
            entry.has_state = true;
        }

        reading->Release();
    }
}

bool GameInput::initialize() {
    if (m_initialized) {
        return true;
    }
    m_accepting_callbacks.store(false, std::memory_order_release);
    HRESULT hr = ::GameInputCreate(&m_game_input);
    if (FAILED(hr) || !m_game_input) {
        if (!m_warned_create_failed) {
            UtilityFunctions::push_warning(
                "GameInput: GameInputCreate() failed (hr=", (int64_t)hr,
                "). The runtime will operate in disabled mode.");
            m_warned_create_failed = true;
        }
        m_game_input = nullptr;
        return false;
    }

    m_accepting_callbacks.store(true, std::memory_order_release);
    hr = m_game_input->RegisterDeviceCallback(
        nullptr,
        (GameInputKind)(GameInputKindGamepad | GameInputKindKeyboard | GameInputKindMouse),
        GameInputDeviceConnected,
        GameInputBlockingEnumeration,
        this,
        &GameInput::_on_device_callback,
        &m_device_callback_token);
    if (FAILED(hr)) {
        m_accepting_callbacks.store(false, std::memory_order_release);
        UtilityFunctions::push_warning(
            "GameInput: RegisterDeviceCallback failed (hr=", (int64_t)hr, ")");
        _clear_pending_events();
        m_game_input->Release();
        m_game_input = nullptr;
        m_device_callback_token = 0;
        return false;
    }

    m_initialized = true;
    m_warned_uninitialized = false;
    UtilityFunctions::print("GameInput: initialized");
    return true;
}

void GameInput::shutdown() {
    if (!m_initialized && !m_game_input) return;

    m_accepting_callbacks.store(false, std::memory_order_release);

    if (m_game_input && m_device_callback_token) {
        // UnregisterCallback blocks until in-flight callbacks finish; StopCallback does not.
        bool unregistered = m_game_input->UnregisterCallback(m_device_callback_token);
        if (!unregistered) {
            UtilityFunctions::push_warning(
                "GameInput: UnregisterCallback() failed; late callbacks will be ignored.");
        }
        m_device_callback_token = 0;
    }

    // Stop rumble + release every device cleanly.
    for (int i = 0; i < m_devices.size(); ++i) {
        IGameInputDevice *d = m_devices[i].native;
        if (d) {
            const GameInputDeviceInfo *info = _get_device_info(d);
            if (info && info->supportedRumbleMotors != GameInputRumbleNone) {
                GameInputRumbleParams zero{};
                _set_rumble_state_checked(d, &zero);
            }
            d->Release();
        }
    }
    m_devices.clear();

    // Drain any leftover queued events to release their AddRef.
    _clear_pending_events();

    if (m_game_input) {
        m_game_input->Release();
        m_game_input = nullptr;
    }

    m_initialized = false;
    m_last_polled_frame = UINT64_MAX;
    UtilityFunctions::print("GameInput: shutdown");
}

bool GameInput::is_initialized() const {
    return m_initialized;
}

void GameInput::poll() {
    if (!m_initialized || !m_game_input) {
        return;
    }
    Engine *engine = Engine::get_singleton();
    uint64_t frame = engine ? engine->get_process_frames() : 0;
    if (frame == m_last_polled_frame && frame != UINT64_MAX) {
        return; // already polled this frame
    }
    m_last_polled_frame = frame;

    _drain_pending_events();
    _real_poll();
}

Array GameInput::get_devices(int kind_mask) {
    Array out;
    if (!_ensure_initialized()) {
        return out;
    }
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].kind_mask & kind_mask) {
            out.push_back(m_devices[i].wrapper);
        }
    }
    return out;
}

Ref<GameInputDevice> GameInput::get_primary_device(int kind_mask) {
    if (!_ensure_initialized()) {
        return Ref<GameInputDevice>();
    }
    for (int i = 0; i < m_devices.size(); ++i) {
        if (m_devices[i].kind_mask & kind_mask) {
            return m_devices[i].wrapper;
        }
    }
    return Ref<GameInputDevice>();
}

Ref<GameInputReading> GameInput::get_current_reading(const Ref<GameInputDevice> &device) {
    Ref<GameInputReading> r;
    if (!_ensure_initialized() || device.is_null()) {
        return r;
    }
    int idx = _find_index_by_id(device->get_device_id());
    if (idx < 0) {
        return r;
    }
    const DeviceEntry &e = m_devices[idx];
    if (!e.has_state) {
        // Try to fetch one on demand (haven't polled yet this frame).
        IGameInputReading *reading = nullptr;
        HRESULT hr = m_game_input->GetCurrentReading(GameInputKindGamepad, e.native, &reading);
        if (FAILED(hr) || !reading) {
            return r;
        }
        GameInputGamepadState s{};
        if (reading->GetGamepadState(&s)) {
            DeviceEntry &we = m_devices.write[idx];
            we.prev_state = s;
            we.cur_state = s;
            we.has_state = true;
            we.has_prev_state = false;
        }
        reading->Release();
    }
    r.instantiate();
    r->_set_state(e.cur_state, e.prev_state, e.has_prev_state);
    return r;
}

bool GameInput::set_vibration(const Ref<GameInputDevice> &device, float low_freq, float high_freq,
                              float left_trigger, float right_trigger) {
    if (!_ensure_initialized() || device.is_null()) {
        return false;
    }
    int idx = _find_index_by_id(device->get_device_id());
    if (idx < 0) return false;

    IGameInputDevice *native = m_devices[idx].native;
    if (!native) return false;

    const GameInputDeviceInfo *info = _get_device_info(native);
    if (!info || info->supportedRumbleMotors == GameInputRumbleNone) {
        return false;
    }

    GameInputRumbleParams params{};
    params.lowFrequency  = std::max(0.0f, std::min(1.0f, low_freq));
    params.highFrequency = std::max(0.0f, std::min(1.0f, high_freq));
    params.leftTrigger   = std::max(0.0f, std::min(1.0f, left_trigger));
    params.rightTrigger  = std::max(0.0f, std::min(1.0f, right_trigger));
    HRESULT hr = _set_rumble_state_checked(native, &params);
    if (FAILED(hr)) {
        UtilityFunctions::push_warning(
            "GameInput: SetRumbleState failed (hr=", (int64_t)hr, ")");
        return false;
    }
    return true;
}

void GameInput::stop_haptics(const Ref<GameInputDevice> &device) {
    set_vibration(device, 0.0f, 0.0f, 0.0f, 0.0f);
}

bool GameInput::device_lookup(int64_t id, IGameInputDevice **out_native, int *out_kind_mask) {
    int idx = _find_index_by_id(id);
    if (idx < 0) return false;
    if (out_native) *out_native = m_devices[idx].native;
    if (out_kind_mask) *out_kind_mask = m_devices[idx].kind_mask;
    return true;
}

String GameInput::device_get_display_name(int64_t id) {
    IGameInputDevice *native = nullptr;
    if (!device_lookup(id, &native, nullptr) || !native) {
        return String();
    }
    const GameInputDeviceInfo *info = _get_device_info(native);
    if (!info || !info->displayName) {
        // Fallback to a synthesized name.
        if (info) {
            return String("GameInput Device ") + String::num_int64(info->vendorId, 16).to_upper()
                   + String(":") + String::num_int64(info->productId, 16).to_upper();
        }
        return String("GameInput Device");
    }
    // GameInput v3 ships `displayName` as a plain null-terminated `const char *`
    // (v1 wrapped it in a `GameInputString` struct with `.data` + `.sizeInBytes`).
    return String::utf8(info->displayName);
}

bool GameInput::device_is_connected(int64_t id) {
    return _find_index_by_id(id) >= 0;
}

Dictionary GameInput::device_get_device_info(int64_t id) {
    Dictionary out;
    IGameInputDevice *native = nullptr;
    int kind_mask = DEVICE_UNKNOWN;
    if (!device_lookup(id, &native, &kind_mask) || !native) {
        return out;
    }
    const GameInputDeviceInfo *info = _get_device_info(native);
    if (!info) {
        return out;
    }

    out["name"] = device_get_display_name(id);
    out["vendor_id"] = (int)info->vendorId;
    out["product_id"] = (int)info->productId;
    out["revision"] = (int)info->revisionNumber;
    out["device_family"] = (int)info->deviceFamily;
    out["supported_input_kinds"] = (int)info->supportedInput;
    out["supported_input_mask"] = (int)kind_mask;
    out["supports_vibration"] = info->supportedRumbleMotors != GameInputRumbleNone;
    // Controller axis/button counts moved under `controllerInfo` in v3; the
    // pointer is null for non-controller devices.
    out["controller_axis_count"] =
        (int)(info->controllerInfo ? info->controllerInfo->controllerAxisCount : 0u);
    out["controller_button_count"] =
        (int)(info->controllerInfo ? info->controllerInfo->controllerButtonCount : 0u);
    out["force_feedback_motor_count"] = (int)info->forceFeedbackMotorCount;
    // GameInput v3 removed the v1 `hapticFeedbackMotorCount` field; haptic
    // capability is now queried via `IGameInputDevice::GetHapticInfo`. We expose
    // an explicit boolean via `device_supports_haptics()` rather than re-deriving
    // a synthetic count here.
    return out;
}

bool GameInput::device_supports_vibration(int64_t id) {
    IGameInputDevice *native = nullptr;
    if (!device_lookup(id, &native, nullptr) || !native) return false;
    const GameInputDeviceInfo *info = _get_device_info(native);
    return info && info->supportedRumbleMotors != GameInputRumbleNone;
}

bool GameInput::device_supports_haptics(int64_t id) {
    IGameInputDevice *native = nullptr;
    if (!device_lookup(id, &native, nullptr) || !native) return false;
    // GameInput v3 dropped `GameInputDeviceInfo::hapticFeedbackMotorCount` and
    // moved haptic capability behind a dedicated `GetHapticInfo` call. A device
    // is considered haptics-capable if the call succeeds and reports at least
    // one haptic location.
    GameInputHapticInfo haptic{};
    HRESULT hr = native->GetHapticInfo(&haptic);
    return SUCCEEDED(hr) && haptic.locationCount > 0;
}

bool GameInput::device_get_state_snapshot(int64_t id, GameInputGamepadState *out_cur,
                                          GameInputGamepadState *out_prev, bool *out_has_prev) {
    int idx = _find_index_by_id(id);
    if (idx < 0) return false;
    const DeviceEntry &e = m_devices[idx];
    if (!e.has_state) return false;
    if (out_cur)  *out_cur  = e.cur_state;
    if (out_prev) *out_prev = e.prev_state;
    if (out_has_prev) *out_has_prev = e.has_prev_state;
    return true;
}

int GameInput::get_connected_device_count() const {
    return m_devices.size();
}

} // namespace godot
