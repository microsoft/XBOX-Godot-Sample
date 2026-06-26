extends GutTest
## GUT coverage for `tutorial_wizard_state.gd` — the slide-navigation pure
## logic extracted from `gdk_tutorial_wizard.gd`. These tests run headless;
## the real wizard is a `Window` and is not instantiated.

const TutorialWizardState = preload("res://addons/godot_gdk_editortools/editor/tutorial_wizard_state.gd")


# ── clamp_slide_index ─────────────────────────────────────────────────────

func test_clamp_within_range_returns_input() -> void:
	assert_eq(TutorialWizardState.clamp_slide_index(3, 8), 3, "in-range index unchanged")


func test_clamp_negative_returns_zero() -> void:
	assert_eq(TutorialWizardState.clamp_slide_index(-5, 8), 0, "negative clamped to 0")


func test_clamp_above_max_returns_last_index() -> void:
	assert_eq(TutorialWizardState.clamp_slide_index(100, 8), 7, "above-max clamped to total - 1")


func test_clamp_zero_total_returns_zero() -> void:
	assert_eq(TutorialWizardState.clamp_slide_index(3, 0), 0, "zero total guarded")
	assert_eq(TutorialWizardState.clamp_slide_index(0, -1), 0, "negative total guarded")


# ── next_slide_index / prev_slide_index ───────────────────────────────────

func test_next_slide_advances_within_range() -> void:
	assert_eq(TutorialWizardState.next_slide_index(0, 8), 1, "0 → 1")
	assert_eq(TutorialWizardState.next_slide_index(5, 8), 6, "5 → 6")


func test_next_slide_clamps_at_last() -> void:
	assert_eq(TutorialWizardState.next_slide_index(7, 8), 7, "last slide stays")


func test_prev_slide_decreases_within_range() -> void:
	assert_eq(TutorialWizardState.prev_slide_index(3, 8), 2, "3 → 2")
	assert_eq(TutorialWizardState.prev_slide_index(7, 8), 6, "7 → 6")


func test_prev_slide_clamps_at_zero() -> void:
	assert_eq(TutorialWizardState.prev_slide_index(0, 8), 0, "first slide stays")


func test_navigation_safe_with_zero_total() -> void:
	assert_eq(TutorialWizardState.next_slide_index(0, 0), 0, "next safe when total=0")
	assert_eq(TutorialWizardState.prev_slide_index(0, 0), 0, "prev safe when total=0")


# ── is_first / is_last ────────────────────────────────────────────────────

func test_is_first_only_at_zero() -> void:
	assert_true(TutorialWizardState.is_first(0, 8), "slide 0 is first")
	assert_false(TutorialWizardState.is_first(1, 8), "slide 1 is not first")
	assert_false(TutorialWizardState.is_first(7, 8), "slide 7 is not first")


func test_is_last_only_at_max_index() -> void:
	assert_false(TutorialWizardState.is_last(0, 8), "slide 0 is not last")
	assert_false(TutorialWizardState.is_last(6, 8), "slide 6 is not last in 8")
	assert_true(TutorialWizardState.is_last(7, 8), "slide 7 is last in 8")


func test_is_first_and_last_when_total_zero() -> void:
	assert_true(TutorialWizardState.is_first(0, 0), "is_first true when total<=0")
	assert_true(TutorialWizardState.is_last(0, 0), "is_last true when total<=0")


# ── next_button_label ─────────────────────────────────────────────────────

func test_next_button_label_intermediate_slides() -> void:
	for i in range(0, 7):
		assert_eq(
			TutorialWizardState.next_button_label(i, 8),
			"  Next →  ",
			"slide %d shows Next →" % i)


func test_next_button_label_final_slide_says_finish() -> void:
	assert_eq(
		TutorialWizardState.next_button_label(7, 8),
		"  Finish ✓  ",
		"final slide shows Finish ✓")


# ── format_counter ────────────────────────────────────────────────────────

func test_counter_uses_one_based_indexing() -> void:
	assert_eq(TutorialWizardState.format_counter(0, 8), "1 / 8", "first slide reads 1 / 8")
	assert_eq(TutorialWizardState.format_counter(7, 8), "8 / 8", "last slide reads 8 / 8")
	assert_eq(TutorialWizardState.format_counter(3, 8), "4 / 8", "middle slide formatted")


func test_counter_clamps_out_of_range_indices() -> void:
	assert_eq(
		TutorialWizardState.format_counter(99, 8),
		"8 / 8",
		"index above max clamped before formatting")
	assert_eq(
		TutorialWizardState.format_counter(-1, 8),
		"1 / 8",
		"negative index clamped before formatting")


func test_counter_zero_total() -> void:
	assert_eq(TutorialWizardState.format_counter(0, 0), "0 / 0", "zero total returns 0 / 0")
