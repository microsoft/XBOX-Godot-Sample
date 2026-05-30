extends "res://addons/godot_gdk_tests/gdk_test_base.gd"


func test_achievement_default_values() -> void:
	if pending_unless_runtime_available():
		return

	var achievement = instantiate_class("GDKAchievement")
	assert_not_null(achievement, "GDKAchievement.new() returns wrapper")
	if achievement == null:
		return

	for method_name in [
		"get_id",
		"get_name",
		"get_service_configuration_id",
		"get_progress_state",
		"get_progress_percent",
		"is_unlocked",
		"is_secret",
		"get_locked_description",
		"get_unlocked_description",
	]:
		assert_has_method_named(achievement, method_name)

	assert_eq(achievement.get_id(), "", "blank GDKAchievement id defaults empty")
	assert_eq(achievement.get_name(), "", "blank GDKAchievement name defaults empty")
	assert_eq(achievement.get_service_configuration_id(), "", "blank GDKAchievement SCID defaults empty")
	assert_eq(achievement.get_progress_state(), "", "blank GDKAchievement progress_state defaults empty")
	assert_eq(achievement.get_progress_percent(), 0, "blank GDKAchievement progress_percent defaults zero")
	assert_eq(achievement.is_unlocked(), false, "blank GDKAchievement unlocked defaults false")
	assert_eq(achievement.is_secret(), false, "blank GDKAchievement secret defaults false")
	assert_eq(achievement.get_locked_description(), "", "blank GDKAchievement locked_description defaults empty")
	assert_eq(achievement.get_unlocked_description(), "", "blank GDKAchievement unlocked_description defaults empty")
