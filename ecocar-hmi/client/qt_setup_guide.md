# Qt6 Setup Guide for NavQ Plus (ARM Linux)

This guide covers setting up Qt6 development environment on the NavQ Plus, including VSCode configuration and CMake setup. Note that a lot of normal qt dev packages aren't exactly easily available for arm linux. Most of the packages should be in /usr/lib/.../qt6 or /usr/lib/qt6 or /usr/share/qt6. The image that you flash onto the navqplus should have documentation on what is included so that you can find it and point to it when you need to import it.

## Prerequisites

### 1. Install Required Qt6 Packages
```bash
# Install essential Qt6 development packages
sudo apt-get install qt6-base-dev \
                     qt6-declarative-dev \
                     libqt6charts6-dev \
                     qt6-tools-dev \
                     qt6-wayland-dev \
                     qt6-base-private-dev \
                     libqt6opengl6-dev \
                     qt6-l10n-tools
```

### 2. Verify Qt6 Installation
```bash
# Check Qt6 location
whereis qt6

# Check QMake version
qmake6 --version

# List Qt include directories
ls /usr/include | grep -i qt
```

## VSCode Configuration

### 1. Create VSCode Configuration Directory
```bash
mkdir -p .vscode
```

### 2. Configure C/C++ IntelliSense
Create `.vscode/c_cpp_properties.json`:
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "/usr/include/aarch64-linux-gnu/qt6/**",
                "/usr/include/aarch64-linux-gnu/qt6/QtCore",
                "/usr/include/aarch64-linux-gnu/qt6/QtNetwork",
                "/usr/include/aarch64-linux-gnu/qt6/QtQml",
                "/usr/include/aarch64-linux-gnu/qt6/QtQuick",
                "/usr/include/aarch64-linux-gnu/qt6/QtCharts"
            ],
            "defines": [],
            "compilerPath": "/usr/bin/g++",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-arm64"
        }
    ],
    "version": 4
}
```

### 3. Configure VSCode Settings
Create `.vscode/settings.json`:
```json
{
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/**",
        "/usr/include/aarch64-linux-gnu/qt6/**",
        "/usr/include/aarch64-linux-gnu/qt6/QtCore",
        "/usr/include/aarch64-linux-gnu/qt6/QtNetwork",
        "/usr/include/aarch64-linux-gnu/qt6/QtQml",
        "/usr/include/aarch64-linux-gnu/qt6/QtQuick",
        "/usr/include/aarch64-linux-gnu/qt6/QtCharts"
    ],
    "C_Cpp.default.compilerPath": "/usr/bin/g++",
    "C_Cpp.default.intelliSenseMode": "linux-gcc-arm64",
    "cmake.configureSettings": {
        "CMAKE_PREFIX_PATH": "/usr/lib/aarch64-linux-gnu/cmake/Qt6"
    }
}
```

## CMake Configuration

### 1. Basic CMakeLists.txt Template
```cmake
cmake_minimum_required(VERSION 3.16)
project(your-project-name VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 installation prefix for ARM64
set(CMAKE_PREFIX_PATH "/usr/lib/aarch64-linux-gnu/cmake/Qt6")

# Find required Qt packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Network
    Quick 
    QuickControls2 
    Charts
)

qt_standard_project_setup()

# Add all C++ source files
qt_add_executable(${PROJECT_NAME}
    src/main.cpp
    src/datamodel.cpp
    src/networkmanager.cpp
)

# Add all QML files
qt_add_qml_module(${PROJECT_NAME}
    URI YourProjectURI
    VERSION 1.0
    QML_FILES
        src/qml/main.qml
        src/qml/DashboardView.qml
        src/qml/VehicleStatusView.qml
        src/qml/SettingsView.qml
        src/qml/components/SpeedGauge.qml
        src/qml/components/MetricCard.qml
        src/qml/components/StatusBar.qml
        src/qml/style/Theme.qml
)

# Link required Qt modules
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
)

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
```

## Qt Include Statements

### 1. Header Files (.h)
```cpp
// Use fully qualified paths for Qt includes
#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QUrl>
```

### 2. Implementation Files (.cpp)
```cpp
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>
#include <QtNetwork/QNetworkRequest>
```

## Building the Project

### 1. Create Build Directory
```bash
mkdir -p build
cd build
```

### 2. Configure with CMake
```bash
cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake/Qt6
```

### 3. Build
```bash
make -j$(nproc)
```

## Troubleshooting

### 1. Include Path Issues
If VSCode still shows include errors:
1. Reload VSCode window (Ctrl+Shift+P -> "Reload Window")
2. Delete build directory and rebuild
3. Verify Qt6 installation paths match your system

### 2. CMake Configuration Issues
Common fixes:
```bash
# Clear CMake cache
rm -rf build/*

# Verify Qt6 installation
dpkg -L qt6-base-dev

# Check Qt6 cmake files
ls /usr/lib/aarch64-linux-gnu/cmake/Qt6
```

### 3. Runtime Issues
Environment variables that might need to be set:
```bash
export QT_QPA_PLATFORM=wayland
export XDG_RUNTIME_DIR=/run/user/$(id -u)
```

## Additional Resources

1. Qt6 Documentation: https://doc.qt.io/qt-6/
2. CMake Qt6 Guide: https://doc.qt.io/qt-6/cmake-manual.html
3. VSCode C++ Guide: https://code.visualstudio.com/docs/cpp/cpp-ide

## Notes

- Always use the fully qualified include paths (e.g., `QtCore/QObject` instead of just `QObject`)...otherwise it can be capricious
- The NavQ Plus uses ARM64 architecture, so make sure to use the correct paths and packages. Double check your apt installs and files for "x86" in the name as it can break things and be hard to debug
- Qt6 on ARM will have different package names than x86 systems (not all are available either)
- Some features will require additional packages not listed here