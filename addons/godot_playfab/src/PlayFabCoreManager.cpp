#include "PlayFabCoreManager.h"
#include "pch.h"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void PlayFabCoreManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("PFInitialize"), &PlayFabManager::PFInitialize);
}

PlayFabCoreManager::PlayFabCoreManager() {
}

PlayFabCoreManager::~PlayFabManager() {

}

int PlayFabCoreManager::PFInitialize()
{
    HRESULT hr = XGameRuntimeInitialize();    
    if (FAILED(hr))
    {
        godot::String Fail = "XGameRuntimeInitialize FAILED\n";
        godot::UtilityFunctions::print(Fail);
        return 0;
    }
    hr = PFInitialize(nullptr);
    if (FAILED(hr))
    {
        godot::String Fail = "PFInitialize FAILED\n";
        godot::UtilityFunctions::print(Fail);
        return 0;
    }
	return 1;
}