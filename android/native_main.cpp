#include "engine_bridge.h"
#include <android/log.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <atomic>
#include <chrono>
#include <thread>

#define MAFIA_LOG_TAG "Mafia-Android"

extern "C" void IJoyAndroid_Reset();
extern "C" void IJoyAndroid_SetAxis(int axis, float value);
extern "C" void IJoyAndroid_SetButton(int button, int pressed);

namespace {
struct Renderer {
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    int width = 0;
    int height = 0;
} g_renderer;

std::atomic<bool> g_running{false};
std::thread g_gameThread;

void shutdownRenderer() {
    if (g_renderer.display != EGL_NO_DISPLAY) {
        MafiaEngine_Shutdown();
        eglMakeCurrent(g_renderer.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (g_renderer.context != EGL_NO_CONTEXT) eglDestroyContext(g_renderer.display, g_renderer.context);
        if (g_renderer.surface != EGL_NO_SURFACE) eglDestroySurface(g_renderer.display, g_renderer.surface);
        eglTerminate(g_renderer.display);
    }
    g_renderer = Renderer{};
}

void gameLoop() {
    using clock = std::chrono::steady_clock;
    auto previous = clock::now();
    uint64_t frameIndex = 0;
    while (g_running.load(std::memory_order_acquire)) {
        const auto frameStart = clock::now();
        float dt = std::chrono::duration<float>(frameStart - previous).count();
        previous = frameStart;
        if (dt > 0.1f) dt = 0.1f;
        MafiaAndroidFrameContext frame{dt, frameIndex++, g_renderer.width, g_renderer.height};
        MafiaEngine_Tick(&frame);
        if (!eglSwapBuffers(g_renderer.display, g_renderer.surface)) {
            __android_log_print(ANDROID_LOG_ERROR, MAFIA_LOG_TAG, "eglSwapBuffers failed: 0x%x", eglGetError());
            g_running.store(false, std::memory_order_release);
            break;
        }
        const auto elapsed = clock::now() - frameStart;
        const auto target = std::chrono::microseconds(16667);
        if (elapsed < target) std::this_thread::sleep_for(target - elapsed);
    }
}

void stopGameLoop() {
    g_running.store(false, std::memory_order_release);
    if (g_gameThread.joinable()) g_gameThread.join();
}

bool initRenderer(ANativeWindow* window) {
    stopGameLoop(); shutdownRenderer();
    if (!window) return false;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr)) return false;
    const EGLint configAttrs[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                  EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_NONE};
    EGLConfig config = nullptr; EGLint count = 0;
    if (!eglChooseConfig(display, configAttrs, &config, 1, &count) || count < 1) { eglTerminate(display); return false; }
    EGLint format = 0; eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(window, 0, 0, format);
    EGLSurface surface = eglCreateWindowSurface(display, config, window, nullptr);
    const EGLint contextAttrs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttrs);
    if (surface == EGL_NO_SURFACE || context == EGL_NO_CONTEXT || !eglMakeCurrent(display, surface, surface, context)) {
        if (context != EGL_NO_CONTEXT) eglDestroyContext(display, context);
        if (surface != EGL_NO_SURFACE) eglDestroySurface(display, surface);
        eglTerminate(display); return false;
    }
    g_renderer.display = display; g_renderer.surface = surface; g_renderer.context = context;
    eglQuerySurface(display, surface, EGL_WIDTH, &g_renderer.width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &g_renderer.height);
    if (!MafiaEngine_Initialize(g_renderer.width, g_renderer.height)) { shutdownRenderer(); return false; }
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "runtime ready: %dx%d", g_renderer.width, g_renderer.height);
    g_running.store(true, std::memory_order_release);
    g_gameThread = std::thread(gameLoop);
    return true;
}

void onStart(ANativeActivity*) { IJoyAndroid_Reset(); }
void onResume(ANativeActivity*) {}
void onPause(ANativeActivity*) { IJoyAndroid_Reset(); }
void onStop(ANativeActivity*) {}
void onDestroy(ANativeActivity*) { stopGameLoop(); shutdownRenderer(); IJoyAndroid_Reset(); }
void onWindowFocusChanged(ANativeActivity*, int focused) { if (!focused) IJoyAndroid_Reset(); }
void onNativeWindowCreated(ANativeActivity*, ANativeWindow* window) {
    if (!initRenderer(window)) __android_log_print(ANDROID_LOG_ERROR, MAFIA_LOG_TAG, "runtime initialization failed");
}
void onNativeWindowResized(ANativeActivity*, ANativeWindow*) {
    if (g_renderer.display == EGL_NO_DISPLAY) return;
    eglQuerySurface(g_renderer.display, g_renderer.surface, EGL_WIDTH, &g_renderer.width);
    eglQuerySurface(g_renderer.display, g_renderer.surface, EGL_HEIGHT, &g_renderer.height);
    MafiaEngine_Resize(g_renderer.width, g_renderer.height);
}
void onNativeWindowDestroyed(ANativeActivity*, ANativeWindow*) { stopGameLoop(); shutdownRenderer(); }
}

extern "C" __attribute__((visibility("default"))) void ANativeActivity_onCreate(ANativeActivity* activity, void*, size_t) {
    activity->callbacks->onStart = onStart; activity->callbacks->onResume = onResume;
    activity->callbacks->onPause = onPause; activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy; activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = onNativeWindowResized;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    IJoyAndroid_Reset();
    __android_log_print(ANDROID_LOG_INFO, MAFIA_LOG_TAG, "Mafia Android bootstrap created");
}

extern "C" __attribute__((visibility("default"))) void MafiaAndroid_OnAxis(int axis, float value) { IJoyAndroid_SetAxis(axis, value); }
extern "C" __attribute__((visibility("default"))) void MafiaAndroid_OnButton(int button, int pressed) { IJoyAndroid_SetButton(button, pressed); }
