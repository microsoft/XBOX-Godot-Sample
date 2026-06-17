#ifndef GDK_SPEECH_SYNTHESIZER_H
#define GDK_SPEECH_SYNTHESIZER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XSpeechSynthesizer.h>

namespace godot {

class GDK;
class GDKResult;
class AudioStreamWAV;

class GDKSpeechSynthesizer : public RefCounted {
    GDCLASS(GDKSpeechSynthesizer, RefCounted);

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    XSpeechSynthesizerHandle m_synthesizer = nullptr;

    Ref<GDKResult> ensure_synthesizer_internal();
    Ref<GDKResult> synthesize_internal(const String &p_input, bool p_is_ssml);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);
    Ref<GDKResult> on_runtime_initialized();
    void shutdown();

    Array get_installed_voices();
    Ref<GDKResult> set_default_voice();
    Ref<GDKResult> set_custom_voice(const String &p_voice_id);
    Ref<GDKResult> synthesize_text(const String &p_text);
    Ref<GDKResult> synthesize_ssml(const String &p_ssml);
    Ref<AudioStreamWAV> synthesize_to_stream(const String &p_text);

    GDKSpeechSynthesizer() = default;
    ~GDKSpeechSynthesizer();
};

} // namespace godot

#endif // GDK_SPEECH_SYNTHESIZER_H
