#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/character_body2D.hpp>
#include <godot_cpp/classes/input.hpp>

namespace godot {
	class myPlayer : public CharacterBody2D
	{
	
	// Godot structure
	private:
		GDCLASS(myPlayer, CharacterBody2D);
		float speed = 100.0;

	protected:
		static void _bind_methods();

	public:
		virtual void _physics_process(double p_delta) override;
		void set_speed(float p_speed);
		float get_speed() const;
		myPlayer();
		~myPlayer();

	};
}
