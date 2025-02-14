Item {
    // Properties
    property bool connected: false
    property string statusMessage: ""
    property int errorCount: 0
    property string lastUpdateTime: ""
    
    // Style properties
    property color backgroundColor: "#212121"
    property color textColor: "#FFFFFF"
    
    // Signals
    signal statusClicked()
    signal errorCountClicked()
}