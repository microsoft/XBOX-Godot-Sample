#ifndef GDK_ACHIEVEMENTS_H
#define GDK_ACHIEVEMENTS_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>
#include <XTaskQueue.h>
#include <xsapi-c/services_c.h>

namespace godot {

class GDKAchievements : public Object {
    GDCLASS(GDKAchievements, Object);

    static GDKAchievements *singleton;
    XblContextHandle m_xbl_context = nullptr;
    bool m_initialized = false;
    String m_scid;

    bool _ensure_context();
    void _destroy_context();

protected:
    static void _bind_methods();

public:
    static GDKAchievements *get_singleton();

    GDKAchievements();
    ~GDKAchievements();

    Error initialize(const String &scid);
    void shutdown();
    void unlock(const String &achievement_id);
    void update_progress(const String &achievement_id, uint32_t percent);
    void check_achievement(const String &achievement_id);
    bool is_initialized() const;

    // Called from async callbacks (main thread via task queue)
    void _on_achievement_complete(const String &achievement_id, HRESULT hr);
    void _on_achievement_checked(const String &achievement_id, XAsyncBlock *async);
};

} // namespace godot

#endif // GDK_ACHIEVEMENTS_H
