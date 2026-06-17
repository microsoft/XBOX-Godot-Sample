#include "gdk_speech_synthesizer.h"

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"

#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <cstring>

namespace godot {

namespace {

bool CALLBACK collect_installed_voice(const XSpeechSynthesizerVoiceInformation *p_info, void *p_context) {
    if (p_info == nullptr || p_context == nullptr) {
        return true;
    }

    Array *voices = static_cast<Array *>(p_context);
    Dictionary voice;
    voice["id"] = p_info->VoiceId != nullptr ? String::utf8(p_info->VoiceId) : String();
    voice["display_name"] = p_info->DisplayName != nullptr ? String::utf8(p_info->DisplayName) : String();
    voice["language"] = p_info->Language != nullptr ? String::utf8(p_info->Language) : String();
    voice["description"] = p_info->Description != nullptr ? String::utf8(p_info->Description) : String();
    voice["gender"] = p_info->Gender == XSpeechSynthesizerVoiceGender::Male ? "male" : "female";
    voices->push_back(voice);
    return true;
}

uint16_t read_u16_le(const uint8_t *p_data) {
    return static_cast<uint16_t>(p_data[0]) | (static_cast<uint16_t>(p_data[1]) << 8);
}

uint32_t read_u32_le(const uint8_t *p_data) {
    return static_cast<uint32_t>(p_data[0]) |
            (static_cast<uint32_t>(p_data[1]) << 8) |
            (static_cast<uint32_t>(p_data[2]) << 16) |
            (static_cast<uint32_t>(p_data[3]) << 24);
}

bool parse_wav(const PackedByteArray &p_wav, PackedByteArray &r_pcm, int &r_channels, int &r_sample_rate, int &r_bits) {
    const int64_t size = p_wav.size();
    if (size < 12) {
        return false;
    }

    const uint8_t *data = p_wav.ptr();
    if (!(data[0] == 'R' && data[1] == 'I' && data[2] == 'F' && data[3] == 'F')) {
        return false;
    }
    if (!(data[8] == 'W' && data[9] == 'A' && data[10] == 'V' && data[11] == 'E')) {
        return false;
    }

    bool have_fmt = false;
    bool have_data = false;
    int64_t offset = 12;
    while (offset + 8 <= size) {
        const uint8_t *chunk = data + offset;
        const uint32_t chunk_size = read_u32_le(chunk + 4);
        const int64_t chunk_data = offset + 8;

        if (chunk[0] == 'f' && chunk[1] == 'm' && chunk[2] == 't' && chunk[3] == ' ') {
            if (chunk_data + 16 <= size) {
                const uint8_t *fmt = data + chunk_data;
                r_channels = static_cast<int>(read_u16_le(fmt + 2));
                r_sample_rate = static_cast<int>(read_u32_le(fmt + 4));
                r_bits = static_cast<int>(read_u16_le(fmt + 14));
                have_fmt = true;
            }
        } else if (chunk[0] == 'd' && chunk[1] == 'a' && chunk[2] == 't' && chunk[3] == 'a') {
            const int64_t available = size - chunk_data;
            int64_t copy = static_cast<int64_t>(chunk_size);
            if (copy > available) {
                copy = available;
            }
            if (copy < 0) {
                copy = 0;
            }
            r_pcm.resize(copy);
            if (copy > 0) {
                std::memcpy(r_pcm.ptrw(), data + chunk_data, static_cast<size_t>(copy));
            }
            have_data = true;
        }

        int64_t advance = 8 + static_cast<int64_t>(chunk_size) + ((chunk_size & 1u) ? 1 : 0);
        if (advance <= 0) {
            break;
        }
        offset += advance;
    }

    return have_fmt && have_data;
}

} // namespace

void GDKSpeechSynthesizer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_installed_voices"), &GDKSpeechSynthesizer::get_installed_voices);
    ClassDB::bind_method(D_METHOD("set_default_voice"), &GDKSpeechSynthesizer::set_default_voice);
    ClassDB::bind_method(D_METHOD("set_custom_voice", "voice_id"), &GDKSpeechSynthesizer::set_custom_voice);
    ClassDB::bind_method(D_METHOD("synthesize_text", "text"), &GDKSpeechSynthesizer::synthesize_text);
    ClassDB::bind_method(D_METHOD("synthesize_ssml", "ssml"), &GDKSpeechSynthesizer::synthesize_ssml);
    ClassDB::bind_method(D_METHOD("synthesize_to_stream", "text"), &GDKSpeechSynthesizer::synthesize_to_stream);
}

void GDKSpeechSynthesizer::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKSpeechSynthesizer::on_runtime_initialized() {
    GDKRuntime *runtime = m_owner != nullptr ? m_owner->get_runtime() : nullptr;
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the speech synthesizer service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKSpeechSynthesizer::shutdown() {
    if (m_synthesizer != nullptr) {
        XSpeechSynthesizerCloseHandle(m_synthesizer);
        m_synthesizer = nullptr;
    }
    m_runtime_ready = false;
}

Ref<GDKResult> GDKSpeechSynthesizer::ensure_synthesizer_internal() {
    if (!m_runtime_ready) {
        return GDKResult::error_result(
                E_FAIL,
                "not_initialized",
                "GDK is not initialized. Call GDK.initialize() first.");
    }

    if (m_synthesizer != nullptr) {
        return GDKResult::ok_result();
    }

    HRESULT hr = XSpeechSynthesizerCreate(&m_synthesizer);
    if (FAILED(hr)) {
        m_synthesizer = nullptr;
        return GDKResult::hresult_error(
                hr,
                "Failed to create the speech synthesizer.",
                "speech_synthesizer_unavailable");
    }
    return GDKResult::ok_result();
}

Array GDKSpeechSynthesizer::get_installed_voices() {
    Array voices;
    if (!m_runtime_ready) {
        return voices;
    }

    HRESULT hr = XSpeechSynthesizerEnumerateInstalledVoices(&voices, &collect_installed_voice);
    if (FAILED(hr)) {
        return Array();
    }
    return voices;
}

Ref<GDKResult> GDKSpeechSynthesizer::set_default_voice() {
    Ref<GDKResult> ensured = ensure_synthesizer_internal();
    if (!ensured->is_ok()) {
        return ensured;
    }

    HRESULT hr = XSpeechSynthesizerSetDefaultVoice(m_synthesizer);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to set the default voice.", "set_default_voice_failed");
    }
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKSpeechSynthesizer::set_custom_voice(const String &p_voice_id) {
    const String voice_id = p_voice_id.strip_edges();
    if (voice_id.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_voice_id", "A non-empty voice id is required.");
    }

    Ref<GDKResult> ensured = ensure_synthesizer_internal();
    if (!ensured->is_ok()) {
        return ensured;
    }

    const CharString voice_utf8 = voice_id.utf8();
    HRESULT hr = XSpeechSynthesizerSetCustomVoice(m_synthesizer, voice_utf8.get_data());
    if (FAILED(hr)) {
        Dictionary data;
        data["voice_id"] = voice_id;
        return GDKResult::hresult_error(hr, "Failed to set the requested custom voice.", "set_custom_voice_failed", data);
    }
    return GDKResult::ok_result();
}

Ref<GDKResult> GDKSpeechSynthesizer::synthesize_text(const String &p_text) {
    return synthesize_internal(p_text, false);
}

Ref<GDKResult> GDKSpeechSynthesizer::synthesize_ssml(const String &p_ssml) {
    return synthesize_internal(p_ssml, true);
}

Ref<GDKResult> GDKSpeechSynthesizer::synthesize_internal(const String &p_input, bool p_is_ssml) {
    if (p_input.strip_edges().is_empty()) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_input",
                p_is_ssml ? "A non-empty SSML document is required." : "A non-empty text string is required.");
    }

    Ref<GDKResult> ensured = ensure_synthesizer_internal();
    if (!ensured->is_ok()) {
        return ensured;
    }

    const CharString input_utf8 = p_input.utf8();
    XSpeechSynthesizerStreamHandle stream = nullptr;
    HRESULT hr = p_is_ssml
            ? XSpeechSynthesizerCreateStreamFromSsml(m_synthesizer, input_utf8.get_data(), &stream)
            : XSpeechSynthesizerCreateStreamFromText(m_synthesizer, input_utf8.get_data(), &stream);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to synthesize speech.", "synthesize_failed");
    }

    size_t buffer_size = 0;
    hr = XSpeechSynthesizerGetStreamDataSize(stream, &buffer_size);
    if (FAILED(hr)) {
        XSpeechSynthesizerCloseStreamHandle(stream);
        return GDKResult::hresult_error(hr, "Failed to read the synthesized audio size.", "synthesize_failed");
    }

    PackedByteArray audio;
    audio.resize(static_cast<int64_t>(buffer_size));
    if (buffer_size > 0) {
        size_t buffer_used = 0;
        hr = XSpeechSynthesizerGetStreamData(stream, buffer_size, audio.ptrw(), &buffer_used);
        if (FAILED(hr)) {
            XSpeechSynthesizerCloseStreamHandle(stream);
            return GDKResult::hresult_error(hr, "Failed to read the synthesized audio data.", "synthesize_failed");
        }
        if (static_cast<int64_t>(buffer_used) != audio.size()) {
            audio.resize(static_cast<int64_t>(buffer_used));
        }
    }

    XSpeechSynthesizerCloseStreamHandle(stream);

    Dictionary data;
    data["audio_wav"] = audio;
    data["byte_count"] = static_cast<int64_t>(audio.size());
    return GDKResult::ok_result(data);
}

Ref<AudioStreamWAV> GDKSpeechSynthesizer::synthesize_to_stream(const String &p_text) {
    Ref<GDKResult> result = synthesize_internal(p_text, false);
    if (!result->is_ok()) {
        return Ref<AudioStreamWAV>();
    }

    Dictionary data = result->get_data();
    PackedByteArray wav = data["audio_wav"];

    PackedByteArray pcm;
    int channels = 1;
    int sample_rate = 24000;
    int bits = 16;
    if (!parse_wav(wav, pcm, channels, sample_rate, bits)) {
        return Ref<AudioStreamWAV>();
    }

    Ref<AudioStreamWAV> stream;
    stream.instantiate();
    if (bits == 16) {
        stream->set_format(AudioStreamWAV::FORMAT_16_BITS);
        stream->set_data(pcm);
    } else if (bits == 8) {
        PackedByteArray signed_pcm;
        signed_pcm.resize(pcm.size());
        for (int64_t i = 0; i < pcm.size(); ++i) {
            signed_pcm.set(i, static_cast<uint8_t>(static_cast<int>(pcm[i]) - 128));
        }
        stream->set_format(AudioStreamWAV::FORMAT_8_BITS);
        stream->set_data(signed_pcm);
    } else {
        return Ref<AudioStreamWAV>();
    }

    stream->set_mix_rate(sample_rate);
    stream->set_stereo(channels == 2);
    return stream;
}

GDKSpeechSynthesizer::~GDKSpeechSynthesizer() {
    if (m_synthesizer != nullptr) {
        XSpeechSynthesizerCloseHandle(m_synthesizer);
        m_synthesizer = nullptr;
    }
}

} // namespace godot
