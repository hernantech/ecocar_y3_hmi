# NavQ Plus GPU and Display Setup Guide

## Display Server Information

The NavQ Plus uses Wayland as its default display server, with X11 available as an alternative. Due to this:
- Direct GPU access via DRM is recommended for most applications
- X11 forwarding requires additional setup and may not provide optimal performance
- Per [docs](https://www.nxp.com/docs/en/user-guide/IMX_YOCTO_PROJECT_USERS_GUIDE.pdf), X11 applications using EGL are not supported
- Applications should be designed with Wayland compatibility in mind

## Setting up X11 Forwarding (Alternative Method)

Note: While X11 forwarding is possible, it's not the recommended approach for most applications due to Wayland being the default display server. Consider using direct GPU access via DRM instead.

### Server Setup (NavQ Plus)

1. Configure SSH server for X11:
```bash
sudo nano /etc/ssh/sshd_config
```

Add/uncomment these lines:
```
X11Forwarding yes
X11DisplayOffset 10
```

2. Restart SSH service:
```bash
sudo systemctl restart sshd
```

### Client Setup (Your Development Machine)

#### For macOS:
1. Install XQuartz:
```bash
brew install --cask xquartz
# or download from https://www.xquartz.org/
```

2. Configure XQuartz:
   - Start XQuartz: `open -a XQuartz`
   - In Preferences → Security:
     - Check "Allow connections from network clients"
   - Restart XQuartz

3. Set up X11 environment:
```bash
export DISPLAY=localhost:0
xhost + localhost
```

4. Connect with X11 forwarding:
```bash
ssh -X user@navq_ip_address
```

#### For Linux:
```bash
ssh -X user@navq_ip_address
```

#### For Windows:
1. Install X server (VcXsrv or Xming)
2. Configure PuTTY:
   - Connection → SSH → X11
   - Enable X11 forwarding
   - X display location: localhost:0

## GPU Setup and Testing

### Required Packages
```bash
sudo apt-get install mesa-utils mesa-utils-extra libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
```

### System Configuration
1. Add user to video group (required for GPU access):
```bash
sudo usermod -a -G video $USER
# Log out and back in for changes to take effect
```

2. Verify setup:
```bash
# Check GPU device:
ls -l /dev/galcore
# Should show: crw-rw-rw- 1 root video 199, 0 <date> /dev/galcore

# Check GPU driver status:
sudo dmesg | grep -i galcore
# Should show: Galcore version 6.4.3.p4.398061 or similar

# Check DRM devices
ls -l /dev/dri/
# Should show card0, card1, and renderD128
```

### GPU Test Program
Create test.c:
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

    // Create GBM device
    struct gbm_device *gbm = gbm_create_device(fd);
    if (!gbm) {
        printf("Failed to create GBM device\n");
        close(fd);
        return 1;
    }

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

Compile and run:
```bash
gcc test.c -o test -lEGL -lGLESv2 -lgbm
./test
```

## Troubleshooting

### Common Issues:

1. Permission Denied Errors:
   ```bash
   # Add user to video group
   sudo usermod -a -G video $USER
   # Log out and back in
   ```

2. Missing Libraries:
   ```bash
   # Install required packages:
   sudo apt-get install mesa-utils mesa-utils-extra libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
   ```

3. No Default Display Support:
   - This error typically occurs when trying to use X11/Wayland directly
   - Solution: Use DRM/GBM approach as shown in the test program

4. X11 Forwarding Issues:
   - Remember that X11 forwarding is not the primary display method
   - Verify DISPLAY variable: `echo $DISPLAY`
   - Check if Wayland is running: `echo $WAYLAND_DISPLAY`
   - Consider using direct GPU access instead

### System Checks
```bash
# Check display server type
loginctl show-session $(loginctl | grep $(whoami) | awk '{print $1}') -p Type

# Check GPU status
cat /sys/kernel/debug/gc/info

# Check GPU load
cat /sys/kernel/debug/gc/idle
```

Note: The NavQ Plus uses the i.MX8M Plus SoC with integrated GC7000UL GPU. The system uses Wayland as the default display server, with the GPU driver built into the kernel. For optimal performance and compatibility, applications should either use Wayland-native approaches or direct GPU access via DRM.