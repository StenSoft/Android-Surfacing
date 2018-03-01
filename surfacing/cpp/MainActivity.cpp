#include "MainActivity.h"

#include "guard.h"
#include "log.h"

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

/** The surface and its native window. */
static jobject surface = nullptr;
static ANativeWindow * window = nullptr;

/** Used to alternate colors. */
static int iteration = 0;

/** Colors to draw. */
static constexpr uint32_t colors[] = {
    0x88bf360c,
    0xcc3e2723,
    0xaaffd600,
    0x6664dd17,
    0x880277bd,
    0xff880e4f
};


/** Classes and methods from JNI. */
namespace JNI
{
    /** android.view.Surface class */
    static jclass surface;

    /** android.view.Surface constructor */
    static jmethodID surface_init;

    /** android.view.Surface.release() */
    static jmethodID surface_release;
}

/** OpenGL stuff. */
namespace EGL
{
    /** OpenGL display. There's only one, the default. */
    static EGLDisplay display = nullptr;

    /** Configuration. We want OpenGL ES2 with RGBA. */
    static EGLConfig config = nullptr;

    /** OpenGL drawing context. Each TextureView has its own. */
    static EGLContext context = nullptr;

    /** Surface to draw to. */
    static EGLSurface surface = nullptr;
}


bool MainActivity_OnLoad(
        JNIEnv* env)
{
    auto surface = env->FindClass("android/view/Surface");
    if (surface == nullptr)
        return false;
    else
        JNI::surface = static_cast<jclass>(env->NewGlobalRef(surface));

    JNI::surface_init = env->GetMethodID(JNI::surface, "<init>", "(Landroid/graphics/SurfaceTexture;)V");
    if (JNI::surface_init == nullptr)
        return false;

    JNI::surface_release = env->GetMethodID(JNI::surface, "release", "()V");
    if (JNI::surface_release == nullptr)
        return false;

    return true;
}


/**
 * Initialize window from the surface.
 *
 * @param env JNI environment.
 * @param activity Activity.
 */
bool initWindow(
        JNIEnv* env,
        jobject activity)
{
    if (::surface == nullptr)
    {
        log("No surface");
        return false;
    }

    ::window = ANativeWindow_fromSurface(env, ::surface);
    if (::window == nullptr)
    {
        log("Failed to obtain window");
        return false;
    }
    return true;
}


/**
 * Create surface for surface texture.
 *
 * @param env JNI environment.
 * @param activity Activity.
 * @param texture Surface texture to create surface for.
 */
void createSurface(
        JNIEnv* env,
        jobject activity,
        jobject texture)
{
    jvalue params[1];
    params[0].l = texture;
    auto surface = env->NewObjectA(JNI::surface, JNI::surface_init, params);
    if (surface == nullptr)
    {
        log("Failed to construct surface");
        return;
    }
    ::surface = env->NewGlobalRef(surface);
}


/**
 * Release surface.
 *
 * @param env JNI environment.
 * @param activity Activity.
 */
void releaseSurface(
        JNIEnv* env,
        jobject activity)
{
    env->CallVoidMethod(::surface, JNI::surface_release);
    env->DeleteGlobalRef(::surface);
    ::surface = nullptr;
}


/**
 * Draw.
 *
 * @param env JNI environment.
 * @param activity Activity.
 * @param color Color to draw (ARGB).
 */
void draw(
        JNIEnv *env,
        jobject activity,
        uint32_t color)
{
    if (EGL::surface == nullptr)
    {
        // -*-*-*-*-*-*- CPU rendering -*-*-*-*-*-*-

        // For our example, scale the surface to 1×1 pixel and fill it with a color
        auto ret = ANativeWindow_setBuffersGeometry(::window, 1, 1,
                                                    AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM);
        if (ret != 0)
        {
            log("Failed to set buffers geometry");
            return;
        }

        ANativeWindow_Buffer surface;
        ARect bounds{ 0, 0, 1, 1 };
        ret = ANativeWindow_lock(window, &surface, &bounds);
        if (ret != 0)
        {
            log("Failed to lock");
            return;
        }

        // TODO Locked bounds can be larger than requested, we should check them

        uint8_t *buffer = static_cast<uint8_t*>(surface.bits);
        // Value in color is ARGB but the surface expects RGBA
        buffer[0] = color >> 16;
        buffer[1] = color >> 8;
        buffer[2] = color >> 0;
        buffer[3] = color >> 24;

        ret = ANativeWindow_unlockAndPost(window);
        if (ret != 0)
        {
            log("Failed to post");
            return;
        }
        log("Drawn %08x using Native Window", color);
        return;
    }

    // -*-*-*-*-*-*- OpenGL rendering -*-*-*-*-*-*-

    /* As we have only one surface on this thread, eglMakeCurrent can be called in initialization
     * but if you would want to draw multiple surfaces on the same thread, you need to change
     * current context and the easiest way to keep track of the current surface is to change it on
     * each draw so that's what is shown here. Each thread has its own current context and one
     * context cannot be current on multiple threads at the same time. */
    if (!eglMakeCurrent(EGL::display, EGL::surface, EGL::surface, EGL::context))
    {
        log("Failed to attach context");
        return;
    }
    /* If you move eglMakeCurrent(EGL::context) to intiialization, eglMakeCurrent(EGL_NO_CONTEXT)
     * should go to deinitialization. Neither eglDestroyContext nor eglTerminate disconnects the
     * surface, only marks it for deletion when it's disconnected. */
    auto contextCurrentGuard = guard([=]{ eglMakeCurrent(EGL::display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT); });

    // Note the dot after divide, the division has to be floating-point
    glClearColor(
        (color & 0x00ff0000) / 16777216.,
        (color & 0x0000ff00) / 65536.,
        (color & 0x000000ff) / 256.,
        (color & 0xff000000) / 4294967296.);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(EGL::display, EGL::surface);
    log("Drawn %08x using OpenGL", color);
}


extern "C"
JNIEXPORT void JNICALL
Java_online_adamek_sten_surfacing_MainActivity_onSelected(
        JNIEnv* env,
        jobject activity,
        jobject texture,
        jint selection)
{
    if (::window == nullptr && selection == 0)
        // Shortcut, don't log
        return;

    // Tear down
    if (::window != nullptr)
    {
        draw(env, activity, 0x0); // Clear the surface

        if (EGL::surface != nullptr)
            eglDestroySurface(EGL::display, EGL::surface);
        EGL::surface = nullptr;

        if (EGL::context != nullptr)
            eglDestroyContext(EGL::display, EGL::context);
        EGL::context = nullptr;

        ANativeWindow_release(::window);
        ::window = nullptr;
    }

    if (::surface != nullptr)
        /* Releasing the surface is extremely important. You can't initialize OpenGL on the same
         * surface which was used for CPU rendering as there is no way how to deinitialize CPU
         * rendering on a surface (OpenGL can be disconnected with eglMakeCurrent(EGL_NO_CONTEXT)).
         * So each time you want to switch, you need to create a new surface for the surface
         * texture, but to be able to do so, you need to release the original surface first. */
        releaseSurface(env, activity);

    if (selection == 0)
    {
        // No implementation selected
        log("Deinitialized");
        return;
    }

    createSurface(env, activity, texture);
    if (!initWindow(env, activity))
        return;

    if (selection == 1 /* CPU rendering */)
    {
        log("CPU rendering initialized");
    }
    else if (selection == 2 /* OpenGL */)
    {
        // Display and config need to be initialized only once
        if (EGL::display == nullptr)
        {
            EGL::display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            eglInitialize(EGL::display, nullptr, nullptr);
            if (EGL::display == nullptr)
            {
                log("Failed to initialize OpenGL display");
                return;
            }

            {
                // We want to use OpenGL ES2 with RGBA
                EGLint attrs[] = {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_RED_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_BLUE_SIZE, 8,
                    EGL_ALPHA_SIZE, 8,
                    EGL_NONE
                };
                int num;
                if (!eglChooseConfig(EGL::display, attrs, &EGL::config, 1, &num) || num != 1)
                {
                    log("No OpenGL display config chosen");
                    return;
                }
            }
        }

        {
            EGLint attrs[] =
            {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
            };
            EGL::context = eglCreateContext(EGL::display, EGL::config, nullptr, attrs);
            if (EGL::context == nullptr)
            {
                log("Failed to create OpenGL context");
                return;
            }
        }

        EGL::surface = eglCreateWindowSurface(EGL::display, EGL::config, ::window, nullptr);
        if (EGL::surface == nullptr)
        {
            log("Failed to create OpenGL surface");
            return;
        }

        log("OpenGL initialized");
    }
}


extern "C"
JNIEXPORT void JNICALL
Java_online_adamek_sten_surfacing_MainActivity_onDraw(
        JNIEnv* env,
        jobject activity)
{
    if (::surface == nullptr)
    {
        log("No engine initialized");
        return;
    }

    if (::window == nullptr)
        if (!initWindow(env, activity))
            return;

    draw(env, activity, colors[iteration++ % (sizeof(colors) / sizeof(*colors))]);
}
