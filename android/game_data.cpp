#include "game_data.h"
#include <android/log.h>
#include <cstdio>

#define MAFIA_DATA_TAG "Mafia-Data"

namespace { ANativeActivity* g_activity = nullptr; }

extern "C" void MafiaAndroid_SetActivity(ANativeActivity* activity) { g_activity = activity; }

std::string MafiaAndroid_GameDataRoot() {
    if (!g_activity || !g_activity->externalDataPath) return std::string();
    return std::string(g_activity->externalDataPath) + "/mafia";
}

static bool safeRelativePath(const char* path) {
    if (!path || !*path || path[0] == '/' || path[0] == '\\') return false;
    const std::string p(path);
    return p.find("..") == std::string::npos;
}

bool MafiaAndroid_ReadGameFile(const char* relativePath, std::vector<unsigned char>& out) {
    out.clear();
    if (!safeRelativePath(relativePath)) return false;
    const std::string root = MafiaAndroid_GameDataRoot();
    if (root.empty()) return false;
    const std::string full = root + "/" + relativePath;
    FILE* file = std::fopen(full.c_str(), "rb");
    if (!file) return false;
    std::fseek(file, 0, SEEK_END); const long size = std::ftell(file); std::rewind(file);
    if (size < 0) { std::fclose(file); return false; }
    out.resize(static_cast<size_t>(size));
    const size_t read = size ? std::fread(out.data(), 1, out.size(), file) : 0;
    std::fclose(file);
    if (read != out.size()) { out.clear(); return false; }
    return true;
}

bool MafiaAndroid_GameFileExists(const char* relativePath) {
    if (!safeRelativePath(relativePath)) return false;
    const std::string root = MafiaAndroid_GameDataRoot(); if (root.empty()) return false;
    FILE* file = std::fopen((root + "/" + relativePath).c_str(), "rb");
    if (!file) return false; std::fclose(file); return true;
}
