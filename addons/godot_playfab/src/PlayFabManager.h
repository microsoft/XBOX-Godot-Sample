#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input.hpp>
#include "pch.h"

namespace godot {
	class PlayFabManager : public Node 
	{
		GDCLASS(PlayFabManager, Node);
		std::string TitleID = "99DA";
		PFUtils utils;
protected:
	static void _bind_methods();
public:
		PlayFabManager();
		~PlayFabManager();
		String returnTitleID();
		int RunPlayFabSDKSample();
	};
}
