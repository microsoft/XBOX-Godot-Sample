#pragma once

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/input.hpp>
#include "pch.h"

namespace godot {
	class PlayFabCoreManager : public Node 
	{
		GDCLASS(PlayFabCoreManager, Node);
protected:
	static void _bind_methods();
public:
		PlayFabCoreManager();
		~PlayFabCoreManager();
		int PFInitialize();
	};
}
