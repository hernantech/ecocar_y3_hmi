# NavQ Plus GPU Programming Guide

## Overview
This guide covers GPU initialization and testing on the NavQ Plus, which features the i.MX8M Plus SoC with integrated GC7000UL GPU. The guide includes both X11 and direct GPU access methods.

## Prerequisites

### Required Packages
```bash
sudo apt-get install mesa-utils mesa-utils-extra libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
```

### System Configuration
1. Verify GPU device exists:
```bash
ls -l /dev/galcore
# Should show: crw-rw-rw- 1 root video 199, 0 <date> /dev/galcore
```

2. Check user permissions:
```bash
# Add user to video group if needed
sudo usermod -a -G video $USER
# Verify groups
groups
```

3. Check GPU driver status:
```bash
# Check kernel messages
sudo dmesg | grep -i galcore
# Should show: Galcore version 6.4.3.p4.398061 or similar

# Check DRM devices
ls -l /dev/dri/
# Should show card0, card1, and renderD128
```

## GPU Testing

### Basic EGL Test Program
Create a file named `test.c`:

```c
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
```

### Compiling and Running
```bash
# Compile the test program
gcc test.c -o test -lEGL -lGLESv2 -lgbm

# Run the test
./test
```

### Expected Output
```
Opened DRM render node
Created GBM device
Got EGL display
EGL initialized successfully! Version 1.5
EGL Vendor: Vivante Corporation
EGL Version: 1.5
EGL Extensions: EGL_KHR_config_attribs EGL_KHR_lock_surface ... [etc]
```

## Understanding the Code

### Key Components

1. **DRM (Direct Rendering Manager)**
   - Uses `/dev/dri/renderD128` for direct GPU access
   - Bypasses window system (X11/Wayland) for GPU access
   - Provides low-level hardware access

2. **GBM (Generic Buffer Management)**
   - Creates and manages GPU buffers
   - Provides platform-agnostic buffer allocation
   - Required for modern GPU programming on Linux

3. **EGL Platform Extension**
   - Uses `EGL_PLATFORM_GBM_KHR` for modern GPU initialization
   - Dynamically loads platform display function
   - Provides proper GPU initialization on modern Linux systems

### Common Issues and Solutions

1. **Permission Denied**
   ```bash
   # Fix with:
   sudo usermod -a -G video $USER
   # Log out and back in
   ```

2. **Missing Libraries**
   ```bash
   # Install required packages:
   sudo apt-get install libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
   ```

3. **No Default Display Support**
   - This error occurs when trying to use X11/Wayland directly
   - Solution: Use DRM/GBM approach as shown in the test program

## Next Steps

1. **Creating an OpenGL ES Context**
   - Choose EGL configuration
   - Create EGL context and surface
   - Initialize OpenGL ES rendering

2. **Buffer Management**
   - Create GBM buffers for rendering
   - Handle buffer swapping
   - Implement proper synchronization

3. **Rendering Pipeline**
   - Set up vertex and fragment shaders
   - Create vertex buffers
   - Implement rendering loop

## Additional Resources

1. **GPU Information**
   ```bash
   # Check GPU status
   cat /sys/kernel/debug/gc/info
   
   # Check GPU load
   cat /sys/kernel/debug/gc/idle
   ```

2. **Driver Information**
   ```bash
   # Check loaded GPU driver
   lsmod | grep galcore
   
   # Check driver version
   dmesg | grep -i galcore
   ```

## Notes
- The NavQ Plus uses the i.MX8M Plus SoC with integrated GC7000UL GPU
- The GPU driver is built into the kernel
- The system supports both X11 and Wayland
- Direct GPU access via DRM is recommended for most applications

## OpenGL ES Rendering Example

### Full Rendering Test Program
Here's a complete example that creates an off-screen rendering surface and draws a simple triangle:

```c
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
```

### Compiling the Rendering Example
```bash
gcc -o render_test render_test.c -lEGL -lGLESv2 -lgbm
```

### Understanding the Rendering Pipeline

1. **Initialization**
   - Set up DRM and GBM
   - Initialize EGL
   - Create surface and context
   - Make context current

2. **Shader Setup**
   - Compile vertex and fragment shaders
   - Create and link shader program
   - Set up vertex attributes

3. **Rendering**
   - Clear framebuffer
   - Draw geometry
   - Swap buffers

4. **Cleanup**
   - Delete OpenGL ES resources
   - Destroy EGL resources
   - Clean up DRM/GBM resources

## Performance Considerations

### Memory Management
1. **Buffer Creation**
   - Use appropriate buffer sizes
   - Consider using shared buffers when possible
   - Properly handle buffer destruction

2. **Texture Management**
   - Use appropriate texture formats
   - Consider compression for large textures
   - Implement proper texture lifecycle management

### Synchronization
1. **Buffer Swapping**
   - Use appropriate swap interval
   - Consider vsync requirements
   - Handle frame timing properly

2. **GPU/CPU Synchronization**
   - Use fence synchronization when needed
   - Avoid unnecessary stalls
   - Consider using multiple command buffers

## Debugging and Profiling

### Debug Tools
1. **GPU Information**
   ```bash
   # Check GPU utilization
   cat /sys/kernel/debug/gc/idle
   
   # Check GPU memory
   cat /sys/kernel/debug/gc/mem_stats
   ```

2. **OpenGL ES Debug Output**
   ```c
   // Enable debug output
   glEnable(GL_DEBUG_OUTPUT);
   glDebugMessageCallback(debugCallback, 0);
   ```

### Common Issues

1. **Performance**
   - Check for redundant state changes
   - Verify proper buffer management
   - Monitor GPU temperature and throttling

2. **Rendering Artifacts**
   - Verify shader compilation
   - Check framebuffer completeness
   - Validate texture formats and sizes

## Advanced Topics

### Multiple Contexts
- Sharing resources between contexts
- Context priority management
- Context switching optimization

### Hardware Features
- Using hardware-specific extensions
- Optimizing for the GC7000UL architecture
- Utilizing special hardware features

## Resources

### Documentation
- i.MX 8M Plus Reference Manual
- Vivante GPU Programming Guide
- OpenGL ES 2.0 Specification

### Tools
- GPU debugging tools
- Performance monitoring utilities
- Development frameworks

## Troubleshooting

### Common Error Messages
1. "Failed to open DRM render node"
   - Check permissions
   - Verify device exists
   - Check group membership

2. "Failed to initialize EGL"
   - Verify GPU driver loaded
   - Check EGL library installation
   - Verify display configuration

### System Checks
```bash
# Check system status
systemctl status

# Check GPU driver
dmesg | grep -i gpu

# Check X11/Wayland status
loginctl show-session $(loginctl | grep $(whoami) | awk '{print $1}') -p Type
```