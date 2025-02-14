Item {
    // Properties
    property string title: ""
    property var value
    property string unit: ""
    property bool isWarning: false
    property bool isError: false
    property string warningThreshold: ""
    property string errorThreshold: ""
    property string description: ""
    
    // Style properties
    property color backgroundColor: "#424242"
    property color textColor: "#FFFFFF"
    property real cornerRadius: 8
    
    // Signals
    signal clicked()
    signal thresholdExceeded(string level, var value)
}