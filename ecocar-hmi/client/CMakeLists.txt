cmake_minimum_required(VERSION 3.16)
project(ecocar-hmi VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Charts)

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
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Charts
)

set_target_properties(ecocar-hmi PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)