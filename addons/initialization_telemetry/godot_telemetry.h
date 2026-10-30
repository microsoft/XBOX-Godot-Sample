#ifndef INITIALIZATION_TELEMETRY_GODOT_TELEMETRY_H
#define INITIALIZATION_TELEMETRY_GODOT_TELEMETRY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <godot_cpp/variant/string.hpp>

namespace initialization_telemetry {

void initialize();
void shutdown();
void record_initialized(bool p_with_options, const godot::String &p_config_path,
        const godot::String &p_playfab_title_id = godot::String());

} // namespace initialization_telemetry

#endif // INITIALIZATION_TELEMETRY_GODOT_TELEMETRY_H
