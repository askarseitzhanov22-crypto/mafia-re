#include <android/input.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <cstdint>

#define MAFIA_LOG_TAG "Mafia-Android"

extern "C" void IJoyAndroid_Reset();
extern "C" void IJoyAndroid_SetAxis(int axis, float value);
extern "C" void IJoyAndroid_SetButton(int button, int pressed);

namespace {
void onStart(ANativeActivity*) {
    IJoyAndroid_Reset();
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "native activity started");
}

void onResume(ANativeActivity*) {
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "native activity resumed");
}

void onPause(ANativeActivity*) {
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "native activity paused");
}

void onStop(ANativeActivity*) {
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "native activity stopped");
}

void onDestroy(ANativeActivity*) {
    IJoyAndroid_Reset();
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "native activity destroyed");
}

void onWindowFocusChanged(ANativeActivity*, int focused) {
    if (!focused) IJoyAndroid_Reset();
}
}

extern "C" __attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity* activity, void*, size_t) {
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    IJoyAndroid_Reset();
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "Mafia Android bootstrap created");
}

// Input dispatch entry points for the future Java/GameActivity glue.
extern "C" __attribute__((visibility("default")))
void MafiaAndroid_OnAxis(int axis, float value) {
    IJoyAndroid_SetAxis(axis, value);
}

extern "C" __attribute__((visibility("default")))
void MafiaAndroid_OnButton(int button, int pressed) {
    IJoyAndroid_SetButton(button, pressed);
}
