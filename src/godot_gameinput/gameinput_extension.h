#ifndef GODOT_GAMEINPUT_EXTENSION_H
#define GODOT_GAMEINPUT_EXTENSION_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class GodotGameInputProbe : public Node {
    GDCLASS(GodotGameInputProbe, Node);

protected:
    static void _bind_methods();

public:
    String get_status_text() const;
};

} // namespace godot

#endif // GODOT_GAMEINPUT_EXTENSION_H
