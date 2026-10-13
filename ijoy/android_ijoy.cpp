#include <android/log.h>
#include <atomic>
#include <cstdint>

#define IJOY_LOG_TAG "Mafia-IJoy"

namespace {
std::atomic<float> g_axes[8];
std::atomic<uint32_t> g_buttons[4];
}

// Android-native input bridge. Java/NativeActivity input handling can feed this
// library without pulling the original DirectInput implementation into ARM64.
extern "C" __attribute__((visibility("default"))) void IJoyAndroid_Reset() {
    for (auto &axis : g_axes) axis.store(0.0f, std::memory_order_relaxed);
    for (auto &buttons : g_buttons) buttons.store(0u, std::memory_order_relaxed);
    __android_log_print(ANDROID_LOG_INFO, IJOY_LOG_TAG, "input state reset");
}

extern "C" __attribute__((visibility("default"))) void IJoyAndroid_SetAxis(int axis, float value) {
    if (axis < 0 || axis >= 8) return;
    if (value < -1.0f) value = -1.0f;
    if (value > 1.0f) value = 1.0f;
    g_axes[axis].store(value, std::memory_order_relaxed);
}

extern "C" __attribute__((visibility("default"))) float IJoyAndroid_GetAxis(int axis) {
    if (axis < 0 || axis >= 8) return 0.0f;
    return g_axes[axis].load(std::memory_order_relaxed);
}

extern "C" __attribute__((visibility("default"))) void IJoyAndroid_SetButton(int button, int pressed) {
    if (button < 0 || button >= 128) return;
    const int bank = button / 32;
    const uint32_t mask = uint32_t(1) << (button % 32);
    uint32_t old = g_buttons[bank].load(std::memory_order_relaxed);
    uint32_t desired;
    do {
        desired = pressed ? (old | mask) : (old & ~mask);
    } while (!g_buttons[bank].compare_exchange_weak(old, desired, std::memory_order_relaxed));
}

extern "C" __attribute__((visibility("default"))) uint32_t IJoyAndroid_GetButtons(int bank) {
    if (bank < 0 || bank >= 4) return 0u;
    return g_buttons[bank].load(std::memory_order_relaxed);
}
