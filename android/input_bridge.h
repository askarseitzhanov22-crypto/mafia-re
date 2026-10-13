#pragma once

#include <android/input.h>

// Translate Android key/gamepad/motion events into the portable IJoy state.
extern "C" int32_t MafiaAndroid_HandleInputEvent(AInputEvent* event);
