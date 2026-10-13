#include "input_bridge.h"
#include <android/keycodes.h>
#include <cmath>

extern "C" void IJoyAndroid_SetAxis(int axis, float value);
extern "C" void IJoyAndroid_SetButton(int button, int pressed);

namespace {
float deadzone(float value) { return std::fabs(value) < 0.12f ? 0.0f : value; }
float clampAxis(float value) { return value < -1.0f ? -1.0f : (value > 1.0f ? 1.0f : value); }

int mapKey(int keyCode) {
    switch (keyCode) {
        case AKEYCODE_BUTTON_A: return 0; case AKEYCODE_BUTTON_B: return 1;
        case AKEYCODE_BUTTON_X: return 2; case AKEYCODE_BUTTON_Y: return 3;
        case AKEYCODE_BUTTON_L1: return 4; case AKEYCODE_BUTTON_R1: return 5;
        case AKEYCODE_BUTTON_L2: return 6; case AKEYCODE_BUTTON_R2: return 7;
        case AKEYCODE_BUTTON_SELECT: return 8; case AKEYCODE_BUTTON_START: return 9;
        case AKEYCODE_BUTTON_THUMBL: return 10; case AKEYCODE_BUTTON_THUMBR: return 11;
        case AKEYCODE_DPAD_UP: return 12; case AKEYCODE_DPAD_DOWN: return 13;
        case AKEYCODE_DPAD_LEFT: return 14; case AKEYCODE_DPAD_RIGHT: return 15;
        default: return -1;
    }
}

void handleTouch(AInputEvent* event) {
    const int action = AMotionEvent_getAction(event);
    const int masked = action & AMOTION_EVENT_ACTION_MASK;
    const size_t count = AMotionEvent_getPointerCount(event);
    if (masked == AMOTION_EVENT_ACTION_UP || masked == AMOTION_EVENT_ACTION_CANCEL) {
        IJoyAndroid_SetAxis(0, 0.0f); IJoyAndroid_SetAxis(1, 0.0f);
        IJoyAndroid_SetAxis(2, 0.0f); IJoyAndroid_SetAxis(3, 0.0f);
        IJoyAndroid_SetButton(0, 0); return;
    }
    for (size_t i = 0; i < count; ++i) {
        const float x = AMotionEvent_getX(event, i);
        const float y = AMotionEvent_getY(event, i);
        // NativeActivity coordinates are pixels. Left side acts as a relative
        // movement stick; right side currently maps touch to the primary action.
        if (x < 900.0f) {
            IJoyAndroid_SetAxis(0, clampAxis((x - 450.0f) / 350.0f));
            IJoyAndroid_SetAxis(1, clampAxis((y - 900.0f) / 350.0f));
        } else {
            IJoyAndroid_SetButton(0, 1);
        }
    }
}
}

extern "C" int32_t MafiaAndroid_HandleInputEvent(AInputEvent* event) {
    if (!event) return 0;
    const int type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_KEY) {
        const int button = mapKey(AKeyEvent_getKeyCode(event));
        if (button < 0) return 0;
        const int action = AKeyEvent_getAction(event);
        if (action == AKEY_EVENT_ACTION_DOWN || action == AKEY_EVENT_ACTION_UP) {
            IJoyAndroid_SetButton(button, action == AKEY_EVENT_ACTION_DOWN); return 1;
        }
        return 0;
    }
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        const int source = AInputEvent_getSource(event);
        if ((source & AINPUT_SOURCE_JOYSTICK) == AINPUT_SOURCE_JOYSTICK ||
            (source & AINPUT_SOURCE_GAMEPAD) == AINPUT_SOURCE_GAMEPAD) {
            IJoyAndroid_SetAxis(0, deadzone(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_X, 0)));
            IJoyAndroid_SetAxis(1, deadzone(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Y, 0)));
            IJoyAndroid_SetAxis(2, deadzone(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_Z, 0)));
            IJoyAndroid_SetAxis(3, deadzone(AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RZ, 0)));
            IJoyAndroid_SetAxis(4, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_LTRIGGER, 0));
            IJoyAndroid_SetAxis(5, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_RTRIGGER, 0));
            IJoyAndroid_SetAxis(6, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_X, 0));
            IJoyAndroid_SetAxis(7, AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_HAT_Y, 0)); return 1;
        }
        if ((source & AINPUT_SOURCE_TOUCHSCREEN) == AINPUT_SOURCE_TOUCHSCREEN) { handleTouch(event); return 1; }
    }
    return 0;
}
