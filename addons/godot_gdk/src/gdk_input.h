#ifndef GDK_INPUT_H
#define GDK_INPUT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <GameInput.h>

namespace godot {

class GDKInput : public Object {
    GDCLASS(GDKInput, Object);

    static GDKInput *singleton;

    IGameInput *m_game_input = nullptr;
    GameInputCallbackToken m_device_callback_token = 0;
    bool m_initialized = false;

    // Track connected devices
    struct DeviceEntry {
        IGameInputDevice *device;
        int godot_joy_id;
        GameInputGamepadButtons prev_buttons;
    };
    static constexpr int MAX_DEVICES = 8;
    DeviceEntry m_devices[MAX_DEVICES] = {};
    int m_device_count = 0;

    int find_device_index(IGameInputDevice *device) const;
    int find_device_index_by_joy_id(int joy_id) const;
    int allocate_joy_id();
    void poll_gamepad(int index);

public:
    // Public so the static C callback can reach them
    void on_device_connected(IGameInputDevice *device);
    void on_device_disconnected(IGameInputDevice *device);

protected:
    static void _bind_methods();

public:
    static GDKInput *get_singleton();

    GDKInput();
    ~GDKInput();

    Error initialize();
    void shutdown();
    void process(); // Call each frame to poll input
    bool is_initialized() const;
    int get_connected_device_count() const;

    // Haptics / rumble
    void set_rumble(int joy_id, float low_frequency, float high_frequency,
                    float left_trigger, float right_trigger);
    void stop_rumble(int joy_id);
};

} // namespace godot

#endif // GDK_INPUT_H
