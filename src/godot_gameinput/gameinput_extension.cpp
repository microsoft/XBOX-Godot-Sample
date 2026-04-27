#include "gameinput_extension.h"

using namespace godot;

void GodotGameInputProbe::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_status_text"), &GodotGameInputProbe::get_status_text);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "status_text"), "", "get_status_text");
}

String GodotGameInputProbe::get_status_text() const {
    return "godot_gameinput loaded";
}
