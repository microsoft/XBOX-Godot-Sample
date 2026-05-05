shader_type canvas_item;

// Retro distorted-CRT post-processing for Pong Royale.
//
// Layers (cheap, all in one pass — fine on integrated GPUs at 640x400):
//   * Barrel curvature   — slight lens bow, plus black bezel outside the bow
//   * Sync wobble        — animated horizontal jitter that scrolls down the screen
//   * Chromatic aberration on R/B
//   * RGB phosphor mask  — every 3rd column biased to one channel
//   * Scanlines          — per-row brightness modulation
//   * Rolling bright bar — slow vertical-scroll brightness ripple
//   * Vignette + corner darkening
//   * Noise grain        — animated per-pixel hash noise
//   * Power flicker      — gentle full-screen brightness wobble
//
// Each effect has a uniform so you can dial them down for a calmer demo.

uniform sampler2D screen_tex : hint_screen_texture, filter_linear_mipmap;
uniform float scanline_strength : hint_range(0.0, 1.0) = 0.34;
uniform float scanline_count : hint_range(120.0, 1080.0) = 380.0;
uniform float vignette_strength : hint_range(0.0, 1.0) = 0.7;
uniform float chromatic_offset : hint_range(0.0, 0.02) = 0.0035;
uniform float curvature : hint_range(0.0, 0.25) = 0.07;
uniform float noise_strength : hint_range(0.0, 0.3) = 0.06;
uniform float jitter_strength : hint_range(0.0, 0.02) = 0.0018;
uniform float roll_strength : hint_range(0.0, 0.4) = 0.08;
uniform float phosphor_strength : hint_range(0.0, 0.4) = 0.10;
uniform float flicker_strength : hint_range(0.0, 0.2) = 0.018;
uniform vec3 tint_color : source_color = vec3(0.85, 1.0, 0.85);

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

void fragment() {
    vec2 uv = SCREEN_UV;

    // Barrel distortion: bend the sample coordinates toward the corners so
    // straight lines bow out, then mask the unsampled corners as the bezel.
    vec2 cuv = uv * 2.0 - 1.0;
    float r2 = cuv.x * cuv.x + cuv.y * cuv.y;
    cuv *= 1.0 + r2 * curvature;
    uv = cuv * 0.5 + 0.5;
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        // Bezel — solid black around the warped picture. Skip every other
        // effect by short-circuiting via a single COLOR write below.
        COLOR = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // Sync wobble — slow horizontal jitter that drifts vertically over time.
        float wobble = sin(uv.y * 30.0 + TIME * 6.0) * jitter_strength;
        wobble += (hash(vec2(floor(uv.y * 200.0), floor(TIME * 4.0))) - 0.5) * jitter_strength * 0.6;
        uv.x += wobble;

        // Chromatic aberration: sample R/B at slightly offset coords pulled toward
        // the screen edges; G stays put.
        vec2 dir = uv - vec2(0.5);
        float r = texture(screen_tex, uv + dir * chromatic_offset).r;
        float g = texture(screen_tex, uv).g;
        float b = texture(screen_tex, uv - dir * chromatic_offset).b;
        vec3 base = vec3(r, g, b) * tint_color;

        // Scanlines.
        float sl = sin(uv.y * scanline_count * 3.14159);
        float scanline = 1.0 - scanline_strength * (0.5 + 0.5 * sl);
        base *= scanline;

        // Per-3-column RGB phosphor mask (subpixel hint).
        float px = mod(FRAGCOORD.x, 3.0);
        vec3 mask = vec3(1.0);
        if (px < 1.0) {
            mask = vec3(1.0 + phosphor_strength, 1.0 - phosphor_strength * 0.5, 1.0 - phosphor_strength * 0.5);
        } else if (px < 2.0) {
            mask = vec3(1.0 - phosphor_strength * 0.5, 1.0 + phosphor_strength, 1.0 - phosphor_strength * 0.5);
        } else {
            mask = vec3(1.0 - phosphor_strength * 0.5, 1.0 - phosphor_strength * 0.5, 1.0 + phosphor_strength);
        }
        base *= mask;

        // Rolling bright bar — slow vertical scroll, scaled by roll_strength.
        float roll = sin((uv.y - TIME * 0.10) * 3.14159 * 1.5);
        base *= 1.0 + roll_strength * roll * 0.18;

        // Vignette: dim the edges, push the corners darker.
        float dist = length(dir);
        float vignette = 1.0 - smoothstep(0.38, 0.95, dist) * vignette_strength;
        base *= vignette;

        // Noise grain.
        float n = hash(FRAGCOORD.xy + TIME * 60.0) - 0.5;
        base += n * noise_strength;

        // Slight power flicker.
        base *= 1.0 - sin(TIME * 18.0) * flicker_strength;

        COLOR = vec4(clamp(base, vec3(0.0), vec3(1.0)), 1.0);
    }
}
