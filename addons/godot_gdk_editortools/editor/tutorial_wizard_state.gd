@tool
extends RefCounted
## Pure state machine extracted from `gdk_tutorial_wizard.gd`.
##
## Captures the slide-navigation rules (clamping, next/prev computation,
## button-label selection, counter formatting) so they can be unit-tested
## without instantiating a `Window` or any UI nodes. The wizard script
## delegates here; visible behavior is unchanged.

## Clamps `index` into the valid range `[0, total - 1]`. Returns `0` when
## `total <= 0` so the state never reports a negative slide.
static func clamp_slide_index(index: int, total: int) -> int:
	if total <= 0:
		return 0
	return clampi(index, 0, total - 1)


## Computes the slide index reached by pressing "Next" from `current`.
## On the last slide this returns `current` (the wizard treats the press
## as "finish"; callers detect that with `is_last`). Below zero is clamped
## to `0`.
static func next_slide_index(current: int, total: int) -> int:
	if total <= 0:
		return 0
	if current >= total - 1:
		return total - 1
	return clamp_slide_index(current + 1, total)


## Computes the slide index reached by pressing "Previous" from `current`.
## At slide `0` this returns `0` (the button is also disabled there).
static func prev_slide_index(current: int, total: int) -> int:
	if total <= 0:
		return 0
	return clamp_slide_index(current - 1, total)


## True when `current` is the first slide. Used to disable the Previous
## button. `total <= 0` is reported as first to keep navigation safe.
static func is_first(current: int, total: int) -> bool:
	if total <= 0:
		return true
	return clamp_slide_index(current, total) == 0


## True when `current` is the last slide. Used to swap the Next button to
## "Finish ✓" and to convert Next into a close action. `total <= 0` is
## reported as last for the same safety reason as `is_first`.
static func is_last(current: int, total: int) -> bool:
	if total <= 0:
		return true
	return clamp_slide_index(current, total) == total - 1


## Returns the label that the Next button should display for the slide
## position. Final-slide label is "  Finish ✓  ", otherwise "  Next →  ".
## Spacing matches `gdk_tutorial_wizard._show_slide`.
static func next_button_label(current: int, total: int) -> String:
	if is_last(current, total):
		return "  Finish ✓  "
	return "  Next →  "


## Returns the slide-counter label, e.g. "3 / 8". Indices are 1-based to
## match the user-facing counter. `total <= 0` returns "0 / 0".
static func format_counter(current: int, total: int) -> String:
	if total <= 0:
		return "0 / 0"
	var clamped: int = clamp_slide_index(current, total)
	return "%d / %d" % [clamped + 1, total]
