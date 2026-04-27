#include "PlayFabManager.h"
#include "pch.h"

using namespace godot;

void PlayFabManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("returnTitleID"), &PlayFabManager::returnTitleID);
	ClassDB::bind_method(D_METHOD("RunPlayFabSDKSample"), &PlayFabManager::RunPlayFabSDKSample);
}

PlayFabManager::PlayFabManager() {
}

PlayFabManager::~PlayFabManager() {

}

String PlayFabManager::returnTitleID() {
	return "99DA";
}

int PlayFabManager::RunPlayFabSDKSample()
{
	utils.RunPlayFabSDKSample(TitleID);
	return 1;
}