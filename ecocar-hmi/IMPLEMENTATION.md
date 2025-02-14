### This md doc holds the guide to implementing it

# EcoCar HMI Implementation Guide

## Table of Contents
1. [Project Structure](#project-structure)
2. [Component Interfaces](#component-interfaces)
3. [Data Types](#data-types)
4. [Error Handling](#error-handling)
5. [Configuration](#configuration)
6. [Testing Requirements](#testing-requirements)

## Project Structure
```
ecocar-hmi/
├── client/
│   ├── src/
│   │   ├── main.cpp
│   │   ├── datamodel.h
│   │   ├── datamodel.cpp
│   │   ├── networkmanager.h
│   │   ├── networkmanager.cpp
│   │   └── qml/
│   │       ├── main.qml
│   │       ├── DashboardView.qml
│   │       ├── VehicleStatusView.qml
│   │       ├── SettingsView.qml
│   │       ├── components/
│   │       │   ├── SpeedGauge.qml
│   │       │   ├── MetricCard.qml
│   │       │   └── StatusBar.qml
│   │       └── style/
│   │           └── Theme.qml
│   ├── assets/
│   │   └── images/
│   └── CMakeLists.txt
├── server/
│   ├── app.py
│   ├── can_buffer.py
│   ├── can_interface.py
│   ├── api/
│   │   ├── __init__.py
│   │   ├── routes.py
│   │   └── models.py
│   ├── utils/
│   │   ├── __init__.py
│   │   └── validation.py
│   └── requirements.txt
└── README.md
```

## Component Interfaces

### Frontend Components

#### 1. QML Components

##### SpeedGauge.qml
```qml
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
```

##### MetricCard.qml
```qml
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
```

##### StatusBar.qml
```qml
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
```

#### 2. C++ Classes

##### DataModel
```cpp
class DataModel : public QObject {
    Q_OBJECT
    
    // Properties
    Q_PROPERTY(double vehicleSpeed READ vehicleSpeed NOTIFY vehicleSpeedChanged)
    Q_PROPERTY(double batteryVoltage READ batteryVoltage NOTIFY batteryVoltageChanged)
    Q_PROPERTY(double motorTemp READ motorTemp NOTIFY motorTempChanged)
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectionStatusChanged)
    
public:
    explicit DataModel(QObject *parent = nullptr);
    
    // Getters
    double vehicleSpeed() const;
    double batteryVoltage() const;
    double motorTemp() const;
    bool isConnected() const;
    
signals:
    void vehicleSpeedChanged();
    void batteryVoltageChanged();
    void motorTempChanged();
    void connectionStatusChanged();
    void error(const QString &message);
    
private slots:
    void updateData();
    void handleNetworkError(const QString &error);
    
private:
    QTimer *updateTimer;
    NetworkManager *network;
    
    double m_vehicleSpeed;
    double m_batteryVoltage;
    double m_motorTemp;
    bool m_connected;
};
```

##### NetworkManager
```cpp
class NetworkManager : public QObject {
    Q_OBJECT
    
public:
    explicit NetworkManager(QObject *parent = nullptr);
    
    void fetchLatestData();
    void fetchSystemStatus();
    
signals:
    void dataReceived(const QJsonObject &data);
    void systemStatusReceived(const QJsonObject &status);
    void error(const QString &message);
    
private:
    QNetworkAccessManager *manager;
    QUrl baseUrl;
    
    void handleNetworkReply(QNetworkReply *reply);
};
```

### Backend Components

#### 1. Python Classes

##### CANBuffer
```python
from typing import Dict, Optional
from datetime import datetime, timedelta
import threading

class CANBuffer:
    def __init__(self):
        self.messages: Dict[str, CANMessage] = {}
        self.stale_threshold: timedelta = timedelta(milliseconds=500)
        self.lock: threading.Lock = threading.Lock()

    def update_message(self, message_id: str, value: float, unit: str) -> None:
        """Update a CAN message in the buffer"""
        with self.lock:
            self.messages[message_id] = {
                "value": value,
                "unit": unit,
                "timestamp": datetime.now(),
                "is_stale": False
            }

    def get_message(self, message_id: str) -> Optional[Dict]:
        """Get a message from the buffer"""
        with self.lock:
            if message_id not in self.messages:
                return None
            
            msg = self.messages[message_id].copy()
            msg["is_stale"] = (datetime.now() - msg["timestamp"]) > self.stale_threshold
            return msg

    def clear_stale_messages(self) -> None:
        """Remove messages older than stale_threshold"""
        with self.lock:
            current_time = datetime.now()
            self.messages = {
                k: v for k, v in self.messages.items()
                if (current_time - v["timestamp"]) <= self.stale_threshold
            }
```

##### CANInterface
```python
from typing import Optional
import threading
import can

class CANInterface:
    def __init__(self, buffer: CANBuffer):
        self.buffer = buffer
        self.running = False
        self.thread: Optional[threading.Thread] = None
        self.bus: Optional[can.Bus] = None
        
    def start(self) -> None:
        """Start the CAN interface thread"""
        if self.thread is not None:
            return
            
        self.running = True
        self.bus = can.Bus(channel='can0', bustype='socketcan')
        self.thread = threading.Thread(target=self._run)
        self.thread.daemon = True
        self.thread.start()
        
    def stop(self) -> None:
        """Stop the CAN interface thread"""
        self.running = False
        if self.thread:
            self.thread.join()
            self.thread = None
        if self.bus:
            self.bus.shutdown()
            self.bus = None
        
    def _run(self) -> None:
        """Main thread loop"""
        while self.running:
            message = self.bus.recv(timeout=0.1)
            if message:
                self._process_message(message)
                
    def _process_message(self, msg: can.Message) -> None:
        """Process incoming CAN message"""
        try:
            decoded_value = self._decode_message(msg)
            self.buffer.update_message(
                hex(msg.arbitration_id),
                decoded_value["value"],
                decoded_value["unit"]
            )
        except ValueError as e:
            print(f"Error processing message: {e}")
```

## Data Types

### API Data Types
```python
from typing import TypedDict, Union, Dict, List

class CANMessage(TypedDict):
    value: float
    unit: str
    timestamp: int
    is_stale: bool

class SystemStatus(TypedDict):
    cpu_temp: float
    gpu_temp: float
    memory_usage: float
    disk_usage: float

class APIResponse(TypedDict):
    status: str
    data: Union[Dict, List]
    timestamp: int
```

### CAN Message Formats
```cpp
// Message ID Definitions
enum class MessageID : uint32_t {
    VEHICLE_SPEED     = 0x100,
    BATTERY_VOLTAGE   = 0x200,
    MOTOR_TEMP        = 0x300,
    MOTOR_RPM         = 0x400,
    BRAKE_PRESSURE    = 0x500,
    ACCELERATOR_POS   = 0x600
};

// Message Structures
struct SpeedMessage {
    uint16_t speed_kph;    // Bytes 0-1: Speed in km/h * 100
    uint8_t direction;     // Byte 2: 0=Forward, 1=Reverse
    uint8_t valid;         // Byte 3: Data validity
};

struct BatteryMessage {
    uint16_t voltage;      // Bytes 0-1: Voltage in V * 100
    uint16_t current;      // Bytes 2-3: Current in A * 100
    uint16_t temp;         // Bytes 4-5: Temperature in °C * 10
    uint8_t soc;          // Byte 6: State of charge (%)
};
```

## Error Handling

### Error Types
```python
class CANError(Exception):
    """Base class for CAN-related errors"""
    pass

class StaleDataError(CANError):
    """Raised when data is older than threshold"""
    pass

class ValidationError(Exception):
    """Raised when data validation fails"""
    pass
```

### Error Response Format
```python
error_response = {
    "status": "error",
    "error": {
        "code": str,      # Error code
        "message": str,   # Human-readable message
        "details": Optional[Dict]  # Additional error details
    },
    "timestamp": int      # Unix timestamp
}
```

## Configuration

### Server Configuration
```python
class ServerConfig:
    # Network settings
    HOST = "0.0.0.0"
    PORT = 5000
    DEBUG = False
    
    # CAN settings
    STALE_THRESHOLD_MS = 500
    CAN_INTERFACE = "can0"
    CAN_BITRATE = 500000
    
    # Security settings
    RATE_LIMIT = "100/minute"
    AUTH_REQUIRED = True
    
    # Logging settings
    LOG_LEVEL = "INFO"
    LOG_FORMAT = "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
```

### Client Configuration
```cpp
struct ClientConfig {
    // Network settings
    static const QString API_BASE_URL = "http://localhost:5000/api/v1";
    static const int POLLING_INTERVAL_MS = 100;
    static const int RETRY_INTERVAL_MS = 1000;
    static const int MAX_RETRIES = 3;
    
    // Display settings
    static const int WINDOW_WIDTH = 1280;
    static const int WINDOW_HEIGHT = 720;
    static const int TARGET_FPS = 60;
    
    // Theme settings
    static const QString THEME_PRIMARY_COLOR = "#4CAF50";
    static const QString THEME_BACKGROUND_COLOR = "#212121";
};
```

## Testing Requirements

### Required Test Coverage
```python
# Backend Tests
- Unit tests for all classes (min 80% coverage)
- Integration tests for API endpoints
- CAN message processing tests
- Error handling tests
- Performance tests

# Frontend Tests
- QML component tests
- C++ class unit tests
- Network integration tests
- UI performance tests
- End-to-end system tests
```

### Test Data Format
```python
test_can_messages = {
    "0x100": {
        "value": 50.0,
        "unit": "km/h",
        "timestamp": 1234567890,
        "is_stale": False
    },
    "0x200": {
        "value": 12.4,
        "unit": "V",
        "timestamp": 1234567890,
        "is_stale": False
    }
}

test_system_status = {
    "cpu_temp": 45.5,
    "gpu_temp": 55.2,
    "memory_usage": 0.75,
    "disk_usage": 0.45
}
```

### Performance Test Requirements
```python
performance_requirements = {
    "frontend_fps": {
        "target": 60,
        "minimum": 55
    },
    "api_response_time": {
        "target": 50,  # ms
        "maximum": 100  # ms
    },
    "data_staleness": {
        "maximum": 500  # ms
    },
    "memory_usage": {
        "maximum": 500  # MB
    }
}
```