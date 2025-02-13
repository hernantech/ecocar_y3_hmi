#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <gbm.h>

// Define the EGL platform extension if not defined
#ifndef EGL_PLATFORM_GBM_KHR
#define EGL_PLATFORM_GBM_KHR 0x31D7
#endif

// Get the platform display function pointer
typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);

int main() 
{
    // Open the DRM render node
    int fd = open("/dev/dri/renderD128", O_RDWR);
    if (fd < 0) {
        printf("Failed to open DRM render node\n");
        return 1;
    }
    printf("Opened DRM render node\n");

    // Create GBM device
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        printf("Failed to create GBM device\n");
        close(fd);
        return 1;
    }
    printf("Created GBM device\n");

    // Get the platform display function
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
    
    if (!eglGetPlatformDisplayEXT) {
        printf("Failed to get eglGetPlatformDisplayEXT function\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Get EGL display from GBM device
    EGLDisplay display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm, NULL);
    if (display == EGL_NO_DISPLAY) {
        printf("Failed to get EGL display\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }
    printf("Got EGL display\n");

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("Failed to initialize EGL\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    printf("EGL initialized successfully! Version %d.%d\n", major, minor);
    printf("EGL Vendor: %s\n", eglQueryString(display, EGL_VENDOR));
    printf("EGL Version: %s\n", eglQueryString(display, EGL_VERSION));
    printf("EGL Extensions: %s\n", eglQueryString(display, EGL_EXTENSIONS));

    // Cleanup
    eglTerminate(display);
    gbm_device_destroy(gbm);
    close(fd);
    return 0;
}