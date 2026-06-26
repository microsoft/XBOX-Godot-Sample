// stub_memory.cpp — GDExtension memory interface stubs.
// Maps mem_alloc / mem_realloc / mem_free (and their _2 pad-align variants)
// to the standard C allocator.

#include "stub_types.h"

#include <cstdlib>

extern "C" {

static void *s_mem_alloc(size_t p_bytes) {
    return ::malloc(p_bytes);
}

static void *s_mem_alloc2(size_t p_bytes, uint8_t /*pad_align*/) {
    return ::malloc(p_bytes);
}

static void *s_mem_realloc(void *p_memory, size_t p_bytes) {
    return ::realloc(p_memory, p_bytes);
}

static void *s_mem_realloc2(void *p_memory, size_t p_bytes, uint8_t /*pad_align*/) {
    return ::realloc(p_memory, p_bytes);
}

static void s_mem_free(void *p_memory) {
    ::free(p_memory);
}

static void s_mem_free2(void *p_memory, uint8_t /*pad_align*/) {
    ::free(p_memory);
}

static void s_print_error(const char *p_desc, const char *p_function,
                          const char *p_file, int32_t p_line, uint8_t /*p_editor_notify*/) {
    ::fprintf(stderr, "STUB ERROR: %s | %s (%s:%d)\n",
              p_desc ? p_desc : "", p_function ? p_function : "",
              p_file ? p_file : "", (int)p_line);
}

static void s_print_error_with_message(const char *p_desc, const char *p_message,
                                       const char *p_function, const char *p_file,
                                       int32_t p_line, uint8_t /*p_editor_notify*/) {
    ::fprintf(stderr, "STUB ERROR: %s (%s) | %s (%s:%d)\n",
              p_desc ? p_desc : "", p_message ? p_message : "",
              p_function ? p_function : "",
              p_file ? p_file : "", (int)p_line);
}

static void s_print_warning(const char *p_desc, const char *p_function,
                             const char *p_file, int32_t p_line, uint8_t /*p_editor_notify*/) {
    ::fprintf(stderr, "STUB WARNING: %s | %s (%s:%d)\n",
              p_desc ? p_desc : "", p_function ? p_function : "",
              p_file ? p_file : "", (int)p_line);
}

static void s_print_warning_with_message(const char *p_desc, const char *p_message,
                                          const char *p_function, const char *p_file,
                                          int32_t p_line, uint8_t /*p_editor_notify*/) {
    (void)p_desc; (void)p_message; (void)p_function; (void)p_file; (void)p_line;
}

} // extern "C"

// Exported function pointer table (used by godot_stub_init.cpp)
void *stub_mem_alloc_ptr               = (void *)s_mem_alloc;
void *stub_mem_alloc2_ptr              = (void *)s_mem_alloc2;
void *stub_mem_realloc_ptr             = (void *)s_mem_realloc;
void *stub_mem_realloc2_ptr            = (void *)s_mem_realloc2;
void *stub_mem_free_ptr                = (void *)s_mem_free;
void *stub_mem_free2_ptr               = (void *)s_mem_free2;
void *stub_print_error_ptr             = (void *)s_print_error;
void *stub_print_error_with_message_ptr = (void *)s_print_error_with_message;
void *stub_print_warning_ptr           = (void *)s_print_warning;
void *stub_print_warning_with_message_ptr = (void *)s_print_warning_with_message;
