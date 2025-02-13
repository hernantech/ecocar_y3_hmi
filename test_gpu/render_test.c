#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <gbm.h>
#include <string.h>

#ifndef EGL_PLATFORM_GBM_KHR
#define EGL_PLATFORM_GBM_KHR 0x31D7
#endif

typedef EGLDisplay (*PFNEGLGETPLATFORMDISPLAYEXTPROC) (EGLenum platform, void *native_display, const EGLint *attrib_list);

// Simple vertex shader
const char *vertex_shader_source =
    "attribute vec4 position;    \n"
    "void main()                 \n"
    "{                          \n"
    "   gl_Position = position; \n"
    "}                          \n";

// Simple fragment shader
const char *fragment_shader_source =
    "precision mediump float;    \n"
    "void main()                 \n"
    "{                          \n"
    "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);  \n"
    "}                          \n";

// Compile shader
GLuint compile_shader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        if (info_len > 1) {
            char* info_log = malloc(sizeof(char) * info_len);
            glGetShaderInfoLog(shader, info_len, NULL, info_log);
            printf("Error compiling shader: %s\n", info_log);
            free(info_log);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

int main() 
{
    // Open DRM render node
    int fd = open("/dev/dri/renderD128", O_RDWR);
    if (fd < 0) {
        printf("Failed to open DRM render node\n");
        return 1;
    }

    // Create GBM device
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        printf("Failed to create GBM device\n");
        close(fd);
        return 1;
    }

    // Get platform display function
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");
    
    if (!eglGetPlatformDisplayEXT) {
        printf("Failed to get eglGetPlatformDisplayEXT function\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Get EGL display
    EGLDisplay display = eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR, gbm, NULL);
    if (display == EGL_NO_DISPLAY) {
        printf("Failed to get EGL display\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(display, &major, &minor)) {
        printf("Failed to initialize EGL\n");
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Choose configuration
    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_config;
    if (!eglChooseConfig(display, config_attribs, &config, 1, &num_config)) {
        printf("Failed to choose config\n");
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Create pbuffer surface
    const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, 256,
        EGL_HEIGHT, 256,
        EGL_NONE,
    };

    EGLSurface surface = eglCreatePbufferSurface(display, config, pbuffer_attribs);
    if (surface == EGL_NO_SURFACE) {
        printf("Failed to create pbuffer surface\n");
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Create context
    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);
    if (context == EGL_NO_CONTEXT) {
        printf("Failed to create context\n");
        eglDestroySurface(display, surface);
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Make current
    if (!eglMakeCurrent(display, surface, surface, context)) {
        printf("Failed to make context current\n");
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    printf("OpenGL ES 2.0 context initialized successfully!\n");

    // Compile shaders
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!vertex_shader || !fragment_shader) {
        printf("Failed to compile shaders\n");
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Create program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        printf("Failed to link program\n");
        glDeleteProgram(program);
        glDeleteShader(fragment_shader);
        glDeleteShader(vertex_shader);
        eglDestroyContext(display, context);
        eglDestroySurface(display, surface);
        eglTerminate(display);
        gbm_device_destroy(gbm);
        close(fd);
        return 1;
    }

    // Use program
    glUseProgram(program);

    // Set up vertex data
    GLfloat vertices[] = {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set up vertex attributes
    GLint pos_attrib = glGetAttribLocation(program, "position");
    glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(pos_attrib);

    // Clear and draw
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Swap buffers
    eglSwapBuffers(display, surface);

    printf("Rendered triangle successfully!\n");

    // Cleanup
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);
    glDeleteShader(fragment_shader);
    glDeleteShader(vertex_shader);
    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);
    gbm_device_destroy(gbm);
    close(fd);

    return 0;
}