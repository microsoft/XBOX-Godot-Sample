#ifndef GODOT_GAMEINPUT_SINGLETON_H
#define GODOT_GAMEINPUT_SINGLETON_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <GameInput.h>

#include <atomic>
#include <cstdint>
#include <mutex>

namespace godot {

class GameInputDevice;
class GameInputReading;

// Engine singleton. Owns GameInput lifetime, the device cache, and main-thread
// poll dispatch. All public methods are GDScript-safe: they soft-fail (return
// safe defaults + push_warning once) when GameInput is not initialized or not
// available on this host.
//
// Threading contract:
//   * IGameInput device callbacks run on a GameInput-owned worker thread. Those
//     callbacks ONLY enqueue events into m_pending_events under m_event_mutex
//     and AddRef the IGameInputDevice* so the pointer survives until the main
//     thread processes the event.
//   * The main thread drains the queue inside poll(), under the same lock,
//     and is the only thread that mutates m_devices.
//   * GameInput::poll() is per-frame idempotent: a counter check against
//     Engine::get_singleton()->get_process_frames() ensures the real refresh
//     only runs once per frame regardless of how many GameInputMapper nodes
//     call poll() defensively.
class GameInput : public Object {
    GDCLASS(GameInput, Object);

public:
    enum DeviceKind {
        DEVICE_UNKNOWN  = 0,
        DEVICE_GAMEPAD  = 1 << 0,
        DEVICE_KEYBOARD = 1 << 1,
        DEVICE_MOUSE    = 1 << 2,
        DEVICE_ALL      = DEVICE_GAMEPAD | DEVICE_KEYBOARD | DEVICE_MOUSE,
    };

private:
    static GameInput *singleton;

    IGameInput *m_game_input = nullptr;
    GameInputCallbackToken m_device_callback_token = 0;
    bool m_initialized = false;
    bool m_warned_uninitialized = false;
    bool m_warned_create_failed = false;

    std::atomic<int64_t> m_next_device_id{1};
    uint64_t m_last_polled_frame = UINT64_MAX;

    // Pending device events from the GameInput callback thread. Drained on
    // the main thread inside poll().
    enum class PendingEventKind { Connected, Disconnected };
    struct PendingDeviceEvent {
        PendingEventKind kind;
        IGameInputDevice *native_device;
    };
    std::mutex m_event_mutex;
    LocalVector<PendingDeviceEvent> m_pending_events;

    // Main-thread device cache. Indexed by m_devices[].id (session-local
    // monotonic, never recycled).
    struct DeviceEntry {
        int64_t id = 0;
        IGameInputDevice *native = nullptr;        // AddRef'd; Released on disconnect
        Ref<GameInputDevice> wrapper;              // shared with GDScript
        int kind_mask = DEVICE_UNKNOWN;
        // Cached gamepad state for prev/cur edge detection. Updated each real poll.
        bool has_state = false;
        bool has_prev_state = false;
        GameInputGamepadState cur_state{};
        GameInputGamepadState prev_state{};
    };
    Vector<DeviceEntry> m_devices;

    void _drain_pending_events();
    void _real_poll();
    int _find_index_by_id(int64_t id) const;
    int _find_index_by_native(IGameInputDevice *native) const;
    Ref<GameInputDevice> _make_wrapper(int64_t id);
    bool _ensure_initialized();
    int _kind_mask_from_supported(GameInputKind k) const;

protected:
    static void _bind_methods();

public:
    static GameInput *get_singleton();

    GameInput();
    ~GameInput();

    bool initialize();
    void shutdown();
    bool is_initialized() const;
    void poll();

    Array get_devices(int kind_mask = DEVICE_GAMEPAD);
    Ref<GameInputDevice> get_primary_device(int kind_mask = DEVICE_GAMEPAD);
    Ref<GameInputReading> get_current_reading(const Ref<GameInputDevice> &device);

    bool set_vibration(const Ref<GameInputDevice> &device, float low_freq, float high_freq,
                       float left_trigger = 0.0f, float right_trigger = 0.0f);
    void stop_haptics(const Ref<GameInputDevice> &device);

    // === Internal helpers used by GameInputDevice / GameInputReading ===
    // Return true if the id is still connected. Out-params filled when true.
    bool device_lookup(int64_t id, IGameInputDevice **out_native, int *out_kind_mask);
    String device_get_display_name(int64_t id);
    bool device_is_connected(int64_t id);
    float device_get_battery_level(int64_t id);
    Dictionary device_get_device_info(int64_t id);
    bool device_supports_vibration(int64_t id);
    bool device_supports_haptics(int64_t id);
    // Reading snapshot helpers — return false if no fresh state for this device.
    bool device_get_state_snapshot(int64_t id, GameInputGamepadState *out_cur,
                                   GameInputGamepadState *out_prev, bool *out_has_prev);

    // Test/debug
    int get_connected_device_count() const;

    // Static C callback (invoked from a GameInput worker thread).
    static void CALLBACK _on_device_callback(GameInputCallbackToken token, void *context,
                                             IGameInputDevice *device, uint64_t timestamp,
                                             GameInputDeviceStatus current_status,
                                             GameInputDeviceStatus previous_status);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GameInput::DeviceKind);

#endif // GODOT_GAMEINPUT_SINGLETON_H
