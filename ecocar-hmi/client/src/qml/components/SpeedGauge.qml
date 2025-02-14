Item {
    // Properties
    property real value: 0.0
    property real minValue: 0.0
    property real maxValue: 200.0
    property string unit: "km/h"
    property bool isStale: false
    property color normalColor: "#4CAF50"
    property color warningColor: "#FFC107"
    property color errorColor: "#F44336"
    
    // Signals
    signal clicked()
    signal valueChanged(real newValue)
    signal thresholdExceeded(real value, string threshold)
    
    // Internal properties
    property real warningThreshold: maxValue * 0.8
    property real errorThreshold: maxValue * 0.9
}