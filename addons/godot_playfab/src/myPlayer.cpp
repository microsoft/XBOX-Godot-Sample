#include "myPlayer.h"

using namespace godot;

void myPlayer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &myPlayer::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &myPlayer::get_speed);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
}

myPlayer::myPlayer() {

}

myPlayer::~myPlayer() {

}

void myPlayer::_physics_process(double p_delta)
{
	Vector2 motion = Vector2(0, 0);


	Input* i = Input::get_singleton();

	if (i->is_action_pressed("ui_up"))
		motion.y -= speed;
	if (i->is_action_pressed("ui_down"))
		motion.y += speed;
	if (i->is_action_pressed("ui_left"))
		motion.x -= speed;
	if (i->is_action_pressed("ui_right"))
		motion.x += speed;

	set_velocity(motion.normalized() * speed);

	move_and_slide();
}

void myPlayer::set_speed(float p_speed) {
	speed = p_speed;
}

float myPlayer::get_speed() const {
	return speed;
}