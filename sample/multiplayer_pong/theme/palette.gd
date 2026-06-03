@tool
class_name PongPalette
extends RefCounted
## Xbox-flavoured retro palette for the Pong Royale demo.
##
## Single source of truth so every scene/script references the same colors.
## Keep this in sync with `xbox_theme.tres` if you tweak values.

# Brand greens.
const XBOX_GREEN: Color = Color8(0x10, 0x7C, 0x10)        # primary
const XBOX_GREEN_DEEP: Color = Color8(0x0E, 0x5A, 0x0E)   # shadow / pressed
const XBOX_GREEN_GLOW: Color = Color8(0x9B, 0xF0, 0x9B)   # highlight / focus

# Surfaces.
const BACKGROUND: Color = Color8(0x05, 0x0A, 0x05)        # near-black with green tint
const SURFACE: Color = Color8(0x0A, 0x14, 0x0A)           # panel
const SURFACE_HIGH: Color = Color8(0x14, 0x28, 0x14)      # raised panel

# Text.
const TEXT_PRIMARY: Color = Color8(0xE6, 0xFF, 0xE6)      # near-white green tint
const TEXT_SECONDARY: Color = Color8(0x7A, 0xC0, 0x7A)
const TEXT_MUTED: Color = Color8(0x4A, 0x70, 0x4A)
const TEXT_DANGER: Color = Color8(0xFF, 0x55, 0x55)

# Gameplay accents.
const BALL_TRAIL: Color = Color8(0x9B, 0xF0, 0x9B)
const PADDLE_PLAYER: Color = Color8(0x9B, 0xF0, 0x9B)
const PADDLE_ENEMY: Color = Color8(0xFF, 0x66, 0x66)
const BOSS_HUD: Color = Color8(0xFF, 0xCC, 0x00)
