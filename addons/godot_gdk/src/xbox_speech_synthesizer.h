#ifndef XBOX_SPEECH_SYNTHESIZER_H
#define XBOX_SPEECH_SYNTHESIZER_H

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

class Xbox;
class XboxResult;
class AudioStreamWAV;

class XboxSpeechSynthesizer : public RefCounted {
    GDCLASS(XboxSpeechSynthesizer, RefCounted);

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    XSpeechSynthesizerHandle m_synthesizer = nullptr;

    Ref<XboxResult> ensure_synthesizer_internal();
    Ref<XboxResult> synthesize_internal(const String &p_input, bool p_is_ssml);

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);
    Ref<XboxResult> on_runtime_initialized();
    void shutdown();

    Array get_installed_voices();
    Ref<XboxResult> set_default_voice();
    Ref<XboxResult> set_custom_voice(const String &p_voice_id);
    Ref<XboxResult> synthesize_text(const String &p_text);
    Ref<XboxResult> synthesize_ssml(const String &p_ssml);
    Ref<AudioStreamWAV> synthesize_to_stream(const String &p_text);

    XboxSpeechSynthesizer() = default;
    ~XboxSpeechSynthesizer();
};

} // namespace godot

#endif // XBOX_SPEECH_SYNTHESIZER_H
