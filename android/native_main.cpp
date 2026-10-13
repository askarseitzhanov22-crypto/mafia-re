#include "engine_bridge.h"
#include "game_data.h"
#include "input_bridge.h"
#include <android/log.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#define MAFIA_LOG_TAG "Mafia-Android"
extern "C" void IJoyAndroid_Reset();
extern "C" void IJoyAndroid_SetAxis(int axis,float value);
extern "C" void IJoyAndroid_SetButton(int button,int pressed);

namespace {
struct Renderer{EGLDisplay display=EGL_NO_DISPLAY;EGLSurface surface=EGL_NO_SURFACE;EGLContext context=EGL_NO_CONTEXT;int width=0;int height=0;};
std::atomic<bool> g_running{false};std::thread g_gameThread;std::mutex g_inputMutex;AInputQueue* g_inputQueue=nullptr;ALooper* g_looper=nullptr;

void processInput(){std::lock_guard<std::mutex> lock(g_inputMutex);if(!g_inputQueue)return;AInputEvent* event=nullptr;while(AInputQueue_getEvent(g_inputQueue,&event)>=0){if(AInputQueue_preDispatchEvent(g_inputQueue,event))continue;const int handled=MafiaAndroid_HandleInputEvent(event);AInputQueue_finishEvent(g_inputQueue,event,handled);event=nullptr;}}
void gameLoop(ANativeWindow* window){Renderer r;EGLDisplay d=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(d==EGL_NO_DISPLAY||!eglInitialize(d,nullptr,nullptr)){g_running=false;ANativeWindow_release(window);return;}const EGLint attrs[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_DEPTH_SIZE,24,EGL_NONE};EGLConfig cfg=nullptr;EGLint count=0;if(!eglChooseConfig(d,attrs,&cfg,1,&count)||count<1){eglTerminate(d);g_running=false;ANativeWindow_release(window);return;}EGLint format=0;eglGetConfigAttrib(d,cfg,EGL_NATIVE_VISUAL_ID,&format);ANativeWindow_setBuffersGeometry(window,0,0,format);EGLSurface s=eglCreateWindowSurface(d,cfg,window,nullptr);const EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};EGLContext c=eglCreateContext(d,cfg,EGL_NO_CONTEXT,ca);if(s==EGL_NO_SURFACE||c==EGL_NO_CONTEXT||!eglMakeCurrent(d,s,s,c)){if(c!=EGL_NO_CONTEXT)eglDestroyContext(d,c);if(s!=EGL_NO_SURFACE)eglDestroySurface(d,s);eglTerminate(d);g_running=false;ANativeWindow_release(window);return;}r.display=d;r.surface=s;r.context=c;eglQuerySurface(d,s,EGL_WIDTH,&r.width);eglQuerySurface(d,s,EGL_HEIGHT,&r.height);MafiaAndroid_SetInputSurfaceSize(r.width,r.height);eglSwapInterval(d,1);if(!MafiaEngine_Initialize(r.width,r.height)){g_running=false;}using clock=std::chrono::steady_clock;auto prev=clock::now();uint64_t frame=0;while(g_running.load()){processInput();auto now=clock::now();float dt=std::chrono::duration<float>(now-prev).count();prev=now;if(dt>0.1f)dt=0.1f;MafiaAndroidFrameContext fc{dt,frame++,r.width,r.height};MafiaEngine_Tick(&fc);if(!eglSwapBuffers(d,s)){g_running=false;break;}}MafiaEngine_Shutdown();eglMakeCurrent(d,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);eglDestroyContext(d,c);eglDestroySurface(d,s);eglTerminate(d);ANativeWindow_release(window);}
void stopGameLoop(){g_running=false;if(g_gameThread.joinable())g_gameThread.join();}
void startGameLoop(ANativeWindow* w){stopGameLoop();if(!w)return;ANativeWindow_acquire(w);g_running=true;g_gameThread=std::thread(gameLoop,w);}
void onStart(ANativeActivity*){IJoyAndroid_Reset();}void onResume(ANativeActivity*){}void onPause(ANativeActivity*){IJoyAndroid_Reset();}void onStop(ANativeActivity*){}void onDestroy(ANativeActivity*){stopGameLoop();IJoyAndroid_Reset();MafiaAndroid_SetActivity(nullptr);}void onWindowFocusChanged(ANativeActivity*,int f){if(!f)IJoyAndroid_Reset();}
void onNativeWindowCreated(ANativeActivity*,ANativeWindow* w){startGameLoop(w);}void onNativeWindowResized(ANativeActivity*,ANativeWindow* w){MafiaAndroid_SetInputSurfaceSize(ANativeWindow_getWidth(w),ANativeWindow_getHeight(w));}void onNativeWindowDestroyed(ANativeActivity*,ANativeWindow*){stopGameLoop();}
void onInputQueueCreated(ANativeActivity*,AInputQueue* q){std::lock_guard<std::mutex> lock(g_inputMutex);g_inputQueue=q;if(g_looper)AInputQueue_attachLooper(q,g_looper,1,nullptr,nullptr);}
void onInputQueueDestroyed(ANativeActivity*,AInputQueue* q){std::lock_guard<std::mutex> lock(g_inputMutex);if(g_inputQueue==q){if(g_looper)AInputQueue_detachLooper(q);g_inputQueue=nullptr;}}
}
extern "C" __attribute__((visibility("default"))) void ANativeActivity_onCreate(ANativeActivity* a,void*,size_t){MafiaAndroid_SetActivity(a);g_looper=ALooper_forThread();if(g_looper)ALooper_acquire(g_looper);a->callbacks->onStart=onStart;a->callbacks->onResume=onResume;a->callbacks->onPause=onPause;a->callbacks->onStop=onStop;a->callbacks->onDestroy=onDestroy;a->callbacks->onWindowFocusChanged=onWindowFocusChanged;a->callbacks->onNativeWindowCreated=onNativeWindowCreated;a->callbacks->onNativeWindowResized=onNativeWindowResized;a->callbacks->onNativeWindowDestroyed=onNativeWindowDestroyed;a->callbacks->onInputQueueCreated=onInputQueueCreated;a->callbacks->onInputQueueDestroyed=onInputQueueDestroyed;IJoyAndroid_Reset();__android_log_print(ANDROID_LOG_INFO,MAFIA_LOG_TAG,"bootstrap data=%s",MafiaAndroid_GameDataRoot().c_str());}
extern "C" __attribute__((visibility("default"))) void MafiaAndroid_OnAxis(int a,float v){IJoyAndroid_SetAxis(a,v);}extern "C" __attribute__((visibility("default"))) void MafiaAndroid_OnButton(int b,int p){IJoyAndroid_SetButton(b,p);}
