import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtCharts

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 720
    title: qsTr("EcoCar HMI")

    // Set Material theme
    Material.theme: Material.Dark
    Material.accent: Material.Green

    // Main content area
    RowLayout {
        anchors.fill: parent
        spacing: 10

        // Left sidebar with buttons
        ColumnLayout {
            Layout.preferredWidth: 200
            Layout.fillHeight: true
            spacing: 10
            
            Button {
                text: "Dashboard"
                Layout.fillWidth: true
                onClicked: console.log("Dashboard clicked")
            }
            
            Button {
                text: "Vehicle Status"
                Layout.fillWidth: true
                onClicked: console.log("Vehicle Status clicked")
            }
            
            Button {
                text: "Settings"
                Layout.fillWidth: true
                onClicked: console.log("Settings clicked")
            }

            Item { Layout.fillHeight: true } // Spacer
        }

        // Main content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: Material.accentColor
            border.width: 1
            radius: 5

            // Sample chart
            ChartView {
                id: chart
                anchors.fill: parent
                antialiasing: true
                theme: ChartView.ChartThemeDark
                
                LineSeries {
                    name: "Sample Data"
                    // Sample data points
                    XYPoint { x: 0; y: 0 }
                    XYPoint { x: 1; y: 2 }
                    XYPoint { x: 2; y: 1 }
                    XYPoint { x: 3; y: 3 }
                    XYPoint { x: 4; y: 2 }
                }
            }
        }
    }

    // Status bar at bottom
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                text: "Status: Connected"
                Layout.fillWidth: true
                padding: 10
            }
            Label {
                text: Qt.formatDateTime(new Date(), "hh:mm:ss")
                padding: 10
            }
        }
    }
}