cmake_minimum_required(VERSION 3.16)
project(ecocar-hmi VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Qt6 installation prefix
set(CMAKE_PREFIX_PATH "/usr/lib/aarch64-linux-gnu/cmake/Qt6")

# Find required Qt packages
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Network
    Quick 
    QuickControls2 
    Charts
)

# Rest of your CMakeLists.txt remains the same...

qt_standard_project_setup()

# Add all C++ source files
qt_add_executable(ecocar-hmi
    src/main.cpp
    src/datamodel.cpp
    src/networkmanager.cpp
)

# Add all QML files
qt_add_qml_module(ecocar-hmi
    URI EcocarHMI
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
target_link_libraries(ecocar-hmi PRIVATE
    Qt6::Core
    Qt6::Network
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
)

# Add include directories
target_include_directories(ecocar-hmi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Network_INCLUDE_DIRS}
    ${Qt6Quick_INCLUDE_DIRS}
    ${Qt6QuickControls2_INCLUDE_DIRS}
    ${Qt6Charts_INCLUDE_DIRS}
)