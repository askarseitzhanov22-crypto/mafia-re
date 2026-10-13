#include "engine_bridge.h"
#include <android/log.h>
#include <GLES3/gl3.h>

#define MAFIA_ENGINE_TAG "Mafia-Engine"

namespace {
bool g_initialized = false;
int g_width = 0;
int g_height = 0;
}

extern "C" bool MafiaEngine_Initialize(int width, int height) {
    g_width = width;
    g_height = height;
    g_initialized = true;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    __android_log_print(ANDROID_LOG_INFO, MAFIA_ENGINE_TAG,
                        "engine bridge initialized: %dx%d", width, height);
    return true;
}

extern "C" void MafiaEngine_Resize(int width, int height) {
    g_width = width;
    g_height = height;
}

extern "C" void MafiaEngine_Tick(const MafiaAndroidFrameContext* frame) {
    if (!g_initialized || !frame) return;
    g_width = frame->surface_width;
    g_height = frame->surface_height;

    // Temporary renderer until LS3DF/Game.exe reconstruction is connected.
    // Keep all placeholder drawing behind this bridge so platform code stays
    // usable when the real engine arrives.
    const float pulse = static_cast<float>(frame->frame_index % 180u) / 180.0f;
    glViewport(0, 0, g_width, g_height);
    glClearColor(0.025f + pulse * 0.025f, 0.035f, 0.055f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

extern "C" void MafiaEngine_Shutdown() {
    if (!g_initialized) return;
    g_initialized = false;
    __android_log_print(ANDROID_LOG_INFO, MAFIA_ENGINE_TAG, "engine bridge shutdown");
}
