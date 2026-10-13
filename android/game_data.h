#pragma once

#include <android/native_activity.h>
#include <cstddef>
#include <string>
#include <vector>

extern "C" void MafiaAndroid_SetActivity(ANativeActivity* activity);
std::string MafiaAndroid_GameDataRoot();
bool MafiaAndroid_ReadGameFile(const char* relativePath, std::vector<unsigned char>& out);
bool MafiaAndroid_GameFileExists(const char* relativePath);
