#pragma once

#include <cstdint>

struct MafiaAndroidFrameContext {
    float delta_seconds;
    uint64_t frame_index;
    int surface_width;
    int surface_height;
};

// Stable boundary between the Android platform layer and the reconstructed
// Mafia/LS3D engine. Engine code can replace these hooks incrementally without
// coupling lifecycle/EGL code to Game.exe internals.
extern "C" bool MafiaEngine_Initialize(int width, int height);
extern "C" void MafiaEngine_Resize(int width, int height);
extern "C" void MafiaEngine_Tick(const MafiaAndroidFrameContext* frame);
extern "C" void MafiaEngine_Shutdown();
